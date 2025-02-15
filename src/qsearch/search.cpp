#include "search.hpp"
#include <cstddef>
#include <queue>
#include "circuit/circuit.hpp"
#include "circuit/layer_combine.hpp"
#include "measure/fingerprint.hpp"
#include "svsim/svsim.hpp"

namespace qsearch {
struct Point {
    Circuit circuit;
    QState state;
    Point(Circuit circuit, QState state) : circuit(circuit), state(state) {}
    // explicit Point(std::pair<Circuit*, QState> pair) : circuit(pair.first), state(pair.second) {}  // NOLINT
};

struct NoHash {
    [[nodiscard]] inline constexpr size_t operator()(size_t x) const noexcept { return x; }
};

std::vector<Circuit> search(const LayerCombine layer_root, std::size_t count) {
    std::vector<Circuit> result;
    auto root = Circuit();

    auto initial_state = QState::random();
    initial_state.normalize_phase();
    fmt::println("{}", initial_state);

    auto bfs_boundry = std::queue<Point>();
    auto visited_states = QState::Set();
    auto fingerprint_table = std::unordered_map<size_t, Circuit, NoHash>();

    bfs_boundry.emplace(root, initial_state);
    visited_states.emplace(initial_state);

    fmt::println("Start Searching ...");
    std::size_t counter = 0;
    fmt::println("decomp: {}", fingerprint_vec(initial_state));

    while (!bfs_boundry.empty()) {
        auto point = bfs_boundry.front();
        bfs_boundry.pop();

        // fmt::println("Start {}", point.circuit);
        generate_adjecent_circuit(
            point.circuit, layer_root,
            [&counter, &visited_states, &bfs_boundry, &result, &fingerprint_table, point, count](Circuit circuit) {
                // fmt::println("  at {}", circuit);
                auto new_state = QState::clone(point.state);
                circuit.head().inner().do_apply(new_state);
                new_state.normalize_phase();
                using namespace qsearch::gate;

                // Return when the state is already visited
                if (!visited_states.insert(new_state).second) {
                    new_state.free();
                    circuit.free();
                    return true;
                }

                if (!Gate::is<Gate1<T>>(circuit.head().inner())) {
                    bfs_boundry.emplace(circuit, new_state);
                    return true;
                }

                auto finger = fingerprint(new_state);
                if (auto it = fingerprint_table.find(finger); it != fingerprint_table.end()) {
                    return true;
                } else {
                    counter += 1;
                    result.emplace_back(circuit);
                    fmt::println("{}: {} {}", counter, circuit, finger);
                    fingerprint_table[finger] = circuit;
                    return counter <= count;
                }
            }
        );
        if (counter > count) { break; }
    }
    return result;
}

}  // namespace qsearch