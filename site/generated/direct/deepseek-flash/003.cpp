#include <bits/stdc++.h>
using namespace std;

static constexpr int MOD = 1000000007;

struct DSU {
    vector<int> p, r;
    DSU(int n = 0) { init(n); }
    void init(int n) {
        p.resize(n);
        r.assign(n, 0);
        iota(p.begin(), p.end(), 0);
    }
    int find(int x) {
        while (p[x] != x) {
            p[x] = p[p[x]];
            x = p[x];
        }
        return x;
    }
    void unite(int a, int b) {
        a = find(a);
        b = find(b);
        if (a == b) return;
        if (r[a] < r[b]) swap(a, b);
        p[b] = a;
        if (r[a] == r[b]) ++r[a];
    }
};

int mod_pow(long long a, long long e) {
    long long r = 1;
    while (e) {
        if (e & 1) r = r * a % MOD;
        a = a * a % MOD;
        e >>= 1;
    }
    return int(r);
}

void zeta_transform(vector<int>& f, int n, int D, int N) {
    for (int bit = 0; bit < n; ++bit) {
        int step = 1 << bit;
        for (int mask = 0; mask < N; ++mask) {
            if (mask & step) {
                int cur = mask * D;
                int pre = (mask ^ step) * D;
                for (int k = 0; k < D; ++k) {
                    int v = f[cur + k] + f[pre + k];
                    if (v >= MOD) v -= MOD;
                    f[cur + k] = v;
                }
            }
        }
    }
}

void mobius_transform(vector<int>& f, int n, int D, int N) {
    for (int bit = 0; bit < n; ++bit) {
        int step = 1 << bit;
        for (int mask = 0; mask < N; ++mask) {
            if (mask & step) {
                int cur = mask * D;
                int pre = (mask ^ step) * D;
                for (int k = 0; k < D; ++k) {
                    int v = f[cur + k] - f[pre + k];
                    if (v < 0) v += MOD;
                    f[cur + k] = v;
                }
            }
        }
    }
}

struct Edge {
    int u, v, w;
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int c, T;
    cin >> c >> T;

    while (T--) {
        int n, m;
        cin >> n >> m;

        vector<Edge> edges(m);
        for (int i = 0; i < m; ++i) {
            int u, v, w;
            cin >> u >> v >> w;
            --u;
            --v;
            edges[i] = {u, v, w};
        }

        vector<int> ord(m);
        iota(ord.begin(), ord.end(), 0);
        sort(ord.begin(), ord.end(), [&](int a, int b) {
            return edges[a].w < edges[b].w;
        });

        vector<char> relevant(m, false);
        DSU dsu(n);

        for (int l = 0; l < m;) {
            int r = l;
            while (r < m && edges[ord[r]].w == edges[ord[l]].w) ++r;

            for (int i = l; i < r; ++i) {
                int id = ord[i];
                int u = edges[id].u;
                int v = edges[id].v;
                if (dsu.find(u) != dsu.find(v)) {
                    relevant[id] = true;
                }
            }

            for (int i = l; i < r; ++i) {
                int id = ord[i];
                dsu.unite(edges[id].u, edges[id].v);
            }

            l = r;
        }

        vector<unsigned int> adj(n, 0);
        int relCnt = 0;
        for (int i = 0; i < m; ++i) {
            if (!relevant[i]) continue;
            ++relCnt;
            int u = edges[i].u;
            int v = edges[i].v;
            adj[u] |= (1u << v);
            adj[v] |= (1u << u);
        }

        int N = 1 << n;
        int D = n + 1;

        vector<int> E(N, 0);
        for (int mask = 1; mask < N; ++mask) {
            int b = __builtin_ctz((unsigned)mask);
            int prev = mask ^ (1 << b);
            E[mask] = E[prev] + __builtin_popcount(adj[b] & (unsigned)prev);
        }

        int maxE = relCnt;
        vector<int> pow2(maxE + 1), powInv2(maxE + 1), powInv4(maxE + 1);
        pow2[0] = powInv2[0] = powInv4[0] = 1;
        int inv2 = (MOD + 1) / 2;
        int inv4 = 1LL * inv2 * inv2 % MOD;
        for (int i = 1; i <= maxE; ++i) {
            pow2[i] = int(2LL * pow2[i - 1] % MOD);
            powInv2[i] = int(1LL * inv2 * powInv2[i - 1] % MOD);
            powInv4[i] = int(1LL * inv4 * powInv4[i - 1] % MOD);
        }

        vector<int> invInt(D, 0);
        for (int i = 1; i < D; ++i) {
            invInt[i] = mod_pow(i, MOD - 2);
        }

        vector<int> f(1LL * N * D, 0);
        vector<int> H(N, 0);

        // ------------------------------------------------------------
        // Compute X = 1 / Y under subset convolution, where
        // Y[mask] = 2^(number of relevant edges inside mask).
        // This is the ranked-zeta subset convolution inverse.
        // ------------------------------------------------------------
        for (int mask = 0; mask < N; ++mask) {
            int pc = __builtin_popcount((unsigned)mask);
            f[1LL * mask * D + pc] = pow2[E[mask]];
        }

        zeta_transform(f, n, D, N);

        vector<int> P(D), invP(D), qlog(D), prod(D);

        for (int mask = 0; mask < N; ++mask) {
            int base = mask * D;
            int pc = __builtin_popcount((unsigned)mask);

            for (int k = 0; k < D; ++k) P[k] = f[base + k];

            invP[0] = 1;
            for (int d = 1; d < D; ++d) {
                long long sum = 0;
                int lim = min(d, pc);
                for (int i = 1; i <= lim; ++i) {
                    sum += 1LL * P[i] * invP[d - i] % MOD;
                }
                sum %= MOD;
                invP[d] = (sum == 0 ? 0 : MOD - sum);
            }

            for (int d = 0; d < D; ++d) {
                f[base + d] = invP[d];
            }
        }

        mobius_transform(f, n, D, N);

        for (int mask = 0; mask < N; ++mask) {
            int pc = __builtin_popcount((unsigned)mask);
            int X = f[1LL * mask * D + pc];
            H[mask] = int(1LL * X * pow2[E[mask]] % MOD);
        }

        // ------------------------------------------------------------
        // Compute C = log(H) under subset convolution.
        // During the same ranked-zeta pass compute C * H, because
        // Q[mask] = -(C * H)[mask] * (1/4)^E[mask]
        // for the final inclusion-exclusion over source SCCs.
        // ------------------------------------------------------------
        fill(f.begin(), f.end(), 0);

        for (int mask = 0; mask < N; ++mask) {
            int pc = __builtin_popcount((unsigned)mask);
            f[1LL * mask * D + pc] = H[mask];
        }

        zeta_transform(f, n, D, N);

        for (int mask = 0; mask < N; ++mask) {
            int base = mask * D;
            int pc = __builtin_popcount((unsigned)mask);

            for (int k = 0; k < D; ++k) P[k] = f[base + k];

            invP[0] = 1;
            for (int d = 1; d < D; ++d) {
                long long sum = 0;
                int lim = min(d, pc);
                for (int i = 1; i <= lim; ++i) {
                    sum += 1LL * P[i] * invP[d - i] % MOD;
                }
                sum %= MOD;
                invP[d] = (sum == 0 ? 0 : MOD - sum);
            }

            qlog[0] = 0;
            for (int d = 1; d < D; ++d) {
                long long sum = 0;
                int lim = min(d, pc);
                for (int i = 1; i <= lim; ++i) {
                    sum += 1LL * i * P[i] % MOD * invP[d - i] % MOD;
                }
                sum %= MOD;
                qlog[d] = int(sum * invInt[d] % MOD);
            }

            for (int k = 0; k < D; ++k) {
                long long sum = 0;
                for (int i = 1; i <= k; ++i) {
                    int j = k - i;
                    if (j <= pc) {
                        sum += 1LL * qlog[i] * P[j] % MOD;
                    }
                }
                prod[k] = int(sum % MOD);
            }

            for (int k = 0; k < D; ++k) {
                f[base + k] = prod[k];
            }
        }

        mobius_transform(f, n, D, N);

        int full = N - 1;
        long long ans = 0;

        for (int mask = 1; mask < N; ++mask) {
            int pc = __builtin_popcount((unsigned)mask);
            int CH = f[1LL * mask * D + pc];

            // A*H = - C*H, and Q = (A*H) * (1/4)^E
            int Q = int((MOD - (long long)CH) % MOD);
            Q = int(1LL * Q * powInv4[E[mask]] % MOD);

            int cut = E[full] - E[mask] - E[full ^ mask];
            int factor = powInv2[cut];

            ans += 1LL * Q * factor % MOD;
            if (ans >= MOD) ans -= MOD;
        }

        ans %= MOD;
        if (ans < 0) ans += MOD;

        cout << ans << '\n';
    }

    return 0;
}
