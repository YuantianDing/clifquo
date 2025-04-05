#pragma once

#include <cassert>
#include <cstddef>
#include <variant>
#include <vector>
#include "fmt/base.h"
#include "fmt/format.h"
#include "range/v3/numeric/accumulate.hpp"
#include "ranges.hpp"
#include "test.hpp"

template <typename NodeT>
class LinkedArray;

template <typename T>
struct LinkedArrayNode {
    std::variant<T, LinkedArray<LinkedArrayNode>> inner;

    [[nodiscard]] inline constexpr const T& operator*() const noexcept { return std::get<T>(inner); }
    [[nodiscard]] inline constexpr T& operator*() noexcept { return std::get<T>(inner); }

    [[nodiscard]] inline constexpr LinkedArray<LinkedArrayNode> next_jump() const noexcept {
        if (auto next = std::get_if<LinkedArray<LinkedArrayNode>>(&inner)) {
            return *next;
        } else {
            return LinkedArray<LinkedArrayNode>();
        }
    }
    [[nodiscard]] inline constexpr bool is_sentinel() const noexcept {
        if (auto next = std::get_if<LinkedArray<LinkedArrayNode>>(&inner)) {
            return next->is_null();
        } else {
            return false;
        }
    }

    static LinkedArrayNode sentinel() noexcept { return LinkedArrayNode(LinkedArray<LinkedArrayNode>()); }
    static LinkedArrayNode jump_node(LinkedArray<LinkedArrayNode> jump) noexcept { return LinkedArrayNode(jump); }
    explicit LinkedArrayNode(const T& inner) noexcept : inner(inner) {}
    explicit LinkedArrayNode(T&& inner) noexcept : inner(inner) {}

   private:
    explicit LinkedArrayNode(LinkedArray<LinkedArrayNode> inner) noexcept : inner(inner) {}
};

template <typename NodeT>
class LinkedArrayBuilder;

template <typename NodeT>
class LinkedArray {
    NodeT* _Nullable node;

    constexpr explicit LinkedArray(NodeT* _Nullable node, bool checked = true) noexcept : node(node) {
        if (node && checked) { assert(node->next_jump().is_null()); }
    }

   public:
    constexpr LinkedArray() noexcept : node(nullptr) {}

    [[nodiscard]] inline constexpr bool operator==(const LinkedArray& other) const noexcept { return node == other.node; }
    [[nodiscard]] inline constexpr bool operator!=(const LinkedArray& other) const noexcept { return node != other.node; }

    [[nodiscard]] inline constexpr explicit operator bool() const noexcept { return node; }  // NOLINT
    [[nodiscard]] inline constexpr bool nonnull() const noexcept { return node; }            // NOLINT
    [[nodiscard]] inline constexpr bool is_null() const noexcept { return !node; }           // NOLINT
    [[nodiscard]] inline constexpr bool accessible() const noexcept { return node && !node->is_sentinel(); }
    using iterator = LinkedArray;
    using value_type = LinkedArray;
    using difference_type = std::ptrdiff_t;

    [[nodiscard]] inline constexpr NodeT* _Nullable raw_unchecked() const noexcept { return node; }
    [[nodiscard]] inline constexpr NodeT* _Nonnull raw() const noexcept {
        assert(node && accessible());
        return node;
    }
    [[nodiscard]] inline constexpr const LinkedArray& operator*() const noexcept {
        assert(accessible());
        return *this;
    }

    struct Sentinel {
        [[nodiscard]] inline constexpr bool operator==(const LinkedArray& other) const noexcept {
            return other.is_null() || other.node->is_sentinel();
        }
    };
    struct Until {
        LinkedArray end;
        [[nodiscard]] inline constexpr bool operator==(const LinkedArray& other) const noexcept {
            return other == end || other.is_null() || other.node->is_sentinel();
        }
    };

    [[nodiscard]] inline constexpr LinkedArray begin() const noexcept { return *this; }
    [[nodiscard]] inline constexpr Sentinel end() const noexcept { return {}; }
    [[nodiscard]] inline constexpr Until end_at(const LinkedArray& node) const noexcept { return {node}; }

    inline constexpr LinkedArray& operator++() noexcept {
        assert(accessible());
        node++;
        if (auto jump = node->next_jump(); jump.nonnull()) { *this = jump; }
        return *this;
    }
    [[nodiscard]] inline constexpr LinkedArray operator++(int) noexcept {  // NOLINT
        auto tmp = *this;
        node++;
        return tmp;
    }
    [[nodiscard]] inline constexpr std::size_t count_segmentation() noexcept {
        std::size_t count = 1;
        for (auto it = begin(); it != end(); ++it) {
            assert(it.accessible());
            auto it2 = it;
            if ((it2.node + 1)->next_jump().nonnull()) { count++; }
        }
        return count;
    }

    template <typename NodeT2>
    friend class LinkedArrayBuilder;

    template <typename Rng>
    [[nodiscard]] inline constexpr static LinkedArray<NodeT> make(Rng&& rng, std::size_t segment_size, LinkedArray<NodeT> jump = {}) noexcept {
        assert(segment_size > 0);
        LinkedArrayBuilder<NodeT> builder(segment_size, jump);
        for (auto&& item : rng) {
            builder.emplace_back(item);
        }
        return builder.build();
    }
};

template <typename NodeT>
class LinkedArrayBuilder {
    std::size_t segment_size;
    LinkedArray<NodeT> jump;
    std::size_t position;
    NodeT* _Nullable result;
    NodeT* _Nullable allocation;

   public:
    inline constexpr explicit LinkedArrayBuilder(std::size_t segment_size, LinkedArray<NodeT> jump = {}) noexcept
        : jump(jump), segment_size(segment_size) {
        assert(segment_size > 0);
        position = 0;
        allocation = nullptr;
        result = allocation;
    }

    template <typename... Args>
    inline constexpr NodeT& emplace_back(Args... args) noexcept {
        if (!allocation) {
            position = 0;
            allocation = reinterpret_cast<NodeT* _Nonnull>(operator new((segment_size + 1) * sizeof(NodeT)));  // NOLINT
            result = allocation;
        }
        if (position == segment_size) {
            auto new_allocation = reinterpret_cast<NodeT* _Nonnull>(operator new((segment_size + 1) * sizeof(NodeT)));  // NOLINT
            new (allocation + position) NodeT(NodeT::jump_node(LinkedArray(new_allocation, false)));
            allocation = new_allocation;
            position = 0;
        }
        new (allocation + position) NodeT(args...);
        position += 1;
        return allocation[position - 1];
    }

    [[nodiscard]] inline constexpr LinkedArray<NodeT> build() noexcept {
        if (jump.accessible()) {
            if (!allocation) { return jump; }
            new (allocation + position) NodeT(NodeT::jump_node(LinkedArray(jump.node)));
        } else {
            if (!allocation) { return LinkedArray<NodeT>(); }
            new (allocation + position) NodeT(NodeT::sentinel());
        }
        jump = LinkedArray<NodeT>(result);
        position = 0;
        allocation = nullptr;
        result = allocation;
        return jump;
    }
};

// static_assert(std::forward_iterator<LinkedArray<LinkedArrayNode<int>>>);
static_assert(std::sentinel_for<LinkedArray<LinkedArrayNode<int>>::Sentinel, LinkedArray<LinkedArrayNode<int>>::iterator>);
static_assert(std::ranges::range<LinkedArray<LinkedArrayNode<int>>>);
static_assert(rgs::range<LinkedArray<LinkedArrayNode<int>>>);

// NOLINTBEGIN
TEST_FN(linked_array) {
    auto total = 100ul;
    for (auto i : vw::ints(0ul, 10ul) | vw::transform([](auto i) { return 1ul << i; })) {
        auto array = LinkedArray<LinkedArrayNode<int>>::make(vw::ints(0, 100), i);
        CHECK_EQ(array.count_segmentation(), total);
        total = (total + 1) / 2;
        CHECK_EQ(rgs::accumulate(array | vw::transform([](auto arr) { return **arr.raw(); }), 0), 4950);
    }
}
// NOLINTEND