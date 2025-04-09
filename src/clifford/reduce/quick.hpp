#pragma once

#include <algorithm>
#include <cstddef>
#include <utility>
#include "../gate.hpp"
#include "boost/container/static_vector.hpp"
#include "leftorder.hpp"
#include "range/v3/algorithm/all_of.hpp"

namespace clfd {

template <typename EqPairT, typename EqF>
void collect_eq_pair(EqPairT& eq_pairs, std::size_t N, EqF&& eqf) {
    std::size_t last_eq = 0;
    for (auto i = 1ul; i < N; i++) {
        if (!eqf(i, last_eq)) {
            if (i - last_eq > 1) { eq_pairs.emplace_back(last_eq, i); }
            last_eq = i;
        }
    }
    if (N - last_eq > 1) { eq_pairs.emplace_back(last_eq, N); }
}

template <std::size_t N>
[[nodiscard]] inline constexpr BitSymplectic<N> quick_reduce(BitSymplectic<N> input) noexcept {
    std::array<int, N> metrics;
    for (auto i = 0ul; i < N; i++) {
        metrics[i] = input.col_metric(i);
    }

    PermutationHelper perm(N, [&input, &metrics](auto a, auto b) {
        input.do_swap_r(a, b);
        std::swap(metrics[a], metrics[b]);
    });

    perm.sort([&metrics](auto a, auto b) { return metrics[a] < metrics[b]; });
    // fmt::println("sorted: {}", perm);
    perm.reset();
    for (auto i = 0ul; i < N - 1; i++) {
        assert(metrics[i] <= metrics[i + 1]);
    }
    boost::container::static_vector<std::pair<std::size_t, std::size_t>, 3> eq_pairs;
    collect_eq_pair(eq_pairs, N, [&metrics](auto a, auto b) { return metrics[a] == metrics[b]; });

    auto reduced = leftorder_reduce(input);
    while (rgs::any_of(eq_pairs, decomposed([&perm](auto a, auto b) { return std::next_permutation(perm.iter_at(a), perm.iter_at(b)); }))) {
        // fmt::println("Reducing: {} {} {}", eq_pairs, metrics, perm);
        auto matrix = leftorder_reduce(input);
        for (auto i = 0ul; i < N; i++) {
            assert(metrics[i] == matrix.col_metric(i));
        }
        if (matrix < reduced) { reduced = matrix; }
    }
    return leftorder_reduce(reduced);
}

template <std::size_t N>
struct QuickReduceBacktrack {
    circ::CircPerm left_perm;
    circ::Symmetry3N<N> left_sym;
    circ::CircPerm right_perm;
};

template <std::size_t N>
[[nodiscard]] inline constexpr QuickReduceBacktrack<N> quick_reduce_backtrack(BitSymplectic<N> base, BitSymplectic<N> target) noexcept {
    auto orig_base = base;
    auto orig_target = target;
    PermutationHelper perm(N, [&base](auto a, auto b) { base.do_swap_r(a, b); });

    do {
        if (leftorder_reduce(base) == leftorder_reduce(target)) { break; }
    } while (std::next_permutation(perm.begin(), perm.end()));

    auto [left_sym, left_perm] = leftorder_reduce_backtrack(base, target);
    auto right_perm = circ::CircPerm::from_inverse(perm.to_vec());
    if ((left_perm * (left_sym * orig_base)) * right_perm != target) {
        auto a = (left_perm * (left_sym * orig_base));
        fmt::println("{} {} == {} != {}", a, right_perm, a * right_perm, orig_target);
    }
    assert(left_perm * ((circ::Symmetry3N<N>(left_sym) * orig_base) * right_perm) == target);
    return {left_perm, left_sym, right_perm};
}

template <std::size_t N>
[[nodiscard]] inline constexpr std::size_t quick_reduce_eqcount(BitSymplectic<N> input) noexcept {
    auto matrix = input;
    std::size_t aut = 1;
    PermutationHelper perm(N, [&matrix](auto a, auto b) { matrix.do_swap_r(a, b); });
    while (std::next_permutation(perm.begin(), perm.end())) {
        if (leftorder_reduce(matrix) == leftorder_reduce(input)) { aut += 1; }
    }
    return utils::factorial(N) * utils::factorial(N) * utils::power(6, N) / aut;
}

inline std::size_t symplectic_matrix_count(std::size_t n) {
    std::size_t result = 1ul << (n * n);  // (1 << n**2)

    for (std::size_t i = 2; i <= 2 * n; i += 2) {
        result *= (1ul << i) - 1;  // (1 << i) - 1
    }

    return result;
}

}  // namespace clfd

// NOLINTBEGIN
TEST_FN(collect_eq_pair) {
    std::vector<std::pair<std::size_t, std::size_t>> eq_pairs;
    clfd::collect_eq_pair(eq_pairs, 10, [](auto a, auto b) { return a / 2 == b / 2; });
    CHECK_EQ(eq_pairs.size(), 5);
    CHECK_EQ(eq_pairs[0], std::pair{0ul, 2ul});
    CHECK_EQ(eq_pairs[1], std::pair{2ul, 4ul});
    CHECK_EQ(eq_pairs[2], std::pair{4ul, 6ul});
    CHECK_EQ(eq_pairs[3], std::pair{6ul, 8ul});
    CHECK_EQ(eq_pairs[4], std::pair{8ul, 10ul});
}

TEST_FN(quick_reduce) {
    const auto reduced = clfd::BitSymplectic<5ul>::identity();
    auto matrix = clfd::quick_reduce(reduced.phase_l(1ul).hadamard_l(1ul).hadamard_l(2ul).phase_l(2ul).hphaseh_l(3ul).phase_l(4ul));
    CHECK_EQ(reduced, matrix);
    for (auto i = 0ul; i < 10000ul; i++) {
        auto original = clfd::BitSymplectic<5z>::identity();
        perform_random_gates(original, 30, clfd::CliffordGate<5z>::all_gates(), Bv<2>(0b11));
        const auto reduced = clfd::quick_reduce(original);
        // perform_random_gates(original, 30, clfd::CliffordGate<5z>::all_level_0(), Bv<2>(0b01));
        perform_random_gates(original, 30, clfd::CliffordGate<5z>::all_swap(), Bv<2>(0b11));
        const auto result = clfd::quick_reduce(original);
        CHECK_EQ(reduced, result);
    }
}

TEST_FN(quick_reduce_clif_gen) {
    using Gen = circ::CliffordGen<4>;
    auto m0 = clfd::BitSymplectic<4>::identity();
    m0 = Gen::phhp_cx(0, 1) * m0;
    m0 = Gen::phhp_cx(2, 3) * m0;
    m0 = Gen::ihp_cx(2, 0) * m0;
    fmt::println("{}", m0.as_raw());

    auto m1 = clfd::BitSymplectic<4>::identity();
    m1 = Gen::phhp_cx(0, 1) * m1;
    m1 = Gen::phhp_cx(2, 3) * m1;
    m1 = Gen::hphp_cx(3, 0) * m1;
    fmt::println("{}", m1.as_raw());

    CHECK_EQ(quick_reduce(m0), quick_reduce(m1));
}

TEST_FN(symplectic_matrix_count) {
    CHECK_EQ(clfd::symplectic_matrix_count(2), 720);
}
// NOLINTEND