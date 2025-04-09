#pragma once

#include <cassert>
#include <cmath>
#include <numbers>
#include <variant>
#include "../../../defines.hpp"
#include "../../../utils/fmt.hpp"
#include "boost/container/static_vector.hpp"

namespace circ::gate {

constexpr const QVal PI = std::numbers::pi_v<QVal>;
using QIdxVec = boost::container::static_vector<QIdx, 2>;

// NOLINTNEXTLINE
#define IMPL_OP1(name)                                                                 \
    [[nodiscard]] inline auto operator<=>(const name& other) const noexcept = default; \
    [[nodiscard]] inline Gate1 operator()(QIdx qb) const noexcept {                    \
        return Gate1(*this, qb);                                                       \
    }

struct Gate1 {
    struct X {
        [[nodiscard]] inline std::string fmt() const noexcept { return fmt::format("X"); }  // NOLINT
        IMPL_OP1(X)
    };
    struct Y {
        [[nodiscard]] inline std::string fmt() const noexcept { return fmt::format("Y"); }  // NOLINT
        IMPL_OP1(Y)
    };
    struct Z {
        [[nodiscard]] inline std::string fmt() const noexcept { return fmt::format("Z"); }  // NOLINT
        IMPL_OP1(Z)
    };
    struct H {
        [[nodiscard]] inline std::string fmt() const noexcept { return fmt::format("H"); }  // NOLINT
        IMPL_OP1(H)
    };
    struct SRN {
        [[nodiscard]] inline std::string fmt() const noexcept { return fmt::format("SRN"); }  // NOLINT
        IMPL_OP1(SRN)
    };
    struct R {
        QVal phase;
        [[nodiscard]] inline std::string fmt() const noexcept { return fmt::format("R({})", phase); }  // NOLINT
        IMPL_OP1(R)
    };
    struct S {
        [[nodiscard]] inline std::string fmt() const noexcept { return fmt::format("S"); }  // NOLINT
        IMPL_OP1(S)
    };
    struct SDG {
        [[nodiscard]] inline std::string fmt() const noexcept { return fmt::format("SDG)"); }  // NOLINT
        IMPL_OP1(SDG)
    };
    struct T {
        [[nodiscard]] inline std::string fmt() const noexcept { return fmt::format("T"); }  // NOLINT
        IMPL_OP1(T)
    };
    struct TDG {
        [[nodiscard]] inline std::string fmt() const noexcept { return fmt::format("TDG"); }  // NOLINT
        IMPL_OP1(TDG)
    };
    struct RX {
        QVal theta;
        [[nodiscard]] inline std::string fmt() const noexcept { return fmt::format("RX({}𝜋)", theta / PI); }
        IMPL_OP1(RX)
    };
    struct RY {
        QVal theta;
        [[nodiscard]] inline std::string fmt() const noexcept { return fmt::format("RY({}𝜋)", theta / PI); }
        IMPL_OP1(RY)
    };
    struct U1 {
        QVal lambda;
        [[nodiscard]] inline std::string fmt() const noexcept { return fmt::format("U1({}𝜋)", lambda / PI); }
        IMPL_OP1(U1)
    };
    struct U2 {
        QVal phi;
        QVal lambda;
        [[nodiscard]] inline std::string fmt() const noexcept { return fmt::format("U2({}𝜋, {}𝜋)", phi / PI, lambda / PI); }
        IMPL_OP1(U2)
    };
    struct U3 {
        QVal theta;
        QVal phi;
        QVal lambda;
        [[nodiscard]] inline std::string fmt() const noexcept { return fmt::format("U3({}𝜋, {}𝜋, {}𝜋)", theta / PI, phi / PI, lambda / PI); }
        IMPL_OP1(U3)
    };
    struct RZ {
        QVal phi;
        [[nodiscard]] inline std::string fmt() const noexcept { return fmt::format("RZ({}𝜋)", phi / PI); }
        IMPL_OP1(RZ)
    };
    using Variant = std::variant<X, Y, Z, H, SRN, R, S, SDG, T, TDG, RX, RY, U1, U2, U3, RZ>;
    Variant gate;
    std::array<QIdx, 1> qubits;

    Gate1(const Variant& gate, QIdx qubit) : gate(gate), qubits{qubit} {}

    [[nodiscard]] inline std::string fmt() const noexcept {
        return std::visit([this](auto&& arg) { return fmt::format("{} {}", arg.fmt(), qubits[0]); }, gate);
    }

    [[nodiscard]] inline constexpr bool operator==(const Gate1& other) const noexcept = default;

    [[nodiscard]] inline constexpr bool operator!=(const Gate1& other) const noexcept = default;

    [[nodiscard]] inline QIdxVec qbvec() const noexcept { return {qubits[0]}; }
};

}  // namespace circ::gate
