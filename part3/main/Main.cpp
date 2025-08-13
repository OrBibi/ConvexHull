// main/Main.cpp
// Stage 3 — STDIN/STDOUT interaction (no networking)

#include "GeometryUtils.hpp"
#include <iostream>
#include <sstream>
#include <string>
#include <deque>
#include <algorithm>
#include <cctype>

static std::deque<Point> point_set;     //Stores the current set of points for the graph
static std::deque<Point> temp_points;   // Temporary container for points while creating a new graph
static bool waiting_for_graph = false;  // Flag indicating if we are currently reading multiple points for a new graph
static int points_to_read = 0;          // How many more points need to be read for the current new graph

/**
 * @brief Removes whitespace characters from the beginning of the string.
 * @param s String to be trimmed.
 */
static inline void ltrim(std::string& s) {
    // std::isspace checks whether the leading char is whitespace.
    // s.front() accesses the first character of the string.
    while (!s.empty() && std::isspace(static_cast<unsigned char>(s.front())))
        s.erase(s.begin());
}

/**
 * @brief Removes whitespace characters (and \r or \n) from the end of the string.
 * @param s String to be trimmed.
 */
static inline void rtrim(std::string& s) {
    // std::isspace checks whether the trailing char is whitespace.
    // s.back() accesses the last character of the string.
    while (!s.empty() && (s.back() == '\r' || s.back() == '\n' ||
                          std::isspace(static_cast<unsigned char>(s.back()))))
        s.pop_back();
}

/**
 * @brief Removes whitespace from both ends of the string.
 * @param s String to be trimmed.
 */
static inline void trim(std::string& s) { rtrim(s); ltrim(s); }

/**
 * @brief Checks whether the given string represents a valid floating-point number.
 * @param s Input string.
 * @return true if s is a valid float, false otherwise.
 */
static bool is_number(const std::string& s) {
    std::istringstream iss(s); //allows us to parse from a string as if it were a stream.
    double d;
    return (iss >> d) && iss.eof(); // (iss >> d) tries to parse a double; iss.eof() ensures no extra characters remain.
}

/**
 * @brief Handles a line containing a point in the format "x,y" during new graph creation.
 * @param line The input line containing coordinates.
 * @return "OK" if successful, error message otherwise.
 */
static std::string handle_point_line(const std::string& line) {
    size_t comma = line.find(',');
    if (comma == std::string::npos) return "ERROR: Invalid point format.";

    std::string x_str = line.substr(0, comma);
    std::string y_str = line.substr(comma + 1);
    if (!is_number(x_str) || !is_number(y_str)) return "ERROR: Invalid point values.";

    Point p{std::stod(x_str), std::stod(y_str)}; // std::stod converts a numeric string (like "2.5") to a double.
    temp_points.push_back(p);
    points_to_read--;
    if (points_to_read == 0) {
        point_set = std::move(temp_points); //moves the contents of temp_points into point_set without copying.
        temp_points.clear();
        waiting_for_graph = false;
    }
    return "OK";
}

/**
 * @brief Handles the "Newgraph" command.
 * @param arg_line The arguments after the command (expected: number of points).
 * @return "OK" if successful, error message otherwise.
 */
static std::string handle_newgraph(const std::string& arg_line) {
    std::istringstream args(arg_line);
    int n;
    if (!(args >> n) || n <= 0) return "ERROR: Invalid number in Newgraph.";

    waiting_for_graph = true;
    points_to_read = n;
    temp_points.clear();
    return "OK";
}

/**
 * @brief Handles the "Newpoint" command (add a single point immediately).
 * @param args The arguments containing "x,y".
 * @return "OK" if successful, error message otherwise.
 */
static std::string handle_newpoint(const std::string& args) {
    size_t comma = args.find(',');
    if (comma == std::string::npos) return "ERROR: Invalid Newpoint format.";

    std::string x_str = args.substr(0, comma);
    std::string y_str = args.substr(comma + 1);
    if (!is_number(x_str) || !is_number(y_str)) return "ERROR: Invalid point values.";

    Point p{std::stod(x_str), std::stod(y_str)}; //string-to-double conversion for X/Y values.
    point_set.push_back(p);
    return "OK";
}

/**
 * @brief Handles the "Removepoint" command (removes all matching points from the set).
 * @param args The arguments containing "x,y".
 * @return "OK" if successful, error message otherwise.
 */
static std::string handle_removepoint(const std::string& args) {
    size_t comma = args.find(',');
    if (comma == std::string::npos) return "ERROR: Invalid Removepoint format.";

    std::string x_str = args.substr(0, comma);
    std::string y_str = args.substr(comma + 1);
    if (!is_number(x_str) || !is_number(y_str)) return "ERROR: Invalid point values.";

    // std::remove_if reorders the range so that elements to "remove" are moved to the end;
    // it returns an iterator to the first of those "removed" elements.
    // The lambda matches points equal to (p.x, p.y).
    Point p{std::stod(x_str), std::stod(y_str)};
    std::deque<Point>::iterator it =
        std::remove_if(point_set.begin(), point_set.end(),
                       [&](const Point& q) { return q.x == p.x && q.y == p.y; });
    if (it != point_set.end()) point_set.erase(it, point_set.end());
    return "OK";
}

/**
 * @brief Handles the "CH" command (compute convex hull and its area).
 * @return The computed area as a string.
 */
static std::string handle_ch() {
    std::deque<Point> hull = compute_convex_hull_deque(point_set);
    double area = compute_area(hull);
    std::ostringstream oss;
    oss << area;
    return oss.str(); // retrieves the built string (implicitly via return below).
}

/**
 * @brief Processes a single input line from stdin.
 * @param line The line of input.
 * @return Output string to be printed, or empty if nothing to print.
 */
static std::string process_line(std::string line) {
    if (line.empty()) return "";
    trim(line);

    // If currently reading points for a new graph, handle directly as point input
    if (waiting_for_graph) {
        return handle_point_line(line);  // expect "x,y" lines
    }

    // Parse the first token as command, rest as arguments
    std::istringstream iss(line);
    std::string command;
    iss >> command;

    std::string args;
    std::getline(iss, args);
    if (!args.empty() && args.front() == ' ') args.erase(args.begin());

    // Command dispatch
    if (command == "Newgraph")       return handle_newgraph(args);
    else if (command == "Newpoint")  return handle_newpoint(args);
    else if (command == "Removepoint") return handle_removepoint(args);
    else if (command == "CH")        return handle_ch();
    else                             return "ERROR: Unknown command.";
}

/**
 * @brief Main loop: reads commands from stdin and writes results to stdout.
 * @return Exit status code (0 for success).
 */
int main() {
    std::ios::sync_with_stdio(false); // Disable C-style sync for faster I/O
    std::cin.tie(nullptr);            // Untie cin from cout for performance

    std::string line;
    while (std::getline(std::cin, line)) {
        std::string out = process_line(line);
        if (!out.empty()) {
            std::cout << out << "\n";
            std::cout.flush();
        }
    }
    return 0;
}