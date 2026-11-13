// Staple Stable
//
// Model: a horizontal cut is a full grid line across the sheet and splits every
// current piece it meets; likewise vertical.  Hence the final pieces are exactly
// the cells of the grid induced by the chosen horizontal cut lines H and vertical
// cut lines V, and the number of cuts is |H| + |V|.
//
// If the largest horizontal piece length is P and the largest vertical piece
// length is D, we need exactly ceil(h/P) horizontal pieces and ceil(w/D) vertical
// pieces, so the answer is
//
//      min over P,D >= 1 with P*D <= s of  (ceil(h/P)-1) + (ceil(w/D)-1).
//
// For a fixed P the best D is floor(s/P), and it is never useful to take P > h or
// P > s, so
//
//      K = min over 1 <= P <= min(s,h) of  G(P),
//      G(P) = ceil(h/P) + ceil(w/floor(s/P)),        answer = K - 2.
//
// G is searched with three exact prunings:
//   * integer bounds:  G(P) < U  =>  ceil(h/P) <= U-2  and  ceil(w/floor(s/P)) <= U-2
//   * convexity bound: G(P) >= phi(P) = h/P + w*P/s, and phi is convex, so only
//     P with phi(P) <= U (a contiguous interval found by binary search) can matter
//   * inside the surviving interval the minimum is attained either at a point that
//     is the maximum of a block of constant floor(s/P), or at a point that is the
//     minimum of a block of constant ceil(h/P); both enumerations are O(#blocks).
//
// Everything is done in exact integer arithmetic (128 bit where needed).

#include <bits/stdc++.h>
using namespace std;

typedef unsigned long long u64;
typedef __uint128_t u128;

static inline u64 cdiv(u64 a, u64 b) { return a / b + (a % b != 0); }

// floor(sqrt(x)) for x < 2^128 (result fits in u64 because x <= 10^36)
static inline u64 isqrt128(u128 x) {
    if (x == 0) return 0;
    u64 r = (u64)sqrtl((long double)x);
    while (r > 0 && (u128)r * r > x) --r;
    while ((u128)(r + 1) * (r + 1) <= x) ++r;
    return r;
}

// G(P) = ceil(h/P) + ceil(w/floor(s/P)); requires 1 <= P <= s
static inline u64 evalG(u64 h, u64 w, u64 s, u64 P) {
    u64 d = s / P;
    return cdiv(h, P) + cdiv(w, d);
}

// phi(P) = h/P + w*P/s <= U   (multiplied by s*P:  h*s <= P*(s*U - w*P))
static inline bool phiLE(u64 h, u64 w, u64 s, u64 U, u64 P) {
    u128 sU = (u128)s * U;
    u128 wP = (u128)w * P;
    if (sU < wP) return false;
    u128 T = sU - wP;                 // s*U - w*P
    u128 hs = (u128)h * s;
    u128 q = hs / P, r = hs % P;      // compare h*s/P with T
    if (q != T) return q < T;
    return r == 0;
}

// phi(P+1) >= phi(P)  <=>  h*s <= w*P*(P+1)   (evaluated without overflow)
static inline bool vertexAt(u64 h, u64 w, u64 s, u64 P) {
    u128 hs = (u128)h * s;
    u128 W = (u128)w * ((u128)P + 1);
    u128 q = hs / P, r = hs % P;      // compare h*s/P with w*(P+1)
    if (q != W) return q < W;
    return r == 0;
}

// K = min over 1 <= P <= min(s,h) of G(P)
static u64 solveK(u64 h, u64 w, u64 s) {
    u64 Pmax = min(s, h);
    u64 U = ~(u64)0;                  // best known value of G

    auto tryP = [&](u64 P) {
        if (P >= 1 && P <= Pmax) U = min(U, evalG(h, w, s, P));
    };

    // ---- a good upper bound: evaluate the continuous optima and their neighbours
    {
        u64 b1 = isqrt128((u128)((u128)h * s / w));   // ~ sqrt(h*s/w)
        u64 b2 = isqrt128((u128)((u128)w * s / h));   // ~ sqrt(w*s/h)
        for (long long d = -8; d <= 8; ++d) {
            long long x = (long long)b1 + d;
            if (x >= 1) tryP((u64)x);
        }
        for (long long d = -8; d <= 8; ++d) {
            long long x = (long long)b2 + d;
            if (x >= 1 && (u64)x <= min(s, w)) tryP(min(Pmax, s / (u64)x));
        }
        tryP(1);
        tryP(Pmax);
        tryP(Pmax > 1 ? Pmax - 1 : 1);
        tryP(min(Pmax, (u64)2));
        for (u64 D = 1; D <= 8; ++D) tryP(min(Pmax, s / D));
        u64 Dw = min(s, w);
        for (long long d = -8; d <= 8; ++d) {
            long long x = (long long)Dw + d;
            if (x >= 1) tryP(min(Pmax, s / (u64)x));
        }
    }
    if (U <= 2) return U;             // cannot be beaten (answer would be <= 0)

    // ---- discrete vertex of the convex phi: smallest P with phi(P+1) >= phi(P)
    u64 Pv;
    if (!vertexAt(h, w, s, Pmax)) {
        Pv = Pmax;
    } else {
        u64 a = 1, b = Pmax;
        while (a < b) {
            u64 m = a + (b - a) / 2;
            if (vertexAt(h, w, s, m)) b = m; else a = m + 1;
        }
        Pv = a;
    }

    // ---- interval where the convex lower bound phi is <= U
    u64 Plo, Phi;
    if (!phiLE(h, w, s, U, Pv)) {
        // cannot happen (phi(Pv) = min_P phi(P) <= G(P_best) = U); keep the search safe
        Plo = 1;
        Phi = Pmax;
    } else {
        {
            u64 a = 1, b = Pv;                   // phi non-increasing on [1,Pv]
            while (a < b) {
                u64 m = a + (b - a) / 2;
                if (phiLE(h, w, s, U, m)) b = m; else a = m + 1;
            }
            Plo = a;
        }
        {
            u64 a = Pv, b = Pmax;                // phi non-decreasing on [Pv,Pmax]
            while (a < b) {
                u64 m = a + (b - a + 1) / 2;
                if (phiLE(h, w, s, U, m)) a = m; else b = m - 1;
            }
            Phi = a;
        }
    }

    // ---- integer necessary conditions for G(P) < U  (both terms are >= 1)
    u64 V = U - 2;
    u64 Alo = cdiv(h, V);                        // ceil(h/P) <= V
    u64 Bhi = min(Pmax, s / cdiv(w, V));         // floor(s/P) >= ceil(w/V)

    u64 A = max(max(Alo, Plo), (u64)1);
    u64 B = min(min(Bhi, Phi), Pmax);
    if (A > B) return U;

    // ---- enumerate the surviving candidates with the cheapest of three strategies
    u64 best = U;
    u64 c1 = B - A + 1;                              // every integer P
    u64 c2 = s / A - s / B + 1;                      // blocks of constant floor(s/P)
    u64 c3 = cdiv(h, A) - cdiv(h, B) + 1;            // blocks of constant ceil(h/P)

    if (c1 <= c2 && c1 <= c3) {
        for (u64 P = A; P <= B; ++P) best = min(best, evalG(h, w, s, P));
    } else if (c2 <= c3) {
        u64 Dhi = s / A, Dlo = s / B;
        for (u64 D = Dlo; D <= Dhi; ++D) {
            u64 P = min(B, s / D);                   // largest P of this block
            best = min(best, evalG(h, w, s, P));
        }
    } else {
        u64 vhi = cdiv(h, A), vlo = cdiv(h, B);
        for (u64 v = vlo; v <= vhi; ++v) {
            u64 P = min(B, max(A, cdiv(h, v)));      // smallest P of this block
            best = min(best, evalG(h, w, s, P));
        }
    }
    return best;
}

int main() {
    int t;
    if (scanf("%d", &t) != 1) return 0;
    while (t--) {
        u64 h, w, s;
        if (scanf("%llu %llu %llu", &h, &w, &s) != 3) return 0;
        u64 ans;
        if ((u128)h * w <= s) ans = 0;               // the whole sheet already fits
        else ans = solveK(h, w, s) - 2;
        printf("%llu\n", ans);
    }
    return 0;
}
