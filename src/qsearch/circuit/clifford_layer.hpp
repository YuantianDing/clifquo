#pragma once

#include <boost/container_hash/hash.hpp>
#include "../../utils/list.hpp"
#include "./gate_layer.hpp"
namespace qsearch {

class CliffordCircInfo {
    struct Hash {
        [[nodiscard]] inline std::size_t operator()(const std::pair<GateLayer, gate::Gate> pair) const noexcept {
            std::size_t seed = std::hash<GateLayer>{}(pair.first);
            boost::hash_combine(seed, pair.second);
            return seed;
        }
    };
    explicit CliffordCircInfo(Bv<NQUBITS> mask, GateLayer first_layer) : mask(mask), first_layer(first_layer) {}

   public:
    const Bv<NQUBITS> mask;
    const GateLayer first_layer;
    using FirstLayerMap = std::unordered_map<std::pair<GateLayer, gate::Gate>, GateLayer, Hash>;

    inline static FirstLayerMap FIRST_LAYER_MAP;

    CliffordCircInfo() : mask(Bv<NQUBITS>::zero()), first_layer(GateLayer::nil()) {}

    [[nodiscard]] inline CliffordCircInfo plus(gate::Gate gate) const noexcept {
        if ((first_layer.info().mask & gate.qidx_mask()).none()) {
            if (auto it = FIRST_LAYER_MAP.find(std::make_pair(first_layer, gate)); it != FIRST_LAYER_MAP.end()) {
                return CliffordCircInfo(this->mask | gate.qidx_mask(), it->second);
            }
            return CliffordCircInfo(this->mask | gate.qidx_mask(), first_layer.plus(gate));
        }
        return CliffordCircInfo(this->mask | gate.qidx_mask(), first_layer);
    }
};

using CliffordLayer = Layer<const gate::Gate, CliffordCircInfo>;

CliffordLayer build_clifford_graph(const std::string& filename);

};  // namespace qsearch