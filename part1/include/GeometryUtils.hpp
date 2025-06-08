#pragma once
#include <vector>

struct Point {
    double x, y;
    bool operator<(const Point& other) const;
};

std::vector<Point> compute_convex_hull(std::vector<Point> points);
double compute_area(const std::vector<Point>& polygon);
