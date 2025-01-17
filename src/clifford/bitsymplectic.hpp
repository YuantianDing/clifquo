#pragma once

#include <doctest/doctest.h>
#include <fmt/core.h>
#include <fmt/ranges.h>
#include <bitset>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <utility>
#include "../utils/bitvec.hpp"
#include "../utils/ranges.hpp"
#include "../utils/test.hpp"
#include "fmt/base.h"
#include "range/v3/algorithm/all_of.hpp"
#include "range/v3/algorithm/none_of.hpp"

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

    [[nodiscard]] static inline constexpr BitSymplectic<N> null() noexcept { return BitSymplectic<N>(0, 0); }
    [[nodiscard]] static inline constexpr BitSymplectic<N> identity() noexcept {
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

    [[nodiscard]] static inline constexpr uint64_t fromArray(const std::array<Bv<N>, VecN> matrix) noexcept {
        auto&& result = BitSymplectic<N>(0ul, 0ul);
        for (auto i = 0z; i < N; i++) {
            result.set_xrow(i, matrix[i]);
            result.set_zrow(i, matrix[i + N]);
        }
        assert(result.check_symplecticity());
        return result;
    }
    [[nodiscard]] static inline constexpr uint64_t raw(Bv<N * VecN> xrows, Bv<N * VecN> zrows) noexcept {
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
};

template <std::size_t N>
auto format_as(BitSymplectic<N> matrix) {
    auto xvec = vw::ints(0ul, N) | vw::transform([matrix](size_t i) { return fmt::format("{}", matrix.xrow(i)); }) | rgs::to<std::vector>();
    auto zvec = vw::ints(0ul, N) | vw::transform([matrix](size_t i) { return fmt::format("{}", matrix.zrow(i)); }) | rgs::to<std::vector>();
    return fmt::format("X[{}] Z[{}]", fmt::join(xvec, " "), fmt::join(zvec, " "));
}

template <std::size_t N>
struct std::hash<BitSymplectic<N>> {  // NOLINT
    std::size_t operator()(const BitSymplectic<N>& s) const noexcept {
        const auto raw = s.as_raw();
        std::size_t h1 = std::hash<uint64_t>{}(raw.first);
        std::size_t h2 = std::hash<uint64_t>{}(raw.second);
        return h1 ^ (h2 << 1ul);
    }
};

// NOLINTBEGIN
TEST_FN(testing_hadamard) {
    auto matrix = BitSymplectic<5ul>::identity();
    matrix.do_hadamard_l(1ul);
    CHECK_NE(matrix, BitSymplectic<5ul>::identity());
    matrix.do_hadamard_l(1ul);
    CHECK_EQ(matrix, BitSymplectic<5ul>::identity());

    matrix.do_hadamard_r(1ul);
    matrix.do_hadamard_r(1ul);
    CHECK_EQ(matrix, BitSymplectic<5ul>::identity());

    matrix.do_hadamard_l(1ul);
    matrix.do_hadamard_l(2ul);
    matrix.do_hadamard_r(1ul);
    matrix.do_hadamard_r(2ul);
    CHECK_EQ(matrix, BitSymplectic<5ul>::identity());

    matrix.do_hadamard_r(4ul);
    matrix.do_hadamard_r(4ul);
    CHECK_EQ(matrix, BitSymplectic<5ul>::identity());
}

TEST_FN(testing_phase) {
    auto matrix = BitSymplectic<5ul>::identity();
    matrix.do_phase_l(4ul);
    CHECK_NE(matrix, BitSymplectic<5ul>::identity());
    matrix.do_phase_l(4ul);
    CHECK_EQ(matrix, BitSymplectic<5ul>::identity());

    matrix.do_phase_r(1ul);
    matrix.do_phase_r(1ul);
    CHECK_EQ(matrix, BitSymplectic<5ul>::identity());

    matrix.do_phase_l(1ul);
    matrix.do_phase_l(2ul);
    matrix.do_phase_r(1ul);
    matrix.do_phase_r(2ul);
    CHECK_EQ(matrix, BitSymplectic<5ul>::identity());

    matrix.do_phase_r(4ul);
    matrix.do_phase_r(4ul);
    CHECK_EQ(matrix, BitSymplectic<5ul>::identity());
}

TEST_FN(testing_cnot) {
    auto matrix = BitSymplectic<5ul>::identity();
    matrix.do_cnot_l(1ul, 4ul);
    CHECK_NE(matrix, BitSymplectic<5ul>::identity());
    matrix.do_cnot_l(1ul, 4ul);
    CHECK_EQ(matrix, BitSymplectic<5ul>::identity());

    matrix.do_cnot_r(2ul, 3ul);
    matrix.do_cnot_r(2ul, 3ul);
    CHECK_EQ(matrix, BitSymplectic<5ul>::identity());

    matrix.do_cnot_l(1ul, 0ul);
    matrix.do_cnot_l(2ul, 0ul);
    matrix.do_cnot_r(1ul, 0ul);
    matrix.do_cnot_r(2ul, 0ul);
    CHECK_EQ(matrix, BitSymplectic<5ul>::identity());

    matrix.do_cnot_l(1ul, 2ul);
    matrix.do_cnot_l(1ul, 3ul);
    matrix.do_cnot_l(1ul, 2ul);
    matrix.do_cnot_l(1ul, 3ul);
    CHECK_EQ(matrix, BitSymplectic<5ul>::identity());

    matrix.do_cnot_r(1ul, 2ul);
    matrix.do_cnot_r(1ul, 3ul);
    matrix.do_cnot_r(1ul, 2ul);
    matrix.do_cnot_r(1ul, 3ul);
    CHECK_EQ(matrix, BitSymplectic<5ul>::identity());

    matrix.do_hadamard_r(4ul);
    matrix.do_cnot_r(3ul, 4ul);
    matrix.do_hadamard_r(4ul);
    CHECK_NE(matrix, BitSymplectic<5ul>::identity());
    matrix.do_hadamard_r(3ul);
    matrix.do_cnot_r(4ul, 3ul);
    matrix.do_hadamard_r(3ul);
    CHECK_EQ(matrix, BitSymplectic<5ul>::identity());
}
// NOLINTEND