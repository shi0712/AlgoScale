#include <bits/stdc++.h>
using namespace std;

using int64 = long long;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int T;
    cin >> T;

    while (T--) {
        int n;
        int64 M;
        cin >> n >> M;

        auto mod_pow = [&](int64 a, int64 e) -> int64 {
            int64 r = 1 % M;
            a %= M;
            while (e > 0) {
                if (e & 1) r = (__int128)r * a % M;
                a = (__int128)a * a % M;
                e >>= 1;
            }
            return r;
        };

        int maxF = 2 * n + 5;
        vector<int64> fact(maxF), invfact(maxF);
        fact[0] = 1;
        for (int i = 1; i < maxF; ++i) {
            fact[i] = (__int128)fact[i - 1] * i % M;
        }
        invfact[maxF - 1] = mod_pow(fact[maxF - 1], M - 2);
        for (int i = maxF - 2; i >= 0; --i) {
            invfact[i] = (__int128)invfact[i + 1] * (i + 1) % M;
        }

        auto C = [&](int N, int K) -> int64 {
            if (K < 0 || K > N) return 0;
            return (__int128)fact[N] * invfact[K] % M * invfact[N - K] % M;
        };

        // c[i] = Catalan(i)^2
        vector<int64> c(n + 1);
        for (int i = 0; i <= n; ++i) {
            int64 cat = C(2 * i, i);
            cat = (__int128)cat * mod_pow(i + 1, M - 2) % M;
            c[i] = (__int128)cat * cat % M;
        }

        int N = n + 1;
        vector<int64> B(n + 1, 0);

        // poly = C2(z)^r, initially r = 0
        vector<int64> poly(1, 1);

        for (int r = 1; r <= N; ++r) {
            int maxDeg = N - r;
            vector<int64> npoly(maxDeg + 1, 0);

            // npoly = poly * C2(z), truncated to maxDeg
            for (int i = 0; i < (int)poly.size(); ++i) {
                if (poly[i] == 0) continue;
                int lim = min((int)c.size() - 1, maxDeg - i);
                for (int j = 0; j <= lim; ++j) {
                    npoly[i + j] += (__int128)poly[i] * c[j] % M;
                    if (npoly[i + j] >= M) npoly[i + j] -= M;
                }
            }

            int k = r - 1;
            int p = n - k; // = n - (r - 1) = N - r
            if (p >= 0 && p < (int)npoly.size()) {
                int64 ways = npoly[p];
                int64 coef = C(2 * n, k);
                coef = (__int128)coef * mod_pow(k + 1, M - 2) % M;
                B[k] = (__int128)coef * ways % M;
            }

            poly.swap(npoly);
        }

        // Binomial inversion:
        // B[k] = sum_{m >= k} C(m,k) F[m]
        // F[k] = sum_{m >= k} (-1)^(m-k) C(m,k) B[m]
        vector<int64> F(n + 1, 0);

        for (int k = 0; k <= n; ++k) {
            int64 res = 0;
            for (int m = k; m <= n; ++m) {
                int64 term = (__int128)C(m, k) * B[m] % M;
                if ((m - k) & 1) {
                    res -= term;
                    if (res < 0) res += M;
                } else {
                    res += term;
                    if (res >= M) res -= M;
                }
            }
            F[k] = res;
        }

        for (int k = 0; k <= n; ++k) {
            if (k) cout << ' ';
            cout << F[k] % M;
        }
        cout << '\n';
    }

    return 0;
}
