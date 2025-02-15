#include "fingerprint.hpp"
#include <complex>
#include <vector>

namespace qsearch {

size_t log_pow(size_t ex) {
    size_t res = 1;
    auto base = static_cast<size_t>(0xA67);  // A Prime number
    while (ex > 0) {
        if (bool(ex % 2)) { res = res * base; }
        base *= base;
        ex /= 2;
    }
    return res;
}

std::vector<std::size_t> fingerprint_vec(QState state) noexcept {
    auto vec =
        pauli_decomposition(state) | vw::transform([](auto sum) { return qval_truncate(std::abs(sum)); }) | rgs::to<std::vector<std::size_t>>();
    std::sort(vec.begin(), vec.end());
    return vec;
}
size_t fingerprint(QState state) noexcept {
    size_t result = 0ul;
    for (const auto sum : pauli_decomposition(state)) {
        const auto total_hash = std::hash<uint32_t>{}(qval_truncate(std::abs(sum)));

        // Hash based on plus, for multisets
        // https://stackoverflow.com/questions/36520235/algorithm-for-hash-crc-of-unordered-multiset
        result += log_pow(total_hash);
    }
    return result;
}

}  // namespace qsearch