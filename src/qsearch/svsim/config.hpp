#pragma once

#include <cmath>
#include <cstddef>
#include <cstdint>

namespace qsearch {

using QIdx = uint_fast8_t;
using QVal = double;
using size_t = std::size_t;

constexpr const QIdx NQUBITS = 4;
constexpr const char* CLIFFORD_CEREAL_PATH = "result/clifford4.cereal";
inline constexpr uint32_t qval_truncate(QVal input) {
    return uint32_t(input * QVal(16384.0));
}

};  // namespace qsearch
