
#include "clifford_layer.hpp"
#include <algorithm>
#include <boost/container_hash/hash.hpp>
#include <unordered_map>
#include "../../clifford/search.hpp"
#include "permutation_helper.hpp"

namespace qsearch {
struct GeneratorHash {
    size_t operator()(const clifford::CliffordGenerator<NQUBITS>& obj) const {
        size_t seed = std::hash<size_t>{}(obj.ictrl);
        boost::hash_combine(seed, obj.inot);
        boost::hash_combine(seed, obj.op_ctrl);
        boost::hash_combine(seed, obj.op_not);
        return seed;
    }
};

struct GateTable {
    std::unordered_map<clifford::CliffordGenerator<NQUBITS>, gate::Gate, GeneratorHash> table;
    explicit GateTable() noexcept = default;

    [[nodiscard]] gate::Gate get(const clifford::CliffordGenerator<NQUBITS>& gate) const noexcept {
        if (auto it = table.find(gate); it != table.end()) {
            return it->second;
        } else {
            return gate::CliffordGen2::make(gate::CliffordGenerator::from_clifford_format(gate));
        }
    }
};

struct ConversionTable {
    std::unordered_map<clifford::BitSymplectic<NQUBITS>, CliffordLayer> table;
    explicit ConversionTable(CliffordLayer root) { table.emplace(clifford::BitSymplectic<NQUBITS>::identity(), root); }

    void add_edge(const clifford::GraphEdge<NQUBITS>& edge, GateTable& gate_table) {
        assert(table.contains(edge.prev_matrix));
        auto prev = table.at(edge.prev_matrix);
        auto cur = prev.plus(gate_table.get(edge.gate));
        prev.add_adjecents(cur);
        table.emplace(edge.cur_matrix, cur);
    }
};

CliffordLayer build_clifford_graph(const std::string& filename) {
    CliffordLayer root = CliffordLayer::nil();
    GateTable gate_table;
    ConversionTable conversion_table(root);

    fmt::println("Loading clifford {} gates", NQUBITS);

    auto counter = 0ul;
    clifford::load_clifford_table<NQUBITS>(filename, [&gate_table, &conversion_table, &counter, root](const auto& edge) mutable {
        auto edge_permed = edge;
        PermutationHelper<clifford::GraphEdge<NQUBITS>> perm(&edge_permed);
        do {  // NOLINT
            conversion_table.add_edge(edge_permed, gate_table);
        } while (std::next_permutation(perm.begin(), perm.end()));
        counter++;

        if (counter % 1000 == 0) { fmt::println("   Loading {} gates", counter); }
    });
    return root;
}
}  // namespace qsearch