#pragma once

#include <cereal/cereal.hpp>
#include <cereal/types/vector.hpp>
#include <cstddef>
#include <iterator>
#include <ranges>
#include <span>
#include <vector>
#include "../../utils/fmt.hpp"
#include "groupedspan.hpp"
#include "range/v3/numeric/accumulate.hpp"

namespace circ::tree {

class Tree : public rgs::view_base {
    using byte = std::byte;

   public:
    std::vector<std::vector<byte>> layers;

    class Iter {
        std::vector<tree::GroupedSpanIter> indices;

       public:
        Iter() { assert(false); }

        explicit Iter(const Tree& tree) {
            indices = tree.layers | vw::transform([](auto& layer) { return GroupedSpanIter(GroupedSpan::from(layer)); }) | rgs::to<std::vector>();
            maintain();
        }
        explicit Iter(const Tree& tree, std::size_t nlayers) {
            indices = tree.layers | vw::transform([](auto& layer) { return GroupedSpanIter(GroupedSpan::from(layer)); }) | vw::take(nlayers) |
                      rgs::to<std::vector>();
            maintain();
        }

        [[nodiscard]] inline constexpr byte operator[](std::size_t i) const noexcept { return *indices[i]; }
        [[nodiscard]] inline bool check() {
            if (*this) {
                return rgs::all_of(indices, [](auto&& it) { return bool(it); });
            }
            return true;
        }
        [[nodiscard]] inline constexpr bool operator==(const Iter& other) const noexcept {
            if (indices.back() == other.indices.back()) { assert(indices == other.indices); }
            return indices.back() == other.indices.back();
        }
        [[nodiscard]] inline constexpr bool operator!=(const Iter& other) const noexcept { return !(*this == other); }
        [[nodiscard]] inline constexpr explicit operator bool() const noexcept { return !indices.empty() && bool(indices.front()); }

       private:
        inline void maintain() {
            while (!maintain_check()) {}
            assert(check());
        }
        [[nodiscard]] inline constexpr bool maintain_check() {
            if (!*this) { return true; }
            for (auto i : vw::ints(0ul, indices.size())) {  // NOLINT
                if (!indices[i]) {
                    assert(!indices[i].finished());
                    indices[i].next_span();
                    increment(i);
                    return false;
                }
            }
            return true;
        }
        inline constexpr void increment(std::size_t lt_layer) {
            for (auto i = lt_layer - 1; i != std::size_t(-1); --i) {
                auto&& it = ++indices[i];
                [[likely]]
                if (it) {
                    return;
                }
                it.next_span();
            }
            assert(!*this);
        }

       public:
        inline constexpr Iter& operator++() {
            assert(*this && check());
            increment(indices.size());
            maintain();
            return *this;
        }

        inline constexpr Iter& next_parent() {
            assert(*this && check());
            indices.back().next_span(false);
            increment(indices.size() - 1);
            maintain();
            return *this;
        }
        using value_type = std::span<const tree::GroupedSpanIter>;
        using difference_type = std::ptrdiff_t;
        [[nodiscard]] inline constexpr value_type operator*() const noexcept { return indices; }
        [[nodiscard]] inline constexpr value_type operator->() const noexcept { return indices; }
        IMPL_FORWARD_ITER(Iter);

        [[nodiscard]] inline std::size_t nlayers() const noexcept { return indices.size(); }
        [[nodiscard]] inline std::string fmt() const noexcept { return fmt::format("{}", fmt::join(indices, " ")); }
    };

    inline explicit Tree() = default;
    template <typename Rng>
    [[nodiscard]] inline static Tree from(Rng&& layer) noexcept {
        GroupedSpanBuilder builder;
        builder.new_span();
        for (auto&& elem : layer) {
            builder.add(std::byte(elem));
        }

        Tree tree;
        tree.add_layer(std::move(builder.build()));
        return tree;
    }

    [[nodiscard]] inline constexpr std::size_t nlayers() const noexcept { return layers.size(); }

    inline void add_layer(std::vector<byte>&& layer) noexcept {
        if (nlayers() > 0) { assert(std::size_t(rgs::distance(begin(), end())) == GroupedSpan::from(layer).count()); }
        layers.emplace_back(std::move(layer));
    }

    using iterator = Iter;
    using sentinel = Iter::Sentinel;
    using value_type = std::span<const tree::GroupedSpanIter>;

    [[nodiscard]] inline Iter iter_layer(std::size_t nlayers) noexcept { return Iter(*this, nlayers); }
    [[nodiscard]] inline Iter begin() noexcept { return Iter(*this); }
    [[nodiscard]] inline Iter::Sentinel end() noexcept { return Iter(*this).end(); }
    [[nodiscard]] inline Iter begin() const noexcept { return Iter(*this); }
    [[nodiscard]] inline Iter::Sentinel end() const noexcept { return Iter(*this).end(); }

    [[nodiscard]] inline std::string fmt() const noexcept {
        return fmt::format("{}", fmt::join(layers | vw::transform([](auto&& a) { return GroupedSpan::from(a); }), ", "));
    }

    template <class Archive>
    void serialize(Archive& ar) const {
        ar(layers);
    }
};

STATIC_ASSERT_RANGE(Tree);

inline auto format_as(const Tree& tree) {
    return tree.fmt();
}
inline auto format_as(const Tree::Iter& iter) {
    return iter.fmt();
}

}  // namespace circ::tree

// NOLINTBEGIN
TEST_FN(tree1) {
    circ::tree::Tree tree;

    circ::tree::GroupedSpanBuilder builder1;
    builder1.new_span();
    builder1.add(std::byte(0));
    builder1.add(std::byte(1));
    tree.add_layer(std::move(builder1.build()));

    circ::tree::GroupedSpanBuilder builder2;
    builder2.new_span();
    builder2.add(std::byte(1));
    builder2.add(std::byte(3));
    builder2.new_span();
    builder2.add(std::byte(1));
    builder2.add(std::byte(3));
    builder2.add(std::byte(5));
    builder2.add(std::byte(7));
    tree.add_layer(std::move(builder2.build()));

    circ::tree::GroupedSpanBuilder builder3;
    for (auto i : vw::ints(0, 6)) {
        builder3.new_span();
        builder3.add(std::byte(1));
        builder3.add(std::byte(3));
    }
    tree.add_layer(std::move(builder3.build()));
    CHECK_EQ(rgs::distance(tree.begin(), tree.end()), 12);

    auto counter = 0;
    for (auto it = tree.begin(); it != tree.end(); it.next_parent()) {
        assert(it.check());
        counter++;
    }
    CHECK_EQ(counter, 6);
}

TEST_FN(tree2) {
    circ::tree::Tree tree;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<std::size_t> dis(0, 3);

    circ::tree::GroupedSpanBuilder builder;
    builder.new_span();
    builder.add(std::byte(101));
    builder.add(std::byte(102));
    builder.add(std::byte(103));
    tree.add_layer(std::move(builder.build()));

    for (auto i : vw::ints(0, 5)) {
        auto counter = 0;
        for (auto node : tree) {
            builder.new_span();
            for (auto i : vw::ints(0ul, dis(gen))) {
                builder.add(std::byte(i + 101));
                counter++;
            }
        }
        tree.add_layer(std::move(builder.build()));
        CHECK_EQ(rgs::distance(tree.begin(), tree.end()), counter);
    }
    // fmt::println("Tree: {}", tree.fmt());
    // for (auto&& node : tree) {
    //     fmt::println("Node: {}", fmt::join(node, " "));
    // }
}
// NOLINTEND