#pragma once

#include <experimental/random>

class Uncopiable {
   public:
    Uncopiable(Uncopiable&&) = delete;
    Uncopiable& operator=(Uncopiable&&) = delete;
    Uncopiable(const Uncopiable&) = delete;
    Uncopiable& operator=(const Uncopiable&) = delete;
};
