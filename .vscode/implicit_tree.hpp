#pragma once

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <random>
#include <span>
#include <vector>
#include "../utils/compress.hpp"
#include "../utils/ranges.hpp"
#include "boost/container/static_vector.hpp"
#include "range/v3/algorithm/all_of.hpp"
#include "range/v3/range/concepts.hpp"
#include "range/v3/range_fwd.hpp"
#include "range/v3/view/transform.hpp"

namespace circ {

class ImplicitTree {
    using byte = std::byte;
    std::vector<std::vector<byte>> layers;

   public:
    class LayerIter : public rgs::view_base {
        CompressedIter iter;
        std::size_t current;

        inline constexpr void read() {
            if (*iter > 0) {
                current += *iter;
            } else {
                current = -1;
            }
        }

       public:
        explicit LayerIter() : current(-1) {}
        explicit LayerIter(CompressedIter iter) : iter(iter), current(-1) {
            if (iter) { read(); }
        }

        [[nodiscard]] inline constexpr bool operator==(const LayerIter& other) const noexcept { return iter == other.iter; }
        [[nodiscard]] inline constexpr bool operator!=(const LayerIter& other) const noexcept { return !(*this == other); }

        [[nodiscard]] inline constexpr explicit operator bool() const noexcept { return bool(iter); }

        inline constexpr LayerIter& operator++() {
            if (iter) {
                iter++;
                if (iter) { read(); }
            }
            return *this;
        }

        [[nodiscard]] inline constexpr std::optional<std::size_t> operator*() const noexcept {
            assert(iter);
            if (current != std::size_t(-1)) {
                return current;
            } else {
                return std::nullopt;
            }
        }

        using iterator = CompressedIter;
        using value_type = std::optional<std::size_t>;
        using difference_type = std::ptrdiff_t;
        IMPL_FORWARD_ITER(LayerIter);
    };

    STATIC_ASSERT_RANGE(LayerIter);

    struct Iter : public rgs::view_base {
        std::vector<LayerIter> indices;

        inline explicit Iter(ImplicitTree& tree) {
            indices = tree.layers | vw::transform([](auto& layer) { return LayerIter(CompressedIter::from(layer)); }) | rgs::to<std::vector>();
        }

        [[nodiscard]] inline constexpr std::size_t operator[](std::size_t i) const noexcept {
            if (auto p = *indices[i]) {
                return *p;
            } else {
                __builtin_unreachable();
            }
        }

        [[nodiscard]] inline constexpr bool operator==(const Iter& other) const noexcept {
            if (indices.back() == other.indices.back()) { assert(indices == other.indices); }
            return indices.back() == other.indices.back();
        }
        [[nodiscard]] inline constexpr bool operator!=(const Iter& other) const noexcept { return !(*this == other); }

        [[nodiscard]] inline constexpr explicit operator bool() const noexcept {
            if (indices.back()) {
                assert(rgs::all_of(indices, [](auto& it) { return bool(it); }));
            }
            return bool(indices.back());
        }
    };
};

}  // namespace circ
