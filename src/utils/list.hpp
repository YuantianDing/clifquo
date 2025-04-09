#pragma once

#include <cstddef>
#include <cstdint>
#include <iterator>
#include "fmt.hpp"
#include "fmt/format.h"
#include "range/v3/algorithm/none_of.hpp"
#include "range/v3/range/concepts.hpp"
#include "range/v3/view/transform.hpp"
#include "ranges.hpp"
#include "test.hpp"

template <typename T>
struct Cons;

template <typename T>
class List {
    Cons<T>* _Nullable ptr;

   protected:
    explicit List(Cons<T>* _Nullable ptr) noexcept : ptr(ptr) {}

   public:
    constexpr void debug_check() const noexcept { assert(ptr == nullptr || uint64_t(ptr) > 0x300ul); }

    using value_type = List<T>;
    using difference_type = std::ptrdiff_t;

    List() noexcept : ptr(nullptr) {}

    bool operator==(const List& other) const noexcept { return ptr == other.ptr; }
    bool operator!=(const List& other) const noexcept { return !(*this == other); }

    // NOLINTNEXTLINE(hicpp-explicit-conversions)
    [[nodiscard]] explicit operator bool() const noexcept { return ptr; }
    [[nodiscard]] constexpr const List<T>& operator*() const noexcept { return *this; }

    constexpr List<T>& operator++() noexcept {
        assert(ptr);
        *this = ptr->tail;
        return *this;
    }
    constexpr List<T> operator++(int) noexcept {  // NOLINT
        auto tmp = *this;
        assert(ptr);
        *this = ptr->tail;
        return tmp;
    }
    [[nodiscard]] constexpr List<T> begin() const noexcept { return *this; }
    [[nodiscard]] constexpr List<T> end() const noexcept { return List(); }

    DEBUG_ONLY(inline static Cons<T>* _Nullable LAST_ALLOCATED = nullptr;)

    [[nodiscard]] constexpr List<T> plus(T e) const noexcept {
        auto nptr = new Cons(e, *this);  // NOLINT(cppcoreguidelines-owning-memory)
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
    [[nodiscard]] constexpr List<T>& tail() const noexcept {
        assert(ptr);
        return ptr->tail;
    }
    [[nodiscard]] constexpr List<T> last() const noexcept {
        auto p = *this;
        while (p.tail()) {
            p = p.tail();
        }
        return p;
    }

    [[nodiscard]] constexpr std::vector<T> to_vec() const noexcept {
        return *this | vw::transform([](auto e) { return e.head(); }) | rgs::to<std::vector>();
    }
};

template <typename T>
struct Cons {
    T head;
    List<T> tail;

    explicit Cons(T head, List<T> tail) : head(head), tail(tail) {}
};

template <typename T, typename InfoTrackT>
auto format_as(List<T> list) {
    return fmt::format("[{}]", fmt::join(list | vw::transform([](auto e) { return fmt::format("{}", e.head()); }), ", "));
}

static_assert(std::forward_iterator<List<int>>);
static_assert(rgs::forward_range<List<int>>);