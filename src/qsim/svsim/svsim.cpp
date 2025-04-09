// Functions borrowed from https://github.com/pnnl/SV-Sim/blob/master/svsim/src/svsim_cpu_sin.hpp

#include "svsim.hpp"
#include <algorithm>
#include <cassert>
#include <cmath>
#include "config.hpp"

namespace qsearch {

#define OP_HEAD                                                \
    for (size_t i = 0; i < HALF_DIM; i += 1) {                   \
        size_t outer = (i >> qubit);                             \
        size_t inner = (i & size_t(((size_t)1 << qubit) - size_t(1))); \
        size_t offset = (outer << size_t(qubit + size_t(1)));        \
        size_t pos0 = offset + inner;                            \
        size_t pos1 = pos0 + ((size_t)1 << qubit);

#define OP_TAIL }

const QVal S2I = 0.70710678118654752440;  // 1/sqrt(2)

// NOLINTBEGIN(bugprone-easily-swappable-parameters)
void CX_GATE(QVal* _Nonnull sv_real, QVal* _Nonnull sv_imag, const size_t ctrl, const size_t qubit) {
    const int tid = 0;
    const size_t q0dim = (size_t(1) << std::max(ctrl, qubit));
    const size_t q1dim = (size_t(1) << std::min(ctrl, qubit));
    assert(ctrl != qubit);  // Non-cloning
    const size_t outer_factor = size_t(DIM + q0dim + q0dim - 1) >> size_t(std::max(ctrl, qubit) + 1);
    const size_t mider_factor = size_t(q0dim + q1dim + q1dim - 1) >> size_t(std::min(ctrl, qubit) + 1);
    const size_t inner_factor = q1dim;
    const size_t ctrldim = (size_t(1) << ctrl);

    for (size_t i = tid; i < outer_factor * mider_factor * inner_factor; i++) {
        size_t outer = ((i / inner_factor) / (mider_factor)) * (q0dim + q0dim);
        size_t mider = ((i / inner_factor) % (mider_factor)) * (q1dim + q1dim);
        size_t inner = i % inner_factor;

        size_t pos0 = outer + mider + inner + ctrldim;
        size_t pos1 = outer + mider + inner + q0dim + q1dim;
        const QVal el0_real = sv_real[pos0];
        const QVal el0_imag = sv_imag[pos0];
        const QVal el1_real = sv_real[pos1];
        const QVal el1_imag = sv_imag[pos1];
        sv_real[pos0] = el1_real;
        sv_imag[pos0] = el1_imag;
        sv_real[pos1] = el0_real;
        sv_imag[pos1] = el0_imag;
    }
}

void X_GATE(QVal* _Nonnull sv_real, QVal* _Nonnull sv_imag, const size_t qubit) {
    OP_HEAD;
    const QVal el0_real = sv_real[pos0];
    const QVal el0_imag = sv_imag[pos0];
    const QVal el1_real = sv_real[pos1];
    const QVal el1_imag = sv_imag[pos1];
    sv_real[pos0] = el1_real;
    sv_imag[pos0] = el1_imag;
    sv_real[pos1] = el0_real;
    sv_imag[pos1] = el0_imag;
    OP_TAIL;
}

void Y_GATE(QVal* _Nonnull sv_real, QVal* _Nonnull sv_imag, const size_t qubit) {
    OP_HEAD;
    const QVal el0_real = sv_real[pos0];
    const QVal el0_imag = sv_imag[pos0];
    const QVal el1_real = sv_real[pos1];
    const QVal el1_imag = sv_imag[pos1];
    sv_real[pos0] = el1_imag;
    sv_imag[pos0] = -el1_real;
    sv_real[pos1] = -el0_imag;
    sv_imag[pos1] = el0_real;
    OP_TAIL;
}

void Z_GATE(QVal* _Nonnull sv_real, QVal* _Nonnull sv_imag, const size_t qubit) {
    OP_HEAD;
    const QVal el1_real = sv_real[pos1];
    const QVal el1_imag = sv_imag[pos1];
    sv_real[pos1] = -el1_real;
    sv_imag[pos1] = -el1_imag;
    OP_TAIL;
}

void H_GATE(QVal* _Nonnull sv_real, QVal* _Nonnull sv_imag, const size_t qubit) {
    OP_HEAD;
    const QVal el0_real = sv_real[pos0];
    const QVal el0_imag = sv_imag[pos0];
    const QVal el1_real = sv_real[pos1];
    const QVal el1_imag = sv_imag[pos1];
    sv_real[pos0] = S2I * (el0_real + el1_real);
    sv_imag[pos0] = S2I * (el0_imag + el1_imag);
    sv_real[pos1] = S2I * (el0_real - el1_real);
    sv_imag[pos1] = S2I * (el0_imag - el1_imag);
    OP_TAIL;
}

void SRN_GATE(QVal* _Nonnull sv_real, QVal* _Nonnull sv_imag, const size_t qubit) {
    OP_HEAD;
    const QVal el0_real = sv_real[pos0];
    const QVal el0_imag = sv_imag[pos0];
    const QVal el1_real = sv_real[pos1];
    const QVal el1_imag = sv_imag[pos1];
    sv_real[pos0] = QVal(0.5) * (el0_real + el1_real);
    sv_imag[pos0] = QVal(0.5) * (el0_imag - el1_imag);
    sv_real[pos1] = QVal(0.5) * (el0_real + el1_real);
    sv_imag[pos1] = QVal(0.5) * (-el0_imag + el1_imag);
    OP_TAIL;
}

void R_GATE(QVal* _Nonnull sv_real, QVal* _Nonnull sv_imag, const QVal phase, const size_t qubit) {
    OP_HEAD;
    const QVal el1_real = sv_real[pos1];
    const QVal el1_imag = sv_imag[pos1];
    sv_real[pos1] = -(el1_imag * phase);
    sv_imag[pos1] = el1_real * phase;
    OP_TAIL;
}

void S_GATE(QVal* _Nonnull sv_real, QVal* _Nonnull sv_imag, const size_t qubit) {
    OP_HEAD;
    const QVal el1_real = sv_real[pos1];
    const QVal el1_imag = sv_imag[pos1];
    sv_real[pos1] = -el1_imag;
    sv_imag[pos1] = el1_real;
    OP_TAIL;
}

void SDG_GATE(QVal* _Nonnull sv_real, QVal* _Nonnull sv_imag, const size_t qubit) {
    OP_HEAD;
    const QVal el1_real = sv_real[pos1];
    const QVal el1_imag = sv_imag[pos1];
    sv_real[pos1] = el1_imag;
    sv_imag[pos1] = -el1_real;
    OP_TAIL;
}

void T_GATE(QVal* _Nonnull sv_real, QVal* _Nonnull sv_imag, const size_t qubit) {
    OP_HEAD;
    const QVal el1_real = sv_real[pos1];
    const QVal el1_imag = sv_imag[pos1];
    sv_real[pos1] = S2I * (el1_real - el1_imag);
    sv_imag[pos1] = S2I * (el1_real + el1_imag);
    OP_TAIL;
}

void TDG_GATE(QVal* _Nonnull sv_real, QVal* _Nonnull sv_imag, const size_t qubit) {
    OP_HEAD;
    const QVal el1_real = sv_real[pos1];
    const QVal el1_imag = sv_imag[pos1];
    sv_real[pos1] = S2I * (el1_real + el1_imag);
    sv_imag[pos1] = S2I * (-el1_real + el1_imag);
    OP_TAIL;
}

void RX_GATE(QVal* _Nonnull sv_real, QVal* _Nonnull sv_imag, const QVal theta, const size_t qubit) {
    QVal rx_real = QVal(cos(theta / 2.0));
    QVal rx_imag = -QVal(sin(theta / 2.0));
    OP_HEAD;
    const QVal el0_real = sv_real[pos0];
    const QVal el0_imag = sv_imag[pos0];
    const QVal el1_real = sv_real[pos1];
    const QVal el1_imag = sv_imag[pos1];
    sv_real[pos0] = (rx_real * el0_real) - (rx_imag * el1_imag);
    sv_imag[pos0] = (rx_real * el0_imag) + (rx_imag * el1_real);
    sv_real[pos1] = -(rx_imag * el0_imag) + (rx_real * el1_real);
    sv_imag[pos1] = +(rx_imag * el0_real) + (rx_real * el1_imag);
    OP_TAIL;
}

void RY_GATE(QVal* _Nonnull sv_real, QVal* _Nonnull sv_imag, const QVal theta, const size_t qubit) {
    QVal e0_real = QVal(cos(theta / 2.0));
    QVal e1_real = -QVal(sin(theta / 2.0));
    QVal e2_real = QVal(sin(theta / 2.0));
    QVal e3_real = QVal(cos(theta / 2.0));

    OP_HEAD;
    const QVal el0_real = sv_real[pos0];
    const QVal el0_imag = sv_imag[pos0];
    const QVal el1_real = sv_real[pos1];
    const QVal el1_imag = sv_imag[pos1];
    sv_real[pos0] = (e0_real * el0_real) + (e1_real * el1_real);
    sv_imag[pos0] = (e0_real * el0_imag) + (e1_real * el1_imag);
    sv_real[pos1] = (e2_real * el0_real) + (e3_real * el1_real);
    sv_imag[pos1] = (e2_real * el0_imag) + (e3_real * el1_imag);
    OP_TAIL;
}

void U1_GATE(QVal* _Nonnull sv_real, QVal* _Nonnull sv_imag, const QVal lambda, const size_t qubit) {
    QVal e3_real = QVal(cos(lambda));
    QVal e3_imag = QVal(sin(lambda));

    OP_HEAD;
    const QVal el0_real = sv_real[pos0];
    const QVal el0_imag = sv_imag[pos0];
    const QVal el1_real = sv_real[pos1];
    const QVal el1_imag = sv_imag[pos1];
    sv_real[pos0] = el0_real;
    sv_imag[pos0] = el0_imag;
    sv_real[pos1] = (e3_real * el1_real) - (e3_imag * el1_imag);
    sv_imag[pos1] = (e3_real * el1_imag) + (e3_imag * el1_real);
    OP_TAIL;
}

void ID_GATE(QVal* _Nonnull sv_real, QVal* _Nonnull sv_imag, const size_t qubit) {}  // NOLINT

void C1_GATE(
    QVal* _Nonnull sv_real,
    QVal* _Nonnull sv_imag,
    const QVal e0_real,
    const QVal e0_imag,
    const QVal e1_real,
    const QVal e1_imag,
    const QVal e2_real,
    const QVal e2_imag,
    const QVal e3_real,
    const QVal e3_imag,
    const size_t qubit
) {
    OP_HEAD;
    const QVal el0_real = sv_real[pos0];
    const QVal el0_imag = sv_imag[pos0];
    const QVal el1_real = sv_real[pos1];
    const QVal el1_imag = sv_imag[pos1];
    sv_real[pos0] = (e0_real * el0_real) - (e0_imag * el0_imag) + (e1_real * el1_real) - (e1_imag * el1_imag);
    sv_imag[pos0] = (e0_real * el0_imag) + (e0_imag * el0_real) + (e1_real * el1_imag) + (e1_imag * el1_real);
    sv_real[pos1] = (e2_real * el0_real) - (e2_imag * el0_imag) + (e3_real * el1_real) - (e3_imag * el1_imag);
    sv_imag[pos1] = (e2_real * el0_imag) + (e2_imag * el0_real) + (e3_real * el1_imag) + (e3_imag * el1_real);
    OP_TAIL;
}

void U2_GATE(QVal* _Nonnull sv_real, QVal* _Nonnull sv_imag, const QVal phi, const QVal lambda, const size_t qubit) {
    QVal e0_real = S2I;
    QVal e0_imag = 0;
    QVal e1_real = -S2I * QVal(cos(lambda));
    QVal e1_imag = -S2I * QVal(sin(lambda));
    QVal e2_real = S2I * QVal(cos(phi));
    QVal e2_imag = S2I * QVal(sin(phi));
    QVal e3_real = S2I * QVal(cos(phi + lambda));
    QVal e3_imag = S2I * QVal(sin(phi + lambda));
    C1_GATE(sv_real, sv_imag, e0_real, e0_imag, e1_real, e1_imag, e2_real, e2_imag, e3_real, e3_imag, qubit);
}

void U3_GATE(QVal* _Nonnull sv_real, QVal* _Nonnull sv_imag, const QVal theta, const QVal phi, const QVal lambda, const size_t qubit) {
    QVal e0_real = QVal(cos(theta / 2.));
    QVal e0_imag = 0;
    QVal e1_real = -QVal(cos(lambda)) * QVal(sin(theta / 2.));
    QVal e1_imag = -QVal(sin(lambda)) * QVal(sin(theta / 2.));
    QVal e2_real = QVal(cos(phi) * QVal(sin(theta / 2.)));
    QVal e2_imag = QVal(sin(phi)) * QVal(sin(theta / 2.));
    QVal e3_real = QVal(cos(phi + lambda)) * QVal(cos(theta / 2.));
    QVal e3_imag = QVal(sin(phi + lambda)) * QVal(cos(theta / 2.));
    C1_GATE(sv_real, sv_imag, e0_real, e0_imag, e1_real, e1_imag, e2_real, e2_imag, e3_real, e3_imag, qubit);
}

void RZ_GATE(QVal* _Nonnull sv_real, QVal* _Nonnull sv_imag, const QVal phi, const size_t qubit) {
    U1_GATE(sv_real, sv_imag, phi, qubit);
}

void CZ_GATE(QVal* _Nonnull sv_real, QVal* _Nonnull sv_imag, const size_t a, const size_t b) {
    H_GATE(sv_real, sv_imag, b);
    CX_GATE(sv_real, sv_imag, a, b);
    H_GATE(sv_real, sv_imag, b);
}

void CY_GATE(QVal* _Nonnull sv_real, QVal* _Nonnull sv_imag, const size_t a, const size_t b) {
    SDG_GATE(sv_real, sv_imag, b);
    CX_GATE(sv_real, sv_imag, a, b);
    S_GATE(sv_real, sv_imag, b);
}

void CH_GATE(QVal* sv_real, QVal* _Nonnull sv_imag, const size_t a, const size_t b) {
    H_GATE(sv_real, sv_imag, b);
    SDG_GATE(sv_real, sv_imag, b);
    CX_GATE(sv_real, sv_imag, a, b);
    H_GATE(sv_real, sv_imag, b);
    T_GATE(sv_real, sv_imag, b);
    CX_GATE(sv_real, sv_imag, a, b);
    T_GATE(sv_real, sv_imag, b);
    H_GATE(sv_real, sv_imag, b);
    S_GATE(sv_real, sv_imag, b);
    X_GATE(sv_real, sv_imag, b);
    S_GATE(sv_real, sv_imag, a);
}

void SWAP_GATE(QVal* _Nonnull sv_real, QVal* _Nonnull sv_imag, const size_t a, const size_t b) {
    CX_GATE(sv_real, sv_imag, a, b);
    CX_GATE(sv_real, sv_imag, b, a);
    CX_GATE(sv_real, sv_imag, a, b);
}
// NOLINTEND(bugprone-easily-swappable-parameters)

}  // namespace qsearch