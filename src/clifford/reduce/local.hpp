#pragma once

#include <doctest/doctest.h>
#include <fmt/core.h>
#include <fmt/ranges.h>
#include <array>
#include <boost/container/small_vector.hpp>
#include <cassert>
#include <range/v3/algorithm/all_of.hpp>
#include <utility>
#include "../bitsymplectic.hpp"
#include "gates.hpp"
#include "left.hpp"
#include "range/v3/algorithm/generate.hpp"
#include "range/v3/view/enumerate.hpp"
#include "range/v3/view/transform.hpp"

namespace clifford {

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
using LocalReduceGateSet = std::array<boost::container::small_vector<CliffordGate<N>, 2>, N>;

template <const std::size_t N>
[[nodiscard]] inline constexpr bool check_gate(CliffordGate<N> gate, const BitSymplectic<N>& matrix) {
    return gate == CliffordGate<N>::i() || left_reduce(matrix) != left_reduce(gate.apply_r(matrix));
}
template <const std::size_t N>
inline constexpr void compute_gate_sets(LocalReduceGateSet<N>& result, const std::vector<Bv<2>>& eps, const BitSymplectic<N>& matrix) {
    for (auto&& [i, eqs] : eps | vw::enumerate) {
        // Distribution: [20031833, 1705279, 0, 0, 0, 189598]
        assert(result[i].size() == 0);
        result[i].push_back(CliffordGate<N>::i());
        if (eqs[0] && check_gate(CliffordGate<N>::h(i), matrix)) { result[i].push_back(CliffordGate<N>::h(i)); }
        if (eqs[1] && check_gate(CliffordGate<N>::hph(i), matrix)) { result[i].push_back(CliffordGate<N>::hph(i)); }
        if (eqs.uint() == 0b11) {
            result[i].reserve(6);
            if (check_gate(CliffordGate<N>::p(i), matrix)) { result[i].push_back(CliffordGate<N>::p(i)); }
            if (check_gate(CliffordGate<N>::ph(i), matrix)) { result[i].push_back(CliffordGate<N>::ph(i)); }
            if (check_gate(CliffordGate<N>::hp(i), matrix)) { result[i].push_back(CliffordGate<N>::hp(i)); }
        }
    }
}

template <const std::size_t N, typename CallbackF>
inline constexpr bool
local_reduced_iter_inner(BitSymplectic<N> input, std::size_t icol, const LocalReduceGateSet<N>& available_gates, CallbackF f) noexcept {
    if (icol == N) { return f(left_reduce(input)); }

    return rgs::all_of(available_gates[icol], [input, icol, available_gates, f](CliffordGate<N> gate) {  // NOLINT
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
    // fmt::println("original: {} eqs: {}", input, eqs);
    LocalReduceGateSet<N> available_gates;
    compute_gate_sets(available_gates, eqs, input);
    // fmt::println("available gates {}", available_gates);
    return local_reduced_iter_inner(input, 0, available_gates, f);
}

template <const std::size_t N>
inline constexpr BitSymplectic<N> local_reduce(BitSymplectic<N> input) noexcept {
    auto minimum = BitSymplectic<N>::null();
    local_reduced_iter(input, [&minimum](BitSymplectic<N> matrix) {
        if (minimum == BitSymplectic<N>::null() || matrix < minimum) { minimum = matrix; }
        return true;
    });
    assert(minimum != BitSymplectic<N>::null());
    return minimum;
}
}  // namespace clifford

// NOLINTBEGIN
TEST_FN(local_reduce0) {
    std::array arr{Bv<6>(0b110001), Bv<6>(0b010111), Bv<6>(0b001010), Bv<6>(0b001110), Bv<6>(0b000110), Bv<6>(0b111010)};
    auto original = clifford::BitSymplectic<3>::from_array(arr);
    auto matrix = original.hphaseh_r(0).hphaseh_l(2);
    CHECK_NE(left_reduce(original.hphaseh_r(0)), left_reduce(original));

    const auto reduced = clifford::local_reduce(original);
    const auto result = clifford::local_reduce(matrix);
    CHECK_EQ(reduced, result);
}
TEST_FN(local_reduce) {
    const auto reduced = clifford::local_reduce(clifford::BitSymplectic<5ul>::identity().cnot_l(2, 0).cnot_r(1, 2));

    const auto matrix1 = reduced.phase_l(1ul).hadamard_l(1ul).hadamard_l(2ul).phase_l(2ul).hphaseh_l(3ul).phase_l(4ul);
    CHECK_EQ(reduced, clifford::local_reduce(matrix1));

    const auto matrix2 = reduced.phase_r(0ul).hadamard_r(1ul).hadamard_r(2ul).phase_r(2ul).hphaseh_r(3ul).phase_r(4ul);
    CHECK_EQ(reduced, clifford::local_reduce(matrix2));

    const auto matrix3 = reduced.phase_r(1ul).hadamard_l(1ul).hadamard_r(2ul).phase_r(2ul).phase_l(2ul).hphaseh_r(3ul).phase_r(4ul);
    CHECK_EQ(reduced, clifford::local_reduce(matrix3));

    for (auto i = 0ul; i < 1000ul; i++) {
        auto original = clifford::BitSymplectic<5z>::identity();
        perform_random_gates(original, 10, clifford::CliffordGate<5z>::all_gates(), Bv<2>(0b11));
        const auto reduced = clifford::local_reduce(original);
        perform_random_gates(original, 20, clifford::CliffordGate<5z>::all_level_0(), Bv<2>(0b11));
        const auto result = clifford::local_reduce(original);
        CHECK_EQ(reduced, result);
    }
}
// NOLINTEND