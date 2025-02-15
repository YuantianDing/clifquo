#pragma once

#include "../../utils/layer.hpp"
#include "../svsim/gate.hpp"

namespace qsearch {

class LayerMask {
    explicit LayerMask(Bv<NQUBITS> mask) : mask(mask) {}

   public:
    const Bv<NQUBITS> mask;
    LayerMask() : mask(Bv<NQUBITS>::zero()) {}

    [[nodiscard]] inline LayerMask plus(gate::Gate gate) const noexcept {
        assert((mask & gate.qidx_mask()).none());
        return LayerMask(mask | gate.qidx_mask());
    }
    [[nodiscard]] inline Bv<NQUBITS> qidx_mask() const noexcept { return mask; }
    [[nodiscard]] inline bool check(gate::Gate gate) const noexcept {
        return (qidx_mask() & gate.qidx_mask()).none() && gate.qindices()[0] > mask.firstl_one();
    }
};

using GateLayer = Layer<gate::Gate, LayerMask>;

// [[nodiscard]] inline bool check(gate::Gate gate) const noexcept {
//     return (info.qidx_mask() & gate.qidx_mask()).none() && gate.qindices()[0] > info.mask.firstl_one();
// }

GateLayer build_layer_graph(std::span<gate::Gate> gate);
}  // namespace qsearch
