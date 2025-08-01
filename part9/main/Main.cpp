#include "../include/GeometryUtils.hpp"
#include "../include/Reactor.hpp"
#include "../include/Proactor.hpp"
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
#include <thread>
#include <mutex>

#define PORT 9034
#define MAX_CLIENTS 10

// Global shared graph state and mutex
std::deque<Point> point_set;
std::deque<Point> temp_points;
bool waiting_for_graph = false;
int points_to_read = 0;
int newgraph_owner_fd = -1;
std::mutex graph_mutex;

// Per-client input buffer
struct ClientState {
    std::string inbuf;
};
std::unordered_map<int, ClientState> clients;

/**
 * @brief Trim newline and leading whitespace from a string.
 * @param s String to be modified.
 */
void trim_crlf(std::string &s) {
    while (!s.empty() && (s.back() == '\n' || s.back() == '\r')) s.pop_back();
    while (!s.empty() && std::isspace((unsigned char)s.front())) s.erase(s.begin());
}

/**
 * @brief Check if a string represents a valid number.
 * @param s Input string.
 * @return True if s is a number, false otherwise.
 */
bool is_number(const std::string& s) {
    std::istringstream iss(s);
    double d;
    return (iss >> d) && iss.eof();
}

/**
 * @brief Handle input line during Newgraph mode.
 * @param line A line containing a point in "x,y" format.
 * @return Response string.
 */
std::string handle_point_line(const std::string& line) {
    std::lock_guard<std::mutex> lock(graph_mutex);
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
 * @brief Handle Newpoint command.
 * @param args A string with point coordinates in "x,y" format.
 * @return Response string.
 */
std::string handle_newpoint(const std::string& args) {
    std::lock_guard<std::mutex> lock(graph_mutex);
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
 * @brief Handle Removepoint command.
 * @param args A string with point coordinates in "x,y" format.
 * @return Response string.
 */
std::string handle_removepoint(const std::string& args) {
    std::lock_guard<std::mutex> lock(graph_mutex);
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
 * @brief Handle CH (Convex Hull) command.
 * @return String representation of the convex hull area.
 */
std::string handle_ch() {
    std::lock_guard<std::mutex> lock(graph_mutex);
    auto hull = compute_convex_hull_deque(point_set);
    double area = compute_area(hull);
    std::ostringstream oss;
    oss << area;
    return oss.str();
}

/**
 * @brief Process a command line from a client.
 * @param fd Client socket file descriptor.
 * @param rawline The raw input line.
 * @return Response string.
 */
std::string process_line(int fd, const std::string& rawline) {
    std::string line = rawline;
    trim_crlf(line);
    if (line.empty()) return "";

    if (waiting_for_graph && fd != newgraph_owner_fd) return "BUSY";
    if (waiting_for_graph && fd == newgraph_owner_fd) return handle_point_line(line);

    std::istringstream iss(line);
    std::string command;
    iss >> command;
    std::string args;
    std::getline(iss, args);
    size_t pos = args.find_first_not_of(" ");
    if (pos != std::string::npos) args.erase(0, pos);
    else args.clear();

    if (command == "Newgraph") {
        std::lock_guard<std::mutex> lock(graph_mutex);
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
 * @brief Proactor client thread function.
 * @param fd Client socket file descriptor.
 * @return nullptr when done.
 */
void* client_thread_handler(int fd) {
    char buffer[1024];
    clients[fd] = ClientState{};

    while (true) {
        int bytes = recv(fd, buffer, sizeof(buffer), 0);
        if (bytes <= 0) {
            std::cout << "Client " << fd << " disconnected or error occurred (recv=" << bytes << "). Closing fd." << std::endl;
            break;
        }

        std::string& inbuf = clients[fd].inbuf;
        inbuf.append(buffer, bytes);
        std::cout << "Received from fd " << fd << ": " << std::string(buffer, bytes) << std::endl;

        size_t pos;
        while ((pos = inbuf.find('\n')) != std::string::npos) {
            std::string line = inbuf.substr(0, pos + 1);
            inbuf.erase(0, pos + 1);
            std::string response = process_line(fd, line);
            std::cout << "Processing line: " << line << " → Response: " << response << "\n" << std::endl;

            if (!response.empty()) {
                response += "\n";
                send(fd, response.c_str(), response.size(), 0);
            }
        }
    }

    close(fd);
    std::lock_guard<std::mutex> lock(graph_mutex);
    clients.erase(fd);
    if (waiting_for_graph && fd == newgraph_owner_fd) {
        std::cout << "Graph construction aborted (owner disconnected)." << std::endl;
        waiting_for_graph = false;
        newgraph_owner_fd = -1;
        temp_points.clear();
    }
    return nullptr;
}

/**
 * @brief Accept a new incoming connection and launch proactor.
 * @param listener_fd Server listening socket.
 */
void handle_new_connection(int listener_fd) {
    int client_fd = accept(listener_fd, nullptr, nullptr);
    if (client_fd >= 0) {
        std::cout << "New client accepted: " << client_fd << std::endl;
        startProactor(client_fd, client_thread_handler);
    }
}

/**
 * @brief Entry point of the server.
 * @return Exit code.
 */
int main() {
    int listener = socket(AF_INET, SOCK_STREAM, 0);
    if (listener < 0) {
        perror("socket failed");
        return 1;
    }

    sockaddr_in server{};
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = INADDR_ANY;

    if (bind(listener, (sockaddr*)&server, sizeof(server)) < 0) {
        perror("bind failed");
        return 1;
    }

    if (listen(listener, MAX_CLIENTS) < 0) {
        perror("listen failed");
        return 1;
    }

    void* reactor = startReactor();
    addFdToReactor(reactor, listener, handle_new_connection);

    std::cout << "Server running on port " << PORT << ". Press Ctrl+C to exit.\n" << std::endl;
    while (true) std::this_thread::sleep_for(std::chrono::seconds(1));

    stopReactor(reactor);
    return 0;
}
