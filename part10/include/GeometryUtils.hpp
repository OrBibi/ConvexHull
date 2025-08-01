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
     * Points are compared first by x-coordinate, then by y-coordinate.
     * 
     * @param other The point to compare with.
     * @return true if this point is less than the other.
     */
    bool operator<(const Point& other) const;
};

/**
 * @brief Computes the convex hull of a set of 2D points using a deque-based approach.
 * 
 * The convex hull is the smallest convex polygon that contains all the given points.
 * 
 * @param points A deque of input points.
 * @return A deque containing the points on the convex hull in counterclockwise order.
 */
std::deque<Point> compute_convex_hull_deque(std::deque<Point> points);

/**
 * @brief Computes the area of a polygon using the shoelace formula.
 * 
 * Assumes the polygon is given in counterclockwise order and is simple (no self-intersections).
 * 
 * @param polygon A deque of points representing the polygon.
 * @return The area of the polygon.
 */
double compute_area(const std::deque<Point>& polygon);
