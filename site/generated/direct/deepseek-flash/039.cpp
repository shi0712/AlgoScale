#include <bits/stdc++.h>
using namespace std;

static const int MOD = 998244353;
static const int G = 3;
static const int MAXN = 200005;

int spf[MAXN];
int pw2v[MAXN];

int mod_pow(long long a, long long e) {
    long long r = 1;
    while (e > 0) {
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
        while (j & bit) {
            j ^= bit;
            bit >>= 1;
        }
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
        for (int& x : a) {
            x = (int)(1LL * x * inv_n % MOD);
        }
    }
}

int calc_b(int d) {
    long long res = 1;
    while (d > 1) {
        int p = spf[d];
        res = res * ((1 - p + MOD) % MOD) % MOD;
        while (d % p == 0) d /= p;
    }
    return (int)res;
}

int calc_S(int q, const vector<int>& xs, const vector<int>& ts) {
    vector<unsigned char> mask(q, 0);

    for (int i = 0; i < (int)xs.size(); ++i) {
        int r = xs[i] % q;
        unsigned char bit = (ts[i] == 1 ? 2 : 1);
        mask[r] |= bit;
        if (mask[r] == 3) return 0;
    }

    vector<int> C, C0, C1;

    for (int r = 0; r < q; ++r) {
        int ms = mask[r];
        if (ms != 0) {
            C.push_back(r);
            if (ms & 1) C0.push_back(r);
            if (ms & 2) C1.push_back(r);
        }
    }

    int Csize = (int)C.size();

    if (Csize == 0) {
        if (q & 1) {
            return (int)(1LL * q * pw2v[(q + 1) / 2] % MOD);
        } else {
            return (int)(1LL * (q / 2) * 3 % MOD * pw2v[q / 2] % MOD);
        }
    }

    int h = q - Csize;

    vector<int> DC(q, 0);
    for (int r : C) {
        DC[(2 * r) % q]++;
    }

    long long opsC = 1LL * Csize * Csize;
    long long opsH = 1LL * h * h;
    long long opsCross = 1LL * C0.size() * C1.size();

    bool bothColors = (!C0.empty() && !C1.empty());

    static const long long THRESH = 60000;
    bool needNTT = (min(opsC, opsH) > THRESH) || (opsCross > THRESH);

    vector<int> nttAll, nttCross;
    vector<int> TC, X;

    if (needNTT) {
        int L = 1;
        long long need = 2LL * q - 1;
        while (L < need) L <<= 1;

        if (bothColors) {
            vector<int> A(L, 0), B(L, 0);

            for (int r : C0) A[r] = 1;
            for (int r : C1) B[r] = 1;

            ntt(A, false);
            ntt(B, false);

            for (int i = 0; i < L; ++i) {
                int ai = A[i];
                int bi = B[i];

                int sum = ai + bi;
                if (sum >= MOD) sum -= MOD;

                A[i] = (int)(1LL * sum * sum % MOD);
                B[i] = (int)(1LL * ai * bi % MOD);
            }

            ntt(A, true);
            ntt(B, true);

            nttAll.swap(A);
            nttCross.swap(B);
        } else {
            vector<int> A(L, 0);

            for (int r : C) A[r] = 1;

            ntt(A, false);

            for (int i = 0; i < L; ++i) {
                A[i] = (int)(1LL * A[i] * A[i] % MOD);
            }

            ntt(A, true);
            nttAll.swap(A);
        }
    } else {
        if (opsC <= opsH) {
            TC.assign(q, 0);

            for (int r : C) {
                for (int s : C) {
                    int idx = r + s;
                    if (idx >= q) idx -= q;
                    TC[idx]++;
                }
            }
        } else {
            vector<int> H;
            H.reserve(h);

            for (int r = 0; r < q; ++r) {
                if (mask[r] == 0) H.push_back(r);
            }

            TC.assign(q, 0);

            for (int r : H) {
                for (int s : H) {
                    int idx = r + s;
                    if (idx >= q) idx -= q;
                    TC[idx]++;
                }
            }

            for (int a = 0; a < q; ++a) {
                TC[a] = q - 2 * h + TC[a];
            }
        }

        X.assign(q, 0);

        if (bothColors) {
            for (int r : C0) {
                for (int s : C1) {
                    int idx = r + s;
                    if (idx >= q) idx -= q;
                    X[idx]++;
                }
            }
        }
    }

    long long result = 0;

    for (int a = 0; a < q; ++a) {
        int tval, xval;

        if (needNTT) {
            tval = nttAll[a];
            if (a + q < 2 * q - 1) {
                tval += nttAll[a + q];
            }

            if (bothColors) {
                xval = nttCross[a];
                if (a + q < 2 * q - 1) {
                    xval += nttCross[a + q];
                }
            } else {
                xval = 0;
            }
        } else {
            tval = TC[a];
            xval = X.empty() ? 0 : X[a];
        }

        if (xval > 0) continue;

        int pairs = (tval - DC[a]) / 2;
        int constrainedOrbits = Csize - pairs;

        int fixedPoints;
        if (q & 1) {
            fixedPoints = 1;
        } else {
            fixedPoints = (a & 1) ? 0 : 2;
        }

        int totalOrbits = (q + fixedPoints) / 2;
        int freeOrbits = totalOrbits - constrainedOrbits;

        result += pw2v[freeOrbits];
        if (result >= MOD) result -= MOD;
    }

    return (int)result;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    for (int i = 2; i < MAXN; ++i) {
        if (spf[i] == 0) {
            for (int j = i; j < MAXN; j += i) {
                if (spf[j] == 0) spf[j] = i;
            }
        }
    }

    pw2v[0] = 1;
    for (int i = 1; i < MAXN; ++i) {
        pw2v[i] = (int)(2LL * pw2v[i - 1] % MOD);
    }

    int T;
    cin >> T;

    while (T--) {
        int n, m;
        cin >> n >> m;

        vector<int> xs(m), ts(m);

        for (int i = 0; i < m; ++i) {
            cin >> xs[i] >> ts[i];
        }

        vector<int> divisors;

        for (int i = 1; 1LL * i * i <= n; ++i) {
            if (n % i == 0) {
                divisors.push_back(i);
                if (i * i != n) divisors.push_back(n / i);
            }
        }

        long long total = pw2v[n - m];
        long long unionSum = 0;

        for (int q : divisors) {
            int S = calc_S(q, xs, ts);

            if (S == 0) continue;

            int d = n / q;
            int b = calc_b(d);

            unionSum += 1LL * b * S % MOD;
            if (unionSum >= MOD) unionSum -= MOD;
        }

        long long answer = (total - unionSum) % MOD;
        if (answer < 0) answer += MOD;

        cout << answer << '\n';
    }

    return 0;
}
