#pragma once
#include <cstddef>
#include <iterator>
#include <range/v3/algorithm/all_of.hpp>
#include <range/v3/algorithm/any_of.hpp>
#include <range/v3/algorithm/find_if.hpp>
#include <range/v3/algorithm/none_of.hpp>
#include <range/v3/numeric/accumulate.hpp>
#include <range/v3/range.hpp>
#include <range/v3/range/concepts.hpp>
#include <range/v3/view.hpp>
#include <range/v3/view/cartesian_product.hpp>
#include <range/v3/view/enumerate.hpp>
#include <range/v3/view/for_each.hpp>
#include <range/v3/view/repeat.hpp>
#include <range/v3/view/transform.hpp>
#include <variant>
#include "range/v3/view/take.hpp"
#include "test.hpp"

namespace rgs = ranges;
namespace vw = ranges::views;

inline constexpr auto size_t_all(bool b) {
    return b ? std::numeric_limits<size_t>::max() / 2 : 0;
}

template <typename RngA, typename RngB>
auto rng_either(bool condition, RngA a, RngB b) {
    return vw::concat(a | vw::take(size_t_all(condition)), b | vw::take(size_t_all(!condition)));
}
template <typename RngA>
auto rng_maybe(bool condition, RngA a) {
    return a | vw::take(size_t_all(condition));
}

// NOLINTNEXTLINE
#define IMPL_RANGE(Self)                                      \
    [[nodiscard]] inline const Self& begin() const noexcept { \
        return *this;                                         \
    }                                                         \
    inline Self operator++(int) noexcept { /* NOLINT */       \
        auto temp = *this;                                    \
        ++(*this);                                            \
        return *this;                                         \
    }

#define STATIC_ASSERT_RANGE(Self)                       \
    static_assert(std::is_constructible_v<Self>);       \
    static_assert(std::is_constructible_v<Self, Self>); \
    static_assert(std::move_constructible<Self>);       \
    static_assert(std::movable<Self>);                  \
    static_assert(std::weakly_incrementable<Self>);     \
    static_assert(std::forward_iterator<Self>);         \
    static_assert(std::ranges::range<Self>);            \
    static_assert(rgs::range<Self>);                    \
    static_assert(rgs::viewable_range<Self>);

// NOLINTNEXTLINE
#define IMPL_FORWARD_ITER(Self)                                                            \
    IMPL_RANGE(Self)                                                                       \
    struct Sentinel {                                                                      \
        [[nodiscard]] inline constexpr bool operator==(const Self& other) const noexcept { \
            return !bool(other);                                                           \
        }                                                                                  \
    };                                                                                     \
                                                                                           \
    [[nodiscard]] inline constexpr Sentinel end() const noexcept {                         \
        return {};                                                                         \
    }

// NOLINTNEXTLINE
#define IMPL_OUT_ITER(Self)                                                              \
    [[nodiscard]] inline constexpr Self& operator++() noexcept { /* NOLINT */            \
        return *this;                                                                    \
    }                                                                                    \
    [[nodiscard]] inline constexpr Self operator++(int) noexcept { /* NOLINT */          \
        return *this;                                                                    \
    }                                                                                    \
    [[nodiscard]] inline constexpr const Self& operator*() const noexcept { /* NOLINT */ \
        return *this;                                                                    \
    }                                                                                    \
    [[nodiscard]] inline constexpr Self& operator*() noexcept { /* NOLINT */             \
        return *this;                                                                    \
    }

TEST_FN(rng_either_test) {
    auto a = vw::ints(0, 10);
    auto b = vw::ints(10, 20);
    auto c = rng_either(true, a, b) | rgs::to<std::vector>();
    CHECK_EQ(rgs::accumulate(c, 0), 45);
}