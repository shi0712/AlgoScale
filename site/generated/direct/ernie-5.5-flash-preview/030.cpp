#include <bits/stdc++.h>
using namespace std;
static const long long MOD = 998244353;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, m;
    if (!(cin >> n >> m)) return 0;
    vector<int> a(m), b(m);
    for (int i = 0; i < m; ++i) cin >> a[i];
    for (int i = 0; i < m; ++i) cin >> b[i];

    long long B = 0;
    for (int i = 0; i < m; ++i) B += b[i];
    if (B == 0) { cout << 0 << "\n"; return 0; }

    // inverses of 1..n+1
    vector<long long> invv(n + 2, 0);
    invv[1] = 1;
    for (int i = 2; i <= n + 1; ++i)
        invv[i] = MOD - (MOD / i) * invv[MOD % i] % MOD;

    // S1 = sum_i sum_{l=0}^{b_i-1} (b_i - l)/(a_i - l)
    long long S1 = 0;
    for (int i = 0; i < m; ++i)
        for (int l = 0; l < b[i]; ++l) {
            S1 += (long long)(b[i] - l) * invv[a[i] - l] % MOD;
            if (S1 >= MOD) S1 -= MOD;
        }

    // G(q) = prod_i F_i(q)
    vector<long long> G(1, 1), P, Pn;
    for (int i = 0; i < m; ++i) {
        int ai = a[i], bi = b[i];
        if (bi == 0) continue;                       // F_i == 1
        vector<long long> Fi(ai + 1, 0);
        P.assign(1, 1);                              // (1-q)^0
        long long comb = 1;                          // C(ai,0)
        for (int j = 0; j <= ai; ++j) {
            if (j >= bi) {                           // add C(ai,j)(1-q)^j q^{ai-j}
                int shift = ai - j;
                for (size_t s = 0; s < P.size(); ++s) {
                    Fi[shift + s] += comb * P[s] % MOD;
                    if (Fi[shift + s] >= MOD) Fi[shift + s] -= MOD;
                }
            }
            if (j < ai) {
                comb = comb * (ai - j) % MOD * invv[j + 1] % MOD;   // C(ai,j+1)
                Pn.assign(P.size() + 1, 0);                         // P *= (1-q)
                for (size_t s = 0; s < P.size(); ++s) {
                    long long v = Pn[s] + P[s]; if (v >= MOD) v -= MOD; Pn[s] = v;
                    long long w = Pn[s + 1] - P[s]; if (w < 0) w += MOD; Pn[s + 1] = w;
                }
                P.swap(Pn);
            }
        }
        vector<long long> Gn(G.size() + Fi.size() - 1, 0);          // G *= Fi
        for (size_t x = 0; x < G.size(); ++x) {
            if (!G[x]) continue;
            long long gx = G[x];
            for (size_t y = 0; y < Fi.size(); ++y) {
                Gn[x + y] += gx * Fi[y] % MOD;
                if (Gn[x + y] >= MOD) Gn[x + y] -= MOD;
            }
        }
        G.swap(Gn);
    }

    // E[tau] = - sum_{d>=1} g_d / d
    long long Etau = 0;
    for (int d = 1; d < (int)G.size(); ++d) {
        Etau -= G[d] * invv[d] % MOD;
        if (Etau < 0) Etau += MOD;
    }

    long long ans = ((n - B) % MOD + MOD) % MOD * Etau % MOD;
    ans = (ans + S1) % MOD;
    cout << ans << "\n";
    return 0;
}
