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

namespace info {

template <typename T>
struct EmptyTracker {
    EmptyTracker plus(const T& /*e*/) { return EmptyTracker(); }
};

template <typename T>
struct LengthTracker {
    std::size_t length = 0;
    LengthTracker plus(const T& /*e*/) { return LengthTracker<T>(length + 1); }
};

}  // namespace info

template <typename T, typename InfoTrackT>
struct Cons;

template <typename T, typename InfoTrackT = info::EmptyTracker<T>>
class List {
    Cons<T, InfoTrackT>* _Nullable ptr;

   protected:
    explicit List(Cons<T, InfoTrackT>* _Nullable ptr) noexcept : ptr(ptr) {}

   public:
    constexpr void debug_check() const noexcept { assert(ptr == nullptr || uint64_t(ptr) > 0x300ul); }

    using value_type = List<T, InfoTrackT>;
    explicit List() noexcept : ptr(nullptr) {}

    // NOLINTNEXTLINE(hicpp-explicit-conversions)
    [[nodiscard]] explicit operator bool() const noexcept { return ptr; }
    [[nodiscard]] constexpr const List<T, InfoTrackT>& operator*() const noexcept { return *this; }

    constexpr List<T, InfoTrackT>& operator++() noexcept {
        assert(ptr);
        *this = ptr->tail;
        return *this;
    }
    constexpr List<T, InfoTrackT> operator++(int) noexcept {  // NOLINT
        auto tmp = *this;
        assert(ptr);
        *this = ptr->tail;
        return tmp;
    }
    [[nodiscard]] constexpr List<T, InfoTrackT> begin() const noexcept { return *this; }
    [[nodiscard]] constexpr List<T, InfoTrackT> end() const noexcept { return List(); }

    DEBUG_ONLY(inline static Cons<T, InfoTrackT>* _Nullable LAST_ALLOCATED = nullptr;)

    [[nodiscard]] constexpr List<T, InfoTrackT> plus(T e) const noexcept {
        auto&& info = this->info().plus(e);
        auto nptr = new Cons(e, *this, info);  // NOLINT(cppcoreguidelines-owning-memory)
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
    [[nodiscard]] constexpr List<T, InfoTrackT>& tail() const noexcept {
        assert(ptr);
        return ptr->tail;
    }
    [[nodiscard]] constexpr List<T, InfoTrackT> last() const noexcept {
        auto p = *this;
        while (p.tail()) {
            p = p.tail();
        }
        return p;
    }
    [[nodiscard]] constexpr InfoTrackT info() const noexcept {
        assert(ptr == nullptr || uint64_t(ptr) > 0x300ul);
        if (ptr) {
            return ptr->info;
        } else {
            return InfoTrackT();
        }
    }

    [[nodiscard]] constexpr std::vector<T> to_vec() const noexcept {
        return *this | vw::transform([](auto e) { return e.head(); }) | rgs::to<std::vector>();
    }
};

template <typename T, typename InfoTrackT>
struct Cons {
    T head;
    List<T, InfoTrackT> tail;
    InfoTrackT info;

    explicit Cons(T head, List<T, InfoTrackT> tail, InfoTrackT info) : head(head), tail(tail), info(info) {}
};

template <typename T, typename InfoTrackT>
auto format_as(List<T, InfoTrackT> list) {
    return fmt::format("[{}]", fmt::join(list | vw::transform([](auto e) { return fmt::format("{}", e.head()); }), ", "));
}

static_assert(std::forward_iterator<List<int>>);
static_assert(rgs::forward_range<List<int>>);