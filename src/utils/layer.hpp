#pragma once

#include <cstddef>
#include <cstdint>
#include <iterator>
#include "fmt/format.h"
#include "fmt/ranges.h"
#include "range/v3/algorithm/find_if.hpp"
#include "range/v3/algorithm/none_of.hpp"
#include "range/v3/range/concepts.hpp"
#include "range/v3/view/transform.hpp"
#include "range/v3/view/unique.hpp"
#include "ranges.hpp"
#include "test.hpp"

template <typename InnerT, typename InfoTrackT>
class Layer {
   public:
    struct Data {
        std::vector<Layer> nexts;

        Data(Data* const prev, const InnerT inner, InfoTrackT&& info) noexcept : prev(prev), inner(inner), info(info) {}

       public:
        const InfoTrackT info;
        Data* const prev;
        const InnerT inner;
    };

   private:
    Data* _Nonnull data;

    inline explicit Layer(Data* _Nonnull data) : data(const_cast<Data*>(data)) {};

   public:
    [[nodiscard]] inline Data* _Nonnull data_unsafe() const { return data; }
    [[nodiscard]] static inline Layer nil() { return Layer{new Data(nullptr, InnerT{}, InfoTrackT())}; }
    template <typename... Args>
    [[nodiscard]] inline Layer plus(const InnerT inner, Args... args) const {
        return Layer(new Data(this->data, inner, data->info.plus(inner, args...)));
    }

    [[nodiscard]] inline constexpr bool operator==(const Layer& other) const noexcept = default;
    [[nodiscard]] inline constexpr bool operator!=(const Layer& other) const noexcept = default;

    [[nodiscard]] inline const std::vector<Layer>& adjecents() const { return data->nexts; }

    [[nodiscard]] inline constexpr explicit operator bool() const { return data->prev; }

    [[nodiscard]] inline constexpr const InfoTrackT& info() const { return data->info; }
    [[nodiscard]] inline constexpr Layer prev() const {
        assert(bool(data->prev));
        return Layer(data->prev);
    }
    [[nodiscard]] inline constexpr const InnerT& inner() const {
        assert(bool(data->prev));
        return data->inner;
    }
    inline void add_adjecents(Layer layer) {
        assert(bool(layer) && layer.prev() == *this);
        // assert(rgs::find_if(adjecents(), [layer](auto a) { return a.inner() == layer.inner(); }) == adjecents().end());
        data->nexts.emplace_back(layer);
    }
    inline std::vector<std::remove_const_t<InnerT>> to_vector() {
        std::vector<std::remove_const_t<InnerT>> vec;
        for (auto l = *this; l; l = l.prev()) {
            vec.push_back(l.inner());
        }
        return vec;
    }
};

template <typename InnerT, typename InfoTrackT>
inline std::string format_as(Layer<InnerT, InfoTrackT> layer) {
    return fmt::format("{}", fmt::join(layer.to_vector(), " "));
}

template <typename InnerT, typename InfoTrackT>
inline std::string format_all(Layer<InnerT, InfoTrackT> layer) {
    std::stringstream ss;
    if (bool(layer)) {
        ss << fmt::format("{}", layer.inner());
    } else {
        ss << "ROOT";
    }
    if (!layer.adjecents().empty()) {
        ss << fmt::format("{{{}}}", fmt::join(layer.adjecents() | vw::transform([](auto a) { return format_all(a); }), ", "));
    }
    return ss.str();
};

template <typename InnerT, typename InfoTrackT>
struct std::hash<Layer<InnerT, InfoTrackT>> {  // NOLINT
    using LayerT = Layer<InnerT, InfoTrackT>;
    std::size_t operator()(const LayerT& layer) const noexcept { return std::hash<typename LayerT::Data*>{}(layer.data_unsafe()); }
};