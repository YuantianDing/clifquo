#pragma once

#include <cereal/archives/binary.hpp>
#include <cstdint>
#include <cstdlib>
#include "../circuit/gateset/clifford_generator.hpp"
#include "../circuit/gateset/permutation.hpp"
#include "../circuit/gateset/symmetry3.hpp"
#include "../defines.hpp"
#include "bitsymplectic.hpp"

namespace clfd {

template <const std::size_t N>
inline constexpr void do_symplectic_multiply_l(BitSymplectic<N>& input, circ::CliffordGenOp op, std::size_t i) noexcept {
    if (op == circ::CliffordGenOp::HP) {
        input.do_hadamard_l(i);
        input.do_phase_l(i);
    } else if (op == circ::CliffordGenOp::PH) {
        input.do_phase_l(i);
        input.do_hadamard_l(i);
    }
}
template <const std::size_t N>
inline constexpr void do_symplectic_multiply_r(BitSymplectic<N>& input, circ::CliffordGenOp op, std::size_t i) noexcept {
    if (op == circ::CliffordGenOp::HP) {
        input.do_hadamard_r(i);
        input.do_phase_r(i);
    } else if (op == circ::CliffordGenOp::PH) {
        input.do_phase_r(i);
        input.do_hadamard_r(i);
    }
}

template <const std::size_t N>
inline constexpr void do_symplectic_multiply_l(BitSymplectic<N>& input, circ::Symmetry3 op, std::size_t i) noexcept {
    if (op.bv()[0]) { input.do_hadamard_l(i); }
    if (op.bv()[1]) { input.do_phase_l(i); }
    if (op.bv()[2]) { input.do_hadamard_l(i); }
}
template <const std::size_t N>
inline constexpr void do_symplectic_multiply_r(BitSymplectic<N>& input, circ::Symmetry3 op, std::size_t i) noexcept {
    if (op.bv()[0]) { input.do_hadamard_r(i); }
    if (op.bv()[1]) { input.do_phase_r(i); }
    if (op.bv()[2]) { input.do_hadamard_r(i); }
}

template <const std::size_t N>
inline constexpr void do_symplectic_multiply_l(BitSymplectic<N>& input, circ::Symmetry3N<N> op) noexcept {
    for (std::size_t i = 0; i < N; i++) {
        do_symplectic_multiply_l(input, op[i], i);
    }
}
template <const std::size_t N>
inline constexpr void do_symplectic_multiply_r(BitSymplectic<N>& input, circ::Symmetry3N<N> op) noexcept {
    for (std::size_t i = 0; i < N; i++) {
        do_symplectic_multiply_r(input, op[i], i);
    }
}

template <const std::size_t N>
inline constexpr void do_symplectic_multiply_l(BitSymplectic<N>& input, circ::CircPerm op) noexcept {
    op.emit_by_swap(N, [&input](auto a, auto b) { input.do_swap_l(a, b); });
}
template <const std::size_t N>
inline constexpr void do_symplectic_multiply_r(BitSymplectic<N>& input, circ::CircPerm op) noexcept {
    op.emit_by_swap(N, [&input](auto a, auto b) { input.do_swap_r(a, b); });
}

template <const std::size_t N>
inline constexpr void do_symplectic_multiply_l(BitSymplectic<N>& /*mut*/ input, circ::CliffordGen<N> g) noexcept {
    assert(g.nonnull());
    input.do_mul_l(g.op_ctrl(), g.ictrl());
    input.do_mul_l(g.op_not(), g.inot());
    input.do_cnot_l(g.ictrl(), g.inot());
}
template <const std::size_t N>
inline constexpr void do_symplectic_multiply_r(BitSymplectic<N>& /*mut*/ input, circ::CliffordGen<N> g) noexcept {
    assert(g.nonnull());
    input.do_mul_r(g.op_ctrl(), g.ictrl());
    input.do_mul_r(g.op_not(), g.inot());
    input.do_cnot_r(g.ictrl(), g.inot());
}

template <const std::size_t N, typename Rng>
inline constexpr void do_symplectic_multiply_l(BitSymplectic<N>& /*mut*/ input, Rng&& g) noexcept {
    for (auto&& op : g) {
        do_symplectic_multiply_l(input, op);
    }
}
template <const std::size_t N, typename Rng>
inline constexpr void do_symplectic_multiply_r(BitSymplectic<N>& /*mut*/ input, Rng&& g) noexcept {
    for (auto&& op : g) {
        do_symplectic_multiply_r(input, op);
    }
}

}  // namespace clfd
