#include "../include/GeometryUtils.hpp"
#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <cctype>

/**
 * @file
 * @brief Convex Hull Area Calculator (Stage 1)
 *
 * Reads a set of 2D points from standard input, computes the convex hull using
 * the Monotone Chain algorithm, and prints the area of the resulting polygon.
 *
 * Input format:
 * - First line: an integer N representing the number of points.
 * - Next N lines: each line contains a point in the format x,y (with floating-point values).
 * - Invalid lines are ignored until N valid points are collected.
 *
 * Output:
 * - A single floating-point number: the area of the convex hull.
 */

/**
 * @brief Checks whether a string represents a valid floating-point number.
 * 
 * @param s The input string to validate.
 * @return true if the string is a valid float, false otherwise.
 */
bool is_number(const std::string& s) {
    std::istringstream iss(s);
    double d;
    return (iss >> d) && iss.eof();
}

/**
 * @brief Main entry point of the program.
 * 
 * Reads input from stdin, collects valid points, computes their convex hull,
 * and prints the area of the convex hull to stdout.
 * 
 * @return 0 on success, 1 on input error.
 */
int main() {
    std::string line;
    int n = 0;

    // Read number of points from the first line
    while (true) {
        if (!std::getline(std::cin, line)) {
            std::cerr << "ERROR: Missing number of points." << std::endl;
            return 1;
        }

        std::istringstream iss(line);
        if ((iss >> n) && n > 0) break;

        std::cerr << "ERROR: Invalid number format." << std::endl;
    }

    std::vector<Point> points;

    // Read exactly N valid points
    while ((int)points.size() < n) {
        if (!std::getline(std::cin, line)) {
            std::cerr << "ERROR: Unexpected end of input — expected " 
                      << (n - points.size()) << " more point(s)." << std::endl;
            return 1;
        }

        if (line.empty()) continue;

        size_t comma = line.find(',');
        if (comma == std::string::npos) {
            std::cerr << "ERROR: Invalid point format." << std::endl;
            continue;
        }

        std::string x_str = line.substr(0, comma);
        std::string y_str = line.substr(comma + 1);

        if (!is_number(x_str) || !is_number(y_str)) {
            std::cerr << "ERROR: Invalid point values." << std::endl;
            continue;
        }

        Point p;
        p.x = std::stod(x_str);
        p.y = std::stod(y_str);
        points.push_back(p);
    }

    // Compute convex hull and area
    std::vector<Point> hull = compute_convex_hull(points);
    double area = compute_area(hull);

    // Print the result
    std::cout << area << std::endl;

    return 0;
}
