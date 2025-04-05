#pragma once

#include <algorithm>
#include <cstdint>
#include <experimental/random>
#include <range/v3/view/concat.hpp>
#include <variant>
#include <vector>
#include "../../utils/decomposed.hpp"
#include "../bitsymplectic.hpp"
#include "range/v3/algorithm/none_of.hpp"
#include "range/v3/view/for_each.hpp"

namespace clfd {
template <typename T>
[[nodiscard]] inline constexpr std::vector<T> concat(const std::vector<T>& a, const std::vector<T>& b) noexcept {  // NOLINT
    std::vector<T> result;
    result.reserve(a.size() + b.size());
    result.insert(result.end(), a.begin(), a.end());
    result.insert(result.end(), b.begin(), b.end());
    return result;
}
template <typename T>
[[nodiscard]] inline constexpr std::vector<T> concat3(const std::vector<T>& a, const std::vector<T>& b, const std::vector<T>& c) noexcept {
    return concat(concat(a, b), c);
}
template <typename T>
[[nodiscard]] inline constexpr std::vector<T>
concat4(const std::vector<T>& a, const std::vector<T>& b, const std::vector<T>& c, const std::vector<T>& d) noexcept {
    return concat3(concat(a, b), c, d);
}
template <typename T>
[[nodiscard]] inline constexpr std::vector<T>
concat5(const std::vector<T>& a, const std::vector<T>& b, const std::vector<T>& c, const std::vector<T>& d, const std::vector<T>& e) noexcept {
    return concat4(concat(a, b), c, d, e);
}
template <typename T>
[[nodiscard]] inline constexpr std::vector<T> concat6(
    const std::vector<T>& a,
    const std::vector<T>& b,
    const std::vector<T>& c,
    const std::vector<T>& d,
    const std::vector<T>& e,
    const std::vector<T>& f
) noexcept {
    return concat5(concat(a, b), c, d, e, f);
}

template <const std::size_t N>
class CliffordGate {
    struct I {
        [[nodiscard]] constexpr bool operator==(const I&) const = default;
    };
    struct H {
        std::size_t iq;
        [[nodiscard]] constexpr bool operator==(const H&) const = default;
    };
    struct P {
        std::size_t iq;
        [[nodiscard]] constexpr bool operator==(const P&) const = default;
    };
    struct HPH {
        std::size_t iq;
        [[nodiscard]] constexpr bool operator==(const HPH&) const = default;
    };
    struct HP {
        std::size_t iq;
        [[nodiscard]] constexpr bool operator==(const HP&) const = default;
    };
    struct PH {
        std::size_t iq;
        [[nodiscard]] constexpr bool operator==(const PH&) const = default;
    };
    struct CNOT {
        std::size_t ictrl;
        std::size_t inot;
        [[nodiscard]] constexpr bool operator==(const CNOT&) const = default;
    };
    struct SWAP {
        std::size_t iqa;
        std::size_t iqb;
        [[nodiscard]] constexpr bool operator==(const SWAP&) const = default;
    };

    using Variant = std::variant<I, H, P, HPH, HP, PH, CNOT, SWAP>;
    Variant gate;
    explicit constexpr CliffordGate<N>(Variant gate) : gate(gate) {}

   public:
    [[nodiscard]] static constexpr CliffordGate<N> i() noexcept { return CliffordGate<N>(Variant(I{})); }
    [[nodiscard]] static constexpr CliffordGate<N> h(std::size_t iq) noexcept { return CliffordGate<N>(Variant(H{iq})); }
    [[nodiscard]] static constexpr CliffordGate<N> p(std::size_t iq) noexcept { return CliffordGate<N>(Variant(P{iq})); }
    [[nodiscard]] static constexpr CliffordGate<N> hph(std::size_t iq) noexcept { return CliffordGate<N>(Variant(HPH{iq})); }
    [[nodiscard]] static constexpr CliffordGate<N> ph(std::size_t iq) noexcept { return CliffordGate<N>(Variant(PH{iq})); }
    [[nodiscard]] static constexpr CliffordGate<N> hp(std::size_t iq) noexcept { return CliffordGate<N>(Variant(HP{iq})); }
    [[nodiscard]] static constexpr CliffordGate<N> cnot(std::size_t ictrl, std::size_t inot) noexcept { return CliffordGate<N>(CNOT{ictrl, inot}); }
    [[nodiscard]] static constexpr CliffordGate<N> swap(std::size_t iqa, std::size_t iqb) noexcept { return CliffordGate<N>(SWAP{iqa, iqb}); }
    [[nodiscard]] constexpr bool operator==(const CliffordGate<N>&) const = default;
    [[nodiscard]] constexpr bool operator!=(const CliffordGate<N>& other) const { return !(*this == other); }

    [[nodiscard]] static constexpr std::vector<CliffordGate<N>> all_h() noexcept {
        return vw::ints(0ul, N) | vw::transform([](std::size_t i) { return h(i); }) | rgs::to<std::vector>();
    }
    [[nodiscard]] static constexpr std::vector<CliffordGate<N>> all_p() noexcept {
        return vw::ints(0ul, N) | vw::transform([](std::size_t i) { return p(i); }) | rgs::to<std::vector>();
    }
    [[nodiscard]] static constexpr std::vector<CliffordGate<N>> all_hph() noexcept {
        return vw::ints(0ul, N) | vw::transform([](std::size_t i) { return hph(i); }) | rgs::to<std::vector>();
    }
    [[nodiscard]] static constexpr std::vector<CliffordGate<N>> all_ph() noexcept {
        return vw::ints(0ul, N) | vw::transform([](std::size_t i) { return ph(i); }) | rgs::to<std::vector>();
    }
    [[nodiscard]] static constexpr std::vector<CliffordGate<N>> all_hp() noexcept {
        return vw::ints(0ul, N) | vw::transform([](std::size_t i) { return hp(i); }) | rgs::to<std::vector>();
    }
    [[nodiscard]] static constexpr std::vector<CliffordGate<N>> all_cnot() noexcept {
        return vw::cartesian_product(vw::ints(0ul, N), vw::ints(0ul, N)) | vw::filter(decomposed([](auto a, auto b) { return a != b; })) |
               vw::transform(decomposed([](auto a, auto b) { return cnot(a, b); })) | rgs::to<std::vector>();
    }
    [[nodiscard]] static constexpr std::vector<CliffordGate<N>> all_swap() noexcept {
        return vw::cartesian_product(vw::ints(0ul, N), vw::ints(0ul, N)) | vw::filter(decomposed([](auto a, auto b) { return a < b; })) |
               vw::transform(decomposed([](auto a, auto b) { return swap(a, b); })) | rgs::to<std::vector>();
    }
    [[nodiscard]] static constexpr std::vector<CliffordGate<N>> all_gates() noexcept { return concat3(all_h(), all_p(), all_cnot()); }
    [[nodiscard]] static constexpr std::vector<CliffordGate<N>> all_level_0() noexcept {
        return concat5(all_h(), all_p(), all_hph(), all_ph(), all_hp());
    }
    [[nodiscard]] static constexpr std::vector<CliffordGate<N>> all_level_0_with_swap() noexcept {
        return concat6(all_h(), all_p(), all_hph(), all_ph(), all_hp(), all_swap());
    }
    [[nodiscard]] static constexpr std::vector<CliffordGate<N>> all_level_0_with_cnot() noexcept {
        return concat6(all_h(), all_p(), all_hph(), all_ph(), all_hp(), all_cnot());
    }

    inline constexpr void do_apply_l(BitSymplectic<N>& /*mut*/ input) const noexcept {
        std::visit(
            [&input](auto gate) {
                if constexpr (std::is_same_v<decltype(gate), I>) {
                    return;
                } else if constexpr (std::is_same_v<decltype(gate), H>) {
                    input.do_hadamard_l(gate.iq);
                } else if constexpr (std::is_same_v<decltype(gate), P>) {
                    input.do_phase_l(gate.iq);
                } else if constexpr (std::is_same_v<decltype(gate), HPH>) {
                    input.do_hphaseh_l(gate.iq);
                } else if constexpr (std::is_same_v<decltype(gate), HP>) {
                    input.do_hadamard_l(gate.iq);
                    input.do_phase_l(gate.iq);
                } else if constexpr (std::is_same_v<decltype(gate), PH>) {
                    input.do_phase_l(gate.iq);
                    input.do_hadamard_l(gate.iq);
                } else if constexpr (std::is_same_v<decltype(gate), CNOT>) {
                    input.do_cnot_l(gate.ictrl, gate.inot);
                } else if constexpr (std::is_same_v<decltype(gate), SWAP>) {
                    input.do_swap_l(gate.iqa, gate.iqb);
                } else {
                    __builtin_unreachable();
                }
            },
            gate
        );
    }
    inline constexpr void do_apply_r(BitSymplectic<N>& /*mut*/ input) const noexcept {
        std::visit(
            [&input](auto gate) {
                if constexpr (std::is_same_v<decltype(gate), I>) {
                    return;
                } else if constexpr (std::is_same_v<decltype(gate), H>) {
                    input.do_hadamard_r(gate.iq);
                } else if constexpr (std::is_same_v<decltype(gate), P>) {
                    input.do_phase_r(gate.iq);
                } else if constexpr (std::is_same_v<decltype(gate), HPH>) {
                    input.do_hphaseh_r(gate.iq);
                } else if constexpr (std::is_same_v<decltype(gate), HP>) {
                    input.do_hadamard_r(gate.iq);
                    input.do_phase_r(gate.iq);
                } else if constexpr (std::is_same_v<decltype(gate), PH>) {
                    input.do_phase_r(gate.iq);
                    input.do_hadamard_r(gate.iq);
                } else if constexpr (std::is_same_v<decltype(gate), CNOT>) {
                    input.do_cnot_r(gate.ictrl, gate.inot);
                } else if constexpr (std::is_same_v<decltype(gate), SWAP>) {
                    input.do_swap_r(gate.iqa, gate.iqb);
                } else {
                    __builtin_unreachable();
                }
            },
            gate
        );
    }
    [[nodiscard]] inline std::vector<std::size_t> used_qubits() const noexcept {
        return std::visit(
            [](auto gate) {
                if constexpr (std::is_same_v<decltype(gate), I>) {
                    return std::vector<std::size_t>();
                } else if constexpr (std::is_same_v<decltype(gate), H>) {
                    return std::vector<std::size_t>{gate.iq};
                } else if constexpr (std::is_same_v<decltype(gate), P>) {
                    return std::vector<std::size_t>{gate.iq};
                } else if constexpr (std::is_same_v<decltype(gate), HPH>) {
                    return std::vector<std::size_t>{gate.iq};
                } else if constexpr (std::is_same_v<decltype(gate), HP>) {
                    return std::vector<std::size_t>{gate.iq};
                } else if constexpr (std::is_same_v<decltype(gate), PH>) {
                    return std::vector<std::size_t>{gate.iq};
                } else if constexpr (std::is_same_v<decltype(gate), CNOT>) {
                    return std::vector<std::size_t>{gate.ictrl, gate.inot};
                } else if constexpr (std::is_same_v<decltype(gate), SWAP>) {
                    return std::vector<std::size_t>{gate.iqa, gate.iqb};
                } else {
                    __builtin_unreachable();
                }
            },
            gate
        );
    }
    [[nodiscard]] inline std::string fmt() const noexcept {
        return std::visit(
            [](auto gate) {
                if constexpr (std::is_same_v<decltype(gate), typename CliffordGate<N>::I>) {
                    return fmt::format("I");
                } else if constexpr (std::is_same_v<decltype(gate), typename CliffordGate<N>::H>) {
                    return fmt::format("H({})", gate.iq);
                } else if constexpr (std::is_same_v<decltype(gate), typename CliffordGate<N>::P>) {
                    return fmt::format("P({})", gate.iq);
                } else if constexpr (std::is_same_v<decltype(gate), typename CliffordGate<N>::HPH>) {
                    return fmt::format("HPH({})", gate.iq);
                } else if constexpr (std::is_same_v<decltype(gate), typename CliffordGate<N>::HP>) {
                    return fmt::format("HP({})", gate.iq);
                } else if constexpr (std::is_same_v<decltype(gate), typename CliffordGate<N>::PH>) {
                    return fmt::format("PH({})", gate.iq);
                } else if constexpr (std::is_same_v<decltype(gate), typename CliffordGate<N>::CNOT>) {
                    return fmt::format("CNOT({}, {})", gate.ictrl, gate.inot);
                } else if constexpr (std::is_same_v<decltype(gate), typename CliffordGate<N>::SWAP>) {
                    return fmt::format("SWAP({}, {})", gate.iqa, gate.iqb);
                } else {
                    __builtin_unreachable();
                }
            },
            gate
        );
    }
    [[nodiscard]] inline constexpr BitSymplectic<N> apply_l(BitSymplectic<N> input) const noexcept {
        auto result = input;
        do_apply_l(result);
        return result;
    }
    [[nodiscard]] inline constexpr BitSymplectic<N> apply_r(BitSymplectic<N> input) const noexcept {
        auto result = input;
        do_apply_r(result);
        return result;
    }
};

template <const std::size_t N>
inline constexpr void
perform_random_gates(BitSymplectic<N>& /*mut*/ input, std::size_t times, std::vector<CliffordGate<N>> gates, Bv<2ul> direction) noexcept {
    for (auto i = 0ul; i < times; i++) {
        if (direction[0]) {
            auto gate = gates[std::experimental::randint(0ul, gates.size() - 1)];
            gate.do_apply_l(input);
        }
        if (direction[1]) {
            auto gate = gates[std::experimental::randint(0ul, gates.size() - 1)];
            gate.do_apply_r(input);
        }
    }
}

template <const std::size_t N>
auto format_as(CliffordGate<N> gate) {
    return gate.fmt();
}

}  // namespace clfd

// template <const std::size_t N>
// auto format_as(clfd::CliffordGeneratorOp gate) {
//     if (gate == clfd::CliffordGeneratorOp::I) { return "I"; }
//     if (gate == clfd::CliffordGeneratorOp::HP) { return "HP"; }
//     if (gate == clfd::CliffordGeneratorOp::PH) { return "PH"; }
//     __builtin_unreachable();
// }
