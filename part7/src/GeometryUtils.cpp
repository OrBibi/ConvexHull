#include "../include/GeometryUtils.hpp"
#include <algorithm>
#include <cmath>

/**
 * @brief Operator overload to compare Points lexicographically (by x, then y).
 * 
 * @param other The point to compare with.
 * @return true if this point is less than other.
 */
bool Point::operator<(const Point& other) const {
    return x < other.x || (x == other.x && y < other.y);
}

/**
 * @brief Computes the cross product of vectors OA and OB.
 * 
 * @param O Origin point.
 * @param A First point.
 * @param B Second point.
 * @return The cross product (positive if counter-clockwise turn).
 */
static double cross(const Point& O, const Point& A, const Point& B) {
    return (A.x - O.x) * (B.y - O.y) - (A.y - O.y) * (B.x - O.x);
}

/**
 * @brief Computes the convex hull of a set of points using the monotone chain algorithm.
 * 
 * @param points A deque of 2D points.
 * @return std::deque<Point> representing the convex hull in counter-clockwise order.
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
 * @brief Computes the area of a simple polygon using the shoelace formula.
 * 
 * @param polygon A deque of points forming the polygon (assumed to be in order).
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
