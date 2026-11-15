#include <bits/stdc++.h>
using namespace std;
typedef unsigned int u32;
typedef long long ll;
static const u32 MOD = 1000000007u;

int main() {
    int n, Q;
    scanf("%d %d", &n, &Q);
    static char buf[2005];
    scanf("%s", buf);
    string s(buf);
    int m = 0;
    for (char ch : s) if (ch == '?') m++;
    int W = m + 2;
    size_t SZ = (size_t)W * W;
    vector<u32> dp[3], nd[3];
    for (int l = 0; l < 3; l++) {
        dp[l].assign(SZ, 0);
        nd[l].assign(SZ, 0);
    }
    // position 0
    int q = 0;
    if (s[0] == '?') {
        q = 1;
        dp[0][1 * W + 0] = 1;
        dp[1][0 * W + 1] = 1;
        dp[2][0] = 1;
    } else {
        int l = s[0] - 'a';
        dp[l][0] = 1;
    }
    for (int p = 1; p < n; p++) {
        // clear new buffers on region i+j<=q+1
        int nq = q + (s[p] == '?');
        for (int l = 0; l < 3; l++)
            for (int i = 0; i <= nq; i++)
                memset(&nd[l][(size_t)i * W], 0, sizeof(u32) * (nq - i + 1));
        if (s[p] == '?') {
            for (int i = 0; i <= q; i++) {
                for (int j = 0; i + j <= q; j++) {
                    size_t id = (size_t)i * W + j;
                    u32 a = dp[0][id], b = dp[1][id], c = dp[2][id];
                    u32 t;
                    t = b + c; if (t >= MOD) t -= MOD;
                    { u32 &r = nd[0][id + W]; r += t; if (r >= MOD) r -= MOD; }
                    t = a + c; if (t >= MOD) t -= MOD;
                    { u32 &r = nd[1][id + 1]; r += t; if (r >= MOD) r -= MOD; }
                    t = a + b; if (t >= MOD) t -= MOD;
                    { u32 &r = nd[2][id]; r += t; if (r >= MOD) r -= MOD; }
                }
            }
        } else {
            int L = s[p] - 'a';
            int o1 = (L + 1) % 3, o2 = (L + 2) % 3;
            for (int i = 0; i <= q; i++) {
                for (int j = 0; i + j <= q; j++) {
                    size_t id = (size_t)i * W + j;
                    u32 t = dp[o1][id] + dp[o2][id];
                    if (t >= MOD) t -= MOD;
                    nd[L][id] = t;
                }
            }
        }
        for (int l = 0; l < 3; l++) swap(dp[l], nd[l]);
        q = nq;
    }
    // f
    vector<u32> Pij(SZ, 0), Pik(SZ, 0), Pjk(SZ, 0);
    for (int i = 0; i <= m; i++) {
        for (int j = 0; i + j <= m; j++) {
            size_t id = (size_t)i * W + j;
            u32 v = (u32)(((ll)dp[0][id] + dp[1][id] + dp[2][id]) % MOD);
            int k = m - i - j;
            Pij[(size_t)i * W + j] = v;
            Pik[(size_t)i * W + k] = v;
            Pjk[(size_t)j * W + k] = v;
        }
    }
    auto build = [&](vector<u32> &P) {
        for (int i = W - 2; i >= 0; i--) {
            for (int j = W - 2; j >= 0; j--) {
                ll v = (ll)P[(size_t)i * W + j] + P[(size_t)(i + 1) * W + j] +
                       P[(size_t)i * W + j + 1] - P[(size_t)(i + 1) * W + j + 1];
                v %= MOD;
                if (v < 0) v += MOD;
                P[(size_t)i * W + j] = (u32)v;
            }
        }
    };
    build(Pij); build(Pik); build(Pjk);
    ll T = Pij[0];
    while (Q--) {
        int x, y, z;
        scanf("%d %d %d", &x, &y, &z);
        int xi = min(x, m) + 1, yi = min(y, m) + 1, zi = min(z, m) + 1;
        ll ans = T;
        ans -= Pij[(size_t)xi * W + 0];
        ans -= Pij[(size_t)0 * W + yi];
        ans -= Pik[(size_t)0 * W + zi];
        ans += Pij[(size_t)xi * W + yi];
        ans += Pik[(size_t)xi * W + zi];
        ans += Pjk[(size_t)yi * W + zi];
        ans %= (ll)MOD;
        if (ans < 0) ans += MOD;
        printf("%lld\n", ans);
    }
    return 0;
}
