#pragma once

#include <cstddef>
#include <cstdint>
#include <iterator>
#include "fmt/format.h"
#include "fmt/ranges.h"
#include "range/v3/algorithm/none_of.hpp"
#include "range/v3/range/concepts.hpp"
#include "range/v3/view/transform.hpp"
#include "ranges.hpp"
#include "test.hpp"

namespace tracker {

template <typename T>
struct EmptyTracker {
    EmptyTracker track(const T& /*e*/) { return EmptyTracker(); }
};

template <typename T>
struct LengthTracker {
    std::size_t length = 0;
    LengthTracker track(const T& /*e*/) { return LengthTracker<T>(length + 1); }
};

}  // namespace tracker

template <typename T, typename TrackerT>
struct Cons;

template <typename T, typename TrackerT = tracker::EmptyTracker<T>>
class List {
    Cons<T, TrackerT>* _Nullable ptr;

    explicit List(Cons<T, TrackerT>* _Nullable ptr) noexcept : ptr(ptr) {}

   public:
    constexpr void debug_check() const noexcept { assert(ptr == nullptr || uint64_t(ptr) > 0x300ul); }

    using value_type = List<T, TrackerT>;
    explicit List() noexcept : ptr(nullptr) {}

    // NOLINTNEXTLINE(hicpp-explicit-conversions)
    [[nodiscard]] operator bool() const noexcept { return ptr; }
    [[nodiscard]] constexpr const List<T, TrackerT>& operator*() const noexcept { return *this; }

    constexpr List<T, TrackerT>& operator++() noexcept {
        assert(ptr);
        *this = ptr->tail;
        return *this;
    }
    constexpr List<T, TrackerT> operator++(int) noexcept {  // NOLINT
        auto tmp = *this;
        assert(ptr);
        *this = ptr->tail;
        return tmp;
    }
    [[nodiscard]] constexpr List<T, TrackerT> begin() const noexcept { return *this; }
    [[nodiscard]] constexpr List<T, TrackerT> end() const noexcept { return List(); }

    DEBUG_ONLY(inline static Cons<T, TrackerT>* _Nullable LAST_ALLOCATED = nullptr;)

    [[nodiscard]] constexpr List<T, TrackerT> append(T e) const noexcept {
        auto&& tracker = this->tracker().track(e);
        auto nptr = new Cons(e, *this, tracker);  // NOLINT(cppcoreguidelines-owning-memory)
        DEBUG_ONLY(List::LAST_ALLOCATED = nptr);
        return List(nptr);
    }
    constexpr void free() noexcept {
        assert(List::LAST_ALLOCATED == ptr);
        delete ptr;
    }

    [[nodiscard]] constexpr T& head() const noexcept {
        assert(ptr);
        return ptr->head;
    }
    [[nodiscard]] constexpr T& tail() const noexcept {
        assert(ptr);
        return ptr->tail;
    }
    [[nodiscard]] constexpr TrackerT tracker() const noexcept {
        assert(ptr == nullptr || uint64_t(ptr) > 0x300ul);
        if (ptr) {
            return ptr->tracker;
        } else {
            return TrackerT();
        }
    }
};

template <typename T, typename TrackerT>
struct Cons {
    T head;
    List<T, TrackerT> tail;
    TrackerT tracker;

    explicit Cons(T head, List<T, TrackerT> tail, TrackerT tracker) : head(head), tail(tail), tracker(tracker) {}
};

template <typename T, typename TrackerT>
auto format_as(List<T, TrackerT> list) {
    return fmt::join(list | vw::transform([](auto e) { return fmt::format("{}", e.head()); }), " -> ");
}

static_assert(std::forward_iterator<List<int>>);
static_assert(rgs::forward_range<List<int>>);