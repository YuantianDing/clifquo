#pragma once
#include <doctest/doctest.h>
// NOLINTNEXTLINE
#define TEST_FN(name) DOCTEST_CREATE_AND_REGISTER_FUNCTION(DOCTEST_ANONYMOUS(testing_##name), #name)
#ifdef NDEBUG
#define DEBUG_ONLY(...)
#else
#define DEBUG_ONLY(...) __VA_ARGS__
#endif