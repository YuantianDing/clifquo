#pragma once

#include <cstddef>
namespace circuit {

template <std::size_t N>
struct CircuitInfo {
    static_assert(N <= 5);
    unsigned int nqubits : 3;
    unsigned int tail_permutation : 15;
};

}  // namespace circuit
