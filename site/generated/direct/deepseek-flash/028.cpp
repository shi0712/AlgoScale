#include <bits/stdc++.h>
using namespace std;

static const int MOD = 998244353;
static const int MAXA = 6000;
static const int K = 21;
static const int N = 1 << 21;
static const int R = 16;
static const int FS = 1 << R;

int cntVal[MAXA + 1];
int smallMaskArr[MAXA + 1];
int largePrimeArr[MAXA + 1];
int pow1p[MAXA + 1];
int invNum[MAXA + 1];

int modpow(long long a, int e) {
    long long r = 1;
    while (e > 0) {
        if (e & 1) r = r * a % MOD;
        a = a * a % MOD;
        e >>= 1;
    }
    return (int)r;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    cin >> n;

    for (int i = 0; i < n; ++i) {
        int a;
        cin >> a;
        ++cntVal[a];
    }

    vector<int> smallPrimes = {
        2, 3, 5, 7, 11, 13, 17, 19, 23, 29,
        31, 37, 41, 43, 47, 53, 59, 61, 67, 71, 73
    };

    invNum[1] = 1;
    for (int i = 2; i <= MAXA; ++i) {
        invNum[i] = MOD - (long long)(MOD / i) * invNum[MOD % i] % MOD;
    }

    for (int v = 1; v <= MAXA; ++v) {
        int temp = v;
        int mask = 0;

        for (int i = 0; i < K; ++i) {
            int p = smallPrimes[i];
            if (temp % p == 0) {
                mask |= (1 << i);
                while (temp % p == 0) temp /= p;
            }
        }

        smallMaskArr[v] = mask;
        largePrimeArr[v] = (temp > 1 ? temp : 0);
    }

    for (int v = 1; v <= MAXA; ++v) {
        pow1p[v] = 1;
        if (cntVal[v] > 0) {
            pow1p[v] = modpow(v + 1, cntVal[v]);
        }
    }

    vector<int> Z(N, 1);

    // Independent elements with no large prime factor.
    for (int v = 1; v <= MAXA; ++v) {
        if (cntVal[v] > 0 && largePrimeArr[v] == 0) {
            int mask = smallMaskArr[v];
            Z[mask] = (long long)Z[mask] * pow1p[v] % MOD;
        }
    }

    // Subset product zeta transform over the 21 small primes.
    for (int i = 0, step = 1; i < K; ++i, step <<= 1) {
        int jump = step << 1;
        for (int base = 0; base < N; base += jump) {
            for (int j = 0; j < step; ++j) {
                int idx = base + step + j;
                Z[idx] = (long long)Z[idx] * Z[base + j] % MOD;
            }
        }
    }

    bool presentLarge[6001] = {};
    for (int v = 1; v <= MAXA; ++v) {
        if (cntVal[v] > 0 && largePrimeArr[v] > 0) {
            presentLarge[largePrimeArr[v]] = true;
        }
    }

    vector<int> groups[76];

    for (int l = 1; l <= MAXA; ++l) {
        if (presentLarge[l]) {
            int B = MAXA / l;
            if (1 <= B && B <= 75) {
                groups[B].push_back(l);
            }
        }
    }

    int prefSmall[76];
    int rcnt = 0;
    for (int B = 0; B <= 75; ++B) {
        while (rcnt < K && smallPrimes[rcnt] <= B) ++rcnt;
        prefSmall[B] = rcnt;
    }

    vector<int> P(N), H(N);
    vector<int> Fsmall(FS, 1);

    for (int B = 1; B <= 75; ++B) {
        if (groups[B].empty()) continue;

        int r = prefSmall[B];
        int sizeH = 1 << r;
        int m = (int)groups[B].size();

        if (m == 1) {
            int l = groups[B][0];

            fill(P.begin(), P.begin() + sizeH, 1);

            for (int b = 1; b <= B; ++b) {
                int v = l * b;
                if (v <= MAXA && cntVal[v] > 0) {
                    int mask = smallMaskArr[b];
                    P[mask] = (long long)P[mask] * pow1p[v] % MOD;
                }
            }

            for (int i = 0, step = 1; i < r; ++i, step <<= 1) {
                int jump = step << 1;
                for (int base = 0; base < sizeH; base += jump) {
                    for (int j = 0; j < step; ++j) {
                        int idx = base + step + j;
                        P[idx] = (long long)P[idx] * P[base + j] % MOD;
                    }
                }
            }

            int invl = invNum[l];
            int lm1 = l - 1;

            for (int T = 0; T < sizeH; ++T) {
                int term = (1 + (long long)lm1 * P[T]) % MOD;
                term = (long long)term * invl % MOD;
                H[T] = term;
            }
        } else {
            fill(H.begin(), H.begin() + sizeH, 1);

            for (int l : groups[B]) {
                fill(P.begin(), P.begin() + sizeH, 1);

                for (int b = 1; b <= B; ++b) {
                    int v = l * b;
                    if (v <= MAXA && cntVal[v] > 0) {
                        int mask = smallMaskArr[b];
                        P[mask] = (long long)P[mask] * pow1p[v] % MOD;
                    }
                }

                for (int i = 0, step = 1; i < r; ++i, step <<= 1) {
                    int jump = step << 1;
                    for (int base = 0; base < sizeH; base += jump) {
                        for (int j = 0; j < step; ++j) {
                            int idx = base + step + j;
                            P[idx] = (long long)P[idx] * P[base + j] % MOD;
                        }
                    }
                }

                int invl = invNum[l];
                int lm1 = l - 1;

                for (int T = 0; T < sizeH; ++T) {
                    int term = (1 + (long long)lm1 * P[T]) % MOD;
                    term = (long long)term * invl % MOD;
                    H[T] = (long long)H[T] * term % MOD;
                }
            }
        }

        if (r <= R) {
            int maskH = sizeH - 1;
            for (int idx = 0; idx < FS; ++idx) {
                Fsmall[idx] = (long long)Fsmall[idx] * H[idx & maskH] % MOD;
            }
        } else {
            for (int base = 0; base < N; base += sizeH) {
                for (int low = 0; low < sizeH; ++low) {
                    Z[base + low] = (long long)Z[base + low] * H[low] % MOD;
                }
            }
        }
    }

    int invProdSmall = 1;
    for (int p : smallPrimes) {
        invProdSmall = (long long)invProdSmall * invNum[p] % MOD;
    }

    int maskF = FS - 1;
    int ans = (long long)Z[0] * Fsmall[0] % MOD;

    // Reuse Z to store pred[S] = product_{i in S} (p_i - 1).
    Z[0] = 1;

    for (int S = 1; S < N; ++S) {
        int lb = S & -S;
        int bit = __builtin_ctz((unsigned)lb);

        int predS = (long long)Z[S ^ lb] * (smallPrimes[bit] - 1) % MOD;

        int curZ = Z[S];
        int z = (long long)curZ * Fsmall[S & maskF] % MOD;
        int term = (long long)z * predS % MOD;

        ans += term;
        if (ans >= MOD) ans -= MOD;

        Z[S] = predS;
    }

    ans = (long long)ans * invProdSmall % MOD;
    cout << ans << '\n';

    return 0;
}
