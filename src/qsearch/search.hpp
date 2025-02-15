#pragma once

#include "circuit/circuit.hpp"
#include "circuit/layer_combine.hpp"

namespace qsearch {

std::vector<Circuit> search(const LayerCombine root, std::size_t count);

}
