#pragma once

#include <algorithm>
#include <cstddef>
#include "../bitsymplectic.hpp"

namespace clfd {

template <const std::size_t N>
class PermutationHelper;

template <const std::size_t N>
struct PermutationHelperIter {
    std::size_t i;
    PermutationHelper<N>* ptr;
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

template <const std::size_t N>
class PermutationHelper {
   public:
    std::array<std::size_t, N> perm;
    BitSymplectic<N>* ptr;

    using iterator = PermutationHelperIter<N>;
    explicit constexpr PermutationHelper(BitSymplectic<N>* ref) noexcept : ptr(ref) {
        for (auto i = 0ul; i < N; i++) {
            perm[i] = i;  // NOLINT
        }
    }

    [[nodiscard]] inline constexpr iterator begin() noexcept { return PermutationHelperIter{0, this}; }
    [[nodiscard]] inline constexpr iterator end() noexcept { return PermutationHelperIter{N, this}; }
};

template <const std::size_t N>
void swap(clfd::PermutationHelperIter<N> a, clfd::PermutationHelperIter<N> b) {
    assert(a.ptr == b.ptr);
    a.ptr->ptr->do_swap(a.i, b.i);
    std::swap(a.ptr->perm[a.i], b.ptr->perm[b.i]);  // NOLINT
}

template <const std::size_t N>
auto format_as(const clfd::PermutationHelper<N>& helper) {
    return fmt::format("{}", helper.perm);
}
}  // namespace clfd

static_assert(std::bidirectional_iterator<clfd::PermutationHelperIter<5>>);