

#include <filesystem>
#include <set>
#include "circuit/gateset/clifford_generator.hpp"
#include "circuit/gateset/permutation.hpp"
#include "clifford/bitsymplectic.hpp"
#include "clifford/gate.hpp"
#include "clifford/reduce/quick.hpp"
#include "range/v3/numeric/accumulate.hpp"
#include "range/v3/view/subrange.hpp"
#include "utils/fmt.hpp"

#define N 4

struct Circ {
    std::vector<circ::CliffordGen<N>> gates;

    Circ() = default;
    Circ(const std::vector<circ::CliffordGen<N>>& v) : gates(v) {}

    [[nodiscard]] inline clfd::BitSymplectic<N> compute() const {
        auto value = clfd::BitSymplectic<N>::identity();
        for (const auto& gate : gates) {
            value = gate * value;
        }
        return value;
    }
    [[nodiscard]] inline Circ init() const {
        auto g2 = gates;
        g2.pop_back();
        return {g2};
    }
    [[nodiscard]] inline Circ tail() const { return {rgs::subrange(gates.begin() + 1, gates.end()) | rgs::to<std::vector>()}; }
};

[[nodiscard]] inline Circ operator*(const circ::CliffordGen<N>& gate, const Circ& circ) {
    auto new_gates = circ.gates;
    new_gates.insert(new_gates.begin(), gate);
    return {new_gates};
}

[[nodiscard]] inline Circ operator*(const Circ& circ, const circ::CliffordGen<N>& gate) {
    auto new_gates = circ.gates;
    new_gates.push_back(gate);
    return {new_gates};
}

auto format_as(const Circ& circ) {
    return fmt::format("{}", fmt::join(circ.gates, " "));
}

void clifsearch() {
    using namespace clfd;
    using namespace std::filesystem;

    std::vector<std::vector<Circ>> layers{{Circ()}, {}};
    std::unordered_map<clfd::BitSymplectic<N>, Circ> visited;
    std::set<std::vector<circ::CliffordGen<N>>> visited_circ;
    auto gates = circ::CliffordGen<N>::all_generator();

    for (auto g : gates) {
        auto circ = Circ();
        circ.gates.push_back(g);
        layers[1].emplace_back(circ);
        visited.emplace(quick_reduce(circ.compute()), circ);
        visited_circ.emplace(circ.gates);
    }

    for (size_t i = 0; i < 15; i++) {
        // if (layers.back().empty()) { break; }
        layers.push_back(std::vector<Circ>{});
        fmt::println("Layer {} -> {}", i, i + 2);
        for (const auto& circ : layers[i]) {
            fmt::println("Working {}", circ);
            for (auto g1 : gates) {
                for (auto g2 : gates) {
                    if (i > 0) {
                        if (!visited_circ.contains((g1 * circ).gates)) { continue; }
                        if (!visited_circ.contains((circ * g2).gates)) { continue; }
                    }
                    auto new_circ = g1 * circ * g2;
                    auto value = new_circ.compute();
                    auto reduced = quick_reduce(value);
                    if (visited.contains(reduced)) {
                        auto existing = visited.at(reduced);
                        auto backtrack = quick_reduce_backtrack(existing.compute(), value);
                        fmt::println(
                            "# {}: ({} {}) {} ({}) {}", new_circ, backtrack.left_perm, backtrack.left_sym, existing, backtrack.right_perm,
                            value.as_raw()
                        );
                    } else {
                        visited.emplace(reduced, new_circ);
                        visited_circ.emplace(new_circ.gates);
                        fmt::println("> {}", new_circ);
                        layers.back().push_back(new_circ);
                    }
                }
            }
        }
        for (auto circ : layers[i]) {
            for (auto g : gates) {
                auto new_circ = i % 2 == 0 ? g * circ : circ * g;
                auto value = new_circ.compute();
                auto reduced = quick_reduce(value);
                if (!visited.contains(reduced)) {
                    fmt::println("!!!!! {} {} {}", g, circ, value.as_raw());
                    std::exit(0);
                }
            }
        }
    }
    auto count = rgs::accumulate(visited | vw::transform([](auto a) { return quick_reduce_eqcount(a.first); }), 0);
    fmt::println("Visited {} circuits", count);
}

int main(int /*argc*/, char** /*argv*/) {
    clifsearch();
    // clifsearch<5>();
    return 0;
}
