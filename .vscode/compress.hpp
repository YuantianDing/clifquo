#pragma once

#include <cassert>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <span>
#include "range/v3/range_fwd.hpp"
#include "ranges.hpp"

class CompressedIter : public rgs::view_base {
    using byte = std::byte;
    using value_t = std::size_t;
    byte* _Nonnull bptr;
    byte* _Nonnull eptr;
    value_t value = 0;

    inline constexpr void read() {
        value = 0;
        for (value_t i = 0; assert(i < 9), true; i++) {
            const auto v = *bptr;
            value <<= value_t(7);
            value |= value_t(v & byte{0b01111111u});
            if ((v & byte{0b10000000u}) == byte{0}) { break; }
            bptr++;
        }
        assert(valid());
    }

   public:
    inline CompressedIter() : bptr(nullptr), eptr(nullptr) {}

    inline explicit CompressedIter(byte* _Nonnull begin, byte* _Nonnull end) : bptr(begin), eptr(end) {
        if (*this) { read(); }
    }
    [[nodiscard]] inline static CompressedIter from(std::span<byte> span) { return CompressedIter(span.data(), span.data() + span.size()); }

    [[nodiscard]] inline constexpr explicit operator bool() const noexcept { return bool(bptr) && bptr != eptr; }
    [[nodiscard]] inline constexpr bool valid() const noexcept { return !bool(*this) || (*bptr & byte(0b10000000)) == byte(0); }

    [[nodiscard]] inline constexpr bool operator==(const CompressedIter& other) const noexcept { return bptr == other.bptr && eptr == other.eptr; }
    [[nodiscard]] inline constexpr bool operator!=(const CompressedIter& other) const noexcept { return !(*this == other); }

    using iterator = CompressedIter;
    using value_type = value_t;
    using difference_type = std::ptrdiff_t;
    inline constexpr CompressedIter& operator++() noexcept {
        if (*this) {
            assert(valid());
            bptr++;
            if (*this) { read(); }
        }
        return *this;
    }
    [[nodiscard]] inline constexpr value_t operator*() const noexcept {
        assert(*this && valid());
        return value;
    }

    // NOLINTNEXTLINE
    IMPL_FORWARD_ITER(CompressedIter);
};

template <typename OutputIt>
class CompressingIter {
    using byte = std::byte;
    using value_t = std::size_t;
    OutputIt out;

   public:
    inline explicit CompressingIter(OutputIt it) : out(it) {}

    using iterator = CompressingIter;
    using value_type = value_t;
    using difference_type = std::ptrdiff_t;

    [[nodiscard]] inline constexpr CompressingIter& operator=(value_t value) noexcept {
        value_t v = value;
        if (v == 0) {
            *out = byte(0);
            out++;
            return *this;
        } else {
            value_t v2 = 0;
            while (v > 0) {
                v2 <<= value_t(7);
                v2 |= v & value_t(0b01111111u);
                v >>= value_t(7);
            }

            while (v2 > 0) {
                auto b = byte(v2 & 0b01111111u);
                v2 >>= value_t(7);
                if (v2 > 0) { b |= byte(0b10000000u); }
                *out = b;
                out++;
            }
            return *this;
        }
    }

    // NOLINTNEXTLINE
    IMPL_OUT_ITER(CompressingIter);
};

// NOLINTBEGIN
TEST_FN(compressed_iter) {
    std::vector<std::size_t> indices{7};
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<std::size_t> dis(1, 10000);

    for (int i = 0; i < 2; ++i) {
        auto a = indices.back();
        indices.push_back(a + dis(gen));
    }

    std::vector<std::byte> compressed;
    auto it = CompressingIter(std::back_inserter(compressed));
    std::copy(indices.begin(), indices.end(), it);

    auto result = CompressedIter::from(compressed) | rgs::to<std::vector>();
    CHECK_EQ(result, indices);
}
// NOLINTEND