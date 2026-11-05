#include <bits/stdc++.h>
using namespace std;

static const int MOD = 998244353;
static const int MAXA = 6000;

int cntLarge[MAXA + 1][76]; // cntLarge[p][s]

int modpow(int a, int e) {
    long long r = 1, b = a;
    while (e > 0) {
        if (e & 1) r = r * b % MOD;
        b = b * b % MOD;
        e >>= 1;
    }
    return (int)r;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    cin >> n;

    // Small primes <= sqrt(6000) = 77
    vector<int> smallPrimes;
    for (int x = 2; x <= 77; ++x) {
        bool ok = true;
        for (int d = 2; d * d <= x; ++d) {
            if (x % d == 0) {
                ok = false;
                break;
            }
        }
        if (ok) smallPrimes.push_back(x);
    }

    int m = (int)smallPrimes.size();
    int SZ = 1 << m;
    int ALL = SZ - 1;

    vector<int> inv(MAXA + 1);
    inv[1] = 1;
    for (int i = 2; i <= MAXA; ++i) {
        inv[i] = MOD - 1LL * (MOD / i) * inv[MOD % i] % MOD;
    }

    // preMask[s] = set of small primes dividing s, for 1 <= s <= 75
    int preMask[76] = {};
    for (int s = 1; s <= 75; ++s) {
        for (int i = 0; i < m; ++i) {
            if (s % smallPrimes[i] == 0) {
                preMask[s] |= (1 << i);
            }
        }
    }

    // small-only part
    vector<int> W(SZ, 1);

    for (int i = 0; i < n; ++i) {
        int a;
        cin >> a;

        int x = a;
        int mask = 0;

        for (int j = 0; j < m; ++j) {
            int p = smallPrimes[j];
            if (x % p == 0) {
                mask |= (1 << j);
                while (x % p == 0) x /= p;
            }
        }

        if (x > 1) {
            // x is a large prime > 77
            int p = x;
            int s = a / p;
            ++cntLarge[p][s];
        } else {
            int val = a + 1;
            W[mask] = 1LL * W[mask] * val % MOD;
        }
    }

    // Subset product zeta for small-only numbers:
    // W[X] = product over mask subset X.
    for (int b = 0; b < m; ++b) {
        int step = 1 << b;
        for (int base = 0; base < SZ; base += step << 1) {
            for (int j = 0; j < step; ++j) {
                int &v = W[base + step + j];
                v = 1LL * v * W[base + j] % MOD;
            }
        }
    }

    vector<vector<int>> pByIdx(m);
    vector<int> constP;

    for (int p = 79; p <= MAXA; ++p) {
        bool any = false;
        int D = 0;

        for (int s = 1; s <= 75; ++s) {
            if (cntLarge[p][s]) {
                any = true;
                D |= preMask[s];
            }
        }

        if (!any) continue;

        if (D == 0) {
            constP.push_back(p);
        } else {
            int idx = 31 - __builtin_clz((unsigned)D);
            pByIdx[idx].push_back(p);
        }
    }

    // level[T] = product over processed large primes
    vector<int> level(1, 1);

    // Large prime groups whose cofactor is always 1
    for (int p : constP) {
        int c1 = cntLarge[p][1];
        int B = modpow((1 + p) % MOD, c1);
        int cp = 1LL * (p - 1) * inv[p] % MOD;

        int diff = B - 1;
        if (diff < 0) diff += MOD;

        int F = 1 + 1LL * cp * diff % MOD;
        if (F >= MOD) F -= MOD;

        level[0] = 1LL * level[0] * F % MOD;
    }

    vector<int> proj;
    proj.reserve(SZ);

    for (int b = 0; b < m; ++b) {
        // Introduce the new small prime bit b
        int oldSize = (int)level.size();
        level.resize(oldSize * 2);
        for (int i = 0; i < oldSize; ++i) {
            level[i + oldSize] = level[i];
        }

        int levelSize = (int)level.size();
        proj.resize(levelSize);

        for (int p : pByIdx[b]) {
            int D = 0;
            for (int s = 1; s <= 75; ++s) {
                if (cntLarge[p][s]) {
                    D |= preMask[s];
                }
            }

            int k = __builtin_popcount((unsigned)D);
            int Fsize = 1 << k;

            vector<int> arr(Fsize, 1);

            int mapGlobal[21];
            for (int i = 0; i < m; ++i) mapGlobal[i] = -1;

            int loc = 0;
            for (int gb = 0; gb < m; ++gb) {
                if ((D >> gb) & 1) {
                    mapGlobal[gb] = loc++;
                }
            }

            for (int s = 1; s <= 75; ++s) {
                int c = cntLarge[p][s];
                if (c == 0) continue;

                int gm = preMask[s];
                int lm = 0;
                int tmp = gm;

                while (tmp) {
                    int bit = __builtin_ctz((unsigned)tmp);
                    lm |= (1 << mapGlobal[bit]);
                    tmp &= tmp - 1;
                }

                int w = 1LL * p * s % MOD;
                w++;
                if (w >= MOD) w -= MOD;

                int factor = modpow(w, c);
                arr[lm] = 1LL * arr[lm] * factor % MOD;
            }

            // Subset product zeta over D
            for (int bit = 0; bit < k; ++bit) {
                int step = 1 << bit;
                for (int base = 0; base < Fsize; base += step << 1) {
                    for (int j = 0; j < step; ++j) {
                        int &v = arr[base + step + j];
                        v = 1LL * v * arr[base + j] % MOD;
                    }
                }
            }

            int Full = Fsize - 1;
            int cp = 1LL * (p - 1) * inv[p] % MOD;

            vector<int> F(Fsize);

            for (int U = 0; U < Fsize; ++U) {
                int val = arr[Full ^ U];
                int diff = val - 1;
                if (diff < 0) diff += MOD;

                int cur = 1 + 1LL * cp * diff % MOD;
                if (cur >= MOD) cur -= MOD;
                F[U] = cur;
            }

            // Apply F(T ∩ D) to all current masks
            proj[0] = 0;
            level[0] = 1LL * level[0] * F[0] % MOD;

            for (int T = 1; T < levelSize; ++T) {
                int lb = T & -T;
                int gb = __builtin_ctz((unsigned)lb);
                int prev = T ^ lb;

                int pv = proj[prev];
                int ml = mapGlobal[gb];

                if (ml >= 0) {
                    pv |= (1 << ml);
                }

                proj[T] = pv;
                level[T] = 1LL * level[T] * F[pv] % MOD;
            }
        }
    }

    // C[T] = product over small primes:
    // if prime in T: 1/p
    // else: (p-1)/p
    int baseC = 1;
    vector<int> ratio(m);

    for (int i = 0; i < m; ++i) {
        int q = smallPrimes[i];

        int c = 1LL * (q - 1) * inv[q] % MOD;
        baseC = 1LL * baseC * c % MOD;

        ratio[i] = inv[q - 1];
    }

    vector<int> C(SZ);
    C[0] = baseC;

    for (int T = 1; T < SZ; ++T) {
        int lb = T & -T;
        int bit = __builtin_ctz((unsigned)lb);
        C[T] = 1LL * C[T ^ lb] * ratio[bit] % MOD;
    }

    int ans = 0;

    for (int T = 0; T < SZ; ++T) {
        int H = W[ALL ^ T];
        int L = level[T];

        int term = 1LL * C[T] * H % MOD * L % MOD;

        ans += term;
        if (ans >= MOD) ans -= MOD;
    }

    cout << ans << '\n';
    return 0;
}
