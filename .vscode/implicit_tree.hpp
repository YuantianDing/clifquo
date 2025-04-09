#pragma once

#include <cassert>
#include <cstddef>
#include <vector>

namespace circ {

class ImplicitTree {
    using byte = std::byte;
    std::vector<std::vector<byte>> layers;

   public:
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
