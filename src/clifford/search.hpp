#pragma once

#include <cassert>
#include <cereal/archives/binary.hpp>
#include <cereal/types/memory.hpp>
#include <cereal/types/tuple.hpp>
#include <cereal/types/vector.hpp>
#include <cstdint>
#include <fstream>
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include "../utils/bfs.hpp"
#include "../utils/list.hpp"
#include "bitsymplectic.hpp"
#include "fmt/base.h"
#include "gates.hpp"
#include "range/v3/algorithm/none_of.hpp"
#include "range/v3/view/enumerate.hpp"
#include "reduce/global.hpp"

namespace clifford {

template <const std::size_t N>
struct CliffordMatrixTracker {
    BitSymplectic<N> matrix = BitSymplectic<N>::identity();
    std::size_t length = 0;
    constexpr CliffordMatrixTracker plus(const CliffordGenerator<N>& gate) { return CliffordMatrixTracker(gate.apply_l(matrix), length + 1); }
};

template <const std::size_t N>
using CliffordCirc = List<CliffordGenerator<N>, CliffordMatrixTracker<N>>;
template <const std::size_t N>
using CliffordTable = std::unordered_map<BitSymplectic<N>, List<CliffordGenerator<N>, CliffordMatrixTracker<N>>>;

template <const std::size_t N>
[[nodiscard]] constexpr auto optimal_circuit_search(const std::vector<CliffordGenerator<N>>& gates) noexcept {
    auto id = BitSymplectic<N>::identity();
    CliffordCirc<N> empty_circ;

    return bfs::search_entirely(
        std::make_pair(id, empty_circ),
        [gates](auto /*matrix*/, auto circ) {
            return gates | vw::transform([circ](auto gate) {
                       const auto newcirc = circ.plus(gate);
                       return std::make_pair(global_reduce(newcirc.info().matrix), newcirc);
                   });
        },
        [](auto circ1, auto circ2) {
            if (circ1.info().length <= circ2.info().length) {
                circ2.free();
                return circ1;
            } else {
                return circ2;
            }
        }
    );
}

template <const std::size_t N>
struct GraphEdge {
    static constexpr const std::size_t SIZE = N;
    BitSymplectic<N> cur_matrix;
    CliffordGenerator<N> gate;
    BitSymplectic<N> prev_matrix;

    template <typename Archive>
    void serialize(Archive& archive) {
        archive(cur_matrix, gate, prev_matrix);
    }

    constexpr void do_swap(std::size_t a, std::size_t b) {
        cur_matrix.do_swap(a, b);
        prev_matrix.do_swap(a, b);
        if (gate.ictrl == a) {
            gate.ictrl = b;
        } else if (gate.ictrl == b) {
            gate.ictrl = a;
        }
        if (gate.inot == a) {
            gate.inot = b;
        } else if (gate.inot == b) {
            gate.inot = a;
        }
    }
};

template <const std::size_t N>
inline constexpr void save_clifford_table(CliffordTable<N>& table, const std::string& filename) {
    fmt::println("Saving Clifford<{}>", N);

    std::vector<std::vector<GraphEdge<N>>> edges;
    for (const auto& [matrix, circ] : table) {
        if (circ.info().length == 0) { continue; }
        if (edges.size() <= circ.info().length) { edges.resize(circ.info().length + 1); }
        const auto m = global_reduce(circ.tail().info().matrix);
        assert(table[m] == circ.tail());
        edges[circ.info().length].emplace_back(matrix, circ.head(), m);
    }
    const auto size = rgs::accumulate(edges | vw::transform([](const auto& v) { return v.size(); }), 0ul);

    try {
        std::ofstream os(filename, std::ios::binary | std::ios::out);
        cereal::BinaryOutputArchive archive(os);
        archive(size);
        for (const auto& [i, vec] : edges | vw::enumerate) {
            fmt::println("    {} edges at {}", vec.size(), i);
            for (auto edge : vec) {
                archive(edge);
            }
        }
    } catch (const std::exception& e) { fmt::print("Saving File Error: {}\n", e.what()); }
}
template <const std::size_t N, typename ForeachF>
inline constexpr void load_clifford_table(const std::string& filename, ForeachF f) {
    std::ifstream is(filename, std::ios::binary);
    cereal::BinaryInputArchive archive(is);
    std::size_t size = 0;

    archive(size);
    for (std::size_t i = 0; i < size; i++) {
        GraphEdge<N> edge{BitSymplectic<N>::identity(), {}, BitSymplectic<N>::identity()};
        archive(edge);
        f(edge);
    }
}

template <std::size_t N>
inline auto format_as(const GraphEdge<N>& edge) {
    return fmt::format("{} --- {} ---> {}", edge.cur_matrix, edge.gate, edge.prev_matrix);
}
}  // namespace clifford

// NOLINTBEGIN
TEST_FN(optimal_circuit_search2) {
    auto gates = clifford::CliffordGenerator<2ul>::all_generator();
    auto result = optimal_circuit_search(gates);
    CHECK_EQ(result.size(), 4ul);
}

TEST_FN(optimal_circuit_search3) {
    auto gates = clifford::CliffordGenerator<3ul>::all_generator();
    auto result = optimal_circuit_search(gates);
    CHECK_EQ(result.size(), 27ul);
}

// TEST_FN(optimal_circuit_search4) {
//     auto gates = CliffordGenerator<4ul>::all_generator();
//     auto result = optimal_circuit_search(gates);
//     CHECK_EQ(result.size(), 2363ul);
// }
// NOLINTEND