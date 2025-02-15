#pragma once

#include <cstddef>
#include <cstdint>
#include <sstream>
#include <vector>
#include "../../utils/ranges.hpp"
#include "../svsim/gate.hpp"

namespace qsearch {

namespace gate {

class PauliGate : public GateOp {
    const Bv<2ul * NQUBITS> vec;

   public:
    explicit PauliGate(Bv<2ul * NQUBITS> vec) noexcept : vec(vec) {}
    inline constexpr void do_apply(QState state) const noexcept override {
        for (auto i = 0ul; i < NQUBITS; i++) {
            if (vec[i] && vec[i + NQUBITS]) {
                Y_GATE(state.real, state.imag, i);
            } else if (vec[i]) {
                X_GATE(state.real, state.imag, i);
            } else if (vec[i + NQUBITS]) {
                Z_GATE(state.real, state.imag, i);
            }
        }
    }
    [[nodiscard]] inline std::string gate_name() const noexcept override { return fmt(); }
    [[nodiscard]] inline std::string fmt() const noexcept override {
        std::stringstream ss;
        for (auto i = 0ul; i < NQUBITS; i++) {
            if (vec[i] && vec[i + NQUBITS]) {
                ss << "Y";
            } else if (vec[i]) {
                ss << "X";
            } else if (vec[i + NQUBITS]) {
                ss << "Z";
            }
        }
        return ss.str();
    }
    [[nodiscard]] inline GateQIdxArgs qindices() const noexcept override {
        GateQIdxArgs qidxs;
        for (auto i = 0ul; i < NQUBITS; i++) {
            if (vec[i] || vec[i + NQUBITS]) { qidxs.push_back(i); }
        }
        return qidxs;
    }
    [[nodiscard]] inline Bv<NQUBITS> qidx_mask() const noexcept override { return Bv<NQUBITS>::slice(vec, 0) | Bv<NQUBITS>::slice(vec, NQUBITS); }
};

}  // namespace gate

[[nodiscard]] inline auto pauli_decomposition(QState state) noexcept {
    return vw::ints(0ul, (1ul << (2ul * NQUBITS))) | vw::transform([state](auto i) {
               auto state2 = QState::clone(state);
               gate::PauliGate(Bv<2ul * NQUBITS>(i)).do_apply(state2);

               auto sum = QVal(0.0);
               for (size_t i = 0; i < DIM; i++) {
                   sum += state.real[i] * state2.real[i] + state.imag[i] * state2.imag[i];
               }
               state2.free();
               return sum;
           });
}

[[nodiscard]] std::size_t fingerprint(QState state) noexcept;
[[nodiscard]] std::vector<std::size_t> fingerprint_vec(QState state) noexcept;

}  // namespace qsearch
