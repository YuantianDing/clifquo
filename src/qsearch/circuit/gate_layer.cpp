
#include "gate_layer.hpp"

namespace qsearch {

void build_layer_inner(GateLayer layer, std::span<gate::Gate> gates) {
    for (const auto gate : gates) {
        if (!layer.info().check(gate)) { continue; }
        auto next_layer = layer.plus(gate);
        layer.add_adjecents(next_layer);
        build_layer_inner(next_layer, gates);
    }
}

GateLayer build_layer_graph(std::span<gate::Gate> gates) {
    auto root = GateLayer::nil();
    build_layer_inner(root, gates);
    return root;
}

}  // namespace qsearch