#pragma once

#include <algorithm>
#include <cstddef>
#include "../../clifford/search.hpp"
#include "../svsim/gate.hpp"
#include "fmt/core.h"

namespace qsearch {
template <typename T>
class PermutationHelper;

template <typename T>
struct PermutationHelperIter {
    std::size_t i;
    PermutationHelper<T>* _Nonnull ptr;
    using difference_type = ptrdiff_t;
    using value_type = PermutationHelperIter;
    inline constexpr bool operator==(const PermutationHelperIter& other) const noexcept = default;
    inline constexpr bool operator!=(const PermutationHelperIter& other) const noexcept = default;
    inline constexpr auto operator<=>(const PermutationHelperIter& other) const noexcept {
        assert(ptr == other.ptr);
        return ptr->perm[i] <=> ptr->perm[other.i];  // NOLINT
    }

    inline constexpr const PermutationHelperIter& operator*() const noexcept { return *this; }
    inline constexpr PermutationHelperIter& operator++() noexcept {
        ++i;
        return *this;
    }
    inline constexpr PermutationHelperIter operator++(int) noexcept {  // NOLINT
        const auto result = *this;
        ++i;
        return result;
    }
    inline constexpr PermutationHelperIter& operator--() noexcept {
        --i;
        return *this;
    }
    inline constexpr PermutationHelperIter operator--(int) noexcept {  // NOLINT
        const auto result = *this;
        --i;
        return result;
    }
};

template <typename T>
class PermutationHelper {
   public:
    std::array<std::size_t, T::SIZE> perm;
    T* _Nonnull ptr;

    using iterator = PermutationHelperIter<T>;
    explicit constexpr PermutationHelper(T* _Nonnull ref) noexcept : ptr(ref) {
        for (auto i = 0ul; i < T::SIZE; i++) {
            perm[i] = i;  // NOLINT
        }
    }

    [[nodiscard]] inline constexpr iterator begin() noexcept { return PermutationHelperIter{0, this}; }
    [[nodiscard]] inline constexpr iterator end() noexcept { return PermutationHelperIter{T::SIZE, this}; }
};

template <typename T>
inline void swap(PermutationHelperIter<T> a, PermutationHelperIter<T> b) {  // NOLINT
    assert(a.ptr == b.ptr);
    a.ptr->ptr->do_swap(a.i, b.i);
    std::swap(a.ptr->perm[a.i], b.ptr->perm[b.i]);  // NOLINT
}

template <typename T>
auto format_as(const PermutationHelper<T>& helper) {
    return fmt::format("{}", helper.perm);
}

}  // namespace qsearch

static_assert(std::bidirectional_iterator<qsearch::PermutationHelperIter<clifford::GraphEdge<qsearch::NQUBITS>>>);
