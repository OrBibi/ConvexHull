#pragma once
#include <deque>
#include <list>

/**
 * @brief Struct representing a 2D point with x and y coordinates.
 */
struct Point {
    double x, y;

    /**
     * @brief Lexicographic comparison operator for sorting points.
     *
     * Compares points first by x, then by y if x values are equal.
     * 
     * @param other The point to compare against.
     * @return true if this point is less than the other, false otherwise.
     */
    bool operator<(const Point& other) const;
};

/**
 * @brief Computes the convex hull of a set of 2D points using a std::deque.
 *
 * Uses the Monotone Chain algorithm to find the convex hull in O(n log n) time.
 * 
 * @param points A deque of input points.
 * @return A deque of points forming the convex hull in counter-clockwise order.
 */
std::deque<Point> compute_convex_hull_deque(std::deque<Point> points);

/**
 * @brief Computes the convex hull of a set of 2D points using a std::list.
 *
 * Converts the list to a sorted vector internally, then builds the hull.
 * 
 * @param points A list of input points.
 * @return A list of points forming the convex hull in counter-clockwise order.
 */
std::list<Point> compute_convex_hull_list(std::list<Point> points);

/**
 * @brief Calculates the area of a polygon represented as a list of points.
 *
 * Uses the shoelace formula for area calculation.
 * 
 * @param polygon A list of ordered points forming a simple polygon.
 * @return The absolute area of the polygon.
 */
double compute_area(const std::list<Point>& polygon);

/**
 * @brief Calculates the area of a polygon represented as a deque of points.
 *
 * Uses the shoelace formula for area calculation.
 * 
 * @param polygon A deque of ordered points forming a simple polygon.
 * @return The absolute area of the polygon.
 */
double compute_area(const std::deque<Point>& polygon);
