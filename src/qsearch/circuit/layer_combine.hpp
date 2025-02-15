#pragma once

#include "../../utils/layer.hpp"
#include "../svsim/gate.hpp"
#include "clifford_layer.hpp"
#include "gate_layer.hpp"

namespace qsearch {

struct LayerCombine {
    const CliffordLayer clifford_layer;
    const GateLayer gate_layer;

    [[nodiscard]] inline constexpr bool operator==(const LayerCombine& other) const noexcept = default;
    [[nodiscard]] inline constexpr bool operator!=(const LayerCombine& other) const noexcept = default;

    [[nodiscard]] inline auto adjecents() const {
        return vw::concat(
            rng_maybe(!bool(gate_layer), clifford_layer.adjecents() | vw::transform([this](auto l) { return LayerCombine{l, gate_layer}; })),
            gate_layer.adjecents() | vw::filter([this](auto l) { return (clifford_layer.info().mask & l.inner().qidx_mask()).none(); }) |
                vw::transform([this](auto l) { return LayerCombine{clifford_layer, l}; })
        );
    }

    [[nodiscard]] inline constexpr explicit operator bool() const {
        if (gate_layer) { assert(clifford_layer); }
        return bool(gate_layer);
    }

    [[nodiscard]] inline LayerCombine prev() const {
        if (gate_layer) {
            return {clifford_layer, gate_layer.prev()};
        } else {
            return {clifford_layer.prev(), gate_layer};
        }
    }
    [[nodiscard]] inline constexpr gate::Gate const& inner() const {
        if (gate_layer) {
            return gate_layer.inner();
        } else {
            return clifford_layer.inner();
        }
    }
    [[nodiscard]] inline auto clifford_info() const noexcept { return clifford_layer.info(); }
    [[nodiscard]] inline auto gate_info() const noexcept { return gate_layer.info(); }

    [[nodiscard]] inline auto mask() const noexcept { return clifford_info().mask | gate_info().mask; }
    [[nodiscard]] inline auto passable(gate::Gate gate) const noexcept { return (mask() & gate.qidx_mask()).none(); }
};

// [[nodiscard]] inline bool check(gate::Gate gate) const noexcept {
//     return (info.qidx_mask() & gate.qidx_mask()).none() && gate.qindices()[0] > info.mask.firstl_one();
// }

inline auto format_as(const LayerCombine& layer) {
    return fmt::format("{}{}", layer.clifford_layer, layer.gate_layer);
}
}  // namespace qsearch