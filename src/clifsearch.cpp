

#include <fmt/core.h>
#include <filesystem>
#include "clifford/search.hpp"

template <std::size_t N>
void clifsearch() {
    auto path = fmt::format("result/clifford{}.cereal", N);

    // if (std::filesystem::exists(path)) { return; }

    auto result = clifford::optimal_clifford_search<N>();
    fmt::println("Clifford{}: {} circuits", N, result);
    // save_clifford_table(result, path);
}

int main(int /*argc*/, char** /*argv*/) {
    clifsearch<2>();
    clifsearch<3>();
    clifsearch<4>();
    // clifsearch<5>();
    return 0;
}
