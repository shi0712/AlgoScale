#include <bits/stdc++.h>
using namespace std;

static const int MOD = 1000000007;

int norm_mod(long long x) {
    x %= MOD;
    if (x < 0) x += MOD;
    return (int)x;
}

int mod_pow(long long a, long long e) {
    long long r = 1;
    while (e) {
        if (e & 1) r = r * a % MOD;
        a = a * a % MOD;
        e >>= 1;
    }
    return (int)r;
}

int mod_inv(int x) {
    return mod_pow(x, MOD - 2);
}

/* ---------- polynomial modulo characteristic polynomial ---------- */

// c is monic of degree N = c.size() - 1.
// multiplies a and b in F[z] / (c)
vector<int> polyMulMod(const vector<int>& a, const vector<int>& b, const vector<int>& c) {
    int N = (int)c.size() - 1;
    vector<__int128_t> tmp(2 * N - 1, 0);

    for (int i = 0; i < N; i++) {
        if (!a[i]) continue;
        long long ai = a[i];
        for (int j = 0; j < N; j++) {
            if (!b[j]) continue;
            tmp[i + j] += (__int128_t)ai * b[j];
        }
    }

    for (int d = 2 * N - 2; d >= N; d--) {
        long long val = (long long)(tmp[d] % MOD);
        if (val < 0) val += MOD;
        if (!val) continue;
        for (int j = 0; j < N; j++) {
            tmp[d - N + j] -= (__int128_t)val * c[j];
        }
    }

    vector<int> res(N, 0);
    for (int i = 0; i < N; i++) {
        long long val = (long long)(tmp[i] % MOD);
        if (val < 0) val += MOD;
        res[i] = (int)(val % MOD);
    }
    return res;
}

// returns x * a mod c
vector<int> shiftPoly(const vector<int>& a, const vector<int>& c) {
    int N = (int)c.size() - 1;
    vector<int> res(N, 0);

    for (int i = 0; i + 1 < N; i++) res[i + 1] = a[i];

    int val = a[N - 1];
    if (val) {
        for (int j = 0; j < N; j++) {
            res[j] = (res[j] - 1LL * val * c[j]) % MOD;
            if (res[j] < 0) res[j] += MOD;
        }
    }
    return res;
}

vector<int> identityPoly(int N) {
    vector<int> v(N, 0);
    v[0] = 1;
    return v;
}

/* ---------- ordinary polynomials for gcd/inverse ---------- */

vector<int> trim(vector<int> v) {
    while (v.size() > 1 && v.back() == 0) v.pop_back();
    return v;
}

vector<int> polyDivMod(const vector<int>& A, const vector<int>& B, vector<int>& R) {
    vector<int> a = trim(A);
    vector<int> b = trim(B);

    if (a.size() < b.size()) {
        R = a;
        return vector<int>{0};
    }

    int db = (int)b.size() - 1;
    int invLead = mod_inv(b.back());

    vector<int> r = a;
    vector<int> q(a.size() - b.size() + 1, 0);

    for (int i = (int)r.size() - 1; i >= db; i--) {
        if (!r[i]) continue;
        int coef = (long long)r[i] * invLead % MOD;
        q[i - db] = coef;
        for (int j = 0; j <= db; j++) {
            r[i - db + j] = (r[i - db + j] - 1LL * coef * b[j]) % MOD;
            if (r[i - db + j] < 0) r[i - db + j] += MOD;
        }
    }

    R = trim(r);
    return trim(q);
}

vector<int> polyMulFull(const vector<int>& a, const vector<int>& b) {
    if (a.empty() || b.empty()) return vector<int>{0};
    vector<int> res(a.size() + b.size() - 1, 0);
    for (int i = 0; i < (int)a.size(); i++) {
        if (!a[i]) continue;
        for (int j = 0; j < (int)b.size(); j++) {
            res[i + j] = (res[i + j] + 1LL * a[i] * b[j]) % MOD;
        }
    }
    return trim(res);
}

vector<int> polySubFull(const vector<int>& a, const vector<int>& b) {
    int sz = max(a.size(), b.size());
    vector<int> res(sz, 0);
    for (int i = 0; i < sz; i++) {
        int x = (i < (int)a.size() ? a[i] : 0);
        int y = (i < (int)b.size() ? b[i] : 0);
        res[i] = x - y;
        if (res[i] < 0) res[i] += MOD;
    }
    return trim(res);
}

vector<int> polyScalarMul(const vector<int>& a, int k) {
    vector<int> res = a;
    for (int& x : res) x = (long long)x * k % MOD;
    return trim(res);
}

// returns empty vector if not invertible modulo mod
vector<int> polyInverseMod(const vector<int>& A, const vector<int>& mod, int N) {
    vector<int> a = trim(A);
    if (a.size() == 1 && a[0] == 0) return {};

    vector<int> r0 = trim(mod);
    vector<int> r1;
    {
        vector<int> rem;
        polyDivMod(a, mod, rem);
        r1 = trim(rem);
    }

    vector<int> s0{0};
    vector<int> s1{1};

    while (!(r1.size() == 1 && r1[0] == 0)) {
        vector<int> rem;
        vector<int> q = polyDivMod(r0, r1, rem);

        r0 = r1;
        r1 = trim(rem);

        vector<int> qs = polyMulFull(q, s1);
        vector<int> ns = polySubFull(s0, qs);

        s0 = s1;
        s1 = ns;
    }

    if (r0.size() != 1 || r0[0] == 0) return {};

    int invConst = mod_inv(r0[0]);
    vector<int> res = polyScalarMul(s0, invConst);

    vector<int> rem;
    polyDivMod(res, mod, rem);
    res = trim(rem);

    res.resize(N, 0);
    return res;
}

/* ---------- characteristic polynomial via Hessenberg form ---------- */

vector<int> characteristicPolynomial(const vector<int>& A, int N) {
    vector<int> H = A;

    auto subRow = [&](int i, int j, int t) {
        for (int c = 0; c < N; c++) {
            H[i * N + c] = (H[i * N + c] - 1LL * t * H[j * N + c]) % MOD;
            if (H[i * N + c] < 0) H[i * N + c] += MOD;
        }
    };

    for (int k = 0; k + 2 < N; k++) {
        int p = -1;
        for (int i = k + 1; i < N; i++) {
            if (H[i * N + k]) {
                p = i;
                break;
            }
        }
        if (p == -1) continue;

        if (p != k + 1) {
            for (int j = 0; j < N; j++) {
                swap(H[p * N + j], H[(k + 1) * N + j]);
            }
            for (int i = 0; i < N; i++) {
                swap(H[i * N + p], H[i * N + k + 1]);
            }
        }

        int pivot = H[(k + 1) * N + k];
        if (!pivot) continue;
        int invPivot = mod_inv(pivot);

        for (int i = k + 2; i < N; i++) {
            int val = H[i * N + k];
            if (!val) continue;

            int t = (long long)val * invPivot % MOD;

            for (int j = 0; j < N; j++) {
                H[i * N + j] = (H[i * N + j] - 1LL * t * H[(k + 1) * N + j]) % MOD;
                if (H[i * N + j] < 0) H[i * N + j] += MOD;
            }

            for (int r = 0; r < N; r++) {
                H[r * N + k + 1] = (H[r * N + k + 1] + 1LL * t * H[r * N + i]) % MOD;
                if (H[r * N + k + 1] < 0) H[r * N + k + 1] += MOD;
            }
        }
    }

    vector<vector<int>> P(N + 1);
    P[0] = {1};

    for (int i = 1; i <= N; i++) {
        vector<int> Pi(i + 1, 0);

        for (int d = 0; d < i; d++) {
            Pi[d + 1] += P[i - 1][d];
            if (Pi[d + 1] >= MOD) Pi[d + 1] -= MOD;

            Pi[d] = (Pi[d] - 1LL * H[(i - 1) * N + (i - 1)] * P[i - 1][d]) % MOD;
            if (Pi[d] < 0) Pi[d] += MOD;
        }

        long long prod = 1;
        for (int k = i - 1; k >= 1; k--) {
            prod = prod * H[k * N + (k - 1)] % MOD;
            int coef = 1LL * H[(k - 1) * N + (i - 1)] * prod % MOD;

            for (int d = 0; d < k; d++) {
                Pi[d] = (Pi[d] - 1LL * coef * P[k - 1][d]) % MOD;
                if (Pi[d] < 0) Pi[d] += MOD;
            }
        }

        for (int& x : Pi) {
            x %= MOD;
            if (x < 0) x += MOD;
        }

        P[i] = Pi;
    }

    return P[N];
}

/* ---------- segment tree for products of R_i ---------- */

struct SegmentTree {
    int size;
    int N;
    vector<int> c;
    vector<vector<int>> tree;
    vector<int> id;

    void init(int K, const vector<vector<int>>& R, const vector<int>& charPoly) {
        N = (int)charPoly.size() - 1;
        c = charPoly;

        size = 1;
        while (size < max(2, K)) size <<= 1;

        id = identityPoly(N);
        tree.assign(2 * size, id);

        for (int i = 1; i <= K - 1; i++) {
            tree[size + i] = R[i];
        }

        for (int i = size - 1; i >= 1; i--) {
            tree[i] = polyMulMod(tree[i << 1], tree[i << 1 | 1], c);
        }
    }

    vector<int> query(int l, int r) {
        if (l > r) return id;
        l = max(l, 1);
        r = min(r, size - 1);
        if (l > r) return id;

        vector<int> leftRes = id;
        vector<int> rightRes = id;

        l += size;
        r += size;

        while (l <= r) {
            if (l & 1) {
                leftRes = polyMulMod(leftRes, tree[l], c);
                l++;
            }
            if (!(r & 1)) {
                rightRes = polyMulMod(tree[r], rightRes, c);
                r--;
            }
            l >>= 1;
            r >>= 1;
        }

        return polyMulMod(leftRes, rightRes, c);
    }
};

/* ---------- dense matrix helpers ---------- */

vector<int> matMulRightA(const vector<int>& X, const vector<int>& At, int N) {
    vector<int> C(N * N, 0);

    for (int i = 0; i < N; i++) {
        const int* xi = &X[i * N];
        for (int j = 0; j < N; j++) {
            const int* atj = &At[j * N];
            unsigned long long sum = 0;

            int k = 0;
            for (; k + 16 <= N; k += 16) {
                for (int u = 0; u < 16; u++) {
                    sum += (unsigned long long)xi[k + u] * atj[k + u];
                }
                sum %= MOD;
            }
            for (; k < N; k++) {
                sum += (unsigned long long)xi[k] * atj[k];
            }

            C[i * N + j] = sum % MOD;
        }
    }

    return C;
}

vector<int> matVecMul(const vector<int>& M, const vector<int>& v, int N) {
    vector<int> res(N, 0);

    for (int i = 0; i < N; i++) {
        const int* row = &M[i * N];
        unsigned long long sum = 0;

        int j = 0;
        for (; j + 16 <= N; j += 16) {
            for (int u = 0; u < 16; u++) {
                sum += (unsigned long long)row[j + u] * v[j + u];
            }
            sum %= MOD;
        }
        for (; j < N; j++) {
            sum += (unsigned long long)row[j] * v[j];
        }

        res[i] = sum % MOD;
    }

    return res;
}

int dotProduct(const int* a, const int* b, int N) {
    unsigned long long sum = 0;

    int i = 0;
    for (; i + 16 <= N; i += 16) {
        for (int u = 0; u < 16; u++) {
            sum += (unsigned long long)a[i + u] * b[i + u];
        }
        sum %= MOD;
    }
    for (; i < N; i++) {
        sum += (unsigned long long)a[i] * b[i];
    }

    return (int)(sum % MOD);
}

vector<int> identityMatrix(int N) {
    vector<int> I(N * N, 0);
    for (int i = 0; i < N; i++) I[i * N + i] = 1;
    return I;
}

/* ---------- queries ---------- */

struct Query {
    int bs, s, bt, t;
    vector<int> P;
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int N, K, Q;
    cin >> N >> K >> Q;

    vector<int> A(N * N, 0);

    for (int i = 0; i < N; i++) {
        string str;
        cin >> str;
        for (int j = 0; j < N; j++) {
            if (str[j] == '1') A[i * N + j] = 1;
        }
    }

    vector<Query> queries(Q);

    for (int qi = 0; qi < Q; qi++) {
        int bs, s, bt, t;
        cin >> bs >> s >> bt >> t;
        --s;
        --t;
        queries[qi].bs = bs;
        queries[qi].s = s;
        queries[qi].bt = bt;
        queries[qi].t = t;
        queries[qi].P.assign(N, 0);
    }

    vector<int> charPoly = characteristicPolynomial(A, N);

    // hs[m] = H_m(z) modulo charPoly, H_0 = 1.
    vector<vector<int>> hs(K + 1, vector<int>(N, 0));
    hs[0][0] = 1;

    for (int m = 1; m <= K; m++) {
        vector<int> sq = polyMulMod(hs[m - 1], hs[m - 1], charPoly);
        vector<int> shifted = shiftPoly(sq, charPoly);

        hs[m] = hs[m - 1];
        for (int i = 0; i < N; i++) {
            hs[m][i] += shifted[i];
            if (hs[m][i] >= MOD) hs[m][i] -= MOD;
        }
    }

    // R[i] = 1 + z * H_i(z), for i = 1 .. K-1.
    vector<vector<int>> R(K, vector<int>(N, 0));
    for (int i = 1; i <= K - 1; i++) {
        R[i] = shiftPoly(hs[i], charPoly);
        R[i][0]++;
        if (R[i][0] >= MOD) R[i][0] -= MOD;
    }

    // suffixProd[i] = product_{j=i}^{K-1} R[j], suffixProd[K] = 1.
    vector<vector<int>> suffixProd(K + 1, vector<int>(N, 0));
    suffixProd[K] = identityPoly(N);

    for (int i = K - 1; i >= 1; i--) {
        suffixProd[i] = polyMulMod(R[i], suffixProd[i + 1], charPoly);
    }

    // S[b] used in F_{b,b} = 1 + z^2 H_{b-1}^2 S[b].
    vector<vector<int>> S(K + 1, vector<int>(N, 0));

    for (int b = K - 1; b >= 1; b--) {
        bool isZero = true;
        for (int x : S[b + 1]) {
            if (x) {
                isZero = false;
                break;
            }
        }

        if (isZero) {
            S[b][0] = 1;
        } else {
            vector<int> r2 = polyMulMod(R[b], R[b], charPoly);
            vector<int> term = polyMulMod(r2, S[b + 1], charPoly);
            term[0]++;
            if (term[0] >= MOD) term[0] -= MOD;
            S[b] = term;
        }
    }

    // For fast formula when denominators are invertible:
    // base[b] = H_{b-1} / H_b.
    vector<char> invValid(K + 1, 0);
    vector<vector<int>> base(K + 1, vector<int>(N, 0));

    for (int b = 1; b <= K; b++) {
        vector<int> inv = polyInverseMod(hs[b], charPoly, N);
        if (!inv.empty()) {
            invValid[b] = 1;
            base[b] = polyMulMod(hs[b - 1], inv, charPoly);
        }
    }

    SegmentTree seg;
    if (K > 1) {
        seg.init(K, R, charPoly);
    }

    auto rangeProduct = [&](int l, int r) -> vector<int> {
        if (l > r) return identityPoly(N);
        if (K <= 1) return identityPoly(N);
        l = max(l, 1);
        r = min(r, K - 1);
        if (l > r) return identityPoly(N);
        return seg.query(l, r);
    };

    // Build F_{bs,bt}(z) modulo charPoly for each query.
    for (int qi = 0; qi < Q; qi++) {
        int bs = queries[qi].bs;
        int bt = queries[qi].bt;

        vector<int> P(N, 0);

        if (bs == bt) {
            vector<int> term = polyMulMod(hs[bs - 1], hs[bs - 1], charPoly);
            term = polyMulMod(term, S[bs], charPoly);
            term = shiftPoly(term, charPoly);
            term = shiftPoly(term, charPoly);

            P = term;
            P[0]++;
            if (P[0] >= MOD) P[0] -= MOD;
        } else {
            if (invValid[bs] && invValid[bt]) {
                vector<int> tmp = polyMulMod(hs[K], base[bs], charPoly);
                tmp = shiftPoly(tmp, charPoly);
                tmp = polyMulMod(tmp, base[bt], charPoly);
                P = tmp;
            } else {
                if (bt > bs) {
                    vector<int> range = rangeProduct(bs, bt - 2);
                    vector<int> tmp = polyMulMod(hs[bs - 1], range, charPoly);
                    tmp = polyMulMod(tmp, suffixProd[bt], charPoly);
                    P = shiftPoly(tmp, charPoly);
                } else {
                    // bt < bs
                    vector<int> range = rangeProduct(bt, bs - 2);
                    vector<int> tmp = polyMulMod(hs[bt - 1], range, charPoly);
                    tmp = polyMulMod(tmp, suffixProd[bs], charPoly);
                    P = shiftPoly(tmp, charPoly);
                }
            }
        }

        queries[qi].P = P;
    }

    // Group queries by ending room t.
    vector<vector<int>> byT(N);

    for (int qi = 0; qi < Q; qi++) {
        byT[queries[qi].t].push_back(qi);
    }

    int uniqueT = 0;
    for (int t = 0; t < N; t++) {
        if (!byT[t].empty()) uniqueT++;
    }

    // Choose block size for baby-step giant-step.
    int block = 1;
    {
        long long N2 = 1LL * N * N;
        long long N3 = 1LL * N * N * N;
        long long best = LLONG_MAX;

        for (int s = 1; s <= N; s++) {
            long long pre = 1LL * (s - 1) * N3;
            int r = (N - 1) / s;
            long long per = 1LL * uniqueT * r * N2;
            long long cost = pre + per;

            if (cost < best) {
                best = cost;
                block = s;
            }
        }
    }

    // Transpose of A.
    vector<int> At(N * N, 0);
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            At[j * N + i] = A[i * N + j];
        }
    }

    // powA[a] = A^a for a = 0..block.
    vector<vector<int>> powA(block + 1);
    powA[0] = identityMatrix(N);

    if (block >= 1) {
        powA[1] = A;
    }

    for (int a = 2; a <= block; a++) {
        powA[a] = matMulRightA(powA[a - 1], At, N);
    }

    vector<int> B = powA[block];

    vector<int> answers(Q, 0);

    for (int T = 0; T < N; T++) {
        if (byT[T].empty()) continue;

        int rMax = (N - 1) / block;

        vector<vector<int>> v(rMax + 1, vector<int>(N, 0));
        v[0][T] = 1;

        for (int b = 1; b <= rMax; b++) {
            v[b] = matVecMul(B, v[b - 1], N);
        }

        for (int qi : byT[T]) {
            int S = queries[qi].s;
            const vector<int>& P = queries[qi].P;

            long long ans = 0;

            for (int k = 0; k < N; k++) {
                int coef = P[k];
                if (!coef) continue;

                int a = k % block;
                int b = k / block;

                int dot;
                if (a == 0) {
                    dot = v[b][S];
                } else {
                    dot = dotProduct(&powA[a][S * N], v[b].data(), N);
                }

                ans += 1LL * coef * dot % MOD;
                if (ans >= (long long)MOD * MOD) ans %= MOD;
            }

            answers[qi] = ans % MOD;
        }
    }

    for (int qi = 0; qi < Q; qi++) {
        cout << answers[qi] << '\n';
    }

    return 0;
}
