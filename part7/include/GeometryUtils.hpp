#pragma once
#include <deque>

/**
 * @struct Point
 * @brief Represents a 2D point with x and y coordinates.
 *
 * Includes a lexicographic comparison operator used for sorting points.
 */
struct Point {
    double x, y;

    /**
     * @brief Lexicographic comparison operator.
     *
     * Compares first by x-coordinate, then by y-coordinate if x values are equal.
     *
     * @param other The point to compare with.
     * @return true if this point is less than the other, false otherwise.
     */
    bool operator<(const Point& other) const;
};

/**
 * @brief Computes the convex hull of a set of 2D points using the Monotone Chain algorithm.
 *
 * The returned deque contains the points of the convex hull in counter-clockwise order.
 * Duplicate and collinear boundary points are handled correctly.
 *
 * @param points A deque of input points.
 * @return A deque of points forming the convex hull.
 */
std::deque<Point> compute_convex_hull_deque(std::deque<Point> points);

/**
 * @brief Computes the area of a simple polygon using the shoelace formula.
 *
 * The input polygon is expected to be simple (non-intersecting) and its vertices
 * must be ordered (clockwise or counter-clockwise).
 *
 * @param polygon A deque of points representing the polygon vertices in order.
 * @return The absolute area of the polygon.
 */
double compute_area(const std::deque<Point>& polygon);
