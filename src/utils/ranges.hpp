#pragma once
#include <cstddef>
#include <iterator>
#include <range/v3/algorithm/all_of.hpp>
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
    return b ? std::numeric_limits<size_t>::max() : 0;
}

template <typename RngA, typename RngB>
auto rng_either(bool condition, RngA a, RngB b) {
    return vw::concat(a | vw::take(size_t_all(condition)), b | vw::take(size_t_all(!condition)));
}
template <typename RngA>
auto rng_maybe(bool condition, RngA a) {
    return a | vw::take(size_t_all(condition));
}

TEST_FN(rng_either_test) {
    auto a = vw::ints(0, 10);
    auto b = vw::ints(10, 20);
    auto c = rng_either(true, a, b) | rgs::to<std::vector>();
    CHECK_EQ(rgs::accumulate(c, 0), 45);
}