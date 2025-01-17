#pragma once

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
    constexpr CliffordMatrixTracker track(const CliffordGate<N>& gate) { return CliffordMatrixTracker(gate.apply_l(matrix), length + 1); }
};

template <const std::size_t N>
using CliffordCirc = List<CliffordGate<N>, CliffordMatrixTracker<N>>;

template <const std::size_t N>
[[nodiscard]] constexpr auto optimal_circuit_search(const std::vector<CliffordGate<N>>& gates) noexcept {
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

// NOLINTBEGIN
TEST_FN(optimal_circuit_search2) {
    auto gates = CliffordGate<2ul>::all_gates();
    auto result = optimal_circuit_search(gates);
    CHECK_EQ(result.size(), 4ul);
}

TEST_FN(optimal_circuit_search3) {
    auto gates = CliffordGate<3ul>::all_gates();
    auto result = optimal_circuit_search(gates);
    CHECK_EQ(result.size(), 27ul);
}
// NOLINTEND