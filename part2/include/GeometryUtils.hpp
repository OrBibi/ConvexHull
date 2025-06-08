#pragma once
#include <deque>
#include <list>

struct Point {
    double x, y;
    bool operator<(const Point& other) const;
};

std::deque<Point> compute_convex_hull_deque(std::deque<Point> points);
std::list<Point> compute_convex_hull_list(std::list<Point> points);
double compute_area(const std::list<Point>& polygon);
double compute_area(const std::deque<Point>& polygon);
