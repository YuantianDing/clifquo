#pragma once

#include <doctest/doctest.h>
#include <bit>
#include <bitset>
#include <cassert>
#include <cstddef>
#include <cstdint>

inline constexpr std::size_t ceil_div(std::size_t dividee, std::size_t divider) {
    return (dividee + divider - 1) / divider;
}

inline constexpr uint64_t n_ones(uint64_t n) noexcept {
    return (1ul << n) - 1ul;
}

template <std::size_t N>
class Bv {
    static_assert(N <= 64ul);
    uint64_t data;

   public:
    static const uint64_t MASK = (1ul << N) - 1ul;
    inline explicit constexpr Bv() noexcept : data(0ul) {}
    inline explicit constexpr Bv(uint64_t data) noexcept : data(data) { assert(data <= MASK); }
    // Bv(const Bv<N>& o) : data(o.data) {}
    [[nodiscard]] static inline constexpr Bv<N> zero() noexcept { return Bv<N>(0ul); }
    [[nodiscard]] static inline constexpr Bv<N> ones() noexcept { return Bv<N>(MASK); }
    [[nodiscard]] static inline constexpr Bv<N> random() noexcept { return Bv<N>(uint64_t(std::rand()) & MASK); }

    [[nodiscard]] inline constexpr bool operator==(const Bv<N>& other) const = default;
    [[nodiscard]] inline constexpr bool operator!=(const Bv<N>& other) const = default;
    [[nodiscard]] inline constexpr auto operator<=>(const Bv<N>& other) const = default;

    [[nodiscard]] inline constexpr bool none() const noexcept { return *this == Bv::zero(); }
    [[nodiscard]] inline constexpr bool any() const noexcept { return *this != Bv::zero(); }
    [[nodiscard]] inline constexpr bool all() const noexcept { return *this == Bv::ones(); }

    [[nodiscard]] inline constexpr Bv<N> operator&(const Bv<N>& other) const noexcept { return Bv<N>(data & other.data); }
    [[nodiscard]] inline constexpr Bv<N> operator|(const Bv<N>& other) const noexcept { return Bv<N>(data | other.data); }
    [[nodiscard]] inline constexpr Bv<N> operator^(const Bv<N>& other) const noexcept { return Bv<N>(data ^ other.data); }
    [[nodiscard]] inline constexpr Bv<N> setminus(const Bv<N>& other) const noexcept { return Bv<N>(data & ~other.data); }
    [[nodiscard]] inline constexpr Bv<N> operator~() const noexcept { return Bv<N>(~data & MASK); }

    inline constexpr Bv<N>& operator&=(const Bv<N>& other) noexcept {
        data &= other.data;
        return *this;
    }
    inline constexpr Bv<N>& operator|=(const Bv<N>& other) noexcept {
        data |= other.data;
        return *this;
    }
    inline constexpr Bv<N>& operator^=(const Bv<N>& other) noexcept {
        data ^= other.data;
        return *this;
    }
    inline constexpr Bv<N>& operator<<=(std::size_t shift) noexcept {
        data <<= shift;
        data &= MASK;
        return *this;
    }
    inline constexpr Bv<N>& operator>>=(std::size_t shift) noexcept {
        data >>= shift;
        return *this;
    }

    [[nodiscard]] inline constexpr Bv<N> operator<<(std::size_t shift) const noexcept { return Bv<N>((data << shift) & MASK); }
    [[nodiscard]] inline constexpr Bv<N> operator>>(std::size_t shift) const noexcept { return Bv<N>(data >> shift); }

    [[nodiscard]] inline constexpr bool operator[](std::size_t i) const noexcept { return ((data >> i) & 1ul) != 0; }
    [[nodiscard]] inline constexpr Bv<N> xor_at(std::size_t i, bool v) const noexcept {
        assert(i < N);
        return v ? Bv<N>(data ^ (1ul << i)) : *this;
    }
    [[nodiscard]] inline constexpr Bv<N> update(std::size_t i, bool v) const noexcept {
        assert(i < N);
        return v ? Bv<N>(data | (1ul << i)) : Bv<N>(data & ~(1ul << i));
    }
    [[nodiscard]] inline constexpr Bv<N> flip(std::size_t i) const noexcept {
        assert(i < N);
        return data ^ (1ul << i);
    }

    template <std::size_t M>
    [[nodiscard]] inline static constexpr Bv<N> slice(Bv<M> orig, std::size_t start) noexcept {
        assert(start + N <= M);
        return Bv<N>((orig.uint() >> start) & Bv<N>::MASK);
    }
    [[nodiscard]] inline constexpr Bv<N> clean_slice(std::size_t start, std::size_t end) const noexcept {
        assert(start <= end && end <= N);
        auto mask = n_ones(end) - n_ones(start);
        return Bv<N>((data & ~mask));
    }
    template <std::size_t M>
    [[nodiscard]] inline constexpr Bv<N> xor_slice(std::size_t start, Bv<M> value) const noexcept {
        assert(start + M <= N);
        return Bv<N>(data ^ (value.uint() << start));
    }
    template <std::size_t M>
    [[nodiscard]] inline constexpr Bv<N> update_slice(std::size_t start, Bv<M> value) const noexcept {
        assert(start + M <= N);
        return Bv<N>(clean_slice(start, start + M).uint() | (value.uint() << start));
    }
    template <std::size_t M>
    [[nodiscard]] inline constexpr Bv<N + M> concat(const Bv<M>& other) const noexcept {
        return Bv<N + M>(other.uint() << N | data);
    }

    [[nodiscard]] inline constexpr std::size_t count_ones() const noexcept { return std::bitset<N>(data).count(); }
    [[nodiscard]] inline constexpr bool dot(Bv<N> vec) const noexcept { return (*this & vec).count_ones() % 2 != 0; }

    [[nodiscard]] inline constexpr uint64_t uint() const noexcept { return data; }
    [[nodiscard]] inline constexpr int countr_zero() const noexcept { return std::countr_zero(data); }
    [[nodiscard]] inline constexpr int countr_one() const noexcept { return std::countr_one(data); }
    [[nodiscard]] inline constexpr int firstr_one() const noexcept { return std::countr_zero(data); }
    [[nodiscard]] inline constexpr int firstr_zero() const noexcept { return std::countr_one(data); }
    [[nodiscard]] inline constexpr int countl_zero() const noexcept { return std::countl_zero(data); }
    [[nodiscard]] inline constexpr int countl_one() const noexcept { return std::countl_one(data); }
    [[nodiscard]] inline constexpr int firstl_one() const noexcept { return 64 - 1 - std::countl_zero(data); }
    [[nodiscard]] inline constexpr int firstl_zero() const noexcept { return 64 - 1 - std::countl_one(data); }

    template <class Archive>
    void serialize(Archive& archive) {
        archive(data);
    }
};

template <std::size_t N>
auto format_as(Bv<N> value) {
    return std::bitset<N>(value.uint()).to_string();
}
