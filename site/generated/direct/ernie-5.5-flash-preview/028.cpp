#include <bits/stdc++.h>
using namespace std;

const int MOD = 998244353;
const int MAXA = 6000;

long long modpow(long long a, long long e) {
    long long r = 1;
    while (e) {
        if (e & 1) r = r * a % MOD;
        a = a * a % MOD;
        e >>= 1;
    }
    return r;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    cin >> n;
    vector<int> cnt(MAXA + 1, 0);
    for (int i = 0; i < n; ++i) {
        int x;
        cin >> x;
        ++cnt[x];
    }

    // smallest prime factor
    vector<int> spf(MAXA + 1, 0);
    vector<int> primes;
    for (int i = 2; i <= MAXA; ++i) {
        if (spf[i] == 0) {
            spf[i] = i;
            primes.push_back(i);
            for (int j = i; j <= MAXA; j += i) {
                if (spf[j] == 0) spf[j] = i;
            }
        }
    }

    // radical
    vector<int> rad(MAXA + 1, 1);
    for (int v = 2; v <= MAXA; ++v) {
        int x = v;
        int r = 1;
        while (x > 1) {
            int p = spf[x];
            r *= p;
            while (x % p == 0) x /= p;
        }
        rad[v] = r;
    }

    // A[d] = product over values v with rad(v)=d of (1+v)^cnt[v]
    vector<int> A(MAXA + 1, 1);
    for (int v = 1; v <= MAXA; ++v) {
        if (cnt[v]) {
            int d = rad[v];
            A[d] = (long long)A[d] * modpow(v + 1, cnt[v]) % MOD;
        }
    }

    // modular inverses up to MAXA
    vector<int> inv(MAXA + 1, 0);
    inv[1] = 1;
    for (int i = 2; i <= MAXA; ++i) {
        inv[i] = (long long)(MOD - MOD / i) * inv[MOD % i] % MOD;
    }

    // small primes <= 77
    vector<int> S;
    for (int p : primes) {
        if (p <= 77) S.push_back(p);
    }
    int k = (int)S.size();
    vector<int> bitOf(MAXA + 1, -1);
    for (int i = 0; i < k; ++i) bitOf[S[i]] = i;

    int fullSmall = (1 << k) - 1;

    // arr[mask] initially A[d] for squarefree d using only small primes
    vector<int> arr(1 << k, 1);
    for (int d = 1; d <= MAXA; ++d) {
        if (rad[d] != d) continue; // not squarefree
        int x = d;
        int mask = 0;
        bool ok = true;
        while (x > 1) {
            int p = spf[x];
            if (p > 77) {
                ok = false;
                break;
            }
            mask |= (1 << bitOf[p]);
            x /= p;
        }
        if (ok) arr[mask] = A[d];
    }

    // subset product zeta: arr[Y] = product_{d subset Y} initial[d]
    for (int i = 0; i < k; ++i) {
        int bit = 1 << i;
        for (int mask = 0; mask <= fullSmall; ++mask) {
            if (mask & bit) {
                arr[mask] = (long long)arr[mask] * arr[mask ^ bit] % MOD;
            }
        }
    }

    // group large primes p > 77 by L = floor(6000 / p)
    vector<vector<int>> groups(78);
    for (int p : primes) {
        if (p > 77 && p <= MAXA) {
            int L = MAXA / p;
            groups[L].push_back(p);
        }
    }

    // H[t][mask] = product of G_L for all L with pi(L)=t
    vector<vector<int>> H(22);

    for (int L = 1; L <= 77; ++L) {
        if (groups[L].empty()) continue;

        int t = upper_bound(S.begin(), S.end(), L) - S.begin();
        int lowLimit = L / 2;
        int t_low = upper_bound(S.begin(), S.end(), lowLimit) - S.begin();

        int lowSize = 1 << t_low;
        int tSize = 1 << t;
        int lowMaskFull = lowSize - 1;

        // all squarefree m_low <= L using primes <= lowLimit
        vector<pair<int, int>> mlist;
        for (int mask = 0; mask < lowSize; ++mask) {
            long long prod = 1;
            bool ok = true;
            for (int j = 0; j < t_low; ++j) {
                if (mask & (1 << j)) {
                    prod *= S[j];
                    if (prod > L) {
                        ok = false;
                        break;
                    }
                }
            }
            if (ok) mlist.push_back({mask, (int)prod});
        }

        vector<int> highPrimes;
        for (int j = t_low; j < t; ++j) highPrimes.push_back(S[j]);
        int hcount = (int)highPrimes.size();
        int highSize = 1 << hcount;

        vector<int> G(tSize, 1);

        for (int p : groups[L]) {
            vector<int> B(lowSize, 1);
            for (auto [mask, prod] : mlist) {
                int idx = p * prod;
                B[mask] = A[idx];
            }

            // subset product zeta on low primes
            for (int i = 0; i < t_low; ++i) {
                int bit = 1 << i;
                for (int mask = 0; mask < lowSize; ++mask) {
                    if (mask & bit) {
                        B[mask] = (long long)B[mask] * B[mask ^ bit] % MOD;
                    }
                }
            }

            vector<int> highFac(highSize, 1);
            for (int hm = 0; hm < highSize; ++hm) {
                int val = 1;
                for (int j = 0; j < hcount; ++j) {
                    if (!(hm & (1 << j))) {
                        int q = highPrimes[j];
                        int idx = p * q;
                        val = (long long)val * A[idx] % MOD;
                    }
                }
                highFac[hm] = val;
            }

            int invp = inv[p];
            int oneMinus = (1 - invp + MOD) % MOD;

            for (int X = 0; X < tSize; ++X) {
                int lowX = X & lowMaskFull;
                int highX = X >> t_low;
                int Y = lowMaskFull ^ lowX;
                int Q = (long long)B[Y] * highFac[highX] % MOD;
                int f = (invp + (long long)oneMinus * Q) % MOD;
                G[X] = (long long)G[X] * f % MOD;
            }
        }

        if (H[t].empty()) H[t].assign(tSize, 1);
        for (int X = 0; X < tSize; ++X) {
            H[t][X] = (long long)H[t][X] * G[X] % MOD;
        }
    }

    // DP over small primes
    vector<int> dp(1, 1);
    for (int i = 0; i < k; ++i) {
        int p = S[i];
        int invp = inv[p];
        int w0 = (1 - invp + MOD) % MOD;
        int w1 = invp;

        int oldSize = 1 << i;
        int newSize = 1 << (i + 1);
        vector<int> ndp(newSize, 0);

        for (int mask = 0; mask < oldSize; ++mask) {
            int val = dp[mask];
            ndp[mask] = (long long)val * w0 % MOD;
            ndp[mask | (1 << i)] = (long long)val * w1 % MOD;
        }

        if (!H[i + 1].empty()) {
            for (int mask = 0; mask < newSize; ++mask) {
                ndp[mask] = (long long)ndp[mask] * H[i + 1][mask] % MOD;
            }
        }

        dp.swap(ndp);
    }

    long long ans = 0;
    for (int mask = 0; mask <= fullSmall; ++mask) {
        ans += (long long)dp[mask] * arr[fullSmall ^ mask] % MOD;
    }
    ans %= MOD;

    if (!H[0].empty()) {
        ans = ans * H[0][0] % MOD;
    }

    cout << ans << '\n';
    return 0;
}
