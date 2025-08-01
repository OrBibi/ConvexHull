#pragma once
#include <vector>

struct Point { // Struct representing a 2D point with x and y coordinates.
    double x, y;

    /**
     * @brief Comparison operator to sort points.
     * 
     * Points are compared first by x-coordinate, then by y-coordinate if x is equal.
     * 
     * @param other The point to compare to.
     * @return true if this point is less than the other.
     */
    bool operator<(const Point& other) const;
};

/**
 * @brief Computes the convex hull of a set of 2D points using the Monotone Chain algorithm.
 * 
 * @param points The input vector of points.
 * @return A vector of points representing the convex hull in counter-clockwise order.
 */
std::vector<Point> compute_convex_hull(std::vector<Point> points);

/**
 * @brief Computes the area of a polygon given its vertices in order.
 * 
 * Uses the shoelace formula.
 * 
 * @param polygon A vector of points representing the polygon's vertices in order.
 * @return The absolute area of the polygon.
 */
double compute_area(const std::vector<Point>& polygon);
