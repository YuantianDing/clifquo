#pragma once

#include <doctest/doctest.h>
#include <fmt/core.h>
#include <bit>
#include <bitset>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <utility>
#include "../utils/bitvec.hpp"
#include "../utils/fmt.hpp"
#include "../utils/ranges.hpp"
#include "../utils/test.hpp"
#include "fmt/base.h"
#include "range/v3/algorithm/all_of.hpp"
#include "range/v3/algorithm/none_of.hpp"
#include "range/v3/view/repeat.hpp"
#include "range/v3/view/unique.hpp"

namespace clfd {
template <std::size_t RowN, std::size_t TimeN>
inline constexpr uint64_t repeat_row(uint64_t n) noexcept {
    auto result = 0ul;
    for (auto i = 0ul; i < TimeN; i++) {
        result |= (n << (i * RowN));
    }
    return result;
}

template <std::size_t N>
class BitSymplectic {
    static_assert(N <= 5ul);
    static const std::size_t VecN = 2z * N;

    Bv<N * VecN> xrows, zrows;

    inline explicit constexpr BitSymplectic(uint64_t xrows, uint64_t zrows) noexcept : xrows(xrows), zrows(zrows) {}
    inline explicit constexpr BitSymplectic(const Bv<N * VecN> xrows, const Bv<N * VecN> zrows) noexcept : xrows(xrows), zrows(zrows) {}

   public:
    [[nodiscard]] inline constexpr std::pair<uint64_t, uint64_t> as_raw() const noexcept { return std::make_pair(xrows.uint(), zrows.uint()); }
    [[nodiscard]] inline static constexpr bool omega(Bv<VecN> vec1, Bv<VecN> vec2) noexcept {
        auto v = Bv<N>::slice(vec2, N).concat(Bv<N>::slice(vec2, 0ul));
        return vec1.dot(v);
    }

    [[nodiscard]] inline constexpr bool check_symplecticity() const noexcept {
        return rgs::all_of(vw::ints(0ul, N), [this](auto i) {
            return omega(xrow(i), zrow(i)) &&
                   rgs::all_of(vw::ints(0ul, i), [this, i](auto j) { return !omega(xrow(i), zrow(j)) && !omega(zrow(i), xrow(j)); });
        });
    }

    [[nodiscard]] inline constexpr auto operator<=>(const BitSymplectic<N>& other) const noexcept = default;

    [[nodiscard]] inline constexpr static BitSymplectic<N> null() noexcept { return BitSymplectic<N>(0, 0); }
    [[nodiscard]] inline constexpr static BitSymplectic<N> identity() noexcept {
        auto result = BitSymplectic<N>(0ul, 0ul);
        auto value = Bv<VecN>(1ul);
        for (auto i = 0ul; i < N; i++) {
            result.set_xrow(i, value);
            result.set_zrow(i, value << N);
            value <<= 1z;
        }
        assert(result.check_symplecticity());
        return result;
    }

    [[nodiscard]] inline constexpr static BitSymplectic<N> from_array(const std::array<Bv<VecN>, VecN>& matrix) noexcept {
        auto&& result = BitSymplectic<N>(0ul, 0ul);
        for (auto i = 0ul; i < N; i++) {
            result.set_xrow(i, matrix[i]);
            result.set_zrow(i, matrix[i + N]);
        }
        assert(result.check_symplecticity());
        return result;
    }

    [[nodiscard]] inline constexpr static BitSymplectic<N> from_qubit_array(const std::array<Bv<VecN * 2>, N>& matrix) noexcept {
        auto&& result = BitSymplectic<N>(0ul, 0ul);
        for (auto i = 0ul; i < N; i++) {
            result.set_xrow(i, Bv<VecN>::slice(matrix[i], 0));
            result.set_zrow(i, Bv<VecN>::slice(matrix[i], VecN));
        }
        assert(result.check_symplecticity());
        return result;
    }

    [[nodiscard]] inline constexpr static BitSymplectic<N> raw(Bv<N * VecN> xrows, Bv<N * VecN> zrows) noexcept {
        auto&& result = BitSymplectic<N>(xrows, zrows);
        assert(result.check_symplecticity());
        return result;
    }

    [[nodiscard]] inline constexpr bool get(std::size_t irow, std::size_t icol) const noexcept {
        if (irow < N) {
            return xrows[irow * VecN + icol];
        } else {
            return zrows[(irow - N) * VecN + icol];
        }
    }
    [[nodiscard]] inline constexpr Bv<2 * VecN> get_row(std::size_t irow) const noexcept { return xrow(irow).concat(zrow(irow)); }
    [[nodiscard]] inline constexpr Bv<VecN> xrow(std::size_t irow) const noexcept { return Bv<VecN>::slice(xrows, irow * VecN); }
    [[nodiscard]] inline constexpr Bv<VecN> zrow(std::size_t irow) const noexcept { return Bv<VecN>::slice(zrows, irow * VecN); }

    [[nodiscard]] inline constexpr Bv<VecN> xcol(const std::size_t icol) const noexcept {
        auto result = Bv<VecN>(0ul);
        for (auto i = 0ul; i < N; i++) {
            result = result.update(i, xrows[i * VecN + icol]);
            result = result.update(i + N, zrows[i * VecN + icol]);
        }
        return result;
    }
    inline static const auto MASK_COL_RAW = Bv<2 * N * N>(repeat_row<2 * N, N>(1));
    [[nodiscard]] inline constexpr int col_metric(const std::size_t icol) const noexcept {
        assert(((xrows >> icol) & MASK_COL_RAW).count_ones() == Bv<N>::slice(xcol(icol), 0).count_ones());
        assert(((zrows >> icol) & MASK_COL_RAW).count_ones() == Bv<N>::slice(xcol(icol), N).count_ones());
        assert(((xrows >> (icol + N)) & MASK_COL_RAW).count_ones() == Bv<N>::slice(zcol(icol), 0).count_ones());
        assert(((zrows >> (icol + N)) & MASK_COL_RAW).count_ones() == Bv<N>::slice(zcol(icol), N).count_ones());

        auto a = (xrows >> icol) & MASK_COL_RAW | ((zrows >> icol) & MASK_COL_RAW);
        auto b = (xrows >> (icol + N)) & MASK_COL_RAW | ((zrows >> (icol + N)) & MASK_COL_RAW);
        auto result = (a | (b << 1)).count_ones();
        assert(
            result == (Bv<N>::slice(xcol(icol), 0) | Bv<N>::slice(xcol(icol), N)).count_ones() +
                          (Bv<N>::slice(zcol(icol), 0) | Bv<N>::slice(zcol(icol), N)).count_ones()
        );
        return result;
    }
    [[nodiscard]] inline constexpr Bv<VecN> zcol(const std::size_t icol) const noexcept { return xcol(icol + N); }

   private:
    inline constexpr void set_xrow(std::size_t irow, Bv<VecN> value) noexcept { xrows = xrows.update_slice(irow * VecN, value); }
    inline constexpr void set_zrow(std::size_t irow, Bv<VecN> value) noexcept { zrows = zrows.update_slice(irow * VecN, value); }

    inline constexpr void set_xcol(const std::size_t icol, Bv<VecN> value) noexcept {
        for (std::size_t i = 0z; i < N; i++) {
            xrows = xrows.update(i * VecN + icol, value[i]);
            zrows = zrows.update(i * VecN + icol, value[i + N]);
        }
    }
    inline constexpr void set_zcol(const std::size_t icol, Bv<VecN> value) noexcept { set_xcol(icol + N, value); }

    inline constexpr void xor_xrow(std::size_t irow, Bv<VecN> value) noexcept { xrows = xrows.xor_slice(irow * VecN, value); }
    inline constexpr void xor_zrow(std::size_t irow, Bv<VecN> value) noexcept { zrows = zrows.xor_slice(irow * VecN, value); }

    inline constexpr void xor_xcol(const std::size_t icol, Bv<VecN> value) noexcept {
        for (std::size_t i = 0z; i < N; i++) {
            xrows = xrows.xor_at(i * VecN + icol, value[i]);
            zrows = zrows.xor_at(i * VecN + icol, value[i + N]);
        }
    }
    inline constexpr void xor_zcol(const std::size_t icol, Bv<VecN> value) noexcept { xor_xcol(icol + N, value); }

   public:
    inline constexpr void do_hadamard_l(const std::size_t& irow) noexcept {
        assert(irow < N);
        auto x = xrow(irow);
        auto z = zrow(irow);
        set_xrow(irow, z);
        set_zrow(irow, x);
        assert(check_symplecticity());
    }
    inline constexpr void do_hadamard_r(const std::size_t& icol) noexcept {
        assert(icol < N);
        auto x = xcol(icol);
        auto z = zcol(icol);
        set_xcol(icol, z);
        set_zcol(icol, x);
        assert(check_symplecticity());
    }
    inline constexpr void do_phase_l(const std::size_t& irow) noexcept {
        assert(irow < N);
        xor_zrow(irow, xrow(irow));
        assert(check_symplecticity());
    }
    inline constexpr void do_phase_r(const std::size_t& icol) noexcept {
        assert(icol < N);
        xor_xcol(icol, zcol(icol));
        assert(check_symplecticity());
    }
    inline constexpr void do_hphaseh_l(const std::size_t& irow) noexcept {
        assert(irow < N);
        xor_xrow(irow, zrow(irow));
        assert(check_symplecticity());
    }
    inline constexpr void do_hphaseh_r(const std::size_t& icol) noexcept {
        assert(icol < N);
        xor_zcol(icol, xcol(icol));
        assert(check_symplecticity());
    }
    inline constexpr void do_cnot_l(const std::size_t& ictrl, const std::size_t& inot) noexcept {
        assert(ictrl < N && inot < N);
        xor_xrow(inot, xrow(ictrl));
        xor_zrow(ictrl, zrow(inot));
        assert(check_symplecticity());
    }
    inline constexpr void do_cnot_r(const std::size_t& ictrl, const std::size_t& inot) noexcept {
        assert(ictrl < N && inot < N);
        xor_xcol(ictrl, xcol(inot));
        xor_zcol(inot, zcol(ictrl));
        assert(check_symplecticity());
    }
    [[nodiscard]] inline constexpr BitSymplectic<N> hadamard_l(const std::size_t& irow) const noexcept {
        auto result = *this;
        result.do_hadamard_l(irow);
        return result;
    }
    [[nodiscard]] inline constexpr BitSymplectic<N> hadamard_r(const std::size_t& icol) const noexcept {
        auto result = *this;
        result.do_hadamard_r(icol);
        return result;
    }
    [[nodiscard]] inline constexpr BitSymplectic<N> phase_l(const std::size_t& irow) const noexcept {
        auto result = *this;
        result.do_phase_l(irow);
        return result;
    }
    [[nodiscard]] inline constexpr BitSymplectic<N> phase_r(const std::size_t& icol) const noexcept {
        auto result = *this;
        result.do_phase_r(icol);
        return result;
    }
    [[nodiscard]] inline constexpr BitSymplectic<N> hphaseh_l(const std::size_t& irow) const noexcept {
        auto result = *this;
        result.do_hphaseh_l(irow);
        return result;
    }
    [[nodiscard]] inline constexpr BitSymplectic<N> hphaseh_r(const std::size_t& icol) const noexcept {
        auto result = *this;
        result.do_hphaseh_r(icol);
        return result;
    }
    [[nodiscard]] inline constexpr BitSymplectic<N> cnot_l(const std::size_t& ictrl, const std::size_t& inot) const noexcept {
        auto result = *this;
        result.do_cnot_l(ictrl, inot);
        return result;
    }
    [[nodiscard]] inline constexpr BitSymplectic<N> cnot_r(const std::size_t& ictrl, const std::size_t& inot) const noexcept {
        auto result = *this;
        result.do_cnot_r(ictrl, inot);
        return result;
    }

    template <typename... Args>
    inline constexpr void do_mul_l(Args... args) noexcept {
        do_symplectic_multiply_l(*this, args...);
    }
    template <typename... Args>
    inline constexpr void do_mul_r(Args... args) noexcept {
        do_symplectic_multiply_r(*this, args...);
    }

    template <typename... Args>
    [[nodiscard]] inline constexpr BitSymplectic mul_l(Args... args) const noexcept {
        auto result = *this;
        result.do_mul_l(args...);
        return result;
    }
    template <typename... Args>
    [[nodiscard]] inline constexpr BitSymplectic mul_r(Args... args) const noexcept {
        auto result = *this;
        result.do_mul_r(args...);
        return result;
    }
    inline constexpr void do_swap_l(const std::size_t& i, const std::size_t& j) noexcept {
        const auto xi = xrow(i);
        const auto xj = xrow(j);
        set_xrow(i, xj);
        set_xrow(j, xi);
        const auto zi = zrow(i);
        const auto zj = zrow(j);
        set_zrow(i, zj);
        set_zrow(j, zi);
    }
    inline constexpr void do_swap_r(const std::size_t& i, const std::size_t& j) noexcept {
        const auto xi = xcol(i);
        const auto xj = xcol(j);
        set_xcol(i, xj);
        set_xcol(j, xi);
        const auto zi = zcol(i);
        const auto zj = zcol(j);
        set_zcol(i, zj);
        set_zcol(j, zi);
    }
    inline constexpr void do_swap(const std::size_t& i, const std::size_t& j) noexcept {
        const auto ones = count_ones();
        do_swap_l(i, j);
        do_swap_r(i, j);
        assert(count_ones() == ones);
    }
    [[nodiscard]] inline constexpr BitSymplectic<N> swap_l(const std::size_t& i, const std::size_t& j) const noexcept {
        auto result = *this;
        result.do_swap_l(i, j);
        return result;
    }
    [[nodiscard]] inline constexpr BitSymplectic<N> swap_r(const std::size_t& i, const std::size_t& j) const noexcept {
        auto result = *this;
        result.do_swap_r(i, j);
        return result;
    }
    [[nodiscard]] inline constexpr BitSymplectic<N> swap(const std::size_t& i, const std::size_t& j) const noexcept {
        auto result = *this;
        result.do_swap(i, j);
        return result;
    }
    [[nodiscard]] inline constexpr std::size_t count_ones() const noexcept { return xrows.count_ones() + zrows.count_ones(); }

    inline static const auto MASK_XCOLS = Bv<2 * N * N>(repeat_row<2 * N, N>(n_ones(N)));
    inline static const auto MASK_ZCOLS = Bv<2 * N * N>(repeat_row<2 * N, N>(n_ones(N) << N));
    [[nodiscard]] inline constexpr Bv<2 * N * N> kappa() const noexcept {
        auto xrows_xcols = xrows & MASK_XCOLS;
        auto xrows_zcols = xrows & MASK_ZCOLS;
        auto zrows_xcols = zrows & MASK_XCOLS;
        auto zrows_zcols = zrows & MASK_ZCOLS;
        auto first_bits = xrows_xcols | (xrows_zcols >> N) | zrows_xcols | (zrows_zcols >> N);
        assert((first_bits & MASK_XCOLS) == first_bits);
        auto second_bits = ((xrows_xcols << N) & zrows_zcols) ^ ((zrows_xcols << N) & xrows_zcols);
        assert((second_bits & MASK_ZCOLS) == second_bits);

        return first_bits | second_bits;
    }

    template <class Archive>
    void serialize(Archive& archive) {
        archive(xrows, zrows);
    }
};

template <std::size_t N>
auto format_as(BitSymplectic<N> matrix) {
    auto xvec = vw::ints(0ul, N) | vw::transform([matrix](size_t i) { return fmt::format("{}", matrix.xrow(i)); }) | rgs::to<std::vector>();
    auto zvec = vw::ints(0ul, N) | vw::transform([matrix](size_t i) { return fmt::format("{}", matrix.zrow(i)); }) | rgs::to<std::vector>();
    return fmt::format("X[{}] Z[{}]", fmt::join(xvec, " "), fmt::join(zvec, " "));
}
}  // namespace clfd

template <std::size_t N, typename GateT>
[[nodiscard]] inline constexpr clfd::BitSymplectic<N> operator*(const clfd::BitSymplectic<N>& matrix, const GateT& gate) noexcept {
    return matrix.mul_r(gate);
}
template <std::size_t N, typename GateT>
[[nodiscard]] inline constexpr clfd::BitSymplectic<N> operator*(const GateT& gate, const clfd::BitSymplectic<N>& matrix) noexcept {
    return matrix.mul_l(gate);
}

template <std::size_t N>
struct std::hash<clfd::BitSymplectic<N>> {  // NOLINT
    std::size_t operator()(const clfd::BitSymplectic<N>& s) const noexcept {
        const auto raw = s.as_raw();
        std::size_t h1 = std::hash<uint64_t>{}(raw.first);
        std::size_t h2 = std::hash<uint64_t>{}(raw.second);
        return h1 ^ (h2 << 1ul);
    }
};

// NOLINTBEGIN
TEST_FN(testing_hadamard) {
    auto matrix = clfd::BitSymplectic<5ul>::identity();
    matrix.do_hadamard_l(1ul);
    CHECK_NE(matrix, clfd::BitSymplectic<5ul>::identity());
    matrix.do_hadamard_l(1ul);
    CHECK_EQ(matrix, clfd::BitSymplectic<5ul>::identity());

    matrix.do_hadamard_r(1ul);
    matrix.do_hadamard_r(1ul);
    CHECK_EQ(matrix, clfd::BitSymplectic<5ul>::identity());

    matrix.do_hadamard_l(1ul);
    matrix.do_hadamard_l(2ul);
    matrix.do_hadamard_r(1ul);
    matrix.do_hadamard_r(2ul);
    CHECK_EQ(matrix, clfd::BitSymplectic<5ul>::identity());

    matrix.do_hadamard_r(4ul);
    matrix.do_hadamard_r(4ul);
    CHECK_EQ(matrix, clfd::BitSymplectic<5ul>::identity());
}

TEST_FN(testing_phase) {
    auto matrix = clfd::BitSymplectic<5ul>::identity();
    matrix.do_phase_l(4ul);
    CHECK_NE(matrix, clfd::BitSymplectic<5ul>::identity());
    matrix.do_phase_l(4ul);
    CHECK_EQ(matrix, clfd::BitSymplectic<5ul>::identity());

    matrix.do_phase_r(1ul);
    matrix.do_phase_r(1ul);
    CHECK_EQ(matrix, clfd::BitSymplectic<5ul>::identity());

    matrix.do_phase_l(1ul);
    matrix.do_phase_l(2ul);
    matrix.do_phase_r(1ul);
    matrix.do_phase_r(2ul);
    CHECK_EQ(matrix, clfd::BitSymplectic<5ul>::identity());

    matrix.do_phase_r(4ul);
    matrix.do_phase_r(4ul);
    CHECK_EQ(matrix, clfd::BitSymplectic<5ul>::identity());
}

TEST_FN(testing_cnot) {
    auto matrix = clfd::BitSymplectic<5ul>::identity();
    matrix.do_cnot_l(1ul, 4ul);
    CHECK_NE(matrix, clfd::BitSymplectic<5ul>::identity());
    matrix.do_cnot_l(1ul, 4ul);
    CHECK_EQ(matrix, clfd::BitSymplectic<5ul>::identity());

    matrix.do_cnot_r(2ul, 3ul);
    matrix.do_cnot_r(2ul, 3ul);
    CHECK_EQ(matrix, clfd::BitSymplectic<5ul>::identity());

    matrix.do_cnot_l(1ul, 0ul);
    matrix.do_cnot_l(2ul, 0ul);
    matrix.do_cnot_r(1ul, 0ul);
    matrix.do_cnot_r(2ul, 0ul);
    CHECK_EQ(matrix, clfd::BitSymplectic<5ul>::identity());

    matrix.do_cnot_l(1ul, 2ul);
    matrix.do_cnot_l(1ul, 3ul);
    matrix.do_cnot_l(1ul, 2ul);
    matrix.do_cnot_l(1ul, 3ul);
    CHECK_EQ(matrix, clfd::BitSymplectic<5ul>::identity());

    matrix.do_cnot_r(1ul, 2ul);
    matrix.do_cnot_r(1ul, 3ul);
    matrix.do_cnot_r(1ul, 2ul);
    matrix.do_cnot_r(1ul, 3ul);
    CHECK_EQ(matrix, clfd::BitSymplectic<5ul>::identity());

    matrix.do_hadamard_r(4ul);
    matrix.do_cnot_r(3ul, 4ul);
    matrix.do_hadamard_r(4ul);
    CHECK_NE(matrix, clfd::BitSymplectic<5ul>::identity());
    matrix.do_hadamard_r(3ul);
    matrix.do_cnot_r(4ul, 3ul);
    matrix.do_hadamard_r(3ul);
    CHECK_EQ(matrix, clfd::BitSymplectic<5ul>::identity());
}
// NOLINTEND