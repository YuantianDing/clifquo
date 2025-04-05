#pragma once

#include <array>
#include <boost/container/static_vector.hpp>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include "../../defines.hpp"
#include "../../utils/bitvec.hpp"
#include "../../utils/decomposed.hpp"
#include "../../utils/fmt.hpp"
#include "../../utils/ranges.hpp"
#include "fmt/format.h"
#include "range/v3/view/any_view.hpp"
#include "range/v3/view/enumerate.hpp"

namespace circ {

class CircPerm {
    uint16_t perm;

   public:
    inline constexpr CircPerm() noexcept : perm(077777) {};
    inline constexpr explicit CircPerm(Bv<15ul> perm) noexcept : perm(perm.uint()) {};

    inline constexpr static CircPerm identity() noexcept { return CircPerm(Bv<15ul>(043210)); }
    template <typename Rng>
    inline constexpr static CircPerm from_inverse(Rng&& rng) noexcept {
        CircPerm result;
        for (auto [i, v] : rng | vw::enumerate) {
            result = result.update_permute(QIdx(v), QIdx(i));
        }
        return result;
    };
    template <typename Rng>
    inline constexpr static CircPerm from(Rng&& rng) noexcept {
        CircPerm result;
        for (auto [i, v] : rng | vw::enumerate) {
            result = result.update_permute(QIdx(i), QIdx(v));
        }
        return result;
    };

    [[nodiscard]] inline constexpr bool operator==(const CircPerm&) const noexcept = default;
    [[nodiscard]] inline constexpr bool operator!=(const CircPerm&) const noexcept = default;
    [[nodiscard]] inline constexpr bool operator<=>(const CircPerm&) const noexcept = default;

    [[nodiscard]] inline constexpr CircPerm inverse(QIdx N) const noexcept {
        CircPerm result;
        for (auto i = 0ul; i < N; i++) {
            result = result.update_permute((*this)[i], i);
        }
        return result;
    }

    [[nodiscard]] inline constexpr CircPerm swapped(QIdx a, QIdx b) const noexcept {
        return update_permute(a, (*this)[b]).update_permute(b, (*this)[a]);
    }
    template <typename SwapF>
    inline constexpr void emit_by_swap(QIdx N, SwapF&& swap) const noexcept {
        assert(N <= 5);
        auto perm = (*this);
        for (QIdx i = 0ul; i < N; i++) {
            for (QIdx j = i + 1; j < N; j++) {
                if (perm[i] > perm[j]) {
                    perm = perm.swapped(i, j);
                    swap(i, j);
                }
            }
        }
    }
    template <typename SwapF>
    inline constexpr void index_by_swap(QIdx N, SwapF&& swap) const noexcept {
        inverse(N).emit_by_swap(N, swap);
    }

    [[nodiscard]] inline constexpr Bv<15ul> vec() const noexcept { return Bv<15ul>(perm); }
    [[nodiscard]] inline constexpr QIdx operator[](QIdx qubit) const noexcept {
        auto result = Bv<3ul>::slice(vec(), std::size_t(qubit * 3)).uint();
        assert(result != 07);
        return QIdx(result);
    }
    [[nodiscard]] inline constexpr std::optional<QIdx> permute(QIdx qubit) const noexcept {
        auto result = Bv<3ul>::slice(vec(), std::size_t(qubit * 3)).uint();
        if (result == 07) { return std::nullopt; }
        return QIdx(result);
    }
    [[nodiscard]] inline constexpr CircPerm update_permute(QIdx from, QIdx to) const noexcept {
        return CircPerm(vec().update_slice(std::size_t(from) * 3, Bv<3ul>(to)));
    }

    template <std::size_t N>
    [[nodiscard]] inline constexpr static Bv<N> mapped_mask(CircPerm perm) noexcept {
        Bv<N> result;
        for (auto i = 0ul; i < N; i++) {
            if (auto q = perm.permute(QIdx(i))) { result = result.update(*q, true); }
        }
        return result;
    }
    [[nodiscard]] inline rgs::any_view<std::pair<QIdx, CircPerm>> generate_maps(QIdx qubit, QIdx qubit_limit) const noexcept {  // NOLINT
        assert(qubit_limit <= 5);
        auto mask = CircPerm::mapped_mask<5ul>(*this);
        if (auto mapped = permute(qubit)) {
            return vw::single(std::make_pair(*mapped, *this));
        } else {
            return vw::ints(0u, uint32_t(qubit_limit)) | vw::filter([mask](QIdx i) { return !mask[i]; }) |
                   vw::transform([qubit, self = *this](QIdx i) { return std::make_pair(QIdx(i), self.update_permute(qubit, i)); });
        }
    }

    template <std::size_t N, typename GateT>
    [[nodiscard]] inline rgs::any_view<std::pair<GateT, CircPerm>> generate_gate_maps(GateT gate, QIdx existing_gates) const noexcept {  // NOLINT
        auto bits = gate.bits();

        if (bits.size() == 1) {
            return generate_maps(bits[0], std::min(QIdx(N), QIdx(existing_gates + 1))) |
                   vw::transform(decomposed([gate](auto q1, auto perm) { return std::make_pair(gate.with_bits({q1}), perm); }));
        } else if (bits.size() == 2) {
            return generate_maps(bits[0], std::min(QIdx(N), QIdx(existing_gates + 1))) |
                   vw::for_each(decomposed([bits, existing_gates, gate](auto q1, auto perm) {
                       return perm.generate_maps(bits[1], std::min(QIdx(N), QIdx(existing_gates + (q1 == existing_gates ? 2 : 1)))) |
                              vw::transform(decomposed([q1, gate](auto q2, auto perm2) { return std::make_pair(gate.with_bits({q1, q2}), perm2); }));
                   }));
        } else {
            throw std::runtime_error("Not implemented");
        }
    }
};

[[nodiscard]] inline auto format_as(const CircPerm& perm) noexcept {
    auto iter = vw::ints(0, 5) | vw::for_each([&perm](auto i) -> rgs::any_view<std::string> {
                    if (auto q = perm.permute(i)) {
                        return vw::single(fmt::format("{}:{}", i, *q));
                    } else {
                        return vw::empty<std::string>;
                    }
                });
    return fmt::format("Perm[{}]", fmt::join(iter, ", "));
}

}  // namespace circ

// NOLINTBEGIN
TEST_FN(generate_gate_maps1) {
    auto perm = circ::CircPerm::identity().swapped(2, 1).swapped(1, 4).swapped(0, 3);
    fmt::println("{}", perm);
    CHECK_EQ(perm, circ::CircPerm::from(std::array{3, 4, 1, 0, 2}));
    auto perm2 = perm;
    perm.emit_by_swap(5, [&perm2](auto a, auto b) { perm2 = perm2.swapped(a, b); });
    CHECK_EQ(perm2, circ::CircPerm::identity());
    perm2 = perm;
    perm.inverse(5).index_by_swap(5, [&perm2](auto a, auto b) { perm2 = perm2.swapped(a, b); });
    CHECK_EQ(perm2, circ::CircPerm::identity());
}

// TEST_FN(generate_gate_maps2) {
//     circ::CircPerm perm;
//     perm = perm.update_permute(2, 1);

//     auto maps1 = perm.generate_gate_maps<5ul>(circ::QIdxVec{2}, 3) | rgs::to<std::vector>();
//     CHECK_EQ(maps1.size(), 1);

//     auto maps2 = perm.generate_gate_maps<5ul>(circ::QIdxVec{2, 0}, 3) | rgs::to<std::vector>();
//     CHECK_EQ(maps2.size(), 3);

//     auto maps3 = perm.generate_gate_maps<5ul>(circ::QIdxVec{0, 1}, 3) | rgs::to<std::vector>();
//     CHECK_EQ(maps3.size(), 7);
// }
// NOLINTEND