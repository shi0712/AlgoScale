#include <bits/stdc++.h>
using namespace std;

static const int MOD = 1000000007;

struct CSR {
    vector<int> start;
    vector<int> entries;
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int T;
    cin >> T;

    vector<array<int, 3>> queries(T);
    int maxN = 1;
    for (int i = 0; i < T; ++i) {
        int A, B, C;
        cin >> A >> B >> C;
        queries[i] = {A, B, C};
        maxN = max({maxN, A, B, C});
    }

    // Linear sieve for Mobius function and smallest prime factor.
    vector<int> primes;
    vector<int> spf(maxN + 1, 0), mu(maxN + 1, 0);
    mu[1] = 1;
    spf[1] = 1;

    for (int i = 2; i <= maxN; ++i) {
        if (spf[i] == 0) {
            spf[i] = i;
            primes.push_back(i);
            mu[i] = -1;
        }
        for (int p : primes) {
            long long v = 1LL * i * p;
            if (v > maxN) break;
            spf[v] = p;
            if (i % p == 0) {
                mu[v] = 0;
                break;
            } else {
                mu[v] = -mu[i];
            }
        }
    }

    // D[n] = sum_{i=1}^{n} floor(n / i) = sum_{k=1}^{n} tau(k).
    vector<int> tau(maxN + 1, 0), D(maxN + 1, 0);
    for (int i = 1; i <= maxN; ++i) {
        for (int j = i; j <= maxN; j += i) {
            ++tau[j];
        }
    }
    long long pref = 0;
    for (int i = 1; i <= maxN; ++i) {
        pref += tau[i];
        D[i] = (int)pref;
    }

    auto buildCSR = [&](int bound, int maxX, int maxZ) -> CSR {
        vector<int> cnt(maxX + 2, 0);

        int fac[16];
        int divs[256];

        auto processX = [&](int x, auto &&f) {
            if (mu[x] == 0) return;

            int temp = x;
            int fc = 0;
            while (temp > 1) {
                int p = spf[temp];
                fac[fc++] = p;
                while (temp % p == 0) temp /= p;
            }

            int sz = 1;
            divs[0] = 1;
            for (int i = 0; i < fc; ++i) {
                int p = fac[i];
                int old = sz;
                for (int j = 0; j < old; ++j) {
                    divs[sz++] = divs[j] * p;
                }
            }

            for (int di = 0; di < sz; ++di) {
                int g = divs[di];
                int t1 = maxZ / g;
                int t2 = bound / x;
                int lim = min(t1, t2);
                if (lim <= 0) continue;

                for (int t = 1; t <= lim; ++t) {
                    if (mu[t] == 0) continue;
                    if (std::gcd(t, x) != 1) continue;
                    f(x, g, t);
                }
            }
        };

        for (int x = 1; x <= maxX; ++x) {
            processX(x, [&](int xx, int g, int t) {
                ++cnt[xx];
            });
        }

        CSR res;
        res.start.assign(maxX + 2, 0);
        for (int x = 1; x <= maxX + 1; ++x) {
            res.start[x] = res.start[x - 1] + (x - 1 <= maxX ? cnt[x - 1] : 0);
        }
        // The above loop is a little confusing; rebuild cleanly.
        res.start.assign(maxX + 2, 0);
        for (int x = 1; x <= maxX; ++x) {
            res.start[x + 1] = res.start[x] + cnt[x];
        }

        int total = res.start[maxX + 1];
        res.entries.resize(total);

        vector<int> pos = res.start;

        for (int x = 1; x <= maxX; ++x) {
            processX(x, [&](int xx, int g, int t) {
                int z = g * t;
                res.entries[pos[xx]++] = z;
            });
        }

        return res;
    };

    for (int qi = 0; qi < T; ++qi) {
        int A = queries[qi][0];
        int B = queries[qi][1];
        int C = queries[qi][2];

        // The answer is symmetric in A, B, C.
        if (A > B) swap(A, B);
        if (B > C) swap(B, C);
        if (A > B) swap(A, B);

        // Formula used:
        // sum_{x,y,z} mu(x)mu(y)mu(z)
        //   D(A/lcm(x,z)) D(B/lcm(x,y)) D(C/lcm(y,z))
        //
        // Sorted: A <= B <= C.
        int X = A;          // x <= min(A,B) = A
        int Y = B;          // y <= min(B,C) = B
        int Z = A;          // z <= min(A,C) = A

        CSR csrX = buildCSR(A, X, Z); // x-z edge
        CSR csrY = buildCSR(C, Y, Z); // y-z edge

        long long ans = 0;

        int fac[16];
        int divs[256];

        for (int x = 1; x <= X; ++x) {
            if (mu[x] == 0) continue;

            int sx = csrX.start[x + 1] - csrX.start[x];

            // Enumerate y with lcm(x,y) <= B.
            int temp = x;
            int fc = 0;
            while (temp > 1) {
                int p = spf[temp];
                fac[fc++] = p;
                while (temp % p == 0) temp /= p;
            }

            int sz = 1;
            divs[0] = 1;
            for (int i = 0; i < fc; ++i) {
                int p = fac[i];
                int old = sz;
                for (int j = 0; j < old; ++j) {
                    divs[sz++] = divs[j] * p;
                }
            }

            for (int di = 0; di < sz; ++di) {
                int g = divs[di];

                int t1 = Y / g;
                int t2 = B / x;
                int lim = min(t1, t2);
                if (lim <= 0) continue;

                for (int t = 1; t <= lim; ++t) {
                    if (mu[t] == 0) continue;
                    if (std::gcd(t, x) != 1) continue;

                    int y = g * t;
                    if (y > Y) continue;

                    int muy = mu[y];
                    if (muy == 0) continue;

                    int lxy = x * t; // gcd(x,y)=g and y=g*t
                    if (lxy > B) continue;

                    int sy = csrY.start[y + 1] - csrY.start[y];
                    int sumZ = 0;

                    if (sx <= sy) {
                        int L = csrX.start[x];
                        int R = csrX.start[x + 1];

                        for (int id = L; id < R; ++id) {
                            int z = csrX.entries[id];

                            int g1 = std::gcd(x, z);
                            int lxz = (int)(1LL * (x / g1) * z);

                            int g2 = std::gcd(y, z);
                            long long lyzLL = 1LL * (y / g2) * z;
                            if (lyzLL > C) continue;
                            int lyz = (int)lyzLL;

                            int term = (int)(1LL * D[A / lxz] * D[C / lyz] % MOD);

                            if (mu[z] > 0) {
                                sumZ += term;
                                if (sumZ >= MOD) sumZ -= MOD;
                            } else {
                                sumZ -= term;
                                if (sumZ < 0) sumZ += MOD;
                            }
                        }
                    } else {
                        int L = csrY.start[y];
                        int R = csrY.start[y + 1];

                        for (int id = L; id < R; ++id) {
                            int z = csrY.entries[id];

                            int g2 = std::gcd(y, z);
                            int lyz = (int)(1LL * (y / g2) * z);

                            int g1 = std::gcd(x, z);
                            long long lxzLL = 1LL * (x / g1) * z;
                            if (lxzLL > A) continue;
                            int lxz = (int)lxzLL;

                            int term = (int)(1LL * D[A / lxz] * D[C / lyz] % MOD);

                            if (mu[z] > 0) {
                                sumZ += term;
                                if (sumZ >= MOD) sumZ -= MOD;
                            } else {
                                sumZ -= term;
                                if (sumZ < 0) sumZ += MOD;
                            }
                        }
                    }

                    int db = D[B / lxy];
                    long long val = 1LL * db * sumZ % MOD;
                    int sgn = mu[x] * muy;

                    if (sgn > 0) {
                        ans += val;
                        if (ans >= MOD) ans -= MOD;
                    } else {
                        ans -= val;
                        if (ans < 0) ans += MOD;
                    }
                }
            }
        }

        ans %= MOD;
        if (ans < 0) ans += MOD;
        cout << ans << '\n';
    }

    return 0;
}
