#include "../include/GeometryUtils.hpp"
#include <algorithm>
#include <cmath>

/**
 * @brief Comparison operator to sort points.
 * 
 * Points are compared first by x-coordinate, then by y-coordinate if x is equal.
 * 
 * @param other The point to compare to.
 * @return true if this point is less than the other.
 */
bool Point::operator<(const Point& other) const {
    return x < other.x || (x == other.x && y < other.y);
}

/**
 * @brief Computes the cross product of vectors OA and OB.
 * 
 * @param O Origin point O.
 * @param A Endpoint of vector OA.
 * @param B Endpoint of vector OB.
 * @return The cross product value.
 */
static double cross(const Point& O, const Point& A, const Point& B) {
    return (A.x - O.x) * (B.y - O.y) - (A.y - O.y) * (B.x - O.x);
}

/**
 * @brief Computes the convex hull of a set of 2D points using the Monotone Chain algorithm.
 * 
 * @param points The input vector of points.
 * @return A vector of points representing the convex hull in counter-clockwise order.
 */
std::vector<Point> compute_convex_hull(std::vector<Point> points) {
    int n = points.size(), k = 0;
    if (n <= 1) return points;

    std::sort(points.begin(), points.end());
    std::vector<Point> hull(2 * n);

    for (int i = 0; i < n; ++i) {
        while (k >= 2 && cross(hull[k - 2], hull[k - 1], points[i]) <= 0) k--;
        hull[k++] = points[i];
    }

    for (int i = n - 2, t = k + 1; i >= 0; --i) {
        while (k >= t && cross(hull[k - 2], hull[k - 1], points[i]) <= 0) k--;
        hull[k++] = points[i];
    }

    hull.resize(k - 1);
    return hull;
}

/**
 * @brief Computes the area of a polygon given its vertices in order.
 * 
 * Uses the shoelace formula.
 * 
 * @param polygon A vector of points representing the polygon's vertices in order.
 * @return The absolute area of the polygon.
 */
double compute_area(const std::vector<Point>& polygon) {
    double area = 0;
    int n = polygon.size();
    for (int i = 0; i < n; ++i) {
        const Point& p1 = polygon[i];
        const Point& p2 = polygon[(i + 1) % n];
        area += (p1.x * p2.y - p2.x * p1.y);
    }
    return std::abs(area) / 2.0;
}
