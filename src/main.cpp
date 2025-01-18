
#ifndef NDEBUG
#warning "Compiling in debug mode!"
#endif

#include "clifford/reduce/local.hpp"
#include "clifford/search.hpp"
int main(int /*argc*/, char** /*argv*/) {
    auto gates = CliffordGate<5ul>::all_generator();
    auto result = optimal_circuit_search(gates);
    fmt::println("{}", result.size());
    return 0;
}
