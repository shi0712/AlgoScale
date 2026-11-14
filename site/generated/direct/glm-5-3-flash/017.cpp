#include <bits/stdc++.h>
using namespace std;
typedef long long ll;

int main(){
    int n, m; 
    scanf("%d %d", &n, &m);
    vector<int> trump(n);
    for (auto &x : trump) scanf("%d", &x);
    vector<vector<ll>> Att(n, vector<ll>(m)), Def(n, vector<ll>(m));
    for (int s = 0; s < n; s++) for (int r = 0; r < m; r++) scanf("%lld", &Att[s][r]);
    for (int s = 0; s < n; s++) for (int r = 0; r < m; r++) scanf("%lld", &Def[s][r]);
    
    int M = 1 << m;
    // rank r (1-based) ↔ bit r-1
    // TrP
    vector<ll> TrW(m, 0);
    for (int s = 0; s < n; s++) if (trump[s]) for (int r = 0; r < m; r++) TrW[r] += Def[s][r];
    vector<ll> TrP(M, 0);
    TrP[0] = 0;
    for (int P = 1; P < M; P++) TrP[P] = TrP[P & (P-1)] + TrW[__builtin_ctz(P)];
    
    vector<ll> G(M), TA(M), HNlhs(M, 0), MaxT(M, 0);
    const ll NEG = LLONG_MIN / 4;
    G[0] = NEG; TA[0] = 0;
    
    for (int s = 0; s < n; s++) {
        // check if suit has attacker cards
        ll tot = 0; for (int r = 0; r < m; r++) tot += Att[s][r];
        if (tot == 0) continue;
        // z[r]
        // DP: for b in 0..m-1: for Q in 0..(1<<b)-1: P = (1<<b)|Q:
        //   G[P] = max(Att[s][b], z[b] + G[Q]); TA[P] = TA[Q] + Att[s][b];
        for (int b = 0; b < m; b++) {
            ll ab = Att[s][b];
            ll zb = Att[s][b] - Def[s][b];
            int base = 1 << b;
            if (trump[s]) {
                for (int Q = 0; Q < base; Q++) {
                    int P = base | Q;
                    ll g = zb + G[Q]; if (ab > g) g = ab;
                    G[P] = g;
                    ll ta = TA[Q] + ab; TA[P] = ta;
                    HNlhs[P] += ta;
                    if (g > MaxT[P]) MaxT[P] = g;
                }
            } else {
                for (int Q = 0; Q < base; Q++) {
                    int P = base | Q;
                    ll g = zb + G[Q]; if (ab > g) g = ab;
                    G[P] = g;
                    HNlhs[P] += g;
                }
            }
        }
    }
    
    // U: union of good P
    vector<bool> inU(m, false);
    for (int P = 1; P < M; P++) {
        if (MaxT[P] == 0 && HNlhs[P] <= TrP[P]) {
            // mark ranks
            int q = P;
            while (q) { inU[__builtin_ctz(q)] = true; q &= q-1; }
        }
    }
    
    ll ans = 0;
    for (int s = 0; s < n; s++) for (int r = 0; r < m; r++)
        if (!inU[r]) ans += Att[s][r];
    printf("%lld\n", ans);
}
