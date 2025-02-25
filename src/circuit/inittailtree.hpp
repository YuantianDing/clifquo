#pragma once
#include <cassert>
#include <cstdint>
#include <iterator>
#include <queue>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>
#include "../utils/linkedarray.hpp"
#include "../utils/ranges.hpp"
#include "fmt/base.h"
#include "fmt/core.h"
#include "fmt/ranges.h"
#include "range/v3/view/reverse.hpp"
#include "range/v3/view/transform.hpp"
#include "range/v3/view/unique.hpp"

namespace circuit {
template <typename InnerT>
class InitTailTree {
    struct Node {
        Node* _Nullable const init;
        Node* _Nullable const tail;
        LinkedArray<Node> children;
        const InnerT last;
        LinkedArray<Node> tail_children_track;

        [[nodiscard]] inline constexpr bool is_root() const noexcept {
            assert((init == nullptr) == (tail == nullptr));
            return init == nullptr && tail == nullptr;
        }
        [[nodiscard]] inline constexpr static Node root() noexcept { return {nullptr, nullptr, {}, InnerT(), {}}; }

        [[nodiscard]] inline constexpr bool is_sentinel() const noexcept { return is_root() && children.is_null(); }
        [[nodiscard]] inline constexpr static Node sentinel() noexcept { return {nullptr, nullptr, {}, InnerT(), {}}; }

        [[nodiscard]] inline constexpr LinkedArray<Node> next_jump() const noexcept { return is_root() ? children : LinkedArray<Node>(); }
        [[nodiscard]] inline constexpr static Node jump_node(LinkedArray<Node> jump) noexcept {
            assert(jump.nonnull());
            return {nullptr, nullptr, jump, InnerT(), {}};
        }

        [[nodiscard]] inline constexpr bool is_regular() const noexcept { return init && tail; }
        [[nodiscard]] inline constexpr static Node regular(InitTailTree init, InitTailTree tail, const InnerT& last) noexcept {
            return {init.raw(), tail.raw(), {}, last, {}};
        }
    };

    Node* _Nonnull node;

    explicit InitTailTree(Node* _Nullable node) noexcept : node(node) { assert(node); }
    explicit InitTailTree(LinkedArray<Node> node) noexcept : node(node.raw()) { assert(node.accessible()); }

   public:
    // ------------------ Basics ------------------

    [[nodiscard]] inline constexpr InitTailTree() noexcept : node(nullptr) { assert(false); }

    [[nodiscard]] inline constexpr static InitTailTree root() noexcept { return InitTailTree(new Node(Node::root())); }
    template <typename Rng>
    [[nodiscard]] inline constexpr static InitTailTree root(Rng&& rng) noexcept {
        auto rt = root();
        LinkedArrayBuilder<Node> builder(15, rt.children());
        for (auto&& elem : rng) {
            builder.emplace_back(Node::regular(rt, rt, elem));
        }
        rt.node->children = builder.build();
        return rt;
    }
    [[nodiscard]] inline constexpr bool operator==(const InitTailTree& other) const noexcept = default;
    [[nodiscard]] inline constexpr bool operator!=(const InitTailTree& other) const noexcept = default;

    // ------------------ Fields ------------------

    [[nodiscard]] inline constexpr bool is_regular() const noexcept { return node->is_regular(); }
    [[nodiscard]] inline constexpr bool is_root() const noexcept { return node->is_root(); }
    [[nodiscard]] inline constexpr Node* _Nonnull raw_unchecked() const noexcept { return node; }
    [[nodiscard]] inline constexpr Node* _Nonnull raw() const noexcept {
        assert(node);
        return node;
    }

    [[nodiscard]] inline constexpr InitTailTree init() const noexcept {
        assert(node->tail && is_regular());
        return InitTailTree(node->init);
    }
    [[nodiscard]] inline constexpr InitTailTree tail() const noexcept {
        assert(node->tail && is_regular());
        return InitTailTree(node->tail);
    }
    [[nodiscard]] inline constexpr const InnerT& last() const noexcept {
        assert(node->tail && is_regular());
        return node->last;
    }

    // ------------------ Iterator ------------------

    [[nodiscard]] inline constexpr const InnerT& operator*() const noexcept { return node->last; }

    [[nodiscard]] inline constexpr InitTailTree& operator++() noexcept {
        assert(is_regular());
        *this = InitTailTree(node->init);
        return *this;
    }
    [[nodiscard]] inline constexpr InitTailTree operator++(int) noexcept {  // NOLINT
        assert(is_regular());
        auto tmp = *this;
        *this = InitTailTree(node->init);
        return tmp;
    }

    using iterator = InitTailTree;
    using value_type = InnerT;
    using difference_type = std::ptrdiff_t;
    struct Sentinel {
        [[nodiscard]] inline constexpr bool operator==(const InitTailTree& other) const noexcept { return other.is_root(); };
    };
    [[nodiscard]] inline constexpr InitTailTree begin() const noexcept { return *this; }
    [[nodiscard]] inline constexpr Sentinel end() const noexcept { return {}; }

    [[nodiscard]] inline constexpr LinkedArray<Node> children() const { return node->children; }

    // ------------------ Construction ------------------
    template <typename ValueT>
    struct Point {
        ValueT value;
        InitTailTree circ;
    };
    template <typename ValueT, typename ComputeF, typename NewNodeF>
    inline constexpr void extend(ValueT initial, ComputeF&& compute, NewNodeF&& new_node) noexcept {
        assert(!node->tail);  // Cannot extend a non-root tree

        std::queue<Point<ValueT>> bfs_boundry;
        std::size_t counter = 0;

        bfs_boundry.emplace(initial, *this);

        while (!bfs_boundry.empty()) {
            const auto current = bfs_boundry.front();
            bfs_boundry.pop();

            // Visit existing children
            for (auto it : current.circ.children()) {
                auto existing = InitTailTree(it);
                if (auto value = compute(current.value, existing.last())) { bfs_boundry.emplace(*value, existing); }
            }

            if (current.circ.is_root()) { continue; }

            // Create & Visit new children
            LinkedArrayBuilder<Node> builder(15, current.circ.children());

            const auto start = current.circ.tail().children();
            const auto end = start.end_at(current.circ.node->tail_children_track);
            for (auto it = start; it != end; ++it) {
                const auto tailchild = InitTailTree(it);
                for (const auto push : new_node(current.circ.last(), tailchild.last())) {
                    if (const auto value = compute(current.value, push)) {
                        const auto currentchild = &builder.emplace_back(Node::regular(current.circ, tailchild, push));

                        counter += 1;
                        if (counter % 10000 == 0) { fmt::println("{}: {}", counter, format_as(InitTailTree(currentchild))); }
                        bfs_boundry.emplace(*value, InitTailTree(currentchild));
                    }
                }
            }
            current.circ.node->children = builder.build();
            current.circ.node->tail_children_track = current.circ.tail().children();
        }
    }

    // template <class Archive>
    // void save(Archive& archive) const;

    // template <class Archive>
    // void load(Archive& archive) const;
};

template <typename InnerT>
[[nodiscard]] inline std::string format_as(InitTailTree<InnerT> tree) {
    return fmt::format("{}", fmt::join(tree, " "));
}

template <typename InnerT>
[[nodiscard]] inline std::string format_all(InitTailTree<InnerT> tree) {
    std::stringstream ss;
    if (bool(tree)) {
        ss << fmt::format("{}", tree.last());
    } else {
        ss << "ROOT";
    }
    if (!tree.adjecents().empty()) {
        ss << fmt::format("{{{}}}", fmt::join(tree.adjecents() | vw::transform([](auto a) { return format_all(a); }), ", "));
    }
    return ss.str();
};

}  // namespace circuit

template <typename InnerT>
struct std::hash<circuit::InitTailTree<InnerT>> {  // NOLINT
    using TreeT = circuit::InitTailTree<InnerT>;
    std::size_t operator()(const TreeT& tree) const noexcept { return std::hash<typename TreeT::Node*>{}(tree.raw_unchecked()); }
};