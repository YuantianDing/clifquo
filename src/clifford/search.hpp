#pragma once

#include <cstddef>
#include <optional>
#include <unordered_set>
#include "../circuit/inittailtree.hpp"
#include "bitsymplectic.hpp"
#include "gate.hpp"
#include "reduce/quick.hpp"

namespace clifford {

template <std::size_t N>
[[nodiscard]] inline constexpr std::size_t optimal_clifford_search() {
    auto gates = CliffordGenerator<N>::all_generator();
    using Tree = circuit::InitTailTree<CliffordGenerator<N>>;
    auto tree = Tree::root(gates);

    std::unordered_set<BitSymplectic<N>> visited{BitSymplectic<N>::identity()};

    tree.extend(
        BitSymplectic<N>::identity(),
        [&visited](const BitSymplectic<N>& input, const CliffordGenerator<N>& gate) -> std::optional<BitSymplectic<N>> {
            const auto value = gate.apply_l(input);
            if (visited.size() % 10000 == 0) { fmt::println("Visited: {}", visited.size()); }
            if (visited.emplace(quick_reduce(value)).second) { return value; }
            return std::nullopt;
        },
        [](const CliffordGenerator<N>& /*init*/, const CliffordGenerator<N>& tail) { return vw::single(tail); }
    );
    return visited.size();
}

}  // namespace clifford