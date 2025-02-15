#pragma once

#include <boost/container/static_vector.hpp>
#include <boost/container_hash/hash.hpp>
#include <memory>
#include <nlohmann/json.hpp>
#include <variant>
#include "../../clifford/gates.hpp"
#include "../../utils/class.hpp"
#include "../../utils/ranges.hpp"
#include "config.hpp"
#include "fmt/core.h"
#include "svsim.hpp"

namespace qsearch::gate {

using GateQIdxArgs = boost::container::static_vector<QIdx, NQUBITS>;
using json = nlohmann::json;

struct GateOp {
    PURE_VIRTUAL(GateOp);

    virtual constexpr void do_apply(QState state) const noexcept = 0;
    [[nodiscard]] virtual std::string gate_name() const noexcept = 0;
    [[nodiscard]] virtual std::string fmt() const noexcept = 0;
    [[nodiscard]] virtual GateQIdxArgs qindices() const noexcept = 0;

    [[nodiscard]] virtual inline Bv<NQUBITS> qidx_mask() const noexcept {
        auto bv = Bv<NQUBITS>::zero();
        for (auto idx : qindices()) {
            bv = bv.update(idx, true);
        }
        return bv;
    }
};

struct Gate {
    const gate::GateOp* op;

    template <typename T>
    static constexpr bool is(Gate gate) noexcept {
        return bool(dynamic_cast<const T*>(gate.op));
    }
    [[nodiscard]] inline constexpr bool operator==(const Gate& other) const noexcept = default;
    [[nodiscard]] inline constexpr bool operator!=(const Gate& other) const noexcept = default;

    constexpr void do_apply(QState state) const noexcept { return op->do_apply(state); }
    [[nodiscard]] std::string fmt() const noexcept { return op->fmt(); }
    [[nodiscard]] GateQIdxArgs qindices() const noexcept { return op->qindices(); }
    [[nodiscard]] json to_json() const noexcept {
        json j = json::array({op->gate_name()});
        for (auto idx : qindices()) {
            j.push_back(idx);
        }
        return j;
    }

    [[nodiscard]] inline Bv<NQUBITS> qidx_mask() const noexcept { return op->qidx_mask(); }
};

struct GateOp1 {
    PURE_VIRTUAL(GateOp1);
    virtual constexpr void do_apply(QState state, QIdx qubit) const noexcept = 0;
    [[nodiscard]] virtual std::string fmt() const noexcept = 0;
};

template <typename GateT>
struct Gate1 : public GateOp {
    static_assert(std::is_base_of<GateOp1, GateT>::value, "Gate1: must provide a subclass of GateOp1");
    GateT gate;
    QIdx qubit;
    Gate1(GateT gate, QIdx qubit) : gate(gate), qubit(qubit) {}

    template <typename... Arg>
    static Gate make(Arg&&... arg) noexcept {
        return Gate(static_cast<const GateOp* _Nonnull>(new Gate1<GateT>(arg...)));
    }

    inline constexpr void do_apply(QState state) const noexcept override { gate.do_apply(state, qubit); }
    [[nodiscard]] inline std::string gate_name() const noexcept override { return gate.fmt(); }
    [[nodiscard]] inline std::string fmt() const noexcept override { return fmt::format("{}({})", gate.fmt(), qubit); }
    [[nodiscard]] inline GateQIdxArgs qindices() const noexcept override { return {qubit}; }

    [[nodiscard]] inline static constexpr auto all(GateT gate) noexcept {
        return vw::ints(QIdx(0), NQUBITS) | vw::transform([gate](auto i) { return Gate1::make(gate, i); });
    }
};

struct X : public GateOp1 {
    inline constexpr void do_apply(QState state, QIdx qubit) const noexcept override { X_GATE(state.real, state.imag, qubit); }
    [[nodiscard]] inline std::string fmt() const noexcept override { return fmt::format("X"); }
};
struct Y : public GateOp1 {
    inline constexpr void do_apply(QState state, QIdx qubit) const noexcept override { Y_GATE(state.real, state.imag, qubit); }
    [[nodiscard]] inline std::string fmt() const noexcept override { return fmt::format("Y"); }
};
struct Z : public GateOp1 {
    inline constexpr void do_apply(QState state, QIdx qubit) const noexcept override { Z_GATE(state.real, state.imag, qubit); }
    [[nodiscard]] inline std::string fmt() const noexcept override { return fmt::format("Z"); }
};
struct H : public GateOp1 {
    inline constexpr void do_apply(QState state, QIdx qubit) const noexcept override { H_GATE(state.real, state.imag, qubit); }
    [[nodiscard]] inline std::string fmt() const noexcept override { return fmt::format("H"); }
};
struct SRN : public GateOp1 {
    inline constexpr void do_apply(QState state, QIdx qubit) const noexcept override { SRN_GATE(state.real, state.imag, qubit); }
    [[nodiscard]] inline std::string fmt() const noexcept override { return fmt::format("SRN"); }
};
struct R : public GateOp1 {
    QVal phase;
    inline constexpr void do_apply(QState state, QIdx qubit) const noexcept override { R_GATE(state.real, state.imag, phase, qubit); }
    [[nodiscard]] inline std::string fmt() const noexcept override { return fmt::format("R({})", phase); }
};
struct S : public GateOp1 {
    inline constexpr void do_apply(QState state, QIdx qubit) const noexcept override { S_GATE(state.real, state.imag, qubit); }
    [[nodiscard]] inline std::string fmt() const noexcept override { return fmt::format("S"); }
};
struct SDG : public GateOp1 {
    inline constexpr void do_apply(QState state, QIdx qubit) const noexcept override { SDG_GATE(state.real, state.imag, qubit); }
    [[nodiscard]] inline std::string fmt() const noexcept override { return fmt::format("SDG)"); }
};
struct T : public GateOp1 {
    inline constexpr void do_apply(QState state, QIdx qubit) const noexcept override { T_GATE(state.real, state.imag, qubit); }
    [[nodiscard]] inline std::string fmt() const noexcept override { return fmt::format("T"); }
};
struct TDG : public GateOp1 {
    inline constexpr void do_apply(QState state, QIdx qubit) const noexcept override { TDG_GATE(state.real, state.imag, qubit); }
    [[nodiscard]] inline std::string fmt() const noexcept override { return fmt::format("TDG"); }
};
template <typename G1, typename G2>
struct Gate1Pair : public GateOp1 {
    G1 g1, g2;
    inline constexpr void do_apply(QState state, QIdx qubit) const noexcept override {
        g1.do_apply(state, qubit);
        g2.do_apply(state, qubit);
    }
    [[nodiscard]] inline std::string fmt() const noexcept override { return fmt::format("{}*{}", g1, g2); }
};
struct RX : public GateOp1 {
    QVal theta;
    inline constexpr void do_apply(QState state, QIdx qubit) const noexcept override { RX_GATE(state.real, state.imag, theta, qubit); }
    [[nodiscard]] inline std::string fmt() const noexcept override { return fmt::format("RX({}𝜋)", theta / PI); }
};
struct RY : public GateOp1 {
    QVal theta;
    inline constexpr void do_apply(QState state, QIdx qubit) const noexcept override { RY_GATE(state.real, state.imag, theta, qubit); }
    [[nodiscard]] inline std::string fmt() const noexcept override { return fmt::format("RY({}𝜋)", theta / PI); }
};
struct U1 : public GateOp1 {
    QVal lambda;
    inline constexpr void do_apply(QState state, QIdx qubit) const noexcept override { U1_GATE(state.real, state.imag, lambda, qubit); }
    [[nodiscard]] inline std::string fmt() const noexcept override { return fmt::format("U1({}𝜋)", lambda / PI); }
};
struct U2 : public GateOp1 {
    QVal phi;
    QVal lambda;
    inline constexpr void do_apply(QState state, QIdx qubit) const noexcept override { U2_GATE(state.real, state.imag, phi, lambda, qubit); }
    [[nodiscard]] inline std::string fmt() const noexcept override { return fmt::format("U2({}𝜋, {}𝜋)", phi / PI, lambda / PI); }
};
struct U3 : public GateOp1 {
    QVal theta;
    QVal phi;
    QVal lambda;
    inline constexpr void do_apply(QState state, QIdx qubit) const noexcept override { U3_GATE(state.real, state.imag, theta, phi, lambda, qubit); }
    [[nodiscard]] inline std::string fmt() const noexcept override { return fmt::format("U3({}𝜋, {}𝜋, {}𝜋)", theta / PI, phi / PI, lambda / PI); }
};
struct RZ : public GateOp1 {
    QVal phi;
    inline constexpr void do_apply(QState state, QIdx qubit) const noexcept override { RZ_GATE(state.real, state.imag, phi, qubit); }
    [[nodiscard]] inline std::string fmt() const noexcept override { return fmt::format("RZ({}𝜋)", phi / PI); }
};

struct GateOp2 {
    PURE_VIRTUAL(GateOp2);
    virtual constexpr void do_apply(QState state, QIdx qubit1, QIdx qubit2) const noexcept = 0;
    [[nodiscard]] virtual std::string fmt() const noexcept = 0;
};

template <typename GateT>
struct Gate2 : public GateOp {
    static_assert(std::is_base_of<GateOp2, GateT>::value, "Gate1: must provide a subclass of GateOp2");
    GateT gate;
    QIdx qubit1;
    QIdx qubit2;
    Gate2(GateT gate, QIdx qubit1, QIdx qubit2) : gate(gate), qubit1(qubit1), qubit2(qubit2) {}

    template <typename... Arg>
    static Gate make(Arg&&... arg) noexcept {
        return Gate(static_cast<const GateOp* _Nonnull>(new Gate2<GateT>(arg...)));
    }

    inline constexpr void do_apply(QState state) const noexcept override { gate.do_apply(state, qubit1, qubit2); }
    [[nodiscard]] inline std::string gate_name() const noexcept override { return gate.fmt(); }
    [[nodiscard]] inline std::string fmt() const noexcept override { return fmt::format("{}({}, {})", gate.fmt(), qubit1, qubit2); }
    [[nodiscard]] inline GateQIdxArgs qindices() const noexcept override { return {qubit1, qubit2}; }

    [[nodiscard]] inline static constexpr auto all(GateT gate) noexcept {
        return vw::cartesian_product(vw::ints(QIdx(0), QIdx(NQUBITS)), vw::ints(QIdx(0), QIdx(NQUBITS))) |
               vw::filter(decomposed([](auto a, auto b) { return a != b; })) |
               vw::transform(decomposed([gate](auto a, auto b) { return Gate2::make(gate, a, b); }));
    }
};

struct CX : public GateOp2 {
    inline constexpr void do_apply(QState state, QIdx a, QIdx b) const noexcept override { CX_GATE(state.real, state.imag, a, b); }
    [[nodiscard]] inline std::string fmt() const noexcept override { return fmt::format("CX"); }
};
struct CZ : public GateOp2 {
    inline constexpr void do_apply(QState state, QIdx a, QIdx b) const noexcept override { CZ_GATE(state.real, state.imag, a, b); }
    [[nodiscard]] inline std::string fmt() const noexcept override { return fmt::format("CZ"); }
};
struct CY : public GateOp2 {
    inline constexpr void do_apply(QState state, QIdx a, QIdx b) const noexcept override { CY_GATE(state.real, state.imag, a, b); }
    [[nodiscard]] inline std::string fmt() const noexcept override { return fmt::format("CY"); }
};
struct CH : public GateOp2 {
    inline constexpr void do_apply(QState state, QIdx a, QIdx b) const noexcept override { CH_GATE(state.real, state.imag, a, b); }
    [[nodiscard]] inline std::string fmt() const noexcept override { return fmt::format("CH"); }
};
struct SWAP : public GateOp2 {
    inline constexpr void do_apply(QState state, QIdx a, QIdx b) const noexcept override { SWAP_GATE(state.real, state.imag, a, b); }
    [[nodiscard]] inline std::string fmt() const noexcept override { return fmt::format("SWAP"); }
};

struct CliffordGenerator : public GateOp2 {
    clifford::CliffordGeneratorOp op_a, op_b;
    inline CliffordGenerator(clifford::CliffordGeneratorOp op_a, clifford::CliffordGeneratorOp op_b) noexcept : op_a(op_a), op_b(op_b) {}

    inline static constexpr void apply_clifford_generator_op(clifford::CliffordGeneratorOp op, QState state, QIdx qubit) {
        switch (op) {
            case clifford::CliffordGeneratorOp::I:
                break;
            case clifford::CliffordGeneratorOp::HP:
                H_GATE(state.real, state.imag, qubit);
                S_GATE(state.real, state.imag, qubit);
                break;
            case clifford::CliffordGeneratorOp::PH:
                S_GATE(state.real, state.imag, qubit);
                H_GATE(state.real, state.imag, qubit);
                break;
            default:
                __builtin_unreachable();
        }
    }
    inline constexpr void do_apply(QState state, QIdx a, QIdx b) const noexcept override {
        apply_clifford_generator_op(op_a, state, a);
        apply_clifford_generator_op(op_b, state, b);
        CX_GATE(state.real, state.imag, a, b);
    }
    [[nodiscard]] inline std::string fmt() const noexcept override { return fmt::format("{}{}-CX", format_as(op_a), format_as(op_b)); }
    inline static Gate2<CliffordGenerator> from_clifford_format(clifford::CliffordGenerator<NQUBITS> gen) {
        return {CliffordGenerator{gen.op_ctrl, gen.op_not}, QIdx(gen.ictrl), QIdx(gen.inot)};
    }
};

using CliffordGen2 = Gate2<CliffordGenerator>;

inline auto format_as(const Gate gate) {
    return gate.fmt();
}

// inline auto hash_value(const qsearch::gate::Gate& gate) {
//     return std::hash<const qsearch::gate::GateOp*>{}(gate.op);
// }

}  // namespace qsearch::gate

template <>
struct boost::hash<qsearch::gate::Gate> {
    std::size_t operator()(const qsearch::gate::Gate& gate) const noexcept { return std::hash<const qsearch::gate::GateOp*>{}(gate.op); }
};