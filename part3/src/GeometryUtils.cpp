#include "../include/GeometryUtils.hpp"
#include <algorithm>
#include <cmath>

/**
 * @brief Comparison operator for sorting points.
 *
 * Points are compared lexicographically: first by x-coordinate, then by y-coordinate.
 *
 * @param other The point to compare with.
 * @return true if this point is less than the other, false otherwise.
 */
bool Point::operator<(const Point& other) const {
    return x < other.x || (x == other.x && y < other.y);
}

/**
 * @brief Computes the cross product of vectors OA and OB.
 *
 * Used to determine the turn direction when constructing the convex hull.
 * A positive value indicates a left turn, negative a right turn, and zero means collinear.
 *
 * @param O Origin point.
 * @param A First endpoint.
 * @param B Second endpoint.
 * @return The cross product value.
 */
static double cross(const Point& O, const Point& A, const Point& B) {
    return (A.x - O.x) * (B.y - O.y) - (A.y - O.y) * (B.x - O.x);
}

/**
 * @brief Computes the convex hull of a set of 2D points using the Monotone Chain algorithm.
 *
 * The input points are first sorted, and then the lower and upper parts of the hull are constructed.
 * The result is a deque of points ordered counter-clockwise representing the convex polygon.
 *
 * @param points A deque of input points.
 * @return A deque of points forming the convex hull.
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

    hull.pop_back(); // remove duplicated point
    return hull;
}

/**
 * @brief Computes the area of a polygon using the shoelace formula.
 *
 * The polygon is assumed to be simple (non-intersecting) and its vertices are ordered.
 *
 * @param polygon A deque of points representing the vertices of the polygon in order.
 * @return The absolute value of the area.
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
