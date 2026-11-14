#include <bits/stdc++.h>
using namespace std;
typedef unsigned long long ull;
typedef long long ll;
static const ull MOD = 1000000007ULL;

static ull pw(ull b, ull e) { ull r = 1; b %= MOD; while (e) { if (e & 1) r = r * b % MOD; b = b * b % MOD; e >>= 1; } return r; }

int n;
vector<vector<int>> adj;
vector<int> par, sz;
vector<ull> fact, inv, ifact;
vector<vector<ull>> lam, lamt; // lam[m][y], lamt[m][y]
vector<ull> phi1, phi2;        // Phi[C(Y,k)]
vector<vector<ull>> Cb;

inline ull C(int a, int b) { if (b < 0 || b > a) return 0; return fact[a] * ifact[b] % MOD * ifact[a - b] % MOD; }

// res (size limited to lim+1) += a*b
static void mulAdd(const vector<ull>& a, const vector<ull>& b, vector<ull>& res, int lim) {
    int na = a.size(), nb = b.size();
    int nr = min(lim + 1, na + nb - 1);
    if (na == 0 || nb == 0) return;
    if ((int)res.size() < nr) res.resize(nr, 0);
    // accumulate per output index with lazy reduction
    static vector<unsigned __int128> dummy;
    for (int k = 0; k < nr; k++) {
        int lo = max(0, k - nb + 1), hi = min(na - 1, k);
        if (lo > hi) continue;
        ull acc = 0; int cnt = 0;
        ull accHi = 0;
        for (int i = lo; i <= hi; i++) {
            acc += a[i] * b[k - i];
            if (++cnt == 16) { accHi = (accHi + acc % MOD); acc = 0; cnt = 0; }
        }
        res[k] = (res[k] + accHi + acc % MOD) % MOD;
    }
}

int main() {
    scanf("%d", &n);
    adj.assign(n + 1, {});
    for (int i = 0; i < n - 1; i++) { int x, y; scanf("%d %d", &x, &y); adj[x].push_back(y); adj[y].push_back(x); }
    int N = n + 5;
    fact.assign(2 * N + 1, 1); ifact.assign(2 * N + 1, 1); inv.assign(2 * N + 1, 1);
    for (int i = 1; i <= 2 * N; i++) fact[i] = fact[i - 1] * i % MOD;
    ifact[2 * N] = pw(fact[2 * N], MOD - 2);
    for (int i = 2 * N; i > 0; i--) ifact[i - 1] = ifact[i] * i % MOD;
    // lambda tables
    int Y = n + 2;
    lam.assign(n + 1, vector<ull>(Y + 1, 0));
    lamt.assign(n + 1, vector<ull>(Y + 1, 0));
    for (int y = 0; y <= Y; y++) {
        lam[0][y] = 1; lamt[0][y] = 0;
        if (n >= 1) { lam[1][y] = y % MOD; lamt[1][y] = y % MOD; }
        for (int m = 1; m + 1 <= n; m++) {
            ull a = (y + MOD * 4 - 2ULL * m) % MOD; // y-2m
            ull b = (ull)m * (m - 1) % MOD;
            lam[m + 1][y] = (a * lam[m][y] + (MOD - b) * lam[m - 1][y]) % MOD;
            lamt[m + 1][y] = ((ull)y * lam[m][y] % MOD + a * lamt[m][y] % MOD + (MOD - b) * lamt[m - 1][y] % MOD) % MOD;
        }
    }
    // Stirling first kind signed s(k,V), phi1[k] = sum_V s(k,V) V! / k!, phi2 with (V-1)!
    {
        int K = n + 2;
        vector<vector<ull>> s(K + 1, vector<ull>(K + 1, 0));
        s[0][0] = 1;
        for (int k = 0; k < K; k++)
            for (int V = 0; V <= k; V++) if (s[k][V]) {
                // x^{\underline{k+1}} = x^{\underline k} (x - k)
                s[k + 1][V + 1] = (s[k + 1][V + 1] + s[k][V]) % MOD;
                s[k + 1][V] = (s[k + 1][V] + (MOD - (ull)k % MOD) * s[k][V]) % MOD;
            }
        phi1.assign(K + 1, 0); phi2.assign(K + 1, 0);
        for (int k = 0; k <= K; k++) {
            ull a1 = 0, a2 = 0;
            for (int V = 0; V <= k; V++) {
                a1 = (a1 + s[k][V] * fact[V]) % MOD;
                if (V >= 1) a2 = (a2 + s[k][V] * fact[V - 1]) % MOD;
            }
            phi1[k] = a1 * ifact[k] % MOD; phi2[k] = a2 * ifact[k] % MOD;
        }
    }
    // tree order
    par.assign(n + 1, 0); sz.assign(n + 1, 1);
    vector<int> order; order.reserve(n);
    {
        vector<int> st = {1}; par[1] = 0;
        vector<char> vis(n + 1, 0); vis[1] = 1;
        while (!st.empty()) { int v = st.back(); st.pop_back(); order.push_back(v); for (int w : adj[v]) if (!vis[w]) { vis[w] = 1; par[w] = v; st.push_back(w); } }
    }
    for (int i = n - 1; i > 0; i--) sz[par[order[i]]] += sz[order[i]];
    vector<vector<int>> ch(n + 1);
    for (int i = 1; i < n; i++) ch[par[order[i]]].push_back(order[i]);
    vector<vector<ull>> T(n + 1);
    for (int idx = n - 1; idx >= 0; idx--) {
        int u = order[idx];
        int su = sz[u];
        int Xm = min(su - 1, n - su);
        auto& cs = ch[u];
        sort(cs.begin(), cs.end(), [&](int a, int b) { return sz[a] > sz[b]; });
        int D = 1;
        for (int c : cs) D += min(sz[c], su - sz[c] + 1);
        // weights
        vector<ull> w1(D + 1, 0), w2(D + 1, 0);
        for (int i = 0; i <= D; i++) {
            ull a1 = 0, a2 = 0;
            for (int k = i; k <= D; k++) {
                ull cc = C(k, i);
                if ((k - i) & 1) cc = (MOD - cc) % MOD;
                a1 = (a1 + cc * phi1[k]) % MOD;
                a2 = (a2 + cc * phi2[k]) % MOD;
            }
            w1[i] = a1; w2[i] = a2;
        }
        // per child precompute
        int k = cs.size();
        vector<int> J(k), flo(k), fhi(k);
        vector<vector<vector<ull>>> Dj(k);
        for (int t = 0; t < k; t++) {
            int c = cs[t]; int sc = sz[c];
            vector<ull>& Tc = T[c];
            int degT = (int)Tc.size() - 1;
            J[t] = min(degT, su - sc + 1);
            Dj[t].resize(J[t] + 1);
            for (int j = 0; j <= J[t]; j++) {
                int len = min(degT - j, Xm) + 1;
                if (len < 0) len = 0;
                Dj[t][j].assign(len, 0);
                for (int x = 0; x < len; x++) Dj[t][j][x] = Tc[x + j] * C(x + j, j) % MOD;
            }
            flo[t] = max(0, sc - (su - sc + 1));
            fhi[t] = min(sc, Xm);
        }
        vector<ull> res(Xm + 1, 0);
        vector<ull> P, Q, nP, nQ, fc, gc;
        for (int y = 0; y <= D; y++) {
            P.assign(1, 1); Q.clear();
            for (int t = 0; t < k; t++) {
                int c = cs[t]; int sc = sz[c];
                fc.assign(max(0, fhi[t] + 1), 0);
                for (int x = flo[t]; x <= fhi[t]; x++) fc[x] = C(sc, x) * lam[sc - x][y] % MOD;
                int glen = 0;
                for (int j = 0; j <= J[t]; j++) glen = max(glen, (int)Dj[t][j].size());
                gc.assign(glen, 0);
                for (int j = 0; j <= J[t]; j++) {
                    ull coef = (w1[y] * lam[j][y] + (MOD - w2[y]) * lamt[j][y]) % MOD;
                    if (!coef) continue;
                    auto& dv = Dj[t][j];
                    for (size_t x = 0; x < dv.size(); x++) gc[x] = (gc[x] + coef * dv[x]) % MOD;
                }
                nQ.clear(); nP.clear();
                mulAdd(Q, fc, nQ, Xm);
                mulAdd(P, gc, nQ, Xm);
                mulAdd(P, fc, nP, Xm);
                swap(P, nP); swap(Q, nQ);
            }
            // res += (X + y) Q + w2[y]*y*P
            for (size_t x = 0; x < Q.size(); x++) {
                res[x] = (res[x] + Q[x] * y) % MOD;
                if ((int)x + 1 <= Xm) res[x + 1] = (res[x + 1] + Q[x]) % MOD;
            }
            ull cf = w2[y] * y % MOD;
            for (size_t x = 0; x < P.size() && (int)x <= Xm; x++) res[x] = (res[x] + cf * P[x]) % MOD;
        }
        while (res.size() > 1 && res.back() == 0) res.pop_back();
        T[u] = res;
        for (int c : cs) { vector<ull>().swap(T[c]); }
    }
    printf("%llu\n", T[1][0] % MOD);
}
