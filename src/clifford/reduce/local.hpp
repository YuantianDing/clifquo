#pragma once

#include <doctest/doctest.h>
#include <fmt/core.h>
#include <fmt/ranges.h>
#include <cassert>
#include <range/v3/algorithm/all_of.hpp>
#include <utility>
#include <vector>
#include "../bitsymplectic.hpp"
#include "../gates.hpp"
#include "left.hpp"
#include "range/v3/view/enumerate.hpp"
#include "range/v3/view/transform.hpp"

#pragma once

template <const std::size_t N>
inline constexpr Bv<N / 2> chi(Bv<N> input) {
    static_assert(N % 2 == 0);
    return Bv<N / 2>::slice(input, 0ul) | Bv<N / 2>::slice(input, N / 2);
}

template <const std::size_t N>
inline constexpr std::pair<BitSymplectic<N>, Bv<2>> right_chi_reduce_col(BitSymplectic<N> input, std::size_t icol) noexcept {
    auto x0 = input.xcol(icol);
    auto z0 = input.zcol(icol);
    auto x = chi(x0);
    auto z = chi(z0);
    auto y = chi(x0 ^ z0);

    if (x > z) {
        std::swap(/*mut*/ x, /*mut*/ z);
        input.do_hadamard_r(icol);
    }

    if (z > y) {
        std::swap(/*mut*/ z, /*mut*/ y);
        input.do_hphaseh_r(icol);
    }

    if (x > z) {
        std::swap(/*mut*/ x, /*mut*/ z);
        input.do_hadamard_r(icol);
    }

    x0 = input.xcol(icol);
    z0 = input.zcol(icol);
    assert(x == chi(x0));
    assert(y == chi(x0 ^ z0));
    assert(z == chi(z0));
    assert(x <= z && z <= y);

    auto eqs = Bv<2>(0ul);
    if (x == z) { eqs = eqs.update(0z, true); }
    if (z == y) { eqs = eqs.update(1z, true); }
    return std::make_pair(input, eqs);
}

template <const std::size_t N>
[[nodiscard]] inline constexpr std::vector<std::vector<CliffordGate<N>>>
compute_gate_sets(const std::vector<Bv<2>>& eps, const BitSymplectic<N>& matrix) {
    return eps | vw::enumerate | vw::transform(decomposed([&matrix](std::size_t i, Bv<2> eqs) {
               auto vec = std::vector<CliffordGate<N>>();
               if (eqs.uint() == 0b00) {
                   vec = {CliffordGate<N>::i()};
               } else if (eqs.uint() == 0b01) {
                   vec = {CliffordGate<N>::i(), CliffordGate<N>::h(i)};
               } else if (eqs.uint() == 0b10) {
                   vec = {CliffordGate<N>::i(), CliffordGate<N>::hph(i)};
               } else if (eqs.uint() == 0b11) {
                   vec = {
                       CliffordGate<N>::i(),    CliffordGate<N>::h(i),  CliffordGate<N>::p(i),
                       CliffordGate<N>::hph(i), CliffordGate<N>::ph(i), CliffordGate<N>::hp(i),
                   };
               } else {
                   __builtin_unreachable();
               }
               return vec | vw::filter([&matrix](CliffordGate<N> gate) {
                          return gate == CliffordGate<N>::i() || left_reduce(matrix) != left_reduce(gate.apply_r(matrix));
                      }) |
                      rgs::to<std::vector>();
           })) |
           rgs::to<std::vector>();
}

template <const std::size_t N, typename CallbackF>
inline constexpr bool local_reduced_iter_inner(
    BitSymplectic<N> input,
    std::size_t icol,
    const std::vector<std::vector<CliffordGate<N>>>& available_gates,
    CallbackF f
) noexcept {
    if (icol == N) { return f(left_reduce(input)); }

    return rgs::all_of(available_gates[icol], [input, icol, available_gates, f](CliffordGate<N> gate) {
        auto matrix = gate.apply_r(input);
        auto x = chi(matrix.xcol(icol));
        auto z = chi(matrix.zcol(icol));
        auto y = chi(matrix.xcol(icol) ^ matrix.zcol(icol));
        assert(x <= z && z <= y);
        return local_reduced_iter_inner(matrix, icol + 1, available_gates, f);
    });
}

template <const std::size_t N, typename CallbackF>
inline constexpr bool local_reduced_iter(BitSymplectic<N> input, CallbackF f) noexcept {
    auto&& eqs = vw::ints(0ul, N) | vw::transform([&input](auto i) {
                     auto [reduced, eq] = right_chi_reduce_col(input, i);
                     input = reduced;
                     return eq;
                 }) |
                 rgs::to<std::vector>();
    auto&& available_gates = compute_gate_sets(eqs, input);

    return local_reduced_iter_inner(input, 0, available_gates, f);
}

template <const std::size_t N>
inline constexpr BitSymplectic<N> local_reduce(BitSymplectic<N> input) noexcept {
    auto leftreduced = left_reduce(input);
    auto minimum = leftreduced;
    local_reduced_iter(input, [&minimum, leftreduced](BitSymplectic<N> matrix) {
        if (minimum == leftreduced || matrix < minimum) { minimum = matrix; }
        return true;
    });
    return minimum;
}

// NOLINTBEGIN
TEST_FN(local_reduce) {
    const auto reduced = local_reduce(BitSymplectic<5ul>::identity().cnot_l(2, 0).cnot_r(1, 2));

    const auto matrix1 = reduced.phase_l(1ul).hadamard_l(1ul).hadamard_l(2ul).phase_l(2ul).hphaseh_l(3ul).phase_l(4ul);
    CHECK_EQ(reduced, local_reduce(matrix1));

    const auto matrix2 = reduced.phase_r(0ul).hadamard_r(1ul).hadamard_r(2ul).phase_r(2ul).hphaseh_r(3ul).phase_r(4ul);
    CHECK_EQ(reduced, local_reduce(matrix2));

    const auto matrix3 = reduced.phase_r(1ul).hadamard_l(1ul).hadamard_r(2ul).phase_r(2ul).phase_l(2ul).hphaseh_r(3ul).phase_r(4ul);
    CHECK_EQ(reduced, local_reduce(matrix3));

    for (auto i = 0ul; i < 100ul; i++) {
        auto original = BitSymplectic<5z>::identity();
        perform_random_gates(original, 10, CliffordGate<5z>::all_gates(), Bv<2>(0b11));
        const auto reduced = local_reduce(original);
        perform_random_gates(original, 20, CliffordGate<5z>::all_level_0(), Bv<2>(0b11));
        const auto result = local_reduce(original);
        CHECK_EQ(reduced, result);
    }
}
// NOLINTEND