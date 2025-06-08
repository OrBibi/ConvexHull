#include "../include/GeometryUtils.hpp"
#include <algorithm>
#include <cmath>

bool Point::operator<(const Point& other) const {
    return x < other.x || (x == other.x && y < other.y);
}

static double cross(const Point& O, const Point& A, const Point& B) {
    return (A.x - O.x) * (B.y - O.y) - (A.y - O.y) * (B.x - O.x);
}

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
