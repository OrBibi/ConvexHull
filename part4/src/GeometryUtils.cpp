#include "../include/GeometryUtils.hpp"
#include <algorithm>
#include <cmath>

/**
 * @brief Comparison operator for sorting points.
 *
 * Points are sorted lexicographically: first by x-coordinate,
 * and if equal, by y-coordinate.
 *
 * @param other The point to compare with.
 * @return true if this point is less than the other, false otherwise.
 */
bool Point::operator<(const Point& other) const {
    return x < other.x || (x == other.x && y < other.y);
}

/**
 * @brief Calculates the cross product of vectors OA and OB.
 *
 * This is used to determine the orientation of the turn:
 * - > 0: left turn
 * - = 0: collinear
 * - < 0: right turn
 *
 * @param O Origin point (pivot).
 * @param A First endpoint.
 * @param B Second endpoint.
 * @return The signed cross product value.
 */
static double cross(const Point& O, const Point& A, const Point& B) {
    return (A.x - O.x) * (B.y - O.y) - (A.y - O.y) * (B.x - O.x);
}

/**
 * @brief Computes the convex hull of a set of 2D points using the Monotone Chain algorithm.
 *
 * The algorithm first sorts the input points, then constructs the lower and upper parts of the hull.
 * The final result is a counter-clockwise ordered deque of points representing the convex hull.
 *
 * @param points A deque of input points.
 * @return A deque of points forming the convex hull in counter-clockwise order.
 */
std::deque<Point> compute_convex_hull_deque(std::deque<Point> points) {
    size_t n = points.size();
    if (n <= 1) return points;

    std::sort(points.begin(), points.end());
    std::deque<Point> hull;

    // Build lower hull
    for (size_t i = 0; i < n; ++i) {
        while (hull.size() >= 2 && cross(hull[hull.size() - 2], hull.back(), points[i]) <= 0)
            hull.pop_back();
        hull.push_back(points[i]);
    }

    // Build upper hull
    size_t t = hull.size() + 1;
    for (size_t i = n - 2; i < n; --i) {
        while (hull.size() >= t && cross(hull[hull.size() - 2], hull.back(), points[i]) <= 0)
            hull.pop_back();
        hull.push_back(points[i]);
        if (i == 0) break; // prevent unsigned underflow
    }

    hull.pop_back(); // Remove duplicated last point
    return hull;
}

/**
 * @brief Computes the area of a polygon using the shoelace formula.
 *
 * Assumes the polygon is simple (non-intersecting) and the vertices are ordered
 * (clockwise or counter-clockwise). The formula sums the cross-products of adjacent edges.
 *
 * @param polygon A deque of ordered points representing the polygon.
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
