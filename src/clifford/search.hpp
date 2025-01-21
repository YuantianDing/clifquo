#pragma once

#include <cereal/archives/binary.hpp>
#include <cereal/types/memory.hpp>
#include <cereal/types/tuple.hpp>
#include <cereal/types/vector.hpp>
#include <cstdint>
#include <fstream>
#include <queue>
#include <unordered_set>
#include <vector>
#include "../utils/bfs.hpp"
#include "../utils/list.hpp"
#include "bitsymplectic.hpp"
#include "fmt/base.h"
#include "gates.hpp"
#include "range/v3/algorithm/none_of.hpp"
#include "reduce/global.hpp"

template <const std::size_t N>
struct CliffordMatrixTracker {
    BitSymplectic<N> matrix = BitSymplectic<N>::identity();
    std::size_t length = 0;
    constexpr CliffordMatrixTracker track(const CliffordGenerator<N>& gate) { return CliffordMatrixTracker(gate.apply_l(matrix), length + 1); }
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
                       const auto newcirc = circ.append(gate);
                       return std::make_pair(global_reduce(newcirc.tracker().matrix), newcirc);
                   });
        },
        [](auto circ1, auto circ2) {
            if (circ1.tracker().length <= circ2.tracker().length) {
                circ2.free();
                return circ1;
            } else {
                return circ2;
            }
        }
    );
}

template <const std::size_t N>
inline constexpr void save_clifford_table(const CliffordTable<N>& table, const std::string& filename) {
    try {
        std::ofstream os(filename, std::ios::binary | std::ios::trunc);
        cereal::BinaryOutputArchive archive(os);
        archive(table.size());
        for (auto [matrix, circ] : table) {
            archive(matrix);
            archive(circ.to_vec());
        }
    } catch (const std::exception& e) { fmt::print("Saving File Error: {}\n", e.what()); }
}
template <const std::size_t N>
inline constexpr std::unordered_map<BitSymplectic<N>, std::vector<CliffordGenerator<N>>> load_clifford_table(const std::string& filename) {
    std::unordered_map<BitSymplectic<N>, std::vector<CliffordGenerator<N>>> table;
    std::ifstream is(filename, std::ios::binary);
    cereal::BinaryInputArchive archive(is);
    std::size_t size = 0;

    archive(size);
    for (std::size_t i = 0; i < size; i++) {
        auto matrix = BitSymplectic<N>::null();
        std::vector<CliffordGenerator<N>> circ;
        archive(matrix);
        archive(circ);
        table[matrix] = circ;
    }
    return table;
}

// NOLINTBEGIN
TEST_FN(optimal_circuit_search2) {
    auto gates = CliffordGenerator<2ul>::all_generator();
    auto result = optimal_circuit_search(gates);
    CHECK_EQ(result.size(), 4ul);
}

TEST_FN(optimal_circuit_search3) {
    auto gates = CliffordGenerator<3ul>::all_generator();
    auto result = optimal_circuit_search(gates);
    CHECK_EQ(result.size(), 27ul);
}

// TEST_FN(optimal_circuit_search4) {
//     auto gates = CliffordGenerator<4ul>::all_generator();
//     auto result = optimal_circuit_search(gates);
//     CHECK_EQ(result.size(), 2363ul);
// }
// NOLINTEND