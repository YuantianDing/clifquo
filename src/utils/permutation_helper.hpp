#pragma once

#include <algorithm>
#include <cstddef>
#include "boost/container/static_vector.hpp"
#include "fmt/core.h"
#include "ranges.hpp"

template <typename SwapperF>
class PermutationHelper;

template <typename SwapperF>
struct PermutationHelperIter {
    std::size_t i;
    PermutationHelper<SwapperF>* _Nonnull ptr;
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

template <typename SwapperF>
class PermutationHelper {
   public:
    boost::container::static_vector<std::size_t, 5> perm;
    SwapperF swapf;

    using iterator = PermutationHelperIter<SwapperF>;
    explicit constexpr PermutationHelper(std::size_t size, SwapperF swapf) noexcept : swapf(swapf) {
        for (auto i = 0ul; i < size; i++) {
            perm.push_back(i);
        }
    }

    inline constexpr void reset() noexcept {
        auto size = perm.size();
        perm.clear();
        for (auto i = 0ul; i < size; i++) {
            perm.push_back(i);
        }
    }
    inline constexpr void do_swap(std::size_t a, std::size_t b) noexcept { swap(iter_at(a), iter_at(b)); }
    [[nodiscard]] inline constexpr iterator iter_at(std::size_t i) noexcept { return PermutationHelperIter{i, this}; }
    [[nodiscard]] inline constexpr iterator begin() noexcept { return PermutationHelperIter{0, this}; }
    [[nodiscard]] inline constexpr iterator end() noexcept { return PermutationHelperIter{perm.size(), this}; }
    [[nodiscard]] inline constexpr std::size_t size() const noexcept { return perm.size(); }
    [[nodiscard]] inline constexpr std::size_t operator[](std::size_t i) const noexcept { return perm[i]; }

    template <typename F>
    inline constexpr void sort(F f) noexcept {
        // std::sort(perm.begin(), perm.end(), [f](auto itera, auto iterb) { return f(itera.i, iterb.i); });
        for (auto it = begin(); it != end(); it++) {
            for (auto it2 = begin(); it2 != it; it2++) {
                if (f(it.i, it2.i)) { swap(it, it2); }
            }
        }
    }
    [[nodiscard]] inline constexpr bool next_permutation() noexcept { return std::next_permutation(perm.begin(), perm.end()); }
    [[nodiscard]] inline constexpr bool prev_permutation() noexcept { return std::prev_permutation(perm.begin(), perm.end()); }
    [[nodiscard]] inline boost::container::static_vector<std::size_t, 5> to_vec() const noexcept { return perm; }
};

template <typename SwapperF>
inline void swap(PermutationHelperIter<SwapperF> a, PermutationHelperIter<SwapperF> b) {  // NOLINT
    assert(a.ptr == b.ptr);
    a.ptr->swapf(a.i, b.i);
    std::swap(a.ptr->perm[a.i], b.ptr->perm[b.i]);  // NOLINT
}

template <typename SwapperF>
auto format_as(const PermutationHelper<SwapperF>& helper) {
    return fmt::format("{}", helper.perm);
}

// static_assert(std::bidirectional_iterator<PermutationHelperIter<int>>);
