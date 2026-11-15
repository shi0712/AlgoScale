#include <bits/stdc++.h>
using namespace std;
typedef long long ll;
const ll NEG = -1000000000000000LL;
const ll BIG = 100000000000LL;
int n, K;
struct It { int w[10]; };
vector<It> grp[11];
vector<ll> prf[11];
ll ans = 0;

static vector<ll> arr_, res_;
static int m_;
static const vector<ll>* P_;
inline ll Pf(int c) {
    const vector<ll>& p = *P_;
    if (c <= m_) return p[c];
    return p[m_] - BIG * (ll)(c - m_);
}
void rec(int lo, int hi, int ol, int oh) {
    if (lo > hi) return;
    int mid = (lo + hi) / 2;
    ll best = LLONG_MIN; int bo = ol;
    int e = min(mid, oh);
    for (int y = ol; y <= e; y++) {
        ll v = arr_[y] + Pf(mid - y);
        if (v > best) { best = v; bo = y; }
    }
    res_[mid] = best;
    rec(lo, mid - 1, ol, bo);
    rec(mid + 1, hi, bo, oh);
}
void addGroup(vector<ll>& dp, int q) {
    int m = grp[q].size();
    if (m == 0) return;
    m_ = m; P_ = &prf[q];
    vector<ll> nd(K + 1, NEG);
    for (int a = 0; a < q && a <= K; a++) {
        int L = (K - a) / q + 1;
        arr_.assign(L, 0);
        res_.assign(L, 0);
        for (int x = 0; x < L; x++) arr_[x] = dp[a + (ll)x * q];
        rec(0, L - 1, 0, L - 1);
        for (int x = 0; x < L; x++) {
            ll v = res_[x];
            if (v < -100000000000000LL) v = NEG;
            nd[a + (ll)x * q] = v;
        }
    }
    dp.swap(nd);
}
void leaf(int q, const vector<ll>& dpEx) {
    // straddler from group q
    int m = grp[q].size();
    if (m == 0) return;
    const vector<ll>& pf = prf[q];
    vector<ll> suf(m + 1), pmB(m);
    for (int r = 1; r < q; r++) {
        suf[m] = LLONG_MIN / 4;
        for (int i = m - 1; i >= 0; i--) suf[i] = max(suf[i + 1], (ll)grp[q][i].w[r - 1]);
        ll cur = LLONG_MIN / 4;
        for (int i = 0; i < m; i++) {
            cur = max(cur, (ll)grp[q][i].w[r - 1] - grp[q][i].w[q - 1]);
            pmB[i] = cur;
        }
        for (int c = 0; c <= m - 1; c++) {
            ll t = (ll)K - r - (ll)c * q;
            if (t < 0) break;
            if (dpEx[t] < -100000000000000LL) continue;
            ll F = max(pf[c] + suf[c], pf[c + 1] + pmB[c]);
            ans = max(ans, dpEx[t] + F);
        }
    }
}
bool doneAll = false;
void solve(int l, int r, vector<ll> dp) {
    if (l == r) {
        leaf(l, dp);
        if (!doneAll) {
            doneAll = true;
            vector<ll> d2 = dp;
            addGroup(d2, l);
            for (int t = 0; t <= K; t++) if (d2[t] > -100000000000000LL) ans = max(ans, d2[t]);
        }
        return;
    }
    int mid = (l + r) / 2;
    {
        vector<ll> d = dp;
        for (int g = mid + 1; g <= r; g++) addGroup(d, g);
        solve(l, mid, d);
    }
    {
        vector<ll> d = dp;
        for (int g = l; g <= mid; g++) addGroup(d, g);
        solve(mid + 1, r, d);
    }
}
int main() {
    scanf("%d %d", &n, &K);
    for (int i = 0; i < n; i++) {
        int p; scanf("%d", &p);
        It it; memset(it.w, 0, sizeof(it.w));
        for (int j = 0; j < p; j++) scanf("%d", &it.w[j]);
        grp[p].push_back(it);
    }
    for (int q = 1; q <= 10; q++) {
        sort(grp[q].begin(), grp[q].end(), [q](const It& a, const It& b) { return a.w[q - 1] > b.w[q - 1]; });
        int m = grp[q].size();
        prf[q].assign(m + 1, 0);
        for (int i = 0; i < m; i++) prf[q][i + 1] = prf[q][i] + grp[q][i].w[q - 1];
    }
    vector<ll> dp(K + 1, NEG);
    dp[0] = 0;
    solve(1, 10, dp);
    printf("%lld\n", ans);
    return 0;
}
