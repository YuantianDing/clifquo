#pragma once

#include <functional>
#include <queue>
#include <unordered_map>
#include <vector>
#include "fmt/base.h"

namespace bfs {

template <typename Point, typename Data, typename AccF, typename NextF>
constexpr std::unordered_map<Point, Data> search_entirely(std::pair<Point, Data> start, NextF next_f, AccF acc_f) {
    std::unordered_map<Point, Data> result;
    std::queue<std::pair<Point, Data>> bfs_boundry;

    bfs_boundry.push(start);
    result[start.first] = start.second;
    while (!bfs_boundry.empty()) {
        auto current = bfs_boundry.front();
        bfs_boundry.pop();
        for (auto&& next_point : next_f(current.first, current.second)) {
            if (auto&& search = result.find(next_point.first); search != result.end()) {
                search->second = acc_f(search->second, next_point.second);
            } else {
                result.insert(next_point);
                bfs_boundry.push(next_point);
                if (result.size() % 5000 == 0) { fmt::println("Searching: {} {}", bfs_boundry.size(), result.size()); }
                if (result.size() % 100000 == 0) { std::abort(); }
            }
        }
    }
    return result;
}

template <typename Point, typename Data, typename AccF, typename NextF>
constexpr std::unordered_map<Point, Data> search_until_depth(std::pair<Point, Data> start, std::size_t depth_limit, NextF next_f, AccF acc_f) {
    std::unordered_map<Point, Data> result;
    std::vector<std::pair<Point, Data>> bfs_boundry_old;
    std::vector<std::pair<Point, Data>> bfs_boundry_new;
    bfs_boundry_old.push(start);

    result[start.first] = start.second;
    for (std::size_t depth = 1; depth <= depth_limit; depth++) {
        for (auto current : bfs_boundry_old) {
            for (auto&& next_point : next_f(depth, current.first, current.second)) {
                if (auto&& search = result.find(next_point.first); search != result.end()) {
                    search->second = acc_f(search->second, next_point.second);
                } else {
                    result.insert(next_point);
                    bfs_boundry_new.push(next_point);
                    if (result.size() % 5000 == 0) {
                        fmt::println("Searching: {} {} {} {}", next_point.first, next_point.second, depth, result.size());
                    }
                }
            }
        }
        bfs_boundry_old = std::move(bfs_boundry_new);
        assert(bfs_boundry_new.empty());
    }
    return result;
}

}  // namespace bfs
