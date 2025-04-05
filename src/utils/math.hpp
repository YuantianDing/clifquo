#pragma once
#include <cstddef>

namespace utils {

inline std::size_t factorial(std::size_t n) {
    std::size_t f = 1;
    for (std::size_t i = 1; i <= n; ++i) {
        f *= i;
    }
    return f;
}

inline std::size_t power(std::size_t x, std::size_t p) {
    std::size_t i = 1;
    for (std::size_t j = 1; j <= p; j++) {
        i *= x;
    }
    return i;
}

}  // namespace utils