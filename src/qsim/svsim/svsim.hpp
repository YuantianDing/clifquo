#pragma once

#include <fmt/format.h>
#include <algorithm>
#include <cassert>
#include <cmath>
#include <complex>
#include <numbers>
#include <unordered_set>
#include <variant>
#include "../../utils/bitvec.hpp"
#include "config.hpp"
#include "fmt/ranges.h"

namespace qsearch {

constexpr const size_t DIM = 1ul << NQUBITS;
constexpr const size_t HALF_DIM = 1ul << size_t(NQUBITS - 1);
constexpr const QVal PI = std::numbers::pi_v<QVal>;

struct QState {
    QVal* _Nonnull real;
    QVal* _Nonnull imag;

    [[nodiscard]] inline static constexpr QState alloc() noexcept {
        return QState{
            .real = static_cast<QVal* _Nonnull>(::operator new(DIM * sizeof(QVal))),
            .imag = static_cast<QVal* _Nonnull>(::operator new(DIM * sizeof(QVal)))
        };
    }
    inline constexpr void normalize_phase() const noexcept {
        std::complex<QVal> unit = QVal(0.0);
        for (size_t i = 0; i < DIM; i++) {
            if (qval_truncate(real[i]) == 0 && qval_truncate(imag[i]) == 0) { continue; }
            if (unit == QVal(0.0)) {
                auto statei = std::complex<QVal>(real[i], imag[i]);
                unit = std::sqrt(std::norm(statei)) / statei;
                assert(std::abs(std::norm(unit) - 1.0) < 1e-6);
            }
            auto statei = std::complex<QVal>(real[i], imag[i]);
            auto new_statei = statei * unit;
            real[i] = new_statei.real();
            imag[i] = new_statei.imag();
        }
    }
    [[nodiscard]] inline constexpr std::complex<QVal> dot(QState other) const noexcept {
        std::complex<QVal> sum = 0.0f;
        for (size_t i = 0; i < DIM; i++) {
            sum += std::complex<QVal>(real[i], imag[i]) * std::complex<QVal>(other.real[i], other.imag[i]);
        }
        return sum;
    }
    [[nodiscard]] inline static constexpr QState random() noexcept {
        auto state = alloc();
        for (size_t i = 0; i < DIM; i++) {
            state.real[i] = static_cast<QVal>(std::rand()) / static_cast<QVal>(RAND_MAX);  // NOLINT
            state.imag[i] = static_cast<QVal>(std::rand()) / static_cast<QVal>(RAND_MAX);  // NOLINT
        }
        state.normalize();
        return state;
    }
    [[nodiscard]] inline constexpr QVal norm() const noexcept {
        QVal norm = 0.0f;
        for (size_t i = 0; i < DIM; i++) {
            norm += real[i] * real[i] + imag[i] * imag[i];
        }
        return std::sqrt(norm);
    }
    inline constexpr void normalize() const noexcept {
        QVal n = norm();
        for (size_t i = 0; i < DIM; i++) {
            real[i] /= n;
            imag[i] /= n;
        }
    }
    [[nodiscard]] inline static constexpr QState zero() noexcept {
        auto state = alloc();
        std::fill(state.real, state.real + DIM, 0.0f);
        std::fill(state.imag, state.imag + DIM, 0.0f);
        state.real[0] = 1.0f;
        return state;
    }
    [[nodiscard]] inline static constexpr QState clone(QState state) noexcept {
        auto nstate = alloc();
        std::copy(state.real, state.real + DIM, nstate.real);
        std::copy(state.imag, state.imag + DIM, nstate.imag);
        return nstate;
    }

    inline constexpr void free() const noexcept {
        delete real;  // NOLINT
        delete imag;  // NOLINT
    }

    struct Equal {
        [[nodiscard]] inline constexpr bool operator()(const QState& self, const QState& other) const noexcept {
            return std::equal(self.real, self.real + DIM, other.real, [](QVal a, QVal b) { return qval_truncate(a) == qval_truncate(b); }) &&
                   std::equal(self.imag, self.imag + DIM, other.imag, [](QVal a, QVal b) { return qval_truncate(a) == qval_truncate(b); });
        }
    };

    struct Hasher {
        [[nodiscard]] inline constexpr std::size_t operator()(const QState& s) const noexcept {
            std::size_t seed = 0;
            for (size_t i = 0; i < DIM; i++) {
                seed ^= std::hash<uint32_t>{}(qval_truncate(s.real[i])) + 0x9e3779b9ul + (seed << 6ul) + (seed >> 2ul);
                seed ^= std::hash<uint32_t>{}(qval_truncate(s.imag[i])) + 0x9e3779b9ul + (seed << 6ul) + (seed >> 2ul);
            }
            return seed;
        }
    };

    template <typename T>
    using Map = std::unordered_map<QState, T, Hasher, Equal>;
    using Set = std::unordered_set<QState, Hasher, Equal>;
};

inline auto format_as(QState state) {
    std::stringstream ss;
    for (size_t i = 0; i < DIM; i++) {
        ss << fmt::format("{}+{}j ", state.real[i], state.imag[i]);
    }
    return ss.str();
}

void CX_GATE(QVal* _Nonnull sv_real, QVal* _Nonnull sv_imag, const size_t ctrl, const size_t qubit);
void X_GATE(QVal* _Nonnull sv_real, QVal* _Nonnull sv_imag, const size_t qubit);
void Y_GATE(QVal* _Nonnull sv_real, QVal* _Nonnull sv_imag, const size_t qubit);
void Z_GATE(QVal* _Nonnull sv_real, QVal* _Nonnull sv_imag, const size_t qubit);
void H_GATE(QVal* _Nonnull sv_real, QVal* _Nonnull sv_imag, const size_t qubit);
void SRN_GATE(QVal* _Nonnull sv_real, QVal* _Nonnull sv_imag, const size_t qubit);
void R_GATE(QVal* _Nonnull sv_real, QVal* _Nonnull sv_imag, const QVal phase, const size_t qubit);
void S_GATE(QVal* _Nonnull sv_real, QVal* _Nonnull sv_imag, const size_t qubit);
void SDG_GATE(QVal* _Nonnull sv_real, QVal* _Nonnull sv_imag, const size_t qubit);
void T_GATE(QVal* _Nonnull sv_real, QVal* _Nonnull sv_imag, const size_t qubit);
void TDG_GATE(QVal* _Nonnull sv_real, QVal* _Nonnull sv_imag, const size_t qubit);
void RX_GATE(QVal* _Nonnull sv_real, QVal* _Nonnull sv_imag, const QVal theta, const size_t qubit);
void RY_GATE(QVal* _Nonnull sv_real, QVal* _Nonnull sv_imag, const QVal theta, const size_t qubit);
void U1_GATE(QVal* _Nonnull sv_real, QVal* _Nonnull sv_imag, const QVal lambda, const size_t qubit);
void ID_GATE(QVal* _Nonnull sv_real, QVal* _Nonnull sv_imag, const size_t qubit);
void U2_GATE(QVal* _Nonnull sv_real, QVal* _Nonnull sv_imag, const QVal phi, const QVal lambda, const size_t qubit);
void U3_GATE(QVal* _Nonnull sv_real, QVal* _Nonnull sv_imag, const QVal theta, const QVal phi, const QVal lambda, const size_t qubit);
void RZ_GATE(QVal* _Nonnull sv_real, QVal* _Nonnull sv_imag, const QVal phi, const size_t qubit);
void CZ_GATE(QVal* _Nonnull sv_real, QVal* _Nonnull sv_imag, const size_t a, const size_t b);
void CY_GATE(QVal* _Nonnull sv_real, QVal* _Nonnull sv_imag, const size_t a, const size_t b);
void CH_GATE(QVal* sv_real, QVal* _Nonnull sv_imag, const size_t a, const size_t b);
void SWAP_GATE(QVal* _Nonnull sv_real, QVal* _Nonnull sv_imag, const size_t a, const size_t b);

}  // namespace qsearch

inline auto format_as(qsearch::QState state) {
    return fmt::format("[{}] + [{}]i", fmt::join(state.real, state.real + qsearch::DIM, " "), fmt::join(state.imag, state.imag + qsearch::DIM, " "));
}