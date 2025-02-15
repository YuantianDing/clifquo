#pragma once

#define PURE_VIRTUAL(name)                  \
    name() = default;                       \
    name(const name&) = default;            \
    name(name&&) = delete;                  \
    name& operator=(const name&) = default; \
    name& operator=(name&&) = delete;       \
    virtual ~name() = default;
