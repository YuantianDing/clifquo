#pragma once

#include <algorithm>
#include <array>
#include "../../circuit/gateset/permutation.hpp"
#include "../../utils/math.hpp"
#include "../../utils/permutation_helper.hpp"
#include "fmt/base.h"
#include "left.hpp"
#include "range/v3/view/unique.hpp"

namespace clfd {

template <const std::size_t N>
[[nodiscard]] inline constexpr BitSymplectic<N> leftorder_reduce(BitSymplectic<N> input) noexcept {
    input = left_reduce(input);

    std::array<Bv<N * 4>, N> rows;
    for (auto i = 0ul; i < N; i++) {
        rows[i] = input.get_row(i);
    }

    std::sort(rows.begin(), rows.end());

    return BitSymplectic<N>::from_qubit_array(rows);
}

template <const std::size_t N>
[[nodiscard]] inline constexpr std::pair<circ::Symmetry3N<N>, circ::CircPerm>
leftorder_reduce_backtrack(BitSymplectic<N> base, BitSymplectic<N> target) noexcept {
    auto orig_base = base;
    auto orig_target = target;
    auto base_reduced = left_reduce(base);
    auto target_reduced = left_reduce(target);
    PermutationHelper base_perm(N, [&base, &base_reduced](auto a, auto b) {
        base_reduced.do_swap_l(a, b);
        base.do_swap_l(a, b);
    });
    PermutationHelper target_perm(N, [&target, &target_reduced](auto a, auto b) {
        target_reduced.do_swap_l(a, b);
        target.do_swap_l(a, b);
    });

    base_perm.sort([&base_reduced](auto a, auto b) { return base_reduced.get_row(a) < base_reduced.get_row(b); });
    target_perm.sort([&target_reduced](auto a, auto b) { return target_reduced.get_row(a) < target_reduced.get_row(b); });

    assert(base_reduced == target_reduced);
    assert(base_reduced == left_reduce(base));
    assert(target_reduced == left_reduce(target));
    circ::Symmetry3N<N> left_sym;
    circ::CircPerm left_perm;
    for (std::size_t i = 0; i < N; i++) {
        left_sym = left_sym.update(base_perm[i], left_reduce_row_backtrack(base, target, i));
        left_perm = left_perm.update_permute(base_perm[i], target_perm[i]);
    }
    // fmt::println("{} {} {} == {} != {}", left_perm, left_sym, orig_base, left_perm * (left_sym * orig_base), orig_target);
    assert(left_perm * (left_sym * orig_base) == orig_target);

    return {{left_sym}, left_perm};
}

}  // namespace clfd

// NOLINTBEGIN
TEST_FN(leftorder_reduce) {
    const auto reduced = clfd::BitSymplectic<5ul>::identity();
    auto matrix = clfd::leftorder_reduce(reduced.phase_l(1ul).hadamard_l(1ul).hadamard_l(2ul).phase_l(2ul).hphaseh_l(3ul).phase_l(4ul));
    CHECK_EQ(reduced, matrix);
    for (auto i = 0ul; i < 10ul; i++) {
        auto original = clfd::BitSymplectic<5z>::identity();
        perform_random_gates(original, 30, clfd::CliffordGate<5z>::all_gates(), Bv<2>(0b11));
        const auto reduced = clfd::leftorder_reduce(original);
        perform_random_gates(original, 30, clfd::CliffordGate<5z>::all_level_0(), Bv<2>(0b01));
        perform_random_gates(original, 30, clfd::CliffordGate<5z>::all_swap(), Bv<2>(0b01));
        const auto result = clfd::leftorder_reduce(original);
        CHECK_EQ(reduced, result);
    }
}
// NOLINTEND