#include <bits/stdc++.h>
using namespace std;

static constexpr int MOD = 998244353;
static constexpr int PRIMITIVE_ROOT = 3;

int mod_pow(long long a, long long e) {
    long long r = 1;
    while (e) {
        if (e & 1) r = r * a % MOD;
        a = a * a % MOD;
        e >>= 1;
    }
    return (int)r;
}

void ntt(vector<int>& a, bool invert) {
    int n = (int)a.size();

    for (int i = 1, j = 0; i < n; i++) {
        int bit = n >> 1;
        while (j & bit) {
            j ^= bit;
            bit >>= 1;
        }
        j ^= bit;
        if (i < j) swap(a[i], a[j]);
    }

    for (int len = 2; len <= n; len <<= 1) {
        int wlen = mod_pow(PRIMITIVE_ROOT, (MOD - 1) / len);
        if (invert) wlen = mod_pow(wlen, MOD - 2);

        for (int i = 0; i < n; i += len) {
            long long w = 1;
            int half = len >> 1;
            for (int j = 0; j < half; j++) {
                int u = a[i + j];
                int v = (int)(w * a[i + j + half] % MOD);

                int x = u + v;
                if (x >= MOD) x -= MOD;

                int y = u - v;
                if (y < 0) y += MOD;

                a[i + j] = x;
                a[i + j + half] = y;

                w = w * wlen % MOD;
            }
        }
    }

    if (invert) {
        int inv_n = mod_pow(n, MOD - 2);
        for (int& x : a) x = (long long)x * inv_n % MOD;
    }
}

vector<int> convolution(const vector<int>& a, const vector<int>& b, int limit) {
    if (a.empty() || b.empty() || limit <= 0) return {};

    int out_len = min((int)a.size() + (int)b.size() - 1, limit);
    if (out_len <= 0) return {};

    if (a.size() == 1) {
        vector<int> res(out_len);
        long long c = a[0];
        for (int i = 0; i < out_len; i++) res[i] = c * b[i] % MOD;
        return res;
    }

    if (b.size() == 1) {
        vector<int> res(out_len);
        long long c = b[0];
        for (int i = 0; i < out_len; i++) res[i] = c * a[i] % MOD;
        return res;
    }

    int full = (int)a.size() + (int)b.size() - 1;

    if (min(a.size(), b.size()) <= 32 || full <= 256) {
        vector<int> res(out_len, 0);

        for (int i = 0; i < (int)a.size() && i < out_len; i++) {
            if (a[i] == 0) continue;
            long long ai = a[i];
            int max_j = min((int)b.size(), out_len - i);

            for (int j = 0; j < max_j; j++) {
                res[i + j] = (res[i + j] + ai * b[j]) % MOD;
            }
        }

        return res;
    }

    int n = 1;
    while (n < full) n <<= 1;

    vector<int> fa(a.begin(), a.end());
    vector<int> fb(b.begin(), b.end());
    fa.resize(n);
    fb.resize(n);

    ntt(fa, false);
    ntt(fb, false);

    for (int i = 0; i < n; i++) {
        fa[i] = (long long)fa[i] * fb[i] % MOD;
    }

    ntt(fa, true);
    fa.resize(out_len);

    return fa;
}

struct Solver {
    int n;
    vector<unsigned long long> a, b;
    vector<int> fact, invfact;

    Solver(int n_) : n(n_) {
        a.resize(n);
        b.resize(n);

        fact.assign(n + 1, 1);
        invfact.assign(n + 1, 1);

        for (int i = 1; i <= n; i++) {
            fact[i] = (long long)fact[i - 1] * i % MOD;
        }

        invfact[n] = mod_pow(fact[n], MOD - 2);

        for (int i = n; i >= 1; i--) {
            invfact[i - 1] = (long long)invfact[i] * i % MOD;
        }
    }

    vector<int> multiply_poly(const vector<int>& x, const vector<int>& y, int limit) {
        return convolution(x, y, limit);
    }

    vector<int> complete_poly(int L, int R) {
        int m = min(L, R);
        vector<int> res(m + 1);

        long long base = (long long)fact[L] * fact[R] % MOD;

        for (int i = 0; i <= m; i++) {
            long long v = base;
            v = v * invfact[i] % MOD;
            v = v * invfact[L - i] % MOD;
            v = v * invfact[R - i] % MOD;
            res[i] = (int)v;
        }

        return res;
    }

    // Given the matching polynomial h of forbidden edges,
    // return the matching polynomial of complete bipartite graph minus forbidden edges.
    vector<int> complement_from_forbidden(const vector<int>& h, int L, int R) {
        int m = min(L, R);

        int d = min((int)h.size() - 1, m);
        if (d < 0) d = 0;

        vector<int> A(d + 1);
        for (int q = 0; q <= d; q++) {
            long long v = h[q];
            v = v * fact[L - q] % MOD;
            v = v * fact[R - q] % MOD;
            if (q & 1) v = (MOD - v) % MOD;
            A[q] = (int)v;
        }

        vector<int> B(m + 1);
        for (int i = 0; i <= m; i++) B[i] = invfact[i];

        vector<int> conv = convolution(A, B, m + 1);

        vector<int> res(m + 1);

        for (int x = 0; x <= m; x++) {
            long long v = (x < (int)conv.size() ? conv[x] : 0);
            v = v * invfact[L - x] % MOD;
            v = v * invfact[R - x] % MOD;
            res[x] = (int)v;
        }

        return res;
    }

    // H on bits [0..p], where K has highest bit p equal to 1.
    // It is assumed that all higher bits are already equal / ignored.
    vector<int> solve_Hcluster(const vector<int>& A, const vector<int>& B, int p, unsigned long long K) {
        if (A.empty() || B.empty()) return {1};

        vector<int> A0, A1, B0, B1;
        A0.reserve(A.size());
        A1.reserve(A.size());
        B0.reserve(B.size());
        B1.reserve(B.size());

        for (int idx : A) {
            if ((a[idx] >> p) & 1ULL) A1.push_back(idx);
            else A0.push_back(idx);
        }

        for (int idx : B) {
            if ((b[idx] >> p) & 1ULL) B1.push_back(idx);
            else B0.push_back(idx);
        }

        unsigned long long Klow = K ^ (1ULL << p);

        vector<int> g0 = solve_G(A0, B1, p - 1, Klow);
        vector<int> g1 = solve_G(A1, B0, p - 1, Klow);

        int m = min((int)A.size(), (int)B.size());
        vector<int> forbidden_g = multiply_poly(g0, g1, m + 1);

        return complement_from_forbidden(forbidden_g, (int)A.size(), (int)B.size());
    }

    // Matching polynomial of edges a xor b >= K on bits [0..P].
    vector<int> solve_G(const vector<int>& A, const vector<int>& B, int P, unsigned long long K) {
        if (A.empty() || B.empty()) return {1};

        if (K == 0) {
            return complete_poly((int)A.size(), (int)B.size());
        }

        int p = 63 - __builtin_clzll(K);

        if (p == P) {
            int bitA = -1;
            bool uniA = true;

            for (int idx : A) {
                int v = (int)((a[idx] >> p) & 1ULL);
                if (bitA == -1) bitA = v;
                else if (bitA != v) {
                    uniA = false;
                    break;
                }
            }

            int bitB = -1;
            bool uniB = true;

            for (int idx : B) {
                int v = (int)((b[idx] >> p) & 1ULL);
                if (bitB == -1) bitB = v;
                else if (bitB != v) {
                    uniB = false;
                    break;
                }
            }

            unsigned long long Klow = K ^ (1ULL << p);

            if (uniA && uniB) {
                if (bitA != bitB) {
                    return solve_G(A, B, p - 1, Klow);
                } else {
                    return {1};
                }
            }

            if (uniA) {
                int need = bitA ^ 1;

                vector<int> Bsub;
                Bsub.reserve(B.size());

                for (int idx : B) {
                    if (((b[idx] >> p) & 1ULL) == need) {
                        Bsub.push_back(idx);
                    }
                }

                if (Bsub.empty()) return {1};

                return solve_G(A, Bsub, p - 1, Klow);
            }

            if (uniB) {
                int need = bitB ^ 1;

                vector<int> Asub;
                Asub.reserve(A.size());

                for (int idx : A) {
                    if (((a[idx] >> p) & 1ULL) == need) {
                        Asub.push_back(idx);
                    }
                }

                if (Asub.empty()) return {1};

                return solve_G(Asub, B, p - 1, Klow);
            }

            vector<int> A0, A1, B0, B1;
            A0.reserve(A.size());
            A1.reserve(A.size());
            B0.reserve(B.size());
            B1.reserve(B.size());

            for (int idx : A) {
                if ((a[idx] >> p) & 1ULL) A1.push_back(idx);
                else A0.push_back(idx);
            }

            for (int idx : B) {
                if ((b[idx] >> p) & 1ULL) B1.push_back(idx);
                else B0.push_back(idx);
            }

            vector<int> g0 = solve_G(A0, B1, p - 1, Klow);
            vector<int> g1 = solve_G(A1, B0, p - 1, Klow);

            int m = min((int)A.size(), (int)B.size());
            return multiply_poly(g0, g1, m + 1);
        }

        // P > p. The bits P .. p+1 of K are zero.
        int diff = P - p;
        unsigned long long mask = (1ULL << diff) - 1;

        auto get_key = [&](unsigned long long v) -> unsigned long long {
            return (v >> (p + 1)) & mask;
        };

        bool uni_key_A = true;
        unsigned long long key_A = 0;

        if (!A.empty()) {
            key_A = get_key(a[A[0]]);
            for (int idx : A) {
                if (get_key(a[idx]) != key_A) {
                    uni_key_A = false;
                    break;
                }
            }
        }

        bool uni_key_B = true;
        unsigned long long key_B = 0;

        if (!B.empty()) {
            key_B = get_key(b[B[0]]);
            for (int idx : B) {
                if (get_key(b[idx]) != key_B) {
                    uni_key_B = false;
                    break;
                }
            }
        }

        if (uni_key_A && uni_key_B) {
            if (key_A == key_B) {
                return solve_G(A, B, p, K);
            } else {
                return complete_poly((int)A.size(), (int)B.size());
            }
        }

        unordered_map<unsigned long long, int> mp;
        mp.reserve((size_t)A.size() * 2 + 10);

        vector<vector<int>> Ag, Bg;

        for (int idx : A) {
            unsigned long long key = get_key(a[idx]);

            auto it = mp.find(key);
            if (it == mp.end()) {
                int id = (int)Ag.size();
                mp[key] = id;
                Ag.push_back({});
                Bg.push_back({});
                Ag[id].push_back(idx);
            } else {
                Ag[it->second].push_back(idx);
            }
        }

        for (int idx : B) {
            unsigned long long key = get_key(b[idx]);

            auto it = mp.find(key);
            if (it != mp.end()) {
                Bg[it->second].push_back(idx);
            }
        }

        int common_count = 0;
        int only_id = -1;

        for (int id = 0; id < (int)Ag.size(); id++) {
            if (!Ag[id].empty() && !Bg[id].empty()) {
                common_count++;
                only_id = id;
            }
        }

        if (common_count == 0) {
            return complete_poly((int)A.size(), (int)B.size());
        }

        if (common_count == 1 &&
            (int)Ag[only_id].size() == (int)A.size() &&
            (int)Bg[only_id].size() == (int)B.size()) {
            return solve_G(A, B, p, K);
        }

        int m = min((int)A.size(), (int)B.size());

        vector<int> h_total = {1};

        for (int id = 0; id < (int)Ag.size(); id++) {
            if (Ag[id].empty() || Bg[id].empty()) continue;

            vector<int> hc = solve_Hcluster(Ag[id], Bg[id], p, K);
            h_total = multiply_poly(h_total, hc, m + 1);
        }

        return complement_from_forbidden(h_total, (int)A.size(), (int)B.size());
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    unsigned long long k;

    cin >> n >> k;

    Solver solver(n);

    for (int i = 0; i < n; i++) {
        cin >> solver.a[i];
    }

    for (int i = 0; i < n; i++) {
        cin >> solver.b[i];
    }

    vector<int> A(n), B(n);
    iota(A.begin(), A.end(), 0);
    iota(B.begin(), B.end(), 0);

    vector<int> answer = solver.solve_G(A, B, 59, k);

    for (int x = 1; x <= n; x++) {
        if (x < (int)answer.size()) cout << answer[x];
        else cout << 0;

        if (x < n) cout << ' ';
    }
    cout << '\n';

    return 0;
}
