#include <bits/stdc++.h>
using namespace std;

static constexpr int MOD = 998244353;

long long mod_pow(long long a, long long e) {
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

    int T;
    cin >> T;

    while (T--) {
        int n;
        cin >> n;

        vector<int> p(n + 1);
        bool sorted = true;
        for (int i = 1; i <= n; ++i) {
            cin >> p[i];
            if (p[i] != i) sorted = false;
        }

        if (sorted) {
            cout << 0 << '\n';
            continue;
        }

        // n >= 2 here
        int inv_n = (int)mod_pow(n, MOD - 2);
        int inv_nm1 = (int)mod_pow(n - 1, MOD - 2);

        // q = n / (n - 1), r = 1 / (n - 1)
        int r = inv_nm1;
        int q = (long long)n * r % MOD;

        int q2 = (long long)q * q % MOD;
        int s = (1 + 2LL * r) % MOD;
        int twoq = 2LL * q % MOD;
        int coeff = (s + q2) % MOD;
        int sq = (long long)s * q % MOD;

        int N = n + 1;
        vector<int> H(N * N, 0);

        /*
            H[a][b] =
            sum_{i=0..a, j=0..b}
                C(a,i) C(b,j) C(i+j+1, i+1) r^(i+j+1)

            Computed by a bivariate generating function recurrence.
        */
        for (int a = 0; a <= n; ++a) {
            for (int b = 0; b <= n; ++b) {
                long long val = 0;

                if (a == 0 && b == 0) val += r;
                else if (a == 0 && b == 1) val -= r;

                if (a > 0)
                    val += (long long)q * H[(a - 1) * N + b] % MOD;

                if (b > 0)
                    val += (long long)twoq * H[a * N + (b - 1)] % MOD;

                if (a > 0 && b > 0)
                    val -= (long long)coeff * H[(a - 1) * N + (b - 1)] % MOD;

                if (b > 1)
                    val -= (long long)q2 * H[a * N + (b - 2)] % MOD;

                if (a > 0 && b > 1)
                    val += (long long)sq * H[(a - 1) * N + (b - 2)] % MOD;

                val %= MOD;
                if (val < 0) val += MOD;
                H[a * N + b] = (int)val;
            }
        }

        vector<int> powq(n + 1, 1);
        for (int i = 1; i <= n; ++i)
            powq[i] = (long long)powq[i - 1] * q % MOD;

        vector<char> present(n + 1, 0);
        vector<int> countLE(n + 1, 0);

        int mex = 1;
        int prefMax = 0;
        int sumF = 0;

        for (int cut = 1; cut <= n - 1; ++cut) {
            int v = p[cut];
            present[v] = 1;

            if (v == mex) {
                while (mex <= n && present[mex]) ++mex;
            }
            if (v > prefMax) prefMax = v;

            int run = 0;
            countLE[0] = 0;
            for (int x = 1; x <= n; ++x) {
                run += present[x];
                countLE[x] = run;
            }

            int z = mex;       // minimum suffix value
            int o = prefMax;   // maximum prefix value

            if (z < o) {
                // Paths with no additional missing value
                int pTotal = cut - countLE[z] - 1;
                int y = powq[pTotal];

                // Additional missing value s
                for (int x = z + 1; x < o; ++x) {
                    if (!present[x]) {
                        int A = x - 2 - countLE[x - 1];
                        int B = cut - countLE[x] - 1;
                        y += H[A * N + B];
                        if (y >= MOD) y -= MOD;
                    }
                }

                int F = (long long)q * y % MOD;
                sumF += F;
                if (sumF >= MOD) sumF -= MOD;
            }
        }

        int ans = (1 + (long long)sumF * inv_n) % MOD;
        cout << ans << '\n';
    }

    return 0;
}
