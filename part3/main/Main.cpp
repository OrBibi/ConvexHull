// ConvexHullServer.cpp
// Stage 4 - TCP server for convex hull graph shared between clients (port 9034)

#include "../include/GeometryUtils.hpp"
#include <iostream>
#include <sstream>
#include <string>
#include <deque>
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

std::deque<Point> point_set; // Global deque containing the current set of points
std::deque<Point> temp_points; // Temporary storage for new graph input
bool waiting_for_graph = false; // Indicates if the server is currently waiting to receive points for a new graph
int points_to_read = 0; // Number of points remaining to read for the new graph

/**
 * @brief Checks if a string is a valid floating-point number.
 * 
 * @param s The input string.
 * @return true if the string is a float, false otherwise.
 */
bool is_number(const std::string& s) {
    std::istringstream iss(s);
    double d;
    return (iss >> d) && iss.eof();
}

/**
 * @brief Handles a line containing a single point when in graph-building mode.
 * 
 * @param line A string of the form x,y.
 * @return "OK" if the point was added, or an error message otherwise.
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
    }
    return "OK";
}

/**
 * @brief Handles the "Newgraph" command to start a new graph.
 * 
 * @param arg_line The argument string containing the number of points.
 * @return "OK" if valid, or an error message otherwise.
 */
std::string handle_newgraph(const std::string& arg_line) {
    std::istringstream args(arg_line);
    int n;
    if (!(args >> n) || n <= 0) return "ERROR: Invalid number in Newgraph.";

    waiting_for_graph = true;
    points_to_read = n;
    temp_points.clear();
    return "OK";
}

/**
 * @brief Handles the "Newpoint" command to add a point to the graph.
 * 
 * @param args A string of the form x,y.
 * @return "OK" if valid, or an error message otherwise.
 */
std::string handle_newpoint(const std::string& args) {
    size_t comma = args.find(',');
    if (comma == std::string::npos) return "ERROR: Invalid Newpoint format.";

    std::string x_str = args.substr(0, comma);
    std::string y_str = args.substr(comma + 1);
    if (!is_number(x_str) || !is_number(y_str)) return "ERROR: Invalid point values.";

    Point p{std::stod(x_str), std::stod(y_str)};
    point_set.push_back(p);
    return "OK";
}

/**
 * @brief Handles the "Removepoint" command to remove a specific point.
 * 
 * @param args A string of the form x,y.
 * @return "OK" if the point was found and removed, or an error message otherwise.
 */

std::string handle_removepoint(const std::string& args) {
    size_t comma = args.find(',');
    if (comma == std::string::npos) return "ERROR: Invalid Removepoint format.";

    std::string x_str = args.substr(0, comma);
    std::string y_str = args.substr(comma + 1);
    if (!is_number(x_str) || !is_number(y_str)) return "ERROR: Invalid point values.";

    Point p{std::stod(x_str), std::stod(y_str)};
    auto it = std::remove_if(point_set.begin(), point_set.end(),
                             [&](const Point& q) { return q.x == p.x && q.y == p.y; });
    if (it != point_set.end()) point_set.erase(it, point_set.end());
    return "OK";
}

/**
 * @brief Handles the "CH" command: computes the convex hull and returns its area.
 * 
 * @return A string representation of the convex hull's area.
 */
std::string handle_ch() {
    auto hull = compute_convex_hull_deque(point_set);
    double area = compute_area(hull);
    std::ostringstream oss;
    oss << area;
    return oss.str();
}

/**
 * @brief Parses and dispatches a single input line command from a client.
 * 
 * @param line The command line input.
 * @return The response string to be sent back to the client.
 */
std::string process_line(const std::string& line) {
    if (line.empty()) return "";
    if (waiting_for_graph) return handle_point_line(line);

    std::istringstream iss(line);
    std::string command;
    iss >> command;

    std::string args;
    std::getline(iss, args);
    args.erase(0, args.find_first_not_of(" "));

    if (command == "Newgraph") return handle_newgraph(args);
    if (command == "Newpoint") return handle_newpoint(args);
    if (command == "Removepoint") return handle_removepoint(args);
    if (command == "CH") return handle_ch();

    return "ERROR: Unknown command.";
}

/**
 * @brief Main server loop: handles incoming TCP client connections and processes commands.
 * 
 * @return Always returns 0 (infinite loop unless manually stopped).
 */
int main() {
    int listener = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in server{};
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = INADDR_ANY;

    bind(listener, (sockaddr*)&server, sizeof(server));
    listen(listener, MAX_CLIENTS);

    fd_set master, read_fds;
    FD_ZERO(&master);
    FD_SET(listener, &master);
    int fdmax = listener;

    char buffer[1024];

    while (true) {
        read_fds = master;
        select(fdmax + 1, &read_fds, nullptr, nullptr, nullptr);

        for (int i = 0; i <= fdmax; ++i) {
            if (FD_ISSET(i, &read_fds)) {
                if (i == listener) {
                    int newfd = accept(listener, nullptr, nullptr);
                    FD_SET(newfd, &master);
                    if (newfd > fdmax) fdmax = newfd;
                } else {
                    memset(buffer, 0, sizeof(buffer));
                    int bytes = recv(i, buffer, sizeof(buffer), 0);
                    if (bytes <= 0) {
                        close(i);
                        FD_CLR(i, &master);
                    } else {
                        std::string response = process_line(buffer);
                        send(i, response.c_str(), response.size(), 0);
                        send(i, "\n", 1, 0);
                    }
                }
            }
        }
    }
}
