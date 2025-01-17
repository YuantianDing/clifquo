#pragma once

#include <algorithm>
#include <cstddef>
#include "../gates.hpp"
#include "local.hpp"
#include "permutation_helper.hpp"
#include "range/v3/view/cartesian_product.hpp"

inline constexpr Bv<2> bit_rank2x2(bool x00, bool x01, bool x10, bool x11) noexcept {
    switch (int(x00) + int(x01) + int(x10) + int(x11)) {
        case 0:
            return Bv<2>(0b00);
        case 1:
            return Bv<2>(0b01);
        case 2:
            return (x00 && x11 || x01 && x10) ? Bv<2>(0b11) : Bv<2>(0b01);
        case 3:
            return Bv<2>(0b11);
        case 4:
            return Bv<2>(0b01);
        default:
            __builtin_unreachable();
    }
}

template <const std::size_t N>
inline constexpr Bv<N * N * 2> kappa(BitSymplectic<N> input) noexcept {
    auto result = Bv<N * N * 2>::zero();
    for (auto [i, j] : vw::cartesian_product(vw::ints(0ul, N), vw::ints(0ul, N))) {
        auto rank = bit_rank2x2(input.get(i, j), input.get(i, j + N), input.get(i + N, j), input.get(i + N, j + N));
        result = result.update_slice(i * N + j, rank);
    }
    return result;
}

template <const std::size_t N>
inline constexpr std::vector<BitSymplectic<N>> compute_permutation_set(BitSymplectic<N> input) noexcept {
    auto kappa_min = kappa(input);
    auto matrix = input;
    std::vector<BitSymplectic<N>> result{matrix};
    PermutationHelper helper(&matrix);

    while (std::next_permutation(helper.begin(), helper.end())) {
        auto current = kappa(matrix);

        if (current > kappa_min) { continue; }
        if (left_reduce(matrix) == left_reduce(input)) { continue; }
        if (current < kappa_min) {
            kappa_min = current;
            result.clear();
            result.push_back(matrix);
        } else if (current == kappa_min) {
            result.push_back(matrix);
        }
    }
    return result;
}

template <const std::size_t N>
inline constexpr BitSymplectic<N> global_reduce(BitSymplectic<N> input) noexcept {
    auto minimum = local_reduce(input);
    for (auto&& matrix : compute_permutation_set(input)) {
        auto&& reduced = local_reduce(matrix);
        if (minimum == input || reduced < minimum) { minimum = reduced; }
    }
    return minimum;
}

// NOLINTBEGIN
TEST_FN(global_reduce) {
    {
        auto original = BitSymplectic<5>::identity();
        perform_random_gates(original, 15, CliffordGate<5z>::all_gates(), Bv<2>(0b11));
        const auto reduced = global_reduce(original);
        original = original.swap(2, 3).swap(1, 3).swap(2, 4).swap(0, 4).hadamard_l(2).phase_r(0);
        const auto result = global_reduce(original);
        CHECK_EQ(reduced, result);
    }
    for (auto i = 0ul; i < 100ul; i++) {
        auto original = BitSymplectic<5z>::identity();
        perform_random_gates(original, 15, CliffordGate<5z>::all_gates(), Bv<2>(0b11));
        const auto reduced = global_reduce(original);
        perform_random_gates(original, 30, CliffordGate<5z>::all_level_0_with_swap(), Bv<2>(0b11));
        const auto result = global_reduce(original);
        CHECK_EQ(reduced, result);
    }
}
// NOLINTEND