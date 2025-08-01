#pragma once
#include <deque>

/**
 * @struct Point
 * @brief Represents a 2D point with x and y coordinates.
 */
struct Point {
    double x, y;

    /**
     * @brief Comparison operator for sorting points.
     *
     * Points are compared first by x-coordinate, and if equal, by y-coordinate.
     *
     * @param other The point to compare against.
     * @return true if this point is less than the other point.
     */
    bool operator<(const Point& other) const;
};

/**
 * @brief Computes the convex hull of a set of 2D points using the Monotone Chain algorithm.
 *
 * @param points A deque of input points.
 * @return A deque of points representing the convex hull in counter-clockwise order.
 */
std::deque<Point> compute_convex_hull_deque(std::deque<Point> points);

/**
 * @brief Computes the area of a polygon using the shoelace formula.
 *
 * @param polygon A deque of points representing the polygon vertices in order.
 * @return The absolute value of the polygon's area.
 */
double compute_area(const std::deque<Point>& polygon);
