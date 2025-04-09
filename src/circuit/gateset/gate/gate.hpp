#pragma once

#include "gate1.hpp"
#include "gate2.hpp"

namespace circ::gate {

// NOLINTNEXTLINE
#define IMPL_OP0(name)                                                                 \
    [[nodiscard]] inline auto operator<=>(const name& other) const noexcept = default; \
    [[nodiscard]] inline Gate0 operator()() const noexcept {                           \
        return Gate0(*this);                                                           \
    }

struct Gate0 {
    struct I {
        [[nodiscard]] inline std::string fmt() const noexcept { return fmt::format("X"); }  // NOLINT
        [[nodiscard]] inline auto operator<=>(const I& other) const noexcept = default;
        [[nodiscard]] inline Gate0 operator()() const noexcept { return Gate0(*this); };
    };
    using Variant = std::variant<I>;
    Variant gate;
    std::array<QIdx, 0> qubits = {};

    inline Gate0() : gate(I{}) {}
    inline explicit Gate0(const Variant& gate) : gate(gate) {}

    [[nodiscard]] inline std::string fmt() const noexcept {
        return std::visit([this](auto&& arg) { return fmt::format("{}", arg.fmt()); }, gate);
    }

    [[nodiscard]] inline constexpr bool operator==(const Gate0& other) const noexcept = default;

    [[nodiscard]] inline constexpr bool operator!=(const Gate0& other) const noexcept = default;

    [[nodiscard]] inline QIdxVec qbvec() const noexcept { return {}; }  // NOLINT
};

using Gate = std::variant<Gate0, Gate1, Gate2>;

}  // namespace circ::gate
