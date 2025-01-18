#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
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
    // auto first_bit = x00 && x01 && x10 && x11;
    // auto second_bit = (x00 && x01) != (x10 && x11);
    // return Bv<2>(uint64_t(second_bit) << 1ul | uint64_t(first_bit));
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
    auto minimum = BitSymplectic<N>::null();
    for (auto&& matrix : compute_permutation_set(input)) {
        auto&& reduced = local_reduce(matrix);
        if (minimum == BitSymplectic<N>::null() || reduced < minimum) { minimum = reduced; }
    }
    assert(minimum != BitSymplectic<N>::null());
    return minimum;
}

// NOLINTBEGIN
TEST_FN(global_reduce0) {
    std::array arr{Bv<6>(0b110101), Bv<6>(0b010000), Bv<6>(0b010001), Bv<6>(0b000100), Bv<6>(0b001010), Bv<6>(0b001100)};
    auto original = BitSymplectic<3>::fromArray(arr);
    const auto matrix = original.swap(1, 2);
    // const auto matrix = original.swap(2, 3).swap(1, 3).swap(2, 4).swap(0, 4).hadamard_l(2).phase_r(0);

    const auto reduced = global_reduce(original);
    const auto result = global_reduce(matrix);
    CHECK_EQ(reduced, result);
}
TEST_FN(global_reduce1) {
    for (auto i = 0ul; i < 100ul; i++) {
        auto original = BitSymplectic<3>::identity();
        perform_random_gates(original, 15, CliffordGate<3>::all_gates(), Bv<2>(0b11));
        auto matrix = original.swap(1, 2);
        // const auto matrix = original.swap(2, 3).swap(1, 3).swap(2, 4).swap(0, 4).hadamard_l(2).phase_r(0);

        const auto reduced = global_reduce(original);
        const auto result = global_reduce(matrix);
        CHECK_EQ(reduced, result);
    }
}
TEST_FN(global_reduce2) {
    for (auto i = 0ul; i < 100ul; i++) {
        auto original = BitSymplectic<5z>::identity();
        perform_random_gates(original, 15, CliffordGate<5z>::all_gates(), Bv<2>(0b11));
        const auto reduced = global_reduce(original);
        for (auto k = 0ul; k < 35; k++) {
            perform_random_gates(original, 2, CliffordGate<5z>::all_level_0(), Bv<2>(0b11));
            auto a = std::experimental::randint(0ul, 4ul);
            auto b = std::experimental::randint(0ul, 4ul);
            original.do_swap(a, (b == a) ? b : (b + 1) % 5);
        }
        const auto result = global_reduce(original);
        CHECK_EQ(reduced, result);
    }
}
// NOLINTEND