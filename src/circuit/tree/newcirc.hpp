#pragma once
#include <algorithm>
#include <compare>
#include <set>
#include "fmt/base.h"
#include "groupedspan.hpp"
#include "tree.hpp"

namespace circ::tree {

class NewCircIter : rgs::view_base {
    using byte = std::byte;
    using ordering = std::strong_ordering;
    Tree::Iter start;
    Tree::Iter init;
    Tree::Iter last;
    GroupedSpanBuilder* builder = nullptr;

   public:
    inline explicit NewCircIter() { assert(false); }
    inline explicit NewCircIter(Tree& tree, GroupedSpanBuilder* builder = nullptr) : start(tree), init(tree), last(tree), builder(builder) {
        if (init && bool(builder)) { builder->new_span(); }
        maintain();
    }

    [[nodiscard]] inline constexpr explicit operator bool() const noexcept { return bool(init); }

    [[nodiscard]] inline ordering order() const noexcept {
        assert(init && last);
        assert(init.nlayers() == last.nlayers());
        for (auto i : vw::ints(0ul, init.nlayers() - 1)) {
            if (init[i + 1] > last[i]) {
                return ordering::greater;
            } else if (init[i + 1] < last[i]) {
                return ordering::less;
            }
        }
        return ordering::equal;
    }

   private:
    inline void maintain() noexcept {
        auto ord = ordering::equal;
        do {
            if (!*this) { return; }
            ord = order();
            if (ord == ordering::greater) {
                increment_last(true);
            } else if (ord == ordering::less) {
                increment_init();
            }
        } while (ord != ordering::equal);
    }
    inline void increment_last(bool parent = false) noexcept {
        assert(init && last);

        if (parent) {
            last.next_parent();
        } else {
            ++last;
        }

        if (!last) {
            last = start;
            auto first = init[0];
            while (init && init[0] == first) {
                ++init;
                if (init && bool(builder)) { builder->new_span(); }
            }
        }
    }
    inline void increment_init() noexcept {
        assert(init && last);
        auto first = init[0];
        ++init;
        if (init && bool(builder)) { builder->new_span(); }

        if (init && init[0] != first) { last = start; }
    }

   public:
    [[nodiscard]] inline auto bytes() const noexcept {
        assert(init && last);
        return vw::concat(*init | vw::transform([](auto i) { return *i; }), vw::single(*(*last).back()));
    }
    using value_type = std::vector<byte>;
    using difference_type = std::ptrdiff_t;
    using iterator = NewCircIter;

    [[nodiscard]] inline std::vector<byte> operator*() const noexcept { return bytes() | rgs::to<std::vector>(); }
    [[nodiscard]] inline NewCircIter& operator++() noexcept {
        assert(init && last);
        increment_last(false);
        maintain();
        return *this;
    }

    // NOLINTNEXTLINE
    IMPL_FORWARD_ITER(NewCircIter);

    [[nodiscard]] inline std::string fmt() const noexcept { return fmt::format("({})", fmt::join(bytes(), " ")); }
};

inline std::string format_as(const NewCircIter& iter) {
    return iter.fmt();
}

static_assert(std ::ranges ::range<NewCircIter>);
static_assert(rgs ::range<NewCircIter>);
static_assert(rgs ::viewable_range<NewCircIter>);

};  // namespace circ::tree

// NOLINTBEGIN
TEST_FN(new_circ) {
    circ::tree::Tree tree;

    for (auto i : vw::ints(0ul, 2ul)) {
        circ::tree::GroupedSpanBuilder builder;
        for (auto j : vw::ints(0ul, 1ul << (2 * i))) {
            builder.new_span();
            builder.add(std::byte(0));
            builder.add(std::byte(1));
            builder.add(std::byte(2));
            builder.add(std::byte(3));
        }
        tree.add_layer(builder.build());
    }
    CHECK_EQ(rgs::distance(tree.begin(), tree.end()), 16);

    auto counter = 0;
    for (auto it = tree.begin(); it != tree.end(); it.next_parent()) {
        ++counter;
    }
    CHECK_EQ(counter, 4);

    circ::tree::NewCircIter iter(tree);

    CHECK_EQ(rgs::distance(iter.begin(), iter.end()), 64);
}

TEST_FN(new_circ2) {
    circ::tree::Tree tree;

    for (auto i : vw::ints(0ul, 5ul)) {
        circ::tree::GroupedSpanBuilder builder;
        for (auto j : vw::ints(0ul, 1ul << (2 * i))) {
            builder.new_span();
            builder.add(std::byte(0));
            builder.add(std::byte(1));
            builder.add(std::byte(2));
            builder.add(std::byte(3));
        }
        tree.add_layer(builder.build());
    }
    CHECK_EQ(rgs::distance(tree.begin(), tree.end()), 1024);

    auto counter = 0;
    for (auto it = tree.begin(); it != tree.end(); it.next_parent()) {
        ++counter;
    }
    CHECK_EQ(counter, 256);

    circ::tree::NewCircIter iter(tree);

    CHECK_EQ(rgs::distance(iter.begin(), iter.end()), 4096);
}

TEST_FN(new_circ3) {
    circ::tree::Tree tree;

    circ::tree::GroupedSpanBuilder builder;
    for (auto i : vw::ints(0ul, 5ul)) {
        for (auto j : vw::ints(0ul, 1ul << (2 * i))) {
            builder.new_span();
            builder.add(std::byte(0));
            builder.add(std::byte(1));
            builder.add(std::byte(2));
            builder.add(std::byte(3));
        }
        tree.add_layer(builder.build());
    }
    CHECK_EQ(rgs::distance(tree.begin(), tree.end()), 1024);

    for (auto j : vw::ints(0ul, 1024ul)) {
        builder.new_span();
    }
    tree.add_layer(builder.build());
    CHECK_EQ(rgs::distance(tree.begin(), tree.end()), 0);

    auto counter = 0;
    for (auto it = tree.begin(); it != tree.end(); it.next_parent()) {
        ++counter;
    }
    CHECK_EQ(counter, 0);

    circ::tree::NewCircIter iter(tree);

    CHECK_EQ(rgs::distance(iter.begin(), iter.end()), 0);
}

TEST_FN(new_circ4) {
    for (auto seed : vw::ints(0, 100)) {
        circ::tree::Tree tree;
        std::mt19937 gen(seed);
        std::uniform_int_distribution<std::size_t> dis(0, 4);

        circ::tree::GroupedSpanBuilder builder;
        builder.new_span();
        builder.add(std::byte(0));
        builder.add(std::byte(1));
        builder.add(std::byte(2));
        builder.add(std::byte(3));
        tree.add_layer(std::move(builder.build()));

        for (auto i : vw::ints(0, 5)) {
            auto counter = 0;
            for (auto node : tree) {
                builder.new_span();
                for (auto i : vw::ints(0ul, dis(gen))) {
                    builder.add(std::byte(i));
                    counter++;
                }
            }
            tree.add_layer(std::move(builder.build()));
            CHECK_EQ(rgs::distance(tree.begin(), tree.end()), counter);
        }
        circ::tree::NewCircIter iter(tree, &builder);
        std::set<std::vector<std::byte>> set1;
        for (auto&& v : iter) {
            set1.emplace(v);
        }

        std::set<std::vector<std::byte>> set2;
        auto tree_vec = tree | vw::transform([](auto&& n) { return n | vw::transform([](auto i) { return *i; }) | rgs::to<std::vector>(); }) |
                        rgs::to<std::vector>();
        for (auto node1 : tree_vec) {
            for (auto node2 : tree_vec) {
                if (std::equal(node1.begin(), --node1.end(), ++node2.begin(), node2.end())) {
                    node2.push_back(node1.back());
                    set2.emplace(node2);
                }
            }
        }

        CHECK_EQ(set1.size(), set2.size());
        CHECK_EQ(set1, set2);

        tree.add_layer(std::move(builder.build()));
    }
}
// NOLINTEND