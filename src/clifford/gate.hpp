#pragma once

#include <cereal/archives/binary.hpp>
#include <cstdint>
#include "../defines.hpp"
#include "bitsymplectic.hpp"

namespace clifford {

enum class CliffordGeneratorOp : uint8_t { I = 0, HP = 1, PH = 2 };
template <const std::size_t N>
constexpr void perform_gen_op_l(BitSymplectic<N>& /*mut*/ input, CliffordGeneratorOp op, std::size_t i) noexcept {
    if (op == CliffordGeneratorOp::HP) {
        input.do_hadamard_l(i);
        input.do_phase_l(i);
    } else if (op == CliffordGeneratorOp::PH) {
        input.do_phase_l(i);
        input.do_hadamard_l(i);
    }
}
template <const std::size_t N>
constexpr void perform_gen_op_r(BitSymplectic<N>& /*mut*/ input, CliffordGeneratorOp op, std::size_t i) noexcept {
    if (op == CliffordGeneratorOp::HP) {
        input.do_hadamard_r(i);
        input.do_phase_r(i);
    } else if (op == CliffordGeneratorOp::PH) {
        input.do_phase_r(i);
        input.do_hadamard_r(i);
    }
}

inline auto format_as(CliffordGeneratorOp op) {
    if (op == clifford::CliffordGeneratorOp::I) { return "I"; }
    if (op == clifford::CliffordGeneratorOp::HP) { return "HP"; }
    if (op == clifford::CliffordGeneratorOp::PH) { return "PH"; }
    __builtin_unreachable();
}

template <std::size_t N>
class __attribute__((packed)) CliffordGenerator {
    static_assert(N <= 5ul);
    QIdx q1 : 4;
    QIdx q2 : 4;

   public:
    inline constexpr explicit CliffordGenerator() noexcept : q1(0), q2(0) {}
    inline constexpr explicit CliffordGenerator(CliffordGeneratorOp op_ctrl, CliffordGeneratorOp op_not, QIdx ictrl, QIdx inot) noexcept  // NOLINT
        : q1(uint8_t(op_ctrl) * 5 + ictrl), q2(uint8_t(op_not) * 5 + inot) {
        assert(ictrl < 5 && inot < 5);
    }
    [[nodiscard]] inline constexpr CliffordGeneratorOp op_ctrl() const noexcept { return CliffordGeneratorOp(q1 / 5); }
    [[nodiscard]] inline constexpr CliffordGeneratorOp op_not() const noexcept { return CliffordGeneratorOp(q2 / 5); }
    [[nodiscard]] inline constexpr QIdx ictrl() const noexcept { return QIdx(q1 % 5); }
    [[nodiscard]] inline constexpr QIdx inot() const noexcept { return QIdx(q2 % 5); }
    [[nodiscard]] inline constexpr bool operator==(const CliffordGenerator&) const = default;
    [[nodiscard]] inline constexpr bool nonnull() const noexcept { return q1 != 0 || q2 != 0; };

    template <typename Archive>
    void serialize(Archive& archive) {
        assert(nonnull());
        archive(cereal::binary_data(this, sizeof(CliffordGenerator)));
    }

    inline constexpr void do_apply_l(BitSymplectic<N>& /*mut*/ input) const noexcept {
        assert(nonnull());
        perform_gen_op_l(/*mut*/ input, op_ctrl(), ictrl());
        perform_gen_op_l(/*mut*/ input, op_not(), inot());
        input.do_cnot_l(ictrl(), inot());
    }
    inline constexpr void do_apply_r(BitSymplectic<N>& /*mut*/ input) const noexcept {
        assert(nonnull());
        perform_gen_op_r(/*mut*/ input, op_ctrl(), ictrl());
        perform_gen_op_r(/*mut*/ input, op_not(), inot());
        input.do_cnot_r(ictrl(), inot());
    }
    [[nodiscard]] inline BitSymplectic<N> apply_l(BitSymplectic<N> input) const noexcept {
        do_apply_l(input);
        return input;
    }
    [[nodiscard]] inline BitSymplectic<N> apply_r(BitSymplectic<N> input) const noexcept {
        do_apply_r(input);
        return input;
    }

    [[nodiscard]] static constexpr std::vector<CliffordGenerator<N>> all_generator() noexcept {
        std::vector<CliffordGeneratorOp> generator_ops{CliffordGeneratorOp::I, CliffordGeneratorOp::HP, CliffordGeneratorOp::PH};
        std::vector<CliffordGenerator<N>> result;
        for (auto [op1, op2] : vw::cartesian_product(generator_ops, generator_ops)) {
            for (auto [a, b] : vw::cartesian_product(vw::ints(QIdx(0), QIdx(N)), vw::ints(QIdx(0), QIdx(N)))) {
                if (a != b) { result.push_back(CliffordGenerator(op1, op2, a, b)); }
            }
        }
        return result;
    }
};

template <const std::size_t N>
auto format_as(const CliffordGenerator<N>& gate) {
    assert(gate.nonnull());
    return fmt::format("{}{}-CX({}, {})", gate.op_ctrl(), gate.op_not(), gate.ictrl(), gate.inot());
}

}  // namespace clifford