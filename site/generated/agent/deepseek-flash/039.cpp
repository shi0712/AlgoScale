#include <bits/stdc++.h>
using namespace std;

typedef long long ll;
static const ll NEG = LLONG_MIN / 4;

int main() {
    int n, k;
    if (scanf("%d %d", &n, &k) != 2) return 0;
    vector<pair<int, int>> cards(n);  // (t, v)
    for (int i = 0; i < n; i++) {
        int v, t;
        if (scanf("%d %d", &v, &t) != 2) return 0;
        cards[i] = {t, v};
    }
    sort(cards.begin(), cards.end());  // ascending t

    int kk = min(k, 7);  // never need more than 7 doublings

    // prefix / suffix sums of t
    vector<long long> pref(n + 1, 0), suff(n + 1, 0);
    for (int i = 0; i < n; i++) pref[i + 1] = pref[i] + cards[i].first;
    for (int i = n - 1; i >= 0; i--) suff[i] = suff[i + 1] + cards[i].first;

    const long long WMAX = 8000;
    long long Wl = min<long long>(WMAX, 2 * suff[0]);
    int W = (int)Wl;
    int sz = 2 * W + 1;

    vector<vector<ll>> dp(kk + 1, vector<ll>(sz, NEG));
    vector<vector<ll>> ndp(kk + 1, vector<ll>(sz, NEG));
    dp[0][W] = 0;

    for (int i = 0; i < n; i++) {
        const int ti = cards[i].first;
        const ll vi = cards[i].second;
        // reachable/useful absolute-balance bound before and after this card
        int curM = (int)min<long long>(W, 2 * min(pref[i], suff[i]));
        int newM = (int)min<long long>(W, 2 * min(pref[i + 1], suff[i + 1]));

        int nlo = W - newM, nhi = W + newM;
        for (int d = 0; d <= kk; d++)
            fill(ndp[d].begin() + nlo, ndp[d].begin() + nhi + 1, NEG);

        int clo = W - curM, chi = W + curM;
        for (int d = 0; d <= kk; d++) {
            const ll *cur = dp[d].data();
            ll *out = ndp[d].data();
            for (int bi = clo; bi <= chi; bi++) {
                ll val = cur[bi];
                if (val == NEG) continue;
                if (val > out[bi]) out[bi] = val;
                int nb = bi + ti;
                if (nb >= nlo && nb <= nhi && val + vi > out[nb]) out[nb] = val + vi;
                nb = bi - ti;
                if (nb >= nlo && nb <= nhi && val + vi > out[nb]) out[nb] = val + vi;
            }
            if (d < kk) {
                ll *out2 = ndp[d + 1].data();
                const int t2 = 2 * ti;
                for (int bi = clo; bi <= chi; bi++) {
                    ll val = cur[bi];
                    if (val == NEG) continue;
                    int nb = bi + t2;
                    if (nb >= nlo && nb <= nhi && val + vi > out2[nb]) out2[nb] = val + vi;
                    nb = bi - t2;
                    if (nb >= nlo && nb <= nhi && val + vi > out2[nb]) out2[nb] = val + vi;
                }
            }
        }
        dp.swap(ndp);
    }

    ll best = 0;
    for (int d = 0; d <= kk; d++) best = max(best, dp[d][W]);
    printf("%lld\n", best);
    return 0;
}
