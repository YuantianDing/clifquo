
#include "clifford/reduce/local.hpp"
#include "clifford/search.hpp"
int main(int /*argc*/, char** /*argv*/) {
    auto gates = CliffordGate<5ul>::all_gates();
    auto result = optimal_circuit_search(gates);
    fmt::print("{}", result.size());
    return 0;
}
