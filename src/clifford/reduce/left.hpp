#pragma once

#include <doctest/doctest.h>
#include <fmt/core.h>
#include <algorithm>
#include <cassert>
#include <range/v3/algorithm/all_of.hpp>
#include <utility>
#include "../../circuit/gateset/symmetry3.hpp"
#include "../../utils/fmt.hpp"
#include "../bitsymplectic.hpp"
#include "gates.hpp"

namespace clfd {

template <const std::size_t N>
[[nodiscard]] inline constexpr BitSymplectic<N> left_reduce_row(BitSymplectic<N> input, std::size_t irow) noexcept {
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
[[nodiscard]] inline constexpr BitSymplectic<N> left_reduce(BitSymplectic<N> input) noexcept {
    for (auto i = 0ul; i < N; i++) {
        input = left_reduce_row(input, i);
    }

    return input;
}

template <std::size_t N>
[[nodiscard]] inline constexpr circ::Symmetry3 left_reduce_row_backtrack(BitSymplectic<N> base, BitSymplectic<N> target, std::size_t irow) noexcept {
    assert(left_reduce_row(base, irow).get_row(irow) == left_reduce_row(target, irow).get_row(irow));
    std::array<circ::Symmetry3, 6> symmetry3_all = circ::Symmetry3::all();
    for (auto perm : symmetry3_all) {
        if (base.mul_l(perm, irow).get_row(irow) == target.get_row(irow)) { return perm; }
    }
    __builtin_unreachable();
}

template <const std::size_t N>
[[nodiscard]] inline constexpr circ::Symmetry3N<N> left_reduce_backtrack(BitSymplectic<N> base, BitSymplectic<N> target) noexcept {
    Bv<3ul * N> result;
    for (auto i = 0ul; i < N; i++) {
        result.update_slice(3ul * i, left_reduce_row(base, target, i));
    }

    return result;
}

}  // namespace clfd
// NOLINTBEGIN
TEST_FN(left_reduce) {
    const auto reduced = clfd::BitSymplectic<5ul>::identity();
    auto matrix = clfd::left_reduce(reduced.phase_l(1ul).hadamard_l(1ul).hadamard_l(2ul).phase_l(2ul).hphaseh_l(3ul).phase_l(4ul));
    CHECK_EQ(reduced, matrix);
    for (auto i = 0ul; i < 5ul; i++) {
        auto original = clfd::BitSymplectic<5z>::identity();
        perform_random_gates(original, 30, clfd::CliffordGate<5z>::all_gates(), Bv<2>(0b11));
        const auto reduced = clfd::left_reduce(original);
        perform_random_gates(original, 30, clfd::CliffordGate<5z>::all_level_0(), Bv<2>(0b01));
        const auto result = clfd::left_reduce(original);
        CHECK_EQ(reduced, result);
    }
}
// NOLINTEND