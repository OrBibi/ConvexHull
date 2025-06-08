#include "../include/GeometryUtils.hpp"
#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <cctype>

/*
 * Convex Hull Area Calculator (Stage 1)
 *
 * - Reads an integer N from the first line (number of points)
 * - Reads N valid points in the format x,y (floats)
 * - Ignores invalid lines and continues until N valid points are received
 * - Computes the convex hull and prints the area
 */

// Check if a string represents a valid float number
bool is_number(const std::string& s) {
    std::istringstream iss(s);
    double d;
    return (iss >> d) && iss.eof();
}

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
