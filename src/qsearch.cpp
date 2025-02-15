
#include <fstream>
#include <range/v3/view/concat.hpp>
#include "qsearch/search.hpp"

int main(int /*argc*/, char** /*argv*/) {
    auto clifford_root = qsearch::build_clifford_graph(qsearch::CLIFFORD_CEREAL_PATH);
    using namespace qsearch::gate;
    std::vector<Gate> gates = vw::concat(Gate1<T>::all({}), Gate1<S>::all({}), Gate1<H>::all({})) | rgs::to<std::vector>();
    auto other_root = qsearch::build_layer_graph(gates);
    fmt::println("Layer Graph: {}", format_all(other_root));

    auto layer_root = qsearch::LayerCombine{clifford_root, other_root};
    auto result = qsearch::search(layer_root, 100000);
    json j = json::array();

    for (auto&& c1 : result) {
        j.push_back(qsearch::circuit_as_json(c1));
    }

    std::ofstream os("neighbors.json", std::ios::out);
    os << j.dump();
    return 0;
}