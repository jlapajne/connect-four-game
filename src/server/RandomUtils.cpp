#include "RandomUtils.h"

#include <cstdint>
#include <random>

namespace {

std::mt19937_64 &getGenerator() {
    static std::random_device rd;
    static std::mt19937_64 gen(rd());
    return gen;
}
} // namespace

std::size_t getRandomInt(std::size_t max) {
    std::uniform_int_distribution<std::size_t> distribution(0U, max);
    return distribution(getGenerator());
}

bool getRandomBool() {
    std::uniform_int_distribution<std::size_t> distribution(0U, 1U);
    return bool(distribution(getGenerator()));
}

float getRandomUniformFloat() {
    std::uniform_real_distribution<float> distribution(0.0F, 1.0F);
    return distribution(getGenerator());
}
