#pragma once

#include <string_view>
#include "layer_combine.hpp"
#include "meta/meta.hpp"
#include "range/v3/view/for_each.hpp"

namespace qsearch {

// class Circuit;

// class CircuitCons {
//     const Circuit* _Nullable _prev;
//     const Layer* _Nullable _layer;

//    public:
//     inline CircuitCons() noexcept : _prev(nullptr), _layer() {}
//     inline explicit CircuitCons(const Circuit* _Nonnull prev, const Layer* _Nonnull layer) noexcept : _prev(prev), _layer(layer) {}
//     [[nodiscard]] inline explicit operator bool() const noexcept { return bool(_prev); }

//     [[nodiscard]] inline constexpr const Circuit* _Nonnull prev() const noexcept {
//         assert(bool(_prev) && bool(_layer));
//         return _prev;
//     }
//     [[nodiscard]] inline constexpr const Layer* _Nonnull layer() const noexcept {
//         assert(bool(_prev) && bool(_layer));
//         return _layer;
//     }
// };

// struct CircuitGateChecker {
//     Bv<NQUBITS> total_mask;
//     Bv<NQUBITS> this_mask;
//     Bv<NQUBITS> last_mask;

//     inline bool check_gate(gate::Gate gate) const noexcept {
//         auto total = total_mask;
//         fmt::println("Checking: {} {} {} {} {}", gate.fmt(), total_mask, this_mask, last_mask, total.firstr_zero());
//         for (auto qidx : gate.qindices()) {
//             if (total.firstr_zero() != qidx) { return false; }
//             total = total.update(qidx, true);
//         }
//         if ((gate.qidx_mask() & this_mask).none() && last_mask.any()) {
//             return (gate.qidx_mask() & last_mask).any();
//         } else {
//             return true;
//         }
//     }
// };

// struct Circuit {
//     CircuitCons cons;
//     Bv<NQUBITS> mask;

//     inline Circuit() noexcept : mask(Bv<NQUBITS>::zero()) {}
//     inline Circuit(const Circuit* _Nonnull circ, const Layer* _Nonnull layer) noexcept : cons(circ, layer), mask(circ->mask | layer->info.mask) {}

//     [[nodiscard]] inline CircuitGateChecker get_checker() const noexcept {
//         return CircuitGateChecker{
//             mask,
//             cons ? cons.layer()->info.mask : Bv<NQUBITS>::zero(),
//             cons && cons.prev()->cons ? cons.prev()->cons.layer()->info.mask : Bv<NQUBITS>::zero(),
//         };
//     }

//     [[nodiscard]] inline auto adjecents_same_layer() const {
//         assert(cons);
//         const auto* prev = cons.prev();
//         const auto check = get_checker();
//         return cons.layer()->adjecents() | vw::filter([check](auto layer) { return check.check_gate(layer->cons.gate()); }) |
//                vw::transform([prev](auto layer) { return Circuit(prev, layer); });
//     }
//     [[nodiscard]] inline gate::Gate generating_gate() const noexcept {
//         assert(cons && cons.layer()->cons);
//         return cons.layer()->cons.gate();
//     }
// };

struct CircuitInfo {
    Bv<NQUBITS> total_mask;
    Bv<NQUBITS> clifford_avail;
    Bv<NQUBITS> gate_avail;

    inline CircuitInfo() noexcept : total_mask(Bv<NQUBITS>::zero()), clifford_avail(Bv<NQUBITS>::zero()), gate_avail(Bv<NQUBITS>::zero()) {}

    inline CircuitInfo(const Bv<NQUBITS>& total_mask, const Bv<NQUBITS>& clifford_avail, const Bv<NQUBITS>& gate_avail) noexcept  // NOLINT
        : total_mask(total_mask), clifford_avail(clifford_avail), gate_avail(gate_avail) {}

    [[nodiscard]] inline CircuitInfo plus(const LayerCombine& layer) const noexcept {
        auto cmask = layer.clifford_info().mask;
        auto gmask = layer.gate_info().mask;
        return CircuitInfo{
            total_mask | cmask | gmask,
            (clifford_avail | cmask).setminus(gmask),
            ~(cmask | gmask),
        };
    }
    [[nodiscard]] inline auto available(gate::Gate gate) const noexcept {
        return !gate::Gate::is<gate::Gate1<gate::T>>(gate) && gate.qidx_mask().setminus(clifford_avail).none() ||
               gate.qidx_mask().setminus(gate_avail).none();
    }
};

using Circuit = List<LayerCombine, CircuitInfo>;

[[nodiscard]] inline bool circuit_check_gate(const Circuit circ, gate::Gate gate) noexcept {
    assert(circ);
    // Check Ordering
    auto total = circ.info().total_mask;
    for (auto qidx : gate.qindices()) {
        if (total[qidx]) { continue; }
        if (total.firstr_zero() != qidx) { return false; }
        total = total.update(qidx, true);
    }

    // Check Availability
    return !circ.head().passable(gate) || !circ.tail().info().available(gate);
}

inline auto circuit_adjecents_same_layer(const Circuit circ) {
    assert(circ);
    return circ.head().adjecents() | vw::filter([circ](auto layer) { return circuit_check_gate(circ, layer.inner()); }) |
           vw::transform([circ](auto layer) { return circ.tail().plus(layer); });
}

template <typename F>
inline void generate_adjecent_circuit(const Circuit circ, const LayerCombine root, F f) {
    if (circ) {
        for (auto a : circuit_adjecents_same_layer(circ)) {
            if (!f(a)) { return; }
        }
    }
    for (auto a : circuit_adjecents_same_layer(circ.plus(root))) {
        if (!f(a)) { return; }
    }
}

inline gate::json circuit_as_json(const Circuit circ) {
    gate::json j = gate::json::array();
    for (auto layer : circ) {
        for (auto l = layer.head().clifford_layer; l; l = l.prev()) {
            j.push_back(l.inner().to_json());
        }
        for (auto l = layer.head().gate_layer; l; l = l.prev()) {
            j.push_back(l.inner().to_json());
        }
    }
    return j;
}

}  // namespace qsearch
