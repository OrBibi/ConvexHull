#include "../include/GeometryUtils.hpp"
#include <iostream>
#include <sstream>
#include <string>
#include <deque>
#include <unordered_map>
#include <algorithm>
#include <cctype>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <cstring>
#include <thread>            // Added for std::thread
#include <mutex>             // Added for std::mutex

#define PORT 9034
#define MAX_CLIENTS 10

std::mutex graph_mutex;     // Added: mutex to protect access to shared graph data

/**
 * @struct ClientState
 * @brief Stores per-client state including partial input buffer.
 */
struct ClientState {
    std::string inbuf;
};

// --- Shared server state ---
std::deque<Point> point_set;
std::deque<Point> temp_points;
bool waiting_for_graph = false;
int points_to_read = 0;
int newgraph_owner_fd = -1;
std::unordered_map<int, ClientState> clients;

/**
 * @brief Checks if a string represents a valid number.
 * @param s The input string.
 * @return True if the string can be parsed as a number.
 */
bool is_number(const std::string& s) {
    std::istringstream iss(s);
    double d;
    return (iss >> d) && iss.eof();
}

/**
 * @brief Trims newline and leading spaces from a string.
 * @param s The string to trim (in-place).
 */
void trim_crlf(std::string &s) {
    while (!s.empty() && (s.back() == '\n' || s.back() == '\r')) s.pop_back();
    while (!s.empty() && std::isspace((unsigned char)s.front())) s.erase(s.begin());
}

/**
 * @brief Checks if a file descriptor is blocked from writing due to active graph input.
 * @param fd File descriptor to check.
 * @return True if the client must wait.
 */
bool is_busy_for_fd(int fd) {
    return waiting_for_graph && fd != newgraph_owner_fd;
}

/**
 * @brief Handles input line containing a single point during Newgraph construction.
 * @param line The input line.
 * @return Response message for the client.
 */
std::string handle_point_line(const std::string& line) {
    std::lock_guard<std::mutex> lock(graph_mutex); // Protect access to shared temp_points
    size_t comma = line.find(',');
    if (comma == std::string::npos) return "ERROR: Invalid point format.";
    std::string x_str = line.substr(0, comma);
    std::string y_str = line.substr(comma + 1);
    if (!is_number(x_str) || !is_number(y_str)) return "ERROR: Invalid point values.";

    Point p{std::stod(x_str), std::stod(y_str)};
    temp_points.push_back(p);
    points_to_read--;
    if (points_to_read == 0) {
        point_set = temp_points;
        temp_points.clear();
        waiting_for_graph = false;
        newgraph_owner_fd = -1;
        return "GRAPH_LOADED";
    }
    return "OK";
}

/**
 * @brief Adds a single point to the graph (via Newpoint).
 * @param args Arguments after the command (x,y).
 * @return Response message.
 */
std::string handle_newpoint(const std::string& args) {
    std::lock_guard<std::mutex> lock(graph_mutex); // Protect access to point_set
    size_t comma = args.find(',');
    if (comma == std::string::npos) return "ERROR: Invalid format.";
    std::string x_str = args.substr(0, comma);
    std::string y_str = args.substr(comma + 1);
    if (!is_number(x_str) || !is_number(y_str)) return "ERROR: Invalid values.";
    Point p{std::stod(x_str), std::stod(y_str)};
    point_set.push_back(p);
    return "OK";
}

/**
 * @brief Removes a specific point from the graph.
 * @param args Arguments after the command (x,y).
 * @return Response message.
 */
std::string handle_removepoint(const std::string& args) {
    std::lock_guard<std::mutex> lock(graph_mutex); // Protect access to point_set
    size_t comma = args.find(',');
    if (comma == std::string::npos) return "ERROR: Invalid format.";
    std::string x_str = args.substr(0, comma);
    std::string y_str = args.substr(comma + 1);
    if (!is_number(x_str) || !is_number(y_str)) return "ERROR: Invalid values.";
    Point p{std::stod(x_str), std::stod(y_str)};
    auto it = std::remove_if(point_set.begin(), point_set.end(),
                             [&](const Point& q) { return q.x == p.x && q.y == p.y; });
    if (it != point_set.end()) point_set.erase(it, point_set.end());
    return "OK";
}

/**
 * @brief Computes the convex hull of the graph and returns its area.
 * @return Area as a string.
 */
std::string handle_ch() {
    std::lock_guard<std::mutex> lock(graph_mutex); // Protect read access to point_set
    auto hull = compute_convex_hull_deque(point_set);
    double area = compute_area(hull);
    std::ostringstream oss;
    oss << area;
    return oss.str();
}

/**
 * @brief Parses and processes a full client command line.
 * @param fd The client's file descriptor.
 * @param rawline The full input line.
 * @return Response string.
 */
std::string process_line(int fd, const std::string& rawline) {
    std::string line = rawline;
    trim_crlf(line);
    if (line.empty()) return "";

    if (waiting_for_graph && fd != newgraph_owner_fd) {
        return "BUSY";
    }

    if (waiting_for_graph && fd == newgraph_owner_fd) {
        return handle_point_line(line);
    }

    std::istringstream iss(line);
    std::string command;
    iss >> command;
    std::string args;
    std::getline(iss, args);
    size_t pos = args.find_first_not_of(" ");
    if (pos != std::string::npos) args.erase(0, pos);
    else args.clear();

    if (command == "Newgraph") {
        std::lock_guard<std::mutex> lock(graph_mutex); // Protect write to shared state
        std::istringstream a(args);
        int n;
        if (!(a >> n) || n <= 0) return "ERROR: Invalid number.";
        waiting_for_graph = true;
        newgraph_owner_fd = fd;
        points_to_read = n;
        temp_points.clear();
        return "OK";
    }
    if (command == "Newpoint") return handle_newpoint(args);
    if (command == "Removepoint") return handle_removepoint(args);
    if (command == "CH") return handle_ch();

    return "ERROR: Unknown command.";
}

/**
 * @brief Handles interaction with a single connected client.
 * @param client_fd The socket file descriptor of the client.
 */
void handle_client(int client_fd) {
    char buffer[1024];
    while (true) {
        int bytes = recv(client_fd, buffer, sizeof(buffer), 0);
        if (bytes <= 0) {
            close(client_fd);
            std::lock_guard<std::mutex> lock(graph_mutex);
            clients.erase(client_fd);
            if (waiting_for_graph && client_fd == newgraph_owner_fd) {
                waiting_for_graph = false;
                temp_points.clear();
                newgraph_owner_fd = -1;
            }
            break;
        }

        std::string& inbuf = clients[client_fd].inbuf;
        inbuf.append(buffer, bytes);
        size_t pos;
        while ((pos = inbuf.find('\n')) != std::string::npos) {
            std::string line = inbuf.substr(0, pos + 1);
            inbuf.erase(0, pos + 1);
            std::string response = process_line(client_fd, line);
            if (!response.empty()) {
                response += "\n";
                send(client_fd, response.c_str(), response.size(), 0);
            }
        }
    }
}

/**
 * @brief Main server loop. Accepts clients and launches threads to serve them.
 * @return Exit code.
 */
int main() {
    int listener = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in server{};
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = INADDR_ANY;

    bind(listener, (sockaddr*)&server, sizeof(server));
    listen(listener, MAX_CLIENTS);
    std::cout << "Server listening on port " << PORT << std::endl;

    while (true) {
        int client_fd = accept(listener, nullptr, nullptr);
        if (client_fd >= 0) {
            {
                std::lock_guard<std::mutex> lock(graph_mutex);
                clients[client_fd] = ClientState{};
            }
            std::thread t(handle_client, client_fd); // Launch new thread per client
            t.detach(); // Detach thread so it runs independently
        }
    }
} 
