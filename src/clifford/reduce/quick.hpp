#pragma once

#include <algorithm>
#include <array>
#include "left.hpp"

namespace clifford {

template <const std::size_t N>
[[nodiscard]] inline constexpr BitSymplectic<N> quick_reduce(BitSymplectic<N> input) noexcept {
    input = left_reduce(input);

    std::array<Bv<N * 4>, N> rows;
    for (auto i = 0ul; i < N; i++) {
        rows[i] = input.get_row(i);
    }

    std::sort(rows.begin(), rows.end());

    return BitSymplectic<N>::from_qubit_array(rows);
}

}  // namespace clifford

// NOLINTBEGIN
TEST_FN(left_reduce) {
    const auto reduced = clifford::BitSymplectic<5ul>::identity();
    auto matrix = clifford::quick_reduce(reduced.phase_l(1ul).hadamard_l(1ul).hadamard_l(2ul).phase_l(2ul).hphaseh_l(3ul).phase_l(4ul));
    CHECK_EQ(reduced, matrix);
    for (auto i = 0ul; i < 5ul; i++) {
        auto original = clifford::BitSymplectic<5z>::identity();
        perform_random_gates(original, 30, clifford::CliffordGate<5z>::all_gates(), Bv<2>(0b11));
        const auto reduced = clifford::quick_reduce(original);
        perform_random_gates(original, 30, clifford::CliffordGate<5z>::all_level_0(), Bv<2>(0b01));
        perform_random_gates(original, 30, clifford::CliffordGate<5z>::all_swap(), Bv<2>(0b01));
        const auto result = clifford::quick_reduce(original);
        CHECK_EQ(reduced, result);
    }
}
// NOLINTEND