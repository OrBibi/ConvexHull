#pragma once
#include <deque>

/**
 * @struct Point
 * @brief Represents a 2D point with x and y coordinates.
 *
 * Used as the basic unit for geometric operations like convex hull and area calculation.
 */
struct Point {
    double x, y;

    /**
     * @brief Comparison operator for sorting points.
     *
     * Points are compared lexicographically: first by x-coordinate, then by y-coordinate.
     *
     * @param other The point to compare with.
     * @return true if this point is less than the other, false otherwise.
     */
    bool operator<(const Point& other) const;
};

/**
 * @brief Computes the convex hull of a set of 2D points using the Monotone Chain algorithm.
 *
 * The algorithm returns the convex polygon that wraps around all the input points.
 * The resulting deque contains the points of the convex hull in counter-clockwise order.
 *
 * @param points A deque of input points.
 * @return A deque of points forming the convex hull.
 */
std::deque<Point> compute_convex_hull_deque(std::deque<Point> points);

/**
 * @brief Computes the area of a polygon using the shoelace formula.
 *
 * Assumes the input points form a simple polygon with vertices ordered sequentially (clockwise or counter-clockwise).
 *
 * @param polygon A deque of points representing the polygon's vertices in order.
 * @return The absolute area of the polygon.
 */
double compute_area(const std::deque<Point>& polygon);
