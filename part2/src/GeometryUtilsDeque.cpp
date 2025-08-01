#include "../include/GeometryUtils.hpp"
#include <algorithm>
#include <cmath>

/**
 * @brief Lexicographic comparison operator for sorting points.
 *
 * Compares points first by x, then by y if x values are equal.
 * 
 * @param other The point to compare against.
 * @return true if this point is less than the other, false otherwise.
 */
bool Point::operator<(const Point& other) const {
    return x < other.x || (x == other.x && y < other.y);
}

/**
 * @brief Computes the cross product of vectors OA and OB.
 *
 * @param O The origin point.
 * @param A The first endpoint.
 * @param B The second endpoint.
 * @return The cross product of OA and OB.
 */
static double cross(const Point& O, const Point& A, const Point& B) {
    return (A.x - O.x) * (B.y - O.y) - (A.y - O.y) * (B.x - O.x);
}

/**
 * @brief Computes the convex hull using the Monotone Chain algorithm with std::deque.
 *
 * Builds the upper and lower hulls by maintaining a deque and removing non-convex points.
 * 
 * @param points The input deque of 2D points.
 * @return A deque of points forming the convex hull in counter-clockwise order.
 */
std::deque<Point> compute_convex_hull_deque(std::deque<Point> points) {
    size_t n = points.size();
    if (n <= 1) return points;

    std::sort(points.begin(), points.end());
    std::deque<Point> hull;

    for (size_t i = 0; i < n; ++i) {
        while (hull.size() >= 2 && cross(hull[hull.size() - 2], hull.back(), points[i]) <= 0)
            hull.pop_back();
        hull.push_back(points[i]);
    }

    size_t t = hull.size() + 1;
    for (size_t i = n - 2; i < n; --i) {
        while (hull.size() >= t && cross(hull[hull.size() - 2], hull.back(), points[i]) <= 0)
            hull.pop_back();
        hull.push_back(points[i]);
        if (i == 0) break; // prevent unsigned underflow
    }

    hull.pop_back();
    return hull;
}

/**
 * @brief Computes the area of a polygon from a deque of ordered points.
 * 
 * Uses the shoelace formula. Assumes the polygon is simple and ordered.
 * 
 * @param polygon A deque of polygon points in order.
 * @return The absolute area of the polygon.
 */
double compute_area(const std::deque<Point>& polygon) {
    double area = 0;
    size_t n = polygon.size();
    for (size_t i = 0; i < n; ++i) {
        const Point& p1 = polygon[i];
        const Point& p2 = polygon[(i + 1) % n];
        area += (p1.x * p2.y - p2.x * p1.y);
    }
    return std::abs(area) / 2.0;
}
