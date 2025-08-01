#include "../include/GeometryUtils.hpp"
#include <iostream>
#include <string>
#include <sstream>
#include <deque>
#include <chrono>

/**
 * @file
 * @brief Convex Hull Area Calculator using std::deque (Stage 2)
 *
 * Reads a list of 2D points from standard input, computes the convex hull using
 * the Monotone Chain algorithm (with std::deque), calculates the area of the hull,
 * and measures the execution time in milliseconds.
 *
 * Input format:
 * - First line: an integer N indicating the number of points.
 * - Next N lines: each line contains a point in the format x,y.
 * - Lines that are invalid are skipped until N valid points are received.
 *
 * Output:
 * - Area of the convex hull.
 * - Time in milliseconds taken to compute the convex hull.
 */

/**
 * @brief Checks whether a string is a valid floating-point number.
 * 
 * @param s The string to check.
 * @return true if the string is a valid float, false otherwise.
 */
bool is_number(const std::string& s) {
    std::istringstream iss(s);
    double d;
    return (iss >> d) && iss.eof();
}

/**
 * @brief Main function to read points, compute convex hull, and display results.
 * 
 * Reads input from stdin, collects N valid points, computes the convex hull using a deque-based method,
 * calculates the area of the hull, measures computation time, and prints both the area and duration.
 * 
 * @return 0 on success, 1 on failure due to invalid or missing input.
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

    std::deque<Point> points;

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
    auto hull = compute_convex_hull_deque(points);
    auto end = std::chrono::high_resolution_clock::now();

    double area = compute_area(hull);
    std::cout << "Area: " << area << std::endl;
    std::cout << "Time (deque): "
              << std::chrono::duration<double, std::milli>(end - start).count()
              << " ms" << std::endl;

    return 0;
}
