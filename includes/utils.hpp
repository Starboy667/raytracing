#pragma once

#include <random>

inline float random_float() {
    static std::random_device rd;
    static std::uniform_real_distribution<float> distribution(0.0f, 1.0f);
    static std::mt19937 generator(rd());
    return distribution(generator);
}

inline float random_float(float min, float max) {
    return min + random_float() * (max - min);
}