#pragma once
#include <range/v3/core.hpp>
#include <range/v3/utility/semiregular.hpp>
#include <range/v3/utility/tuple_algorithm.hpp>
#include <type_traits>
#include <utility>

template <class F>
struct decomposed_fn {
   private:
    ranges::semiregular_t<F> f_;

    template <class FF>
    struct caller {
        FF& f_;

        template <class... Args>
        auto operator()(Args&&... args) RANGES_DECLTYPE_AUTO_RETURN_NOEXCEPT(ranges::invoke(f_, std::forward<Args>(args)...))
    };

   public:
    decomposed_fn() = default;
    explicit decomposed_fn(F f) noexcept(std::is_nothrow_move_constructible<F>::value) : f_(std::move(f)) {}

    template <class T>
    auto operator()(T&& t) RANGES_DECLTYPE_AUTO_RETURN_NOEXCEPT(ranges::tuple_apply(caller<F>{f_}, std::forward<T>(t)))

        template <class T>
        auto operator()(T&& t) const RANGES_DECLTYPE_AUTO_RETURN_NOEXCEPT(ranges::tuple_apply(caller<F const>{f_}, std::forward<T>(t)))
};

template <class F>
auto decomposed(F&& f) RANGES_DECLTYPE_AUTO_RETURN_NOEXCEPT(decomposed_fn<std::decay_t<F>>(std::forward<F>(f)))