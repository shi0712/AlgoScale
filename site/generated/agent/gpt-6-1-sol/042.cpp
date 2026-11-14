#include <bits/stdc++.h>
using namespace std;

static constexpr int MOD = 998244353;

/*
Let E_m(x) = sum_{i=0}^m x^i/i!, and, for 1 <= j <= n, define
  G_{j,p}(x) = sum_{m>=0} m^p (x^m/m!)^j E_m(x)^(n-j).
For p=0 the m=0 term is 1. Write g_{j,p}(t)=[x^t]G_{j,p}.

Differentiating, with D=x*d/dx, gives
  D G_{j,p} = j G_{j,p+1} + (n-j)x(G_{j,p}-G_{j+1,p}).
Also, shifting m by one and expanding E_{m-1}=E_m-x^m/m! gives
  sum_{l=0}^{n-j} (-1)^l C(n-j,l) G_{j+l,j} = x^j G_{j,0}.

For t>0 store c[j][p]=g_{j,p}(t)/t^p. The second identity first
computes c[j][j], in decreasing order of j. The first then computes
c[j][p] in decreasing order of p, using only the preceding t:
  c[j][p] = j*c[j][p+1]
     + (n-j)/t * ((t-1)/t)^p * (prev[j][p]-prev[j+1][p]).
Only j previous values of g_{j,0} are needed.

Finally, F(x)=sum_{m>=0}(exp(nx)-E_m(x)^n) is the EGF of the answer.
Its derivative is F'=n*F+n*G_{1,0}. Thus
  answer[t+1] = n*(answer[t] + t!*g_{1,0}(t)).
The algorithm uses O(n^2*k) time and O(k+n^2) memory (the O(k)
array holds modular inverses).
*/
int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, k;
    cin >> n >> k;
    if (n == 1) {
        cout << k << '\n';
        return 0;
    }
    if (k == 1) {
        cout << n << '\n';
        return 0;
    }

    vector<int> inv(k);
    inv[1] = 1;
    for (int t = 2; t < k; ++t)
        inv[t] = MOD - (long long)(MOD / t) * inv[MOD % t] % MOD;

    int choose[11][11] = {};
    for (int i = 0; i <= n; ++i) {
        choose[i][0] = choose[i][i] = 1;
        for (int j = 1; j < i; ++j)
            choose[i][j] = choose[i-1][j-1] + choose[i-1][j];
    }
    int prev[12][11] = {}, cur[12][11] = {};
    int history[11][10] = {};
    for (int j = 1; j <= n; ++j) {
        prev[j][0] = 1;
        history[j][0] = 1;
    }
    int answer = n, factorial = 1;
    int inv_power[11], coefficient[11];
    for (int t = 1; t < k; ++t) {
        const int it = inv[t];
        const int ratio = (it == 1 ? 0 : MOD + 1 - it);
        inv_power[0] = 1;
        coefficient[0] = it;
        for (int p = 1; p <= n; ++p) {
            inv_power[p] = (long long)inv_power[p-1] * it % MOD;
            coefficient[p] = (long long)coefficient[p-1] * ratio % MOD;
        }
        for (int j = n; j >= 1; --j) {
            int pos = t % j;
            long long value = (long long)inv_power[j] * history[j][pos] % MOD;
            for (int l = 1; l <= n-j; ++l) {
                long long term = (long long)choose[n-j][l] * cur[j+l][j];
                if (l & 1) value += term;
                else value -= term;
            }
            value %= MOD;
            if (value < 0) value += MOD;
            cur[j][j] = (int)value;
            for (int p = j-1; p >= 0; --p) {
                long long difference = prev[j][p] - prev[j+1][p];
                // n-j <= 9, so this signed 64-bit product cannot overflow.
                long long v = (long long)j * cur[j][p+1]
                            + (long long)(n-j) * coefficient[p] * difference;
                v %= MOD;
                if (v < 0) v += MOD;
                cur[j][p] = (int)v;
            }
            history[j][pos] = cur[j][0];
        }
        factorial = (long long)factorial * t % MOD;
        answer = (long long)n * (answer + (long long)factorial * cur[1][0] % MOD) % MOD;
        memcpy(prev, cur, sizeof(prev));
    }
    cout << answer << '\n';
    return 0;
}
