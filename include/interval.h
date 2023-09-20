#ifndef INTERVAL_H
#define INTERVAL_H

#include <iostream>

class interval {
public:
    double min, max;
    static const interval empty, universe;

    interval() : min(INFINITY), max(-1 * INFINITY) {} // Default interval is empty

    interval(double _min, double _max) : min(_min), max(_max) {}

    bool contains(double x) const {
        return min <= x && x <= max;
    }

    bool surrounds(double x) const {
        return min < x && x < max;
    }

    double clamp(double x) const {
        if (x < min) return min;
        if (x > max) return max;
        return x;
    }

};

const static interval empty(INFINITY, -1 * INFINITY);
const static interval universe(-1 * INFINITY, INFINITY);

#endif