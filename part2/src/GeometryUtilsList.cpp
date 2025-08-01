#include "../include/GeometryUtils.hpp"
#include <algorithm>
#include <cmath>
#include <vector>

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
 * @brief Computes the convex hull using the Monotone Chain algorithm with std::list.
 *
 * Converts the list to a sorted vector, builds lower and upper hulls using iterators.
 * 
 * @param input A list of 2D points.
 * @return A list of points forming the convex hull in counter-clockwise order.
 */
std::list<Point> compute_convex_hull_list(std::list<Point> input) {
    if (input.size() <= 1) return input;

    std::vector<Point> points(input.begin(), input.end());
    std::sort(points.begin(), points.end());
    std::list<Point> hull;

    for (const auto& p : points) {
        while (hull.size() >= 2) {
            auto it2 = hull.rbegin();
            auto it1 = std::next(it2);
            if ((it2->x - it1->x) * (p.y - it1->y) - (it2->y - it1->y) * (p.x - it1->x) > 0)
                break;
            hull.pop_back();
        }
        hull.push_back(p);
    }

    size_t lower_size = hull.size();
    for (auto it = points.rbegin(); it != points.rend(); ++it) {
        while (hull.size() > lower_size) {
            auto it2 = hull.rbegin();
            auto it1 = std::next(it2);
            if ((it2->x - it1->x) * (it->y - it1->y) - (it2->y - it1->y) * (it->x - it1->x) > 0)
                break;
            hull.pop_back();
        }
        hull.push_back(*it);
    }

    hull.pop_back();
    return hull;
}

/**
 * @brief Computes the area of a polygon from a list of ordered points.
 * 
 * Uses the shoelace formula. Assumes the polygon is simple and ordered.
 * 
 * @param polygon A list of polygon points in order.
 * @return The absolute area of the polygon.
 */
double compute_area(const std::list<Point>& polygon) {
    double area = 0;
    auto it1 = polygon.begin();
    for (auto it2 = std::next(it1); it2 != polygon.end(); ++it1, ++it2)
        area += (it1->x * it2->y - it2->x * it1->y);
    area += (polygon.back().x * polygon.front().y - polygon.front().x * polygon.back().y);
    return std::abs(area) / 2.0;
}
