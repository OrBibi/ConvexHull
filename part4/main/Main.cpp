#include "../include/GeometryUtils.hpp"
#include <iostream>
#include <sstream>
#include <string>
#include <deque>
#include <algorithm>
#include <cctype>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <csignal>

#define PORT 9034
#define BUFFER_SIZE 1024

std::deque<Point> point_set;
std::deque<Point> temp_points;
bool waiting_for_graph = false;
int points_to_read = 0;
int server_fd = -1;  // used in signal handler

bool is_number(const std::string& s) {
    std::istringstream iss(s);
    double d;
    return (iss >> d) && iss.eof();
}

void handle_point_line(const std::string& line) {
    size_t comma = line.find(',');
    if (comma == std::string::npos) return;

    std::string x_str = line.substr(0, comma);
    std::string y_str = line.substr(comma + 1);
    if (!is_number(x_str) || !is_number(y_str)) return;

    Point p{std::stod(x_str), std::stod(y_str)};
    temp_points.push_back(p);
    points_to_read--;

    if (points_to_read == 0) {
        point_set = temp_points;
        temp_points.clear();
        waiting_for_graph = false;
    }
}

void handle_newgraph(const std::string& arg_line) {
    std::istringstream args(arg_line);
    int n;
    if (!(args >> n) || n <= 0) return;
    waiting_for_graph = true;
    points_to_read = n;
    temp_points.clear();
}

void handle_newpoint(const std::string& args) {
    size_t comma = args.find(',');
    if (comma == std::string::npos) return;

    std::string x_str = args.substr(0, comma);
    std::string y_str = args.substr(comma + 1);
    if (!is_number(x_str) || !is_number(y_str)) return;

    Point p{std::stod(x_str), std::stod(y_str)};
    point_set.push_back(p);
}

void handle_removepoint(const std::string& args) {
    size_t comma = args.find(',');
    if (comma == std::string::npos) return;

    std::string x_str = args.substr(0, comma);
    std::string y_str = args.substr(comma + 1);
    if (!is_number(x_str) || !is_number(y_str)) return;

    Point p{std::stod(x_str), std::stod(y_str)};
    auto it = std::remove_if(point_set.begin(), point_set.end(),
                             [&](const Point& q) { return q.x == p.x && q.y == p.y; });
    point_set.erase(it, point_set.end());
}

std::string handle_ch() {
    auto hull = compute_convex_hull_deque(point_set);
    double area = compute_area(hull);
    std::ostringstream oss;
    oss << area << "\n";
    return oss.str();
}

void process_command(const std::string& line, std::string& response) {
    if (waiting_for_graph) {
        handle_point_line(line);
        return;
    }

    std::istringstream iss(line);
    std::string command;
    iss >> command;

    std::string args;
    std::getline(iss, args);
    args.erase(0, args.find_first_not_of(" "));

    if (command == "Newgraph") {
        handle_newgraph(args);
    } else if (command == "Newpoint") {
        handle_newpoint(args);
    } else if (command == "Removepoint") {
        handle_removepoint(args);
    } else if (command == "CH") {
        response += handle_ch();
    }
}

void handle_sigint(int) {
    std::cout << "\nShutting down server...\n";
    if (server_fd != -1) close(server_fd);
    exit(0);
}

int main() {
    signal(SIGINT, handle_sigint);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        return 1;
    }

    int yes = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        close(server_fd);
        return 1;
    }

    if (listen(server_fd, 5) < 0) {
        perror("listen");
        close(server_fd);
        return 1;
    }

    std::cout << "Server listening on port " << PORT << std::endl;

    while (true) {
        sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int client_sock = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
        if (client_sock < 0) {
            perror("accept");
            continue;
        }

        char buffer[BUFFER_SIZE];
        ssize_t bytes_received;
        while ((bytes_received = recv(client_sock, buffer, sizeof(buffer) - 1, 0)) > 0) {
            buffer[bytes_received] = '\0';
            std::string input(buffer);
            std::istringstream input_stream(input);
            std::string line;
            std::string output;

            while (std::getline(input_stream, line)) {
                if (!line.empty()) {
                    process_command(line, output);
                }
            }

            if (!output.empty()) {
                send(client_sock, output.c_str(), output.size(), 0);
            }
        }

        close(client_sock);
    }

    close(server_fd);
    return 0;
}
