#include <bits/stdc++.h>
using namespace std;
typedef long long ll;

static const ll NEG = -(1LL << 60);
static const ll NEG2 = NEG / 2;
static const ll M = 1LL << 20;

int n;
vector<unsigned char> s;
vector<unsigned char> mk;
vector<ll> F, X, O;

static const int Lp[6] = {3, 3, 3, 4, 4, 5};
static const int xoff[6] = {0, 0, 1, 0, 0, 0};

// returns V (gain - lambda*cnt) and cnt (minimal count among optima)
pair<ll, ll> run(int lam) {
    ll delta[6];
    for (int L = 3; L <= 5; L++) delta[L] = (ll)(L - lam) * M - 1;
    for (int i = 0; i <= n + 5; i++) F[i] = X[i] = O[i] = NEG;
    F[0] = 0;
    for (int q = 0; q < n; q++) {
        int p = q + 1;
        ll f = F[q], xv = X[q], ov = O[q];
        ll best = f;
        if (xv > NEG2 && s[p] != s[q]) best = max(best, xv);
        if (ov > NEG2 && s[p] != s[q - 1]) best = max(best, ov);
        F[q + 1] = best;
        unsigned m = mk[p];
        while (m) {
            int id = __builtin_ctz(m);
            m &= m - 1;
            unsigned char x = s[p + xoff[id]];
            int L = Lp[id];
            ll b = NEG;
            if (f > NEG2 && s[q] != x) b = f;
            if (xv > NEG2 && !(s[q] == x && s[p] == x)) b = max(b, xv);
            if (ov > NEG2 && !(s[p] == s[q - 1] && s[q] == x)) b = max(b, ov);
            if (b > NEG2) {
                ll val = b + delta[L];
                if (id == 0) {
                    if (val > O[q + L]) O[q + L] = val;
                } else {
                    if (val > X[q + L]) X[q + L] = val;
                }
            }
        }
    }
    ll key = max(F[n], max(X[n], O[n]));
    ll V = (key + M - 1) / M;
    ll cnt = V * M - key;
    return {V, cnt};
}

int main() {
    int T;
    scanf("%d", &T);
    static char buf[200005];
    while (T--) {
        int K;
        scanf("%s %d", buf, &K);
        n = strlen(buf);
        s.assign(n + 8, 0);
        for (int i = 1; i <= n; i++) s[i] = buf[i - 1] - 'a' + 1;
        mk.assign(n + 8, 0);
        for (int p = 1; p <= n; p++) {
            unsigned m = 0;
            if (p + 2 <= n) {
                if (s[p] == s[p + 1] && s[p + 2] != s[p]) m |= 1;
                if (s[p] == s[p + 2] && s[p + 1] != s[p]) m |= 2;
                if (s[p + 1] == s[p + 2] && s[p] != s[p + 1]) m |= 4;
            }
            if (p + 3 <= n) {
                if (s[p] == s[p + 1] && s[p + 1] == s[p + 3] && s[p + 2] != s[p]) m |= 8;
                if (s[p] == s[p + 2] && s[p + 2] == s[p + 3] && s[p + 1] != s[p]) m |= 16;
            }
            if (p + 4 <= n) {
                if (s[p] == s[p + 1] && s[p + 1] == s[p + 3] && s[p + 3] == s[p + 4] && s[p + 2] != s[p]) m |= 32;
            }
            mk[p] = (unsigned char)m;
        }
        F.assign(n + 8, NEG);
        X.assign(n + 8, NEG);
        O.assign(n + 8, NEG);
        pair<ll, ll> res[6];
        bool have[6] = {false, false, false, false, false, false};
        auto get = [&](int lam) -> pair<ll, ll> {
            if (!have[lam]) {
                res[lam] = run(lam);
                have[lam] = true;
            }
            return res[lam];
        };
        int lo = 0, hi = 5;
        while (lo < hi) {
            int mid = (lo + hi) / 2;
            if (get(mid).second <= K) hi = mid;
            else lo = mid + 1;
        }
        pair<ll, ll> r = get(lo);
        ll ans = r.first + (ll)lo * K;
        printf("%lld\n", ans);
    }
    return 0;
}
