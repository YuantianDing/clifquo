#pragma once

#include <algorithm>
#include <cassert>
#include <cmath>
#include <compare>
#include <numbers>
#include <variant>
#include "../../../defines.hpp"
#include "../../../utils/class.hpp"
#include "../../../utils/fmt.hpp"
#include "boost/container/static_vector.hpp"

namespace circ::gate {
using QIdxVec = boost::container::static_vector<QIdx, 2>;

struct Gate2 {
    struct CX {
        [[nodiscard]] inline std::string fmt() const noexcept { return fmt::format("CX"); }  // NOLINT
    };
    struct CZ {
        [[nodiscard]] inline std::string fmt() const noexcept { return fmt::format("CZ"); }  // NOLINT
    };
    struct CY {
        [[nodiscard]] inline std::string fmt() const noexcept { return fmt::format("CY"); }  // NOLINT
    };
    struct CH {
        [[nodiscard]] inline std::string fmt() const noexcept { return fmt::format("CH"); }  // NOLINT
    };
    struct SWAP {
        [[nodiscard]] inline std::string fmt() const noexcept { return fmt::format("SWAP"); }  // NOLINT
    };

    using Variant = std::variant<CX, CZ, CY, CH, SWAP>;
    Variant gate;
    std::array<QIdx, 2> qubits;

    Gate2(const Variant& gate, QIdx qubit1, QIdx qubit2) : gate(gate), qubits{qubit1, qubit2} {}

    [[nodiscard]] inline std::string fmt() const noexcept {
        return std::visit([this](auto&& arg) { return fmt::format("{} {} {}", arg.fmt(), qubits[0], qubits[1]); }, gate);
    }

    [[nodiscard]] inline constexpr bool operator==(const Gate2& other) const noexcept = default;
    [[nodiscard]] inline constexpr bool operator!=(const Gate2& other) const noexcept = default;
    [[nodiscard]] inline QIdxVec qbvec() const noexcept { return {qubits[0], qubits[1]}; }
};

}  // namespace circ::gate
