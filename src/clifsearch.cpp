

#include <filesystem>
#include "clifford/search.hpp"
#include "fmt/base.h"

template <std::size_t N>
void clifsearch() {
    auto path = fmt::format("result/clifford{}.cereal", N);

    if (std::filesystem::exists(path)) { return; }

    auto gates = CliffordGenerator<N>::all_generator();
    auto result = optimal_circuit_search(gates);
    fmt::println("Clifford{}: {} circuits", N, result.size());
    save_clifford_table(result, path);
}

int main(int /*argc*/, char** /*argv*/) {
    clifsearch<2>();
    clifsearch<3>();
    clifsearch<4>();
    clifsearch<5>();
    return 0;
}
