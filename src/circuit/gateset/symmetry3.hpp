#pragma once

#include <sys/types.h>
#include <array>
#include <cereal/archives/binary.hpp>
#include <cstdint>
#include "../../utils/bitvec.hpp"
#include "../../utils/fmt.hpp"
#include "../../utils/ranges.hpp"
#include "range/v3/view/transform.hpp"

namespace circ {

struct Symmetry3 {
    uint8_t data;
    inline constexpr Symmetry3() noexcept : data(0) {}
    inline constexpr Symmetry3(uint8_t data) noexcept : data(data) {}         // NOLINT
    inline constexpr Symmetry3(Bv<3ul> data) noexcept : data(data.uint()) {}  // NOLINT

    [[nodiscard]] inline constexpr bool operator<=>(const Symmetry3&) const noexcept = default;
    [[nodiscard]] inline constexpr bool operator==(const Symmetry3&) const noexcept = default;
    [[nodiscard]] inline constexpr bool operator!=(const Symmetry3&) const noexcept = default;

    [[nodiscard]] inline constexpr Bv<3ul> bv() const noexcept { return Bv<3ul>{data}; }

    [[nodiscard]] inline constexpr static Symmetry3 i() { return {0b000}; }
    [[nodiscard]] inline constexpr static Symmetry3 h() { return {0b001}; }
    [[nodiscard]] inline constexpr static Symmetry3 p() { return {0b010}; }
    [[nodiscard]] inline constexpr static Symmetry3 hph() { return {0b111}; }
    [[nodiscard]] inline constexpr static Symmetry3 hp() { return {0b011}; }
    [[nodiscard]] inline constexpr static Symmetry3 ph() { return {0b110}; }
    [[nodiscard]] inline constexpr static std::array<Symmetry3, 6> all() {
        return std::array<Symmetry3, 6>{Symmetry3(0b000), Symmetry3(0b001), Symmetry3(0b010), Symmetry3(0b111), Symmetry3(0b110), Symmetry3(0b011)};
    }
};

inline auto format_as(const Symmetry3& gate) {
    if (gate == Symmetry3::i()) { return "I"; }
    if (gate == Symmetry3::h()) { return "H"; }
    if (gate == Symmetry3::p()) { return "P"; }
    if (gate == Symmetry3::hph()) { return "HPH"; }
    if (gate == Symmetry3::hp()) { return "HP"; }
    if (gate == Symmetry3::ph()) { return "PH"; }
    __builtin_unreachable();
}

template <std::size_t N>
struct Symmetry3N {
    static_assert(N <= 5);
    uint16_t data;

    inline constexpr Symmetry3N() noexcept : data(0) {}                            // NOLINT
    inline constexpr Symmetry3N(uint16_t data) noexcept : data(data) {}            // NOLINT
    inline constexpr Symmetry3N(Bv<N * 3ul> data) noexcept : data(data.uint()) {}  // NOLINT
    [[nodiscard]] inline constexpr Bv<N * 3ul> bv() const noexcept { return Bv<N * 3ul>(data); }

    [[nodiscard]] inline constexpr Symmetry3 operator[](std::size_t i) const noexcept { return uint8_t(Bv<3ul>::slice(bv(), i * 3).uint()); }
    [[nodiscard]] inline constexpr Symmetry3N update(std::size_t i, Symmetry3 g) const noexcept { return bv().update_slice(i * 3, g.bv()); }
};

template <const std::size_t N>
auto format_as(const Symmetry3N<N>& gate) {
    return fmt::format("[{}]", fmt::join(vw::ints(0ul, N) | vw::transform([&gate](auto i) { return fmt::format("{}:{}", i, gate[i]); }), " "));
}

};  // namespace circ