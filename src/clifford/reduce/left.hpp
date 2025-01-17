#pragma once

#include <doctest/doctest.h>
#include <fmt/core.h>
#include <fmt/ranges.h>
#include <cassert>
#include <range/v3/algorithm/all_of.hpp>
#include <utility>
#include "../bitsymplectic.hpp"
#include "../gates.hpp"

template <const std::size_t N>
inline constexpr BitSymplectic<N> left_reduce_row(BitSymplectic<N> input, std::size_t irow) noexcept {
    auto x = input.xrow(irow);
    auto z = input.zrow(irow);
    auto y = x ^ z;

    assert(x != z && x != y && y != z);

    if (x > z) {
        std::swap(/*mut*/ x, /*mut*/ z);
        input.do_hadamard_l(irow);
    }

    if (z > y) {
        std::swap(/*mut*/ z, /*mut*/ y);
        input.do_phase_l(irow);
    }

    if (x > z) {
        std::swap(/*mut*/ x, /*mut*/ z);
        input.do_hadamard_l(irow);
    }

    assert(x == input.xrow(irow));
    assert(y == (input.xrow(irow) ^ input.zrow(irow)));
    assert(z == input.zrow(irow));
    assert(x < z && z < y);
    return input;
}

template <const std::size_t N>
inline constexpr BitSymplectic<N> left_reduce(BitSymplectic<N> input) noexcept {
    for (auto i = 0ul; i < N; i++) {
        input = left_reduce_row(input, i);
    }

    return input;
}

// NOLINTBEGIN
TEST_FN(left_reduce) {
    const auto reduced = BitSymplectic<5ul>::identity();
    auto matrix = left_reduce(reduced.phase_l(1ul).hadamard_l(1ul).hadamard_l(2ul).phase_l(2ul).hphaseh_l(3ul).phase_l(4ul));
    CHECK_EQ(reduced, matrix);
    for (auto i = 0ul; i < 5ul; i++) {
        auto original = BitSymplectic<5z>::identity();
        perform_random_gates(original, 30, CliffordGate<5z>::all_gates(), Bv<2>(0b11));
        const auto reduced = left_reduce(original);
        perform_random_gates(original, 30, CliffordGate<5z>::all_level_0(), Bv<2>(0b01));
        const auto result = left_reduce(original);
        CHECK_EQ(reduced, result);
    }
}
// NOLINTEND