#include <bits/stdc++.h>
using namespace std;

/*
  Randomized test based on the Matrix-Tree theorem.

  Over GF(2^31), assign an independent random value x_i to every edge.
  Let Q(x) = sum over spanning trees of A of prod x_i.
  Let P(x) = sum over common spanning trees of A and B of prod x_i.

  Alice succeeds iff there is a spanning tree of A which is not a spanning tree of B.
  Thus Alice succeeds iff Q != P as polynomials.

  By Cauchy-Binet:
      Q = det(A * diag(x) * A^T)
      P = det(A * diag(x) * B^T)
  where A and B are reduced incidence matrices over characteristic 2.

  We compare these two determinants for random x. If they differ, YES.
  Otherwise, with very high probability, NO.
*/

static const uint32_t MOD_POLY = 0x80000009u; // x^31 + x^3 + 1

static uint32_t mulTab[1024][256][4];

uint32_t reduce64(uint64_t x) {
    for (int i = 60; i >= 31; --i) {
        if ((x >> i) & 1ULL) {
            x ^= (uint64_t)MOD_POLY << (i - 31);
        }
    }
    return (uint32_t)x;
}

uint64_t clmul(uint32_t a, uint32_t b) {
    uint64_t res = 0;
    for (int i = 0; i < 31; ++i) {
        if ((b >> i) & 1u) res ^= (uint64_t)a << i;
    }
    return res;
}

void initField() {
    for (int pb = 0; pb < 4; ++pb) {
        for (int c = 0; c < 256; ++c) {
            int idx = pb * 256 + c;
            uint32_t s = reduce64((uint64_t)c << (8 * pb));

            for (int pa = 0; pa < 4; ++pa) {
                for (int v = 0; v < 256; ++v) {
                    uint32_t chunk = reduce64((uint64_t)v << (8 * pa));
                    mulTab[idx][v][pa] = reduce64(clmul(chunk, s));
                }
            }
        }
    }
}

inline uint32_t gfMul(uint32_t a, uint32_t b) {
    uint32_t res = 0;
    for (int pb = 0; pb < 4; ++pb) {
        uint32_t c = (b >> (8 * pb)) & 255u;
        if (!c) continue;
        int idx = pb * 256 + c;
        for (int pa = 0; pa < 4; ++pa) {
            uint32_t v = (a >> (8 * pa)) & 255u;
            res ^= mulTab[idx][v][pa];
        }
    }
    return res;
}

uint32_t gfPow(uint32_t a, uint32_t e) {
    uint32_t res = 1;
    while (e) {
        if (e & 1u) res = gfMul(res, a);
        a = gfMul(a, a);
        e >>= 1;
    }
    return res;
}

inline uint32_t gfInv(uint32_t a) {
    // inverse = a^(2^31 - 2)
    return gfPow(a, 2147483646u);
}

using Row = unordered_map<int, uint32_t>;

uint32_t determinant(vector<Row>& mat) {
    int n = (int)mat.size();
    uint32_t det = 1;

    for (int col = 0; col < n; ++col) {
        int pivot = -1;

        auto it0 = mat[col].find(col);
        if (it0 != mat[col].end() && it0->second != 0) {
            pivot = col;
        } else {
            for (int r = col + 1; r < n; ++r) {
                auto it = mat[r].find(col);
                if (it != mat[r].end() && it->second != 0) {
                    pivot = r;
                    break;
                }
            }
        }

        if (pivot == -1) return 0;

        if (pivot != col) swap(mat[col], mat[pivot]);

        uint32_t pivVal = mat[col][col];
        det = gfMul(det, pivVal);

        uint32_t invPivot = gfInv(pivVal);

        for (int i = col + 1; i < n; ++i) {
            auto it = mat[i].find(col);
            if (it == mat[i].end() || it->second == 0) continue;

            uint32_t entry = it->second;
            uint32_t factor = gfMul(entry, invPivot);

            for (const auto& kv : mat[col]) {
                int c = kv.first;
                if (c == col) continue;

                uint32_t add = gfMul(factor, kv.second);
                auto it2 = mat[i].find(c);

                if (it2 == mat[i].end()) {
                    mat[i].emplace(c, add);
                } else {
                    it2->second ^= add;
                    if (it2->second == 0) mat[i].erase(it2);
                }
            }

            mat[i].erase(col);
        }
    }

    return det;
}

inline void addCell(vector<Row>& mat, int r, int c, uint32_t val) {
    if (val == 0) return;

    auto& row = mat[r];
    auto it = row.find(c);

    if (it == row.end()) {
        row.emplace(c, val);
    } else {
        it->second ^= val;
        if (it->second == 0) row.erase(it);
    }
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    initField();

    mt19937 rng(chrono::steady_clock::now().time_since_epoch().count());

    int T;
    cin >> T;

    while (T--) {
        int n, m;
        cin >> n >> m;

        vector<int> ua(m), va(m), ub(m), vb(m);

        for (int i = 0; i < m; ++i) {
            cin >> ua[i] >> va[i] >> ub[i] >> vb[i];
            --ua[i];
            --va[i];
            --ub[i];
            --vb[i];
        }

        int sz = n - 1;
        bool answer = false;

        for (int trial = 0; trial < 2 && !answer; ++trial) {
            vector<Row> matQ(sz), matP(sz);

            for (int i = 0; i < m; ++i) {
                uint32_t x = (uint32_t)rng() & 0x7FFFFFFFu;
                if (x == 0) x = 1;

                vector<int> ea, eb;

                if (ua[i] != n - 1) ea.push_back(ua[i]);
                if (va[i] != n - 1) ea.push_back(va[i]);

                if (ub[i] != n - 1) eb.push_back(ub[i]);
                if (vb[i] != n - 1) eb.push_back(vb[i]);

                // matQ += x * a * a^T
                for (int a : ea) {
                    for (int c : ea) {
                        addCell(matQ, a, c, x);
                    }
                }

                // matP += x * a * b^T
                for (int a : ea) {
                    for (int b : eb) {
                        addCell(matP, a, b, x);
                    }
                }
            }

            uint32_t detQ = determinant(matQ);
            uint32_t detP = determinant(matP);

            if (detQ != detP) {
                answer = true;
            }
        }

        cout << (answer ? "YES\n" : "NO\n");
    }

    return 0;
}
