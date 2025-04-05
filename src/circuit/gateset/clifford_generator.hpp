#pragma once

#include <cassert>
#include "../../defines.hpp"
#include "../../utils/fmt.hpp"
#include "../../utils/ranges.hpp"
#include "cereal/cereal.hpp"

namespace circ {

enum class CliffordGenOp : uint8_t { I = 0, HP = 1, PH = 2 };
inline auto format_as(CliffordGenOp op) {
    if (op == CliffordGenOp::I) { return "I"; }
    if (op == CliffordGenOp::HP) { return "HP"; }
    if (op == CliffordGenOp::PH) { return "PH"; }
    __builtin_unreachable();
}

template <std::size_t N>
class __attribute__((packed)) CliffordGen {
    static_assert(N <= 5ul);
    QIdx q1 : 4;
    QIdx q2 : 4;

   public:
    inline constexpr CliffordGen() noexcept : q1(0), q2(0) {}
    inline constexpr CliffordGen(CliffordGenOp op_ctrl, CliffordGenOp op_not, QIdx ictrl, QIdx inot) noexcept  // NOLINT
        : q1(uint8_t(op_ctrl) * 5 + ictrl), q2(uint8_t(op_not) * 5 + inot) {
        assert(ictrl < 5 && inot < 5);
    }
    [[nodiscard]] inline constexpr CliffordGenOp op_ctrl() const noexcept { return CliffordGenOp(q1 / 5); }
    [[nodiscard]] inline constexpr CliffordGenOp op_not() const noexcept { return CliffordGenOp(q2 / 5); }
    [[nodiscard]] inline constexpr QIdx ictrl() const noexcept { return QIdx(q1 % 5); }
    [[nodiscard]] inline constexpr QIdx inot() const noexcept { return QIdx(q2 % 5); }
    [[nodiscard]] inline constexpr bool operator==(const CliffordGen&) const noexcept = default;
    [[nodiscard]] inline constexpr auto operator<=>(const CliffordGen&) const noexcept = default;
    [[nodiscard]] inline constexpr std::array<QIdx, 2> bits() const noexcept { return {ictrl(), inot()}; }
    [[nodiscard]] inline constexpr CliffordGen with_bits(std::array<QIdx, 2> bits) const noexcept {
        return CliffordGen(op_ctrl(), op_not(), bits[0], bits[1]);
    }

    [[nodiscard]] inline constexpr bool nonnull() const noexcept { return q1 != 0 || q2 != 0; };

    template <typename Archive>
    void serialize(Archive& archive) {
        assert(nonnull());
        archive(cereal::binary_data(this, sizeof(CliffordGen)));
    }

    [[nodiscard]] static constexpr std::vector<CliffordGen<N>> all_generator() noexcept {
        std::vector<CliffordGenOp> generator_ops{CliffordGenOp::I, CliffordGenOp::HP, CliffordGenOp::PH};
        std::vector<CliffordGen<N>> result;
        for (auto [op1, op2] : vw::cartesian_product(generator_ops, generator_ops)) {
            for (auto [a, b] : vw::cartesian_product(vw::ints(QIdx(0), QIdx(N)), vw::ints(QIdx(0), QIdx(N)))) {
                if (a != b) { result.push_back(CliffordGen(op1, op2, a, b)); }
            }
        }
        return result;
    }
    [[nodiscard]] static constexpr std::vector<CliffordGen<N>> all_generator(QIdx a, QIdx b) noexcept {
        assert(a != b);
        std::vector<CliffordGenOp> generator_ops{CliffordGenOp::I, CliffordGenOp::HP, CliffordGenOp::PH};
        std::vector<CliffordGen<N>> result;
        for (auto [op1, op2] : vw::cartesian_product(generator_ops, generator_ops)) {
            result.push_back(CliffordGen(op1, op2, a, b));
        }
        return result;
    }
    [[nodiscard]] inline constexpr static CliffordGen<N> ii_cx(QIdx q1, QIdx q2) noexcept { return {CliffordGenOp::I, CliffordGenOp::I, q1, q1}; }
    [[nodiscard]] inline constexpr static CliffordGen<N> ihp_cx(QIdx q1, QIdx q2) noexcept { return {CliffordGenOp::I, CliffordGenOp::HP, q1, q2}; }
    [[nodiscard]] inline constexpr static CliffordGen<N> iph_cx(QIdx q1, QIdx q2) noexcept { return {CliffordGenOp::I, CliffordGenOp::PH, q1, q2}; }
    [[nodiscard]] inline constexpr static CliffordGen<N> hpi_cx(QIdx q1, QIdx q2) noexcept { return {CliffordGenOp::HP, CliffordGenOp::I, q1, q2}; }
    [[nodiscard]] inline constexpr static CliffordGen<N> hphp_cx(QIdx q1, QIdx q2) noexcept { return {CliffordGenOp::HP, CliffordGenOp::HP, q1, q2}; }
    [[nodiscard]] inline constexpr static CliffordGen<N> hpph_cx(QIdx q1, QIdx q2) noexcept { return {CliffordGenOp::HP, CliffordGenOp::PH, q1, q2}; }
    [[nodiscard]] inline constexpr static CliffordGen<N> phi_cx(QIdx q1, QIdx q2) noexcept { return {CliffordGenOp::PH, CliffordGenOp::I, q1, q2}; }
    [[nodiscard]] inline constexpr static CliffordGen<N> phhp_cx(QIdx q1, QIdx q2) noexcept { return {CliffordGenOp::PH, CliffordGenOp::HP, q1, q2}; }
    [[nodiscard]] inline constexpr static CliffordGen<N> phph_cx(QIdx q1, QIdx q2) noexcept { return {CliffordGenOp::PH, CliffordGenOp::PH, q1, q2}; }
};

template <const std::size_t N>
auto format_as(const CliffordGen<N>& gate) {
    if (!gate.nonnull()) { return fmt::format("nullgate"); }
    return fmt::format("{}{}-CX({}, {})", gate.op_ctrl(), gate.op_not(), gate.ictrl(), gate.inot());
}

}  // namespace circ