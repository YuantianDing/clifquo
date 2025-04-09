#pragma once

#include <algorithm>
#include <cstddef>
#include <vector>
#include "../circuit/gateset/clifford_generator.hpp"
#include "../circuit/tree/newcirc.hpp"
#include "../table/bsearch_vec.hpp"
#include "../utils/list.hpp"
#include "../utils/ranges.hpp"
#include "./bitsymplectic.hpp"
#include "./gate.hpp"
#include "reduce/quick.hpp"

template <std::size_t N>
using CliffordCirc = List<circ::CliffordGen<N>>;

namespace clfd::search {

template <std::size_t N>
struct Point {
    CliffordCirc<N> circ;
    BitSymplectic<N> value;
};

template <std::size_t N>
circ::tree::Tree search(bool verbose = false) {  // NOLINT
    auto all_gen = circ::CliffordGen<N>::all_generator();
    auto tree = circ::tree::Tree::from(vw::ints(0ul, all_gen.size()));
    auto last2_layer = std::vector<BitSymplectic<N>>();
    auto last_layer = std::vector<BitSymplectic<N>>();
    auto symplectic_count = 0ul;
    auto percentage = 0ul;
    auto symplectic_count_total = symplectic_matrix_count(N);

    for (auto size = 2;; size++) {
        table::BSearchVec<clfd::BitSymplectic<N>> bsvec;
        circ::tree::GroupedSpanBuilder builder;

        for (auto node : tree) {
            builder.new_span();
            auto result = BitSymplectic<N>::identity();
            for (auto g : node) {
                result = all_gen[std::size_t(*g)] * result;
            }
            for (auto g : vw::ints(0ul, all_gen.size())) {
                auto reduced_result = quick_reduce(all_gen[g] * result);
                if (std::binary_search(last_layer.begin(), last_layer.end(), reduced_result)) { continue; }
                if (std::binary_search(last2_layer.begin(), last2_layer.end(), reduced_result)) { continue; }
                if (bsvec.contains(reduced_result)) { continue; }
                builder.add(std::byte(g));
                bsvec.insert(reduced_result);
                symplectic_count += quick_reduce_eqcount(reduced_result);
                auto p = symplectic_count * 100 / symplectic_count_total;
                if (p != percentage && verbose) {
                    percentage = p;
                    fmt::println("Searching Symplectic<{}> ({}%): size{} {}/{} ", N, percentage, size, symplectic_count, symplectic_count_total);
                }
            }
        }

        last2_layer = std::move(last_layer);
        last_layer = std::move(bsvec.build_sorted());
        tree.add_layer(std::move(builder.build()));

        if (last_layer.size() == 0) { break; }
    }

    return std::move(tree);
}

}  // namespace clfd::search