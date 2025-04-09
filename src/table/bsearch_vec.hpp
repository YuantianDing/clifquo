#pragma once

#include <algorithm>
#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <set>
#include <span>
#include <vector>
#include "../utils/ranges.hpp"

namespace table {

template <typename T>
class BSearchVec {
    std::vector<T> vec;

   public:
    inline BSearchVec() = default;

    [[nodiscard]] inline std::size_t size() const noexcept { return vec.size(); }

    struct SplitIter : rgs::view_base {
        std::span<const T> rest;
        std::span<const T> current;

        inline SplitIter() = default;

        [[nodiscard]] inline constexpr bool operator==(const SplitIter& other) const noexcept {
            return rest.data() == other.rest.data() && current.data() == other.current.data();
        }
        [[nodiscard]] inline constexpr bool operator!=(const SplitIter& other) const noexcept { return !(*this == other); }

        [[nodiscard]] inline static SplitIter from(std::span<const T> input) noexcept {
            SplitIter result;
            if (input.size() > 0) {
                std::size_t index = std::countr_zero(input.size());
                assert(index < sizeof(const T) * 8 && (input.size() & (1ul << index)) > 0);
                auto split_point = input.size() & ~(1ul << index);
                result.rest = input.subspan(0, split_point);
                result.current = input.subspan(split_point);
            }
            return result;
        }

        using value_type = std::span<const T>;
        using difference_type = std::ptrdiff_t;
        using iterator = SplitIter;
        [[nodiscard]] inline value_type operator*() const noexcept {
            assert(current.size() > 0);
            return current;
        }
        inline SplitIter& operator++() noexcept {
            assert(current.size() > 0);
            *this = SplitIter::from(rest);
            return *this;
        }

        [[nodiscard]] inline constexpr explicit operator bool() const noexcept { return current.size() > 0; }
        [[nodiscard]] inline constexpr std::size_t total_size() const noexcept { return current.size() + rest.size(); }

        IMPL_FORWARD_ITER(SplitIter);
    };

    [[nodiscard]] inline bool contains(const T& elem) const noexcept {
        return rgs::any_of(SplitIter::from(vec), [&elem](auto span) { return std::binary_search(span.begin(), span.end(), elem); });
    }

    inline void insert(const T& elem) {
        vec.push_back(elem);
        auto iter = SplitIter::from(std::span(vec).subspan(0, vec.size() - 1));
        for (; iter; ++iter) {
            [[unlikely]]
            if (vec.size() - iter.total_size() == iter.current.size()) {
                auto begin = vec.begin() + std::ptrdiff_t(iter.rest.size());
                auto middle = begin + std::ptrdiff_t(iter.current.size());
                auto end = vec.end();
                std::inplace_merge(begin, middle, end);
            } else {
                break;
            }
        }
        assert(check_sorted());
    }

    [[nodiscard]] inline constexpr bool check_sorted() {
        return rgs::all_of(SplitIter::from(vec), [](auto span) { return std::is_sorted(span.begin(), span.end()); });
    }

    [[nodiscard]] inline std::vector<T> build_sorted() {
        auto iter = SplitIter::from(vec);
        for (; iter; ++iter) {
            auto begin = vec.begin() + std::ptrdiff_t(iter.rest.size());
            auto middle = begin + std::ptrdiff_t(iter.current.size());
            auto end = vec.end();
            std::inplace_merge(begin, middle, end);
        }
        vec.shrink_to_fit();
        return std::move(vec);
    }
};

}  // namespace table

TEST_FN(bsearch_vec) {
    for (auto seed : vw::ints(0, 20)) {
        std::mt19937 gen(seed);
        std::uniform_int_distribution<std::size_t> dis1(0, 10000);
        std::uniform_int_distribution<std::size_t> dis(0, 10000);

        table::BSearchVec<std::uint64_t> bsearch;
        std::set<std::uint64_t> set;
        for (auto i : vw::ints(0ul, dis1(gen))) {
            auto value = dis(gen);
            set.emplace(value);
            if (!bsearch.contains(value)) { bsearch.insert(value); }
        }
        auto sorted = bsearch.build_sorted();
        CHECK_EQ(sorted.size(), sorted.capacity());
        CHECK(std::equal(sorted.begin(), sorted.end(), set.begin(), set.end()));
    }
}