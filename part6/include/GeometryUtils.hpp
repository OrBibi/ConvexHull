#pragma once
#include <deque>

/**
 * @struct Point
 * @brief Represents a 2D point with x and y coordinates.
 *
 * Supports lexicographic comparison (first by x, then by y) to assist in sorting.
 */
struct Point {
    double x, y;
    /**
     * @brief Lexicographic comparison operator for sorting.
     *
     * @param other The point to compare with.
     * @return true if this point is less than the other, false otherwise.
     */

    bool operator<(const Point& other) const;
};

/**
 * @brief Computes the convex hull of a set of 2D points using the Monotone Chain algorithm.
 *
 * The result is a polygon (as a deque of points) that forms the convex hull of the input.
 * The points in the output are ordered counter-clockwise.
 *
 * @param points A deque of input 2D points.
 * @return A deque of points representing the convex hull.
 */
std::deque<Point> compute_convex_hull_deque(std::deque<Point> points);

/**
 * @brief Computes the area of a polygon using the shoelace formula.
 *
 * The input is assumed to be a simple polygon with vertices ordered (either clockwise or counter-clockwise).
 *
 * @param polygon A deque of points representing the polygon's vertices.
 * @return The absolute area of the polygon.
 */
double compute_area(const std::deque<Point>& polygon);
