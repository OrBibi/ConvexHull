#include "../include/GeometryUtils.hpp"
#include <iostream>
#include <string>
#include <sstream>
#include <list>
#include <chrono>

/**
 * @file
 * @brief Convex Hull Area Calculator using std::list (Stage 3)
 *
 * This program reads 2D points from standard input, computes their convex hull
 * using the Monotone Chain algorithm adapted for `std::list`, calculates the area
 * of the convex hull, and prints both the area and the computation time.
 *
 * Input format:
 * - First line: an integer N representing the number of points.
 * - Next N lines: each line in the format x,y (floating-point values).
 * - Invalid or empty lines are ignored until N valid points are collected.
 *
 * Output:
 * - The area of the convex hull.
 * - The time taken to compute the convex hull (in milliseconds).
 */

/**
 * @brief Checks if a given string is a valid floating-point number.
 *
 * @param s The input string to check.
 * @return true if the string represents a valid float, false otherwise.
 */
bool is_number(const std::string& s) {
    std::istringstream iss(s);
    double d;
    return (iss >> d) && iss.eof();
}

/**
 * @brief Main function that reads input, computes convex hull, and prints area and time.
 *
 * Reads an integer N and then reads N valid 2D points in the form x,y from standard input.
 * It uses a list-based implementation of the convex hull algorithm, calculates the area
 * using the shoelace formula, and prints both the area and execution time.
 *
 * @return 0 on success, 1 on input or processing error.
 */
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
