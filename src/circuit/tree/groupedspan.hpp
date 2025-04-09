#pragma once

#include <cstddef>
#include <span>
#include "../../utils/fmt.hpp"
#include "../../utils/ranges.hpp"
#include "range/v3/range_fwd.hpp"

namespace circ::tree {

class GroupedSpan : rgs::view_base {
    using byte = std::byte;

    const byte* _Nonnull bptr;
    const byte* _Nonnull eptr;

   public:
    inline GroupedSpan() : bptr(nullptr), eptr(nullptr) {}
    inline explicit GroupedSpan(const byte* _Nonnull begin, const byte* _Nonnull end) : bptr(begin), eptr(end) {}

    [[nodiscard]] inline static GroupedSpan from(std::span<const byte> span) { return GroupedSpan(span.data(), span.data() + span.size()); }

    [[nodiscard]] inline constexpr explicit operator bool() const noexcept { return bool(bptr) && bptr != eptr; }
    [[nodiscard]] inline constexpr bool last() const noexcept {
        assert(*this);
        return bptr + 1 + std::size_t(*bptr) >= eptr;
    }

    [[nodiscard]] inline constexpr bool operator==(const GroupedSpan& other) const noexcept { return bptr == other.bptr && eptr == other.eptr; }
    [[nodiscard]] inline constexpr bool operator!=(const GroupedSpan& other) const noexcept { return !(*this == other); }

    using value_type = std::span<const byte>;
    using difference_type = std::ptrdiff_t;
    [[nodiscard]] inline value_type operator*() const noexcept {
        assert(*this);
        auto inner_size = *bptr;
        auto* ptr = bptr + 1;
        return std::span(ptr, std::size_t(inner_size));
    }

    inline GroupedSpan& operator++() noexcept {
        assert(*this);
        auto inner_size = *bptr;
        bptr = bptr + 1 + std::size_t(inner_size);
        assert(bptr <= eptr);
        return *this;
    }

    // NOLINTNEXTLINE
    IMPL_FORWARD_ITER(GroupedSpan);

    [[nodiscard]] inline GroupedSpan with_size_unchecked(std::size_t size) const noexcept { return GroupedSpan(bptr, bptr + size); }
    [[nodiscard]] inline GroupedSpan skip_size_unchecked(std::size_t size) const noexcept { return GroupedSpan(bptr + size, eptr); }

    [[nodiscard]] inline std::size_t size() const noexcept { return eptr - bptr; }
    [[nodiscard]] inline std::size_t count() const noexcept { return rgs::distance(begin(), end()); }
    [[nodiscard]] inline std::span<const byte> span() const noexcept { return std::span(bptr, size()); }
};

static_assert(std ::ranges ::range<GroupedSpan>);
static_assert(rgs ::range<GroupedSpan>);
static_assert(rgs ::viewable_range<GroupedSpan>);

class GroupedSpanIter {
    using byte = std::byte;

   public:
    GroupedSpan parent;
    std::span<const byte> current;

    inline explicit GroupedSpanIter(GroupedSpan parent) : parent(parent), current(parent ? *parent : std::span<const byte>{}) {}

    [[nodiscard]] inline constexpr bool last() const noexcept { return current.size() == 1 && parent.last(); }
    [[nodiscard]] inline constexpr bool finished() const noexcept { return !bool(parent); }
    [[nodiscard]] inline constexpr explicit operator bool() const noexcept { return current.size() != 0; }

    [[nodiscard]] inline constexpr bool operator==(const GroupedSpanIter& other) const noexcept {
        return parent == other.parent && current.data() == other.current.data();
    }
    [[nodiscard]] inline constexpr bool operator!=(const GroupedSpanIter& other) const noexcept { return !(*this == other); }

    using value_type = byte;
    [[nodiscard]] inline byte operator[](std::size_t i) const noexcept {
        assert(*this && i < current.size());
        return current[i];
    }

    [[nodiscard]] inline constexpr byte operator*() const noexcept {
        assert(*this);
        return current[0];
    }

    inline GroupedSpanIter& operator++() noexcept {
        assert(*this);
        current = current.subspan(1);
        return *this;
    }

    inline constexpr void next_span(bool check = true) noexcept {
        if (check) { assert(!(*this)); }
        parent++;
        if (parent) { current = *parent; }
    }

    // NOLINTNEXTLINE
    IMPL_FORWARD_ITER(GroupedSpanIter);
};

class GroupedSpanBuilder {
    using byte = std::byte;
    std::vector<byte> buffer;
    std::size_t size_index = 0;

   public:
    inline GroupedSpanBuilder() = default;
    inline GroupedSpanBuilder(const GroupedSpanBuilder&) = delete;
    inline GroupedSpanBuilder(GroupedSpanBuilder&&) = default;

    inline GroupedSpanBuilder& operator=(const GroupedSpanBuilder&) = delete;
    inline GroupedSpanBuilder& operator=(GroupedSpanBuilder&&) = default;
    inline ~GroupedSpanBuilder() = default;

    [[nodiscard]] inline std::span<const byte> span() const noexcept { return std::span(buffer); }
    [[nodiscard]] inline std::size_t size() const noexcept { return buffer.size(); }

    [[nodiscard]] inline std::vector<byte> build() {
        if (buffer.size() == 0) { return {}; }
        assert(size_index < buffer.size());
        buffer[size_index] = byte(buffer.size() - size_index - 1);
        size_index = 0;
        buffer.shrink_to_fit();
        return std::move(buffer);
    }
    inline void new_span() {
        if (buffer.size() > 0) {
            assert(size_index < buffer.size());
            buffer[size_index] = byte(buffer.size() - size_index - 1);
        }
        buffer.push_back(byte(0));
        size_index = buffer.size() - 1;
    }

    inline void add(byte b) {
        assert(buffer.size() > 0);
        buffer.push_back(b);
    }
};

inline std::string format_as(GroupedSpan iter) {
    return fmt::format("[{}]", fmt::join(iter | vw::transform([](auto a) { return fmt::format("({})", fmt::join(a, " ")); }), ", "));
}
inline std::byte format_as(GroupedSpanIter iter) {
    return *iter;
}

};  // namespace circ::tree
