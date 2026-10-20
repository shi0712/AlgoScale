#include <bits/stdc++.h>
using namespace std;
typedef long long ll;
static const ll MOD = 998244353;

static ll pw(ll b, ll e) {
    b %= MOD; if (b < 0) b += MOD;
    ll r = 1;
    while (e > 0) { if (e & 1) r = r * b % MOD; b = b * b % MOD; e >>= 1; }
    return r;
}
static int rd() { int t; if (scanf("%d", &t) != 1) exit(0); return t; }

/*
 * For a level v let S_v be its stations, mn[v]/mx[v] the extreme positions.
 * A service i stops at a level-v station iff (pos <= p_i and v >= x_i) or
 * (pos > p_i and v >= y_i).  Hence the service covers ALL of S_v iff
 *   p_i >= mx[v]            -> needs v >= x_i
 *   p_i <  mn[v]            -> needs v >= y_i
 *   mn[v] <= p_i < mx[v]    -> needs v >= x_i and v >= y_i.
 * Every pair of a level is covered iff the extreme pair is, so level v
 * (with >=2 stations) just needs one service fully covering it.
 *
 * free[v]  := exists i with x_i <= v and p_i >= mx[v]   (covered for every y).
 * For a non-free level every service with x_i <= v has p_i < mx[v], so the
 * condition becomes
 *      exists i : y_i <= v and (x_i <= v or p_i < mn[v]).
 * Constraint (v1,mn1) implies (v2,mn2) when v1<=v2 and mn1<=mn2, so keep the
 * Pareto-minimal constraints: v_1<...<v_q with mn_1>...>mn_q.
 *
 * Inclusion-exclusion over the set S of violated constraints: service i then
 * needs y_i > M_i(S), M_i(S) = max{ v in S : x_i <= v or p_i < mn[v] }.
 * With b_i = first index with v_j >= x_i and a_i = last index with mn_j > p_i,
 * that maximum is v_{max{j in S : j >= b_i or j <= a_i}}.  Fixing L = max S,
 * services with b_i<=L or a_i>=L get the factor (k - v_L); the remaining ones
 * (a_i < L < b_i) only look at the prefix max of S inside [1,L-1], which a
 * small DP over the "current prefix maximum" handles.
 */

int main() {
    int n = rd(), m = rd(), k = rd();
    vector<int> cnt(k + 1, 0), mn(k + 1, INT_MAX), mx(k + 1, -1);
    for (int j = 1; j <= n; j++) {
        int v = rd();
        cnt[v]++;
        mn[v] = min(mn[v], j);
        mx[v] = max(mx[v], j);
    }
    vector<int> p(m), x(m);
    for (int i = 0; i < m; i++) { p[i] = rd(); x[i] = rd(); }

    // required levels that are not covered for free
    vector<int> R;
    for (int v = 1; v <= k; v++) {
        if (cnt[v] < 2) continue;
        bool freev = false;
        for (int i = 0; i < m; i++)
            if (x[i] <= v && p[i] >= mx[v]) { freev = true; break; }
        if (!freev) R.push_back(v);
    }
    if (R.empty()) { printf("%lld\n", pw(k, m)); return 0; }

    // Pareto-minimal constraints (v increasing, mn strictly decreasing)
    vector<int> V, Mn;
    int best = INT_MAX;
    for (int v : R) if (mn[v] < best) { V.push_back(v); Mn.push_back(mn[v]); best = mn[v]; }
    int q = (int)V.size();

    // a_i: last index with Mn > p_i ; b_i: first index with V >= x_i (q+1 if none)
    vector<int> ai(m), bi(m);
    for (int i = 0; i < m; i++) {
        int A = 0;
        while (A < q && Mn[A] > p[i]) A++;
        ai[i] = A;
        int B = q + 1;
        for (int j = 0; j < q; j++) if (V[j] >= x[i]) { B = j + 1; break; }
        bi[i] = B;
    }

    // ca[L][r] = #services with a_i = r and b_i > L (only r < L is used)
    vector<vector<int>> ca(q + 1, vector<int>(q + 1, 0));
    for (int i = 0; i < m; i++) {
        int r = ai[i], hi = min(bi[i] - 1, q);
        for (int L = r + 1; L <= hi; L++) ca[L][r]++;
    }

    // P[idx][e] = (k - value_idx)^e, value_0 = 0, value_idx = V[idx-1]
    vector<vector<ll>> P(q + 1, vector<ll>(m + 1));
    for (int idx = 0; idx <= q; idx++) {
        ll base = (idx == 0 ? k : k - V[idx - 1]);
        base %= MOD; if (base < 0) base += MOD;
        P[idx][0] = 1;
        for (int e = 1; e <= m; e++) P[idx][e] = P[idx][e - 1] * base % MOD;
    }

    ll ans = pw(k, m);          // contribution of the empty violated set
    vector<ll> dp(q + 1);
    for (int L = 1; L <= q; L++) {
        // dp[s] = signed weight of subsets of [1..j] whose maximum is s,
        // including the factors of all services with a_i <= j.
        int used = ca[L][0];
        dp[0] = P[0][used];
        int tot = used;
        for (int j = 1; j <= L - 1; j++) {
            ll oldsum = 0;
            for (int s = 0; s <= j - 1; s++) oldsum += dp[s];
            oldsum %= MOD;
            int e = ca[L][j];
            if (e) for (int s = 0; s <= j - 1; s++) dp[s] = dp[s] * P[s][e] % MOD;
            dp[j] = (MOD - oldsum) % MOD * P[j][e] % MOD;
            tot += e;
        }
        ll D = 0;
        for (int s = 0; s <= L - 1; s++) D += dp[s];
        D %= MOD;
        // sign (-1) for including L ; remaining services use the factor (k - v_L)
        ans = (ans - P[L][m - tot] * D) % MOD;
    }
    ans %= MOD; if (ans < 0) ans += MOD;
    printf("%lld\n", ans);
    return 0;
}
