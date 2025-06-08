#include "../include/GeometryUtils.hpp"
#include <iostream>
#include <sstream>
#include <string>
#include <deque>
#include <algorithm>
#include <cctype>

/*
 * Convex Hull Interactive Program — Part 3 Final with Preserved State
 * Commands:
 *   - Newgraph N
 *   - Newpoint x,y
 *   - Removepoint x,y
 *   - CH
 * On invalid Newgraph input, old graph is preserved.
 */

std::deque<Point> point_set;
std::deque<Point> temp_points;  // temporary storage during Newgraph
bool waiting_for_graph = false;
int points_to_read = 0;

// Checks whether string is numeric
bool is_number(const std::string& s) {
    std::istringstream iss(s);
    double d;
    return (iss >> d) && iss.eof();
}

// Handles point input during Newgraph (into temp_points)
void handle_point_line(const std::string& line) {
    size_t comma = line.find(',');
    if (comma == std::string::npos) {
        std::cerr << "ERROR: Invalid point format." << std::endl;
        return; // wait for a valid point
    }

    std::string x_str = line.substr(0, comma);
    std::string y_str = line.substr(comma + 1);

    if (!is_number(x_str) || !is_number(y_str)) {
        std::cerr << "ERROR: Invalid point values." << std::endl;
        return; // wait for a valid point
    }

    Point p{std::stod(x_str), std::stod(y_str)};
    temp_points.push_back(p);
    points_to_read--;

    if (points_to_read == 0) {
        point_set = temp_points;
        temp_points.clear();
        waiting_for_graph = false;
    }
}


// Parses Newgraph N and sets up temporary state
void handle_newgraph(const std::string& arg_line) {
    std::istringstream args(arg_line);
    int n;
    if (!(args >> n) || n <= 0) {
        std::cerr << "ERROR: Invalid number in Newgraph." << std::endl;
        return;
    }

    waiting_for_graph = true;
    points_to_read = n;
    temp_points.clear();
}

// Adds a new point (outside Newgraph)
void handle_newpoint(const std::string& args) {
    size_t comma = args.find(',');
    if (comma == std::string::npos) {
        std::cerr << "ERROR: Invalid Newpoint format." << std::endl;
        return;
    }

    std::string x_str = args.substr(0, comma);
    std::string y_str = args.substr(comma + 1);
    if (!is_number(x_str) || !is_number(y_str)) {
        std::cerr << "ERROR: Invalid point values." << std::endl;
        return;
    }

    Point p{std::stod(x_str), std::stod(y_str)};
    point_set.push_back(p);
}

// Removes a point
void handle_removepoint(const std::string& args) {
    size_t comma = args.find(',');
    if (comma == std::string::npos) {
        std::cerr << "ERROR: Invalid Removepoint format." << std::endl;
        return;
    }

    std::string x_str = args.substr(0, comma);
    std::string y_str = args.substr(comma + 1);
    if (!is_number(x_str) || !is_number(y_str)) {
        std::cerr << "ERROR: Invalid point values." << std::endl;
        return;
    }

    Point p{std::stod(x_str), std::stod(y_str)};
    auto it = std::remove_if(point_set.begin(), point_set.end(),
                             [&](const Point& q) { return q.x == p.x && q.y == p.y; });
    if (it != point_set.end()) {
        point_set.erase(it, point_set.end());
    }
}

// Computes and prints convex hull area
void handle_ch() {
    auto hull = compute_convex_hull_deque(point_set);
    double area = compute_area(hull);
    std::cout << area << std::endl;
}

// Main loop
int main() {
    std::string line;

    while (std::getline(std::cin, line)) {
        if (line.empty()) continue;

        // If expecting points for Newgraph
        if (waiting_for_graph) {
            handle_point_line(line);
            continue;
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
            handle_ch();
        } else {
            std::cerr << "ERROR: Unknown command." << std::endl;
        }
    }

    return 0;
}
