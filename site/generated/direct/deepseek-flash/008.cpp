#include <bits/stdc++.h>
using namespace std;

using int64 = long long;
using i128 = __int128_t;

static const int MOD_NTT = 998244353;
static const int G_NTT = 3;

int wlog[11];

int mod_pow(long long a, long long e, int mod) {
    long long r = 1 % mod;
    a %= mod;
    while (e > 0) {
        if (e & 1) r = r * a % mod;
        a = a * a % mod;
        e >>= 1;
    }
    return (int)r;
}

int ntt_mod_pow(int a, int e) {
    long long r = 1, base = a;
    while (e) {
        if (e & 1) r = r * base % MOD_NTT;
        base = base * base % MOD_NTT;
        e >>= 1;
    }
    return (int)r;
}

void ntt(vector<int>& a, bool inv) {
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
        int wlen = ntt_mod_pow(G_NTT, (MOD_NTT - 1) / len);
        if (inv) wlen = ntt_mod_pow(wlen, MOD_NTT - 2);
        for (int i = 0; i < n; i += len) {
            long long w = 1;
            for (int j = 0; j < len / 2; j++) {
                int u = a[i + j];
                int v = (int)(w * a[i + j + len / 2] % MOD_NTT);
                int x = u + v;
                if (x >= MOD_NTT) x -= MOD_NTT;
                int y = u - v;
                if (y < 0) y += MOD_NTT;
                a[i + j] = x;
                a[i + j + len / 2] = y;
                w = w * wlen % MOD_NTT;
            }
        }
    }

    if (inv) {
        int ninv = ntt_mod_pow(n, MOD_NTT - 2);
        for (int& x : a) x = (long long)x * ninv % MOD_NTT;
    }
}

int small_comb(int n, int k, int p) {
    if (k < 0 || k > n) return 0;
    if (k == 0 || k == n) return 1;
    long long res = 1;
    for (int i = 1; i <= k; i++) {
        res = res * (n - k + i) / i;
    }
    return (int)(res % p);
}

// C(r + c, c) mod p
int binom_add_c(long long r, long long c, int p) {
    long long carry = 0;
    long long res = 1;
    while (r > 0 || c > 0) {
        int rd = r % p;
        int cd = c % p;
        int s = rd + cd + (int)carry;
        int y = s % p;
        carry = s / p;
        res = res * small_comb(y, cd, p) % p;
        r /= p;
        c /= p;
    }
    return (int)res;
}

struct Query {
    long long n;
    long long c;
};

int process_prime(int p, long long n, long long c) {
    // P = smallest power of p strictly greater than c
    long long P = 1;
    int rdigits = 0;
    while (P <= c) {
        P *= p;
        rdigits++;
    }

    // Full period sum T = sum_{r=0}^{P-1} C(r+c,c) mod p
    int dp[2] = {1, 0};
    for (int i = 0; i < rdigits; i++) {
        int cd = (int)((c / (long long)pow(p, i)) % p);
        // Avoid floating pow in correctness; recompute below if needed.
    }

    // Compute base-p digits of c.
    vector<int> cdig(r digits_max(1, rdigits), 0);
    {
        long long x = c;
        for (int i = 0; i < rdigits; i++) {
            cdig[i] = x % p;
            x /= p;
        }
    }

    int T = 0;
    {
        int dp0[2] = {1, 0};
        for (int i = 0; i < rdigits; i++) {
            int ndp[2] = {0, 0};
            int cd = cdig[i];
            for (int carry = 0; carry <= 1; carry++) {
                if (!dp0[carry]) continue;
                for (int rd = 0; rd < p; rd++) {
                    int s = rd + cd + carry;
                    int y = s % p;
                    int nc = s / p;
                    int fac = small_comb(y, cd, p);
                    ndp[nc] = (ndp[nc] + dp0[carry] * fac) % p;
                }
            }
            dp0[0] = ndp[0];
            dp0[1] = ndp[1];
        }
        T = (dp0[0] + dp0[1]) % p;
    }

    int hsplit = rdigits / 2;
    long long Hlow = 1;
    for (int i = 0; i < hsplit; i++) Hlow *= p;
    long long Hhigh = P / Hlow;

    // Low digit contribution and carry into high part.
    vector<int> LF(Hlow), carryLow(Hlow);
    for (int low = 0; low < Hlow; low++) {
        int x = low;
        long long carry = 0;
        long long fac = 1;
        for (int i = 0; i < hsplit; i++) {
            int rd = x % p;
            x /= p;
            int cd = cdig[i];
            int s = rd + cd + (int)carry;
            int y = s % p;
            carry = s / p;
            fac = fac * small_comb(y, cd, p) % p;
        }
        LF[low] = (int)fac;
        carryLow[low] = (int)carry;
    }

    // High digit contribution for initial carry 0/1.
    vector<int> Hval0(Hhigh), Hval1(Hhigh);
    for (int yv = 0; yv < Hhigh; yv++) {
        for (int initCarry = 0; initCarry <= 1; initCarry++) {
            int y = yv;
            long long carry = initCarry;
            long long fac = 1;
            for (int i = 0; i < rdigits - hsplit; i++) {
                int rd = y % p;
                y /= p;
                int pos = hsplit + i;
                int cd = (pos < (int)cdig.size()) ? cdig[pos] : 0;
                int s = rd + cd + (int)carry;
                int z = s % p;
                carry = s / p;
                fac = fac * small_comb(z, cd, p) % p;
            }
            if (initCarry == 0) Hval0[yv] = (int)fac;
            else Hval1[yv] = (int)fac;
        }
    }

    // Prepare NTT transform of Hval arrays if worthwhile.
    int nttN = 1;
    while (nttN < 2 * Hhigh - 1) nttN <<= 1;
    vector<int> Hfreq0, Hfreq1;
    bool hasNTT = false;
    if (Hhigh > 1000) {
        Hfreq0.assign(nttN, 0);
        Hfreq1.assign(nttN, 0);
        for (int i = 0; i < Hhigh; i++) {
            Hfreq0[i] = Hval0[i];
            Hfreq1[i] = Hval1[i];
        }
        ntt(Hfreq0, false);
        ntt(Hfreq1, false);
        hasNTT = true;
    }

    auto beta_for = [&](int e) -> int {
        int cur;
        if (e >= 0 && e <= 10) cur = wlog[e] % p;
        else cur = 0;
        int prv;
        if (e - 1 >= 0 && e - 1 <= 10) prv = wlog[e - 1] % p;
        else prv = 0;
        int b = (cur - prv) % p;
        if (b < 0) b += p;
        return b;
    };

    const long long NAIVE_LIMIT = 2000000LL;

    auto get_S = [&](long long b, long long L, vector<int>& S0, vector<int>& S1) {
        long long b1 = b / Hlow;
        S0.assign(Hhigh, 0);
        S1.assign(Hhigh, 0);
        if (b1 == 0) return;

        long long LmodH = L % Hhigh;
        long long prod = b1 * Hhigh;

        if (!hasNTT || prod <= NAIVE_LIMIT) {
            long long z = 0;
            for (long long qi = 0; qi < b1; qi++) {
                int zz = (int)z;
                int split = (int)Hhigh - zz;
                for (int y = 0; y < Hhigh; y++) {
                    int idx = y + zz;
                    if (idx >= Hhigh) idx -= Hhigh;
                    S0[idx] += Hval0[y];
                    S1[idx] += Hval1[y];
                }
                z += LmodH;
                if (z >= Hhigh) z %= Hhigh; // safe
            }
            for (int i = 0; i < Hhigh; i++) {
                S0[i] %= p;
                S1[i] %= p;
            }
            return;
        }

        vector<int> A(nttN, 0);
        long long z = 0;
        for (long long qi = 0; qi < b1; qi++) {
            A[(int)z] = 1;
            z += LmodH;
            if (z >= Hhigh) z %= Hhigh;
        }
        ntt(A, false);

        vector<int> C(nttN);
        for (int i = 0; i < nttN; i++) C[i] = (long long)A[i] * Hfreq0[i] % MOD_NTT;
        ntt(C, true);
        for (int v = 0; v < Hhigh; v++) {
            long long val = C[v];
            if (v + Hhigh < nttN) val += C[v + Hhigh];
            S0[v] = val % p;
        }

        for (int i = 0; i < nttN; i++) C[i] = (long long)A[i] * Hfreq1[i] % MOD_NTT;
        ntt(C, true);
        for (int v = 0; v < Hhigh; v++) {
            long long val = C[v];
            if (v + Hhigh < nttN) val += C[v + Hhigh];
            S1[v] = val % p;
        }
    };

    auto eval_H = [&](long long b, long long a, long long L,
                      const vector<int>& S0, const vector<int>& S1) -> int {
        long long b0 = b % Hlow;
        long long b1 = b / Hlow;
        long long LmodP = L % P;

        long long sum = 0;
        long long rr = a % P;
        for (long long q0 = 0; q0 < Hlow; q0++) {
            int low = (int)(rr % Hlow);
            int high = (int)(rr / Hlow);
            int carry = carryLow[low];
            int lf = LF[low];
            int sv = (carry == 0 ? S0[high] : S1[high]);
            sum += (long long)lf * sv;

            rr -= LmodP;
            if (rr < 0) rr += P;
        }

        if (b0 > 0) {
            long long qbase = (Hlow * b1) % P;
            for (long long q0 = 0; q0 < b0; q0++) {
                long long q = qbase + q0;
                long long prod = (LmodP * (q % P)) % P;
                long long r = (a - prod) % P;
                if (r < 0) r += P;
                sum += binom_add_c(r, c, p);
            }
        }

        return (int)(sum % p);
    };

    int Sp = 0;
    long long h = 1;

    while (h <= n) {
        i128 L128 = (i128)11 * h;

        if (L128 > n) {
            // q = 0 only
            for (int e = 1; e <= 11; e++) {
                i128 temp = (i128)e * h;
                if (temp > n) continue;
                long long m = n - (long long)temp;
                int B = binom_add_c(m, c, p);
                int beta = beta_for(e);
                Sp = (Sp + beta * B) % p;
            }
        } else {
            long long L = (long long)L128;

            struct Item {
                int e;
                long long a;
                int sign;
                long long full;
            };
            vector<pair<long long, vector<Item>>> groups;

            for (int e = 1; e <= 11; e++) {
                i128 temp = (i128)e * h;
                if (temp > n) continue;
                long long m = n - (long long)temp;
                long long Q = m / L;
                long long len = Q + 1;
                long long full = len / P;
                long long rem = len % P;
                long long a = m % P;

                long long b;
                long long aa;
                int sign;
                long long extraFull;

                if (rem > P / 2) {
                    b = P - rem;
                    long long prod = (L % P) * (rem % P) % P;
                    aa = a - prod;
                    aa %= P;
                    if (aa < 0) aa += P;
                    sign = -1;
                    extraFull = full + 1;
                } else {
                    b = rem;
                    aa = a;
                    sign = 1;
                    extraFull = full;
                }

                bool found = false;
                for (auto& gr : groups) {
                    if (gr.first == b) {
                        gr.second.push_back({e, aa, sign, extraFull});
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    groups.push_back({b, vector<Item>()});
                    groups.back().second.push_back({e, aa, sign, extraFull});
                }
            }

            for (auto& gr : groups) {
                long long b = gr.first;
                vector<int> S0, S1;
                if (b > 0) {
                    get_S(b, L, S0, S1);
                }

                for (const Item& it : gr.second) {
                    int H = 0;
                    if (b > 0) H = eval_H(b, it.a, L, S0, S1);

                    long long fullMod = it.full % p;
                    int C = (int)((fullMod * T + it.sign * H) % p);
                    if (C < 0) C += p;

                    int beta = beta_for(it.e);
                    Sp = (Sp + beta * C) % p;
                }
            }
        }

        if (h > n / 11) break;
        h *= 11;
    }

    return (Sp % p + p) % p;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    // c_d = coefficients of (1 - 2x - 3x^2)^5 mod 11
    int cd[11] = {1, 1, 3, 7, 8, 7, 9, 8, 7, 4, 10};

    // discrete logs base 2 modulo 11
    int pw[11];
    int cur = 1;
    for (int i = 0; i < 10; i++) {
        pw[cur] = i;
        cur = cur * 2 % 11;
    }
    for (int d = 0; d <= 10; d++) {
        wlog[d] = pw[cd[d]];
    }

    int TQ;
    cin >> TQ;

    while (TQ--) {
        long long n, c;
        cin >> n >> c;

        int s2 = process_prime(2, n, c);
        int s5 = process_prime(5, n, c);

        int s10 = 0;
        for (int x = 0; x < 10; x++) {
            if (x % 2 == s2 && x % 5 == s5) {
                s10 = x;
                break;
            }
        }

        int ans = mod_pow(2, s10, 11);
        cout << ans << '\n';
    }

    return 0;
}
