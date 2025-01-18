#pragma once

#include <array>
#include <atomic>
#include <cstdint>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <variant>
#include <vector>
#include "bitsymplectic.hpp"
#include "gates.hpp"
#include "range/v3/algorithm/max.hpp"
#include "reduce/left.hpp"
#include "search.hpp"

template <const std::size_t N>
struct LayerGraph {
    CliffordCirc<N> circ;
    std::vector<LayerGraph<N>* _Nonnull> adjacents;
    std::array<uint_fast8_t, N> used_times;
    explicit LayerGraph() : circ(), used_times() {};
    explicit LayerGraph(CliffordCirc<N> circ, const std::array<uint_fast8_t, N>& used_times) : circ(circ), used_times(used_times), adjacents() {}

    // [[nodiscard]] inline constexpr LayerGraph<N>* _Nullable new_adjacent(const CliffordGate<N>& gate, std::size_t depth_limit) {

    // }
};

template <const std::size_t N>
LayerGraph<N>* _Nonnull build_layer_graph(std::size_t depth, const std::vector<CliffordGate<N>>& gates) {
    auto start = new LayerGraph<N>();  // NOLINT(cppcoreguidelines-owning-memory)

    std::unordered_set<BitSymplectic<N>> found;
    std::queue<LayerGraph<N>* _Nonnull> bfs_boundry;

    bfs_boundry.push(start);
    found.insert(start->circ.tracker().matrix);
    while (!bfs_boundry.empty()) {
        auto current = bfs_boundry.front();
        bfs_boundry.pop();

        for (auto&& gate : gates) {
            auto arr = current->used_times;

            for (const auto qubit : gate.used_qubits()) {
                arr[qubit] += 1;  // NOLINT(cppcoreguidelines-pro-bounds-constant-array-index)
            }

            auto max = rgs::max(arr);
            if (max > depth) { continue; }

            auto new_circ = current->circ.append(gate);
            auto matrix = new_circ.tracker().matrix;
            if (found.insert(matrix).second) {
                auto new_graph = new LayerGraph<N>(new_circ, arr);  // NOLINT(cppcoreguidelines-owning-memory)
                current->adjacents.emplace_back(new_graph);
                bfs_boundry.push(new_graph);
            }
            if (found.size() % 5000 == 0) { fmt::println("Searching: {} {}", bfs_boundry.size(), found.size()); }
        }
    }
    return start;
};

template <const std::size_t N>
auto format_as(const LayerGraph<N>* graph) {
    if (!graph) { return std::string("null"); }
    if (graph->adjacents.size() > 0) {
        auto adjacents = graph->adjacents | vw::transform([](auto graph) { return format_as(graph); });
        return fmt::format("{} <- ({})", graph->circ ? graph->circ.head() : CliffordGate<N>::i(), fmt::join(adjacents, " | "));
    }
    return fmt::format("{}", graph->circ ? graph->circ.head() : CliffordGate<N>::i());
}

// NOLINTBEGIN
TEST_FN(build_layer_graph) {
    auto gates = CliffordGate<2>::all_gates();
    auto result = build_layer_graph<2>(2, gates);
    fmt::println("{}", format_as(result));
    // CHECK_EQ(result.size(), 4ul);
}
// NOLINTEND
