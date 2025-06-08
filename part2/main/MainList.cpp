#include "../include/GeometryUtils.hpp"
#include <iostream>
#include <string>
#include <sstream>
#include <list>
#include <chrono>

// Check if a string is a valid float number
bool is_number(const std::string& s) {
    std::istringstream iss(s);
    double d;
    return (iss >> d) && iss.eof();
}

int main() {
    std::string line;
    int n = 0;

    // Read number of points
    while (true) {
        if (!std::getline(std::cin, line)) {
            std::cerr << "ERROR: Missing number of points." << std::endl;
            return 1;
        }

        std::istringstream iss(line);
        if ((iss >> n) && n > 0) break;
        std::cerr << "ERROR: Invalid number format." << std::endl;
    }

    std::list<Point> points;

    // Read exactly n valid points
    while ((int)points.size() < n) {
        if (!std::getline(std::cin, line)) {
            std::cerr << "ERROR: Unexpected end of input." << std::endl;
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

        Point p{std::stod(x_str), std::stod(y_str)};
        points.push_back(p);
    }

    auto start = std::chrono::high_resolution_clock::now();
    auto hull = compute_convex_hull_list(points);
    auto end = std::chrono::high_resolution_clock::now();

    double area = compute_area(hull);
    std::cout << "Area: " << area << std::endl;
    std::cout << "Time (list): "
              << std::chrono::duration<double, std::milli>(end - start).count()
              << " ms" << std::endl;

    return 0;
}
