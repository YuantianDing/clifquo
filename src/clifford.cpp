
#include <fstream>
#include "cereal/archives/binary.hpp"
#include "clifford/search.hpp"

template <typename T>
void save_binary(const std::string& filename, const T& obj) {
    std::ofstream ofs(filename, std::ios::binary);
    if (!ofs) { throw std::runtime_error("Failed to open file for writing"); }
    cereal::BinaryOutputArchive archive(ofs);
    archive(obj);
}

void clifsearch() {
    save_binary("result/clifford2.tree.cereal", clfd::search::search<2>(true));
    save_binary("result/clifford3.tree.cereal", clfd::search::search<3>(true));
}

int main(int /*argc*/, char** /*argv*/) {
    clifsearch();
    // clifsearch<5>();
    return 0;
}
