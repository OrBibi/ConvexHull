#pragma once
#include <deque>

struct Point {
    double x, y;
    bool operator<(const Point& other) const;
};

std::deque<Point> compute_convex_hull_deque(std::deque<Point> points);
double compute_area(const std::deque<Point>& polygon);
