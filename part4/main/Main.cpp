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
#include <sys/select.h>
#include <cstring>

#define PORT 9034
#define MAX_CLIENTS 10

/**
 * @struct ClientState
 * @brief Represents per-client state, including input buffer for incomplete messages.
 */
struct ClientState {
    std::string inbuf;  // Accumulate input until newline
};

// === Global State ===

std::deque<Point> point_set; // Current set of points forming the shared graph.
std::deque<Point> temp_points; // Temporary buffer for points being read during a Newgraph command.
bool waiting_for_graph = false; // True if a client is currently building a new graph.
int points_to_read = 0; // Number of remaining points expected after Newgraph.
int newgraph_owner_fd = -1;  // fd of the client building the new graph
std::unordered_map<int, ClientState> clients; // Map of connected clients and their associated state.


/**
 * @brief Checks if a given string is a valid floating-point number.
 * 
 * @param s The input string.
 * @return true if valid float, false otherwise.
 */
bool is_number(const std::string& s) {
    std::istringstream iss(s);
    double d;
    return (iss >> d) && iss.eof();
}

/**
 * @brief Trims CR/LF and leading spaces from a string.
 * 
 * @param s The string to trim (modified in-place).
 */
void trim_crlf(std::string &s) {
    while (!s.empty() && (s.back() == '\n' || s.back() == '\r')) s.pop_back();
    while (!s.empty() && std::isspace((unsigned char)s.front())) s.erase(s.begin());
}

/**
 * @brief Checks if a client is currently blocked due to another client's Newgraph.
 *
 * @param fd The file descriptor of the client.
 * @return true if the client is blocked, false otherwise.
 */
bool is_busy_for_fd(int fd) {
    return waiting_for_graph && fd != newgraph_owner_fd;
}

/**
 * @brief Handles a point line received during a Newgraph phase.
 * 
 * @param line Input in the format x,y
 * @return "OK" or "GRAPH_LOADED" or an error message.
 */
std::string handle_point_line(const std::string& line) {
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
 * @brief Handles a Newpoint command from any client.
 * 
 * @param args Input in the format x,y
 * @return "OK" or error message.
 */
std::string handle_newpoint(const std::string& args) {
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
 * @brief Handles a Removepoint command from any client.
 * 
 * @param args Input in the format x,y
 * @return "OK" or error message.
 */
std::string handle_removepoint(const std::string& args) {
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
 * @brief Handles the CH command: computes and returns the convex hull area.
 *
 * @return String containing the area.
 */
std::string handle_ch() {
    auto hull = compute_convex_hull_deque(point_set);
    double area = compute_area(hull);
    std::ostringstream oss;
    oss << area;
    return oss.str();
}

/**
 * @brief Processes a full line received from a client.
 * 
 * @param fd The client's socket file descriptor.
 * @param rawline The raw input line from the client.
 * @return Response to be sent back to the client.
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
    size_t pos = args.find_first_not_of(" "); // finds the index of the first non-space character.
    if (pos != std::string::npos) args.erase(0, pos); // if found—removes leading whitespace.
    else args.clear(); // else—clears (no arguments).

    if (command == "Newgraph") {
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
 * @brief Main function that runs the TCP server event loop.
 * 
 * Accepts multiple clients, manages their states, and processes their commands.
 * 
 * @return Always returns 0 (infinite loop unless externally interrupted).
 */
int main() {
    // Create a TCP socket (IPv4, stream-based)
    int listener = socket(AF_INET, SOCK_STREAM, 0);

    // Prepare server address structure
    sockaddr_in server{};
    server.sin_family = AF_INET; // IPv4
    server.sin_port = htons(PORT); // Set port (convert to network byte order)
    server.sin_addr.s_addr = INADDR_ANY; // Accept connections from any IP address

    // Bind the socket to the address and port
    bind(listener, (sockaddr*)&server, sizeof(server));

    // Start listening for incoming connections
    listen(listener, MAX_CLIENTS);

    // Prepare file descriptor sets for select()
    fd_set master, read_fds;
    FD_ZERO(&master); // Clear the master set
    FD_SET(listener, &master); // Add the listener socket to the master set
    int fdmax = listener; // Keep track of the max file descriptor number

    // Main loop to handle incoming connections and data
    while (true) {
        read_fds = master; // Copy master set to a temporary set
        select(fdmax + 1, &read_fds, nullptr, nullptr, nullptr); // Wait for activity

        // Loop through all file descriptors
        for (int i = 0; i <= fdmax; ++i) {
            if (FD_ISSET(i, &read_fds)) {  // Check if this fd is ready to read
                if (i == listener) { // New incoming connection
                    int newfd = accept(listener, nullptr, nullptr); // Accept the connection
                    FD_SET(newfd, &master); // Add new client socket to the master set
                    if (newfd > fdmax) fdmax = newfd; // Update max fd if needed
                    clients[newfd] = ClientState{};  // Initialize client state
                } else {   // Data from an existing client
                    char buffer[1024];
                    int bytes = recv(i, buffer, sizeof(buffer), 0); // Receive data
                    if (bytes <= 0) { // Connection closed or error
                        close(i);  // Close the socket
                        FD_CLR(i, &master);  // Remove from master set
                        clients.erase(i); // Remove client from map
                        // If this client was uploading a graph, reset graph state
                        if (waiting_for_graph && i == newgraph_owner_fd) {
                            waiting_for_graph = false;
                            temp_points.clear();
                            newgraph_owner_fd = -1;
                        }
                    } else {                   // Received some data
                        std::string& inbuf = clients[i].inbuf; // Get client's input buffer
                        inbuf.append(buffer, bytes);           // Append new data
                        size_t pos;
                        // Process each complete line of data
                        while ((pos = inbuf.find('\n')) != std::string::npos) {
                            std::string line = inbuf.substr(0, pos + 1); // Extract line
                            inbuf.erase(0, pos + 1);                    // Remove it from buffer
                            std::string response = process_line(i, line); // Handle the line
                            if (!response.empty()) { // Send response back to client
                                response += "\n";
                                send(i, response.c_str(), response.size(), 0);
                            }
                        }
                    }
                }
            }
        }
    }
}
