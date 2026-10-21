#include <bits/stdc++.h>
using namespace std;

static const int MOD = 998244353;
static const int G = 3;
static const int MAXN = 200000;

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
    for (int i = 1, j = 0; i < n; ++i) {
        int bit = n >> 1;
        for (; j & bit; bit >>= 1) j ^= bit;
        j ^= bit;
        if (i < j) swap(a[i], a[j]);
    }
    for (int len = 2; len <= n; len <<= 1) {
        int wlen = mod_pow(G, (MOD - 1) / len);
        if (invert) wlen = mod_pow(wlen, MOD - 2);
        for (int i = 0; i < n; i += len) {
            long long w = 1;
            int half = len >> 1;
            for (int j = 0; j < half; ++j) {
                int u = a[i + j];
                int v = (int)(a[i + j + half] * w % MOD);
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
        for (int& x : a) x = (int)((long long)x * inv_n % MOD);
    }
}

struct Constraint {
    int x, t;
};

struct DivItem {
    int d;
    int weight; // w(n/d) = product_{p | n/d} (1-p), modulo MOD
};

struct Solver {
    int n, m;
    vector<Constraint> cons;
    vector<int> pow2, invpow2;
    vector<int> sameCnt, crossCnt;

    explicit Solver(int n_, int m_, vector<Constraint> c)
        : n(n_), m(m_), cons(std::move(c)),
          pow2(n + 2), invpow2(n + 2),
          sameCnt(n + 1, 0), crossCnt(n + 1, 0) {
        pow2[0] = invpow2[0] = 1;
        const int inv2 = (MOD + 1) / 2;
        for (int i = 1; i <= n + 1; ++i) {
            pow2[i] = pow2[i - 1] * 2LL % MOD;
            invpow2[i] = (long long)invpow2[i - 1] * inv2 % MOD;
        }
    }

    vector<DivItem> enumerate_divisors() const {
        int x = n;
        vector<pair<int,int>> fac;
        for (int p = 2; 1LL * p * p <= x; ++p) {
            if (x % p == 0) {
                int e = 0;
                while (x % p == 0) x /= p, ++e;
                fac.push_back({p, e});
            }
        }
        if (x > 1) fac.push_back({x, 1});

        vector<DivItem> res;
        function<void(int,int,int)> dfs = [&](int idx, int d, int w) {
            if (idx == (int)fac.size()) {
                res.push_back({d, w});
                return;
            }
            auto [p, a] = fac[idx];
            int cur = 1;
            for (int e = 0; e <= a; ++e) {
                int nw = w;
                if (e < a) {
                    int factor = (1 - p) % MOD;
                    if (factor < 0) factor += MOD;
                    nw = (long long)nw * factor % MOD;
                }
                dfs(idx + 1, d * cur, nw);
                if (e < a) cur *= p;
            }
        };
        dfs(0, 1, 1);
        sort(res.begin(), res.end(), [](const DivItem& a, const DivItem& b) {
            return a.d < b.d;
        });
        return res;
    }

    // Project original fixed positions to Z_d. Return false if two constraints
    // collapse to the same residue with different colors.
    bool project(int d, vector<pair<int,int>>& pins) const {
        vector<int> color(d, -1);
        pins.clear();
        pins.reserve(min(m, d));
        for (const auto& c : cons) {
            int r = c.x % d;
            if (color[r] == -1) {
                color[r] = c.t;
                pins.push_back({r, c.t});
            } else if (color[r] != c.t) {
                return false;
            }
        }
        return true;
    }

    int orbit_count(int d, int s) const {
        if (d & 1) return (d + 1) / 2;
        return d / 2 + ((s & 1) == 0);
    }

    int calc_sparse(int d, const vector<pair<int,int>>& pins) {
        const int q = (int)pins.size();
        vector<int> touched;
        touched.reserve(min<long long>(1LL * q * (q - 1) / 2, d));

        for (int i = 0; i < q; ++i) {
            for (int j = i + 1; j < q; ++j) {
                int s = pins[i].first + pins[j].first;
                if (s >= d) s -= d;
                if (sameCnt[s] == 0 && crossCnt[s] == 0) touched.push_back(s);
                if (pins[i].second == pins[j].second) ++sameCnt[s];
                else ++crossCnt[s];
            }
        }

        long long ans = 0;
        if (d & 1) {
            int base = (long long)pow2[(d + 1) / 2] * invpow2[q] % MOD;
            ans = (long long)(d % MOD) * base % MOD;
        } else {
            int base_even = (long long)pow2[d / 2 + 1] * invpow2[q] % MOD;
            int base_odd  = (long long)pow2[d / 2] * invpow2[q] % MOD;
            ans = (long long)(d / 2) % MOD * ((base_even + base_odd) % MOD) % MOD;
        }

        for (int s : touched) {
            int c = orbit_count(d, s);
            int base = (long long)pow2[c] * invpow2[q] % MOD;
            int actual = 0;
            if (crossCnt[s] == 0) {
                actual = (long long)base * pow2[sameCnt[s]] % MOD;
            }
            ans += actual - base;
            if (ans >= MOD) ans -= MOD;
            if (ans < 0) ans += MOD;
        }

        for (int s : touched) {
            sameCnt[s] = 0;
            crossCnt[s] = 0;
        }
        return (int)ans;
    }

    int calc_dense(int d, const vector<pair<int,int>>& pins) const {
        const int q = (int)pins.size();
        int len = 1;
        while (len < 2 * d - 1) len <<= 1;

        vector<int> u(len, 0), v(len, 0);
        vector<int> self(d, 0);
        for (auto [r, t] : pins) {
            u[r] = 1;
            v[r] = (t == 0 ? 1 : MOD - 1);
            ++self[(2 * r) % d];
        }

        ntt(u, false);
        for (int& x : u) x = (long long)x * x % MOD;
        ntt(u, true);

        ntt(v, false);
        for (int& x : v) x = (long long)x * x % MOD;
        ntt(v, true);

        const int inv2 = (MOD + 1) / 2;
        const int inv4 = (long long)inv2 * inv2 % MOD;
        long long ans = 0;

        for (int s = 0; s < d; ++s) {
            int cu = u[s];
            int cv = v[s];
            if (s + d <= 2 * d - 2) {
                cu += u[s + d];
                if (cu >= MOD) cu -= MOD;
                cv += v[s + d];
                if (cv >= MOD) cv -= MOD;
            }

            int sameOrdered = (long long)(cu + cv >= MOD ? cu + cv - MOD : cu + cv) * inv2 % MOD;
            int diff = cu - cv;
            if (diff < 0) diff += MOD;
            int cross = (long long)diff * inv4 % MOD;

            // All convolution coefficients are at most q <= 2e5 < MOD,
            // so these modular values equal their exact integer counts.
            if (cross != 0) continue;
            int rem = sameOrdered - self[s];
            // rem = twice the number of same-colored pairs joined by this reflection.
            if (rem < 0) rem += MOD; // Should not happen for exact counts, kept defensive.
            int paired = rem / 2;
            int exponent = orbit_count(d, s) - q + paired;
            if (exponent < 0 || exponent > n) {
                cerr << "internal exponent error\n";
                exit(3);
            }
            ans += pow2[exponent];
            if (ans >= MOD) ans -= MOD;
        }
        return (int)ans;
    }

    int calc_S(int d) {
        vector<pair<int,int>> pins;
        if (!project(d, pins)) return 0;
        int q = (int)pins.size();

        int len = 1, lg = 0;
        while (len < 2 * d - 1) len <<= 1, ++lg;
        // Sparse: O(q^2). Dense: two NTT squares, O(len log len).
        // Constants favor the sparse branch somewhat for small q.
        long long sparse_cost = 1LL * q * q;
        long long dense_cost = 3LL * len * max(1, lg);
        if (sparse_cost <= dense_cost) return calc_sparse(d, pins);
        return calc_dense(d, pins);
    }

    int solve() {
        auto divs = enumerate_divisors();
        long long bad = 0;
        for (const auto& it : divs) {
            int Sd = calc_S(it.d);
            bad += (long long)it.weight * Sd % MOD;
            if (bad >= MOD) bad -= MOD;
        }
        int total = pow2[n - m];
        int ans = total - (int)bad;
        if (ans < 0) ans += MOD;
        return ans;
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int T;
    if (!(cin >> T)) return 0;
    while (T--) {
        int n, m;
        cin >> n >> m;
        vector<Constraint> cons(m);
        for (int i = 0; i < m; ++i) cin >> cons[i].x >> cons[i].t;
        Solver solver(n, m, std::move(cons));
        cout << solver.solve() << '\n';
    }
    return 0;
}
