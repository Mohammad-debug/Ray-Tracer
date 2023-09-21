#ifndef HELPERS_H
#define HELPERS_H

double random_double(double min, double max) {
    return min + (max - min) * (rand() / (RAND_MAX + 1.0));
}

#endif