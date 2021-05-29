#pragma once

#include <random>
#include <iostream>

using namespace std;

random_device rd;
uniform_real_distribution<double> dist(.0, 1.);

long randl(long low, long high) {
    uniform_int_distribution<long> dist_minmax(low, high - 1);
    return dist_minmax(rd);
}

float randf() {
    return (float) dist(rd);
}
