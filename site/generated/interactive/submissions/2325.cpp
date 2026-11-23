#include <bits/stdc++.h>
using namespace std;
typedef long long ll;
typedef unsigned long long ull;

const int QM = 200, CM = 200;
const ll INF = (ll)4e18;
ll G[QM+1][CM+1];

ll sat(ll a, ll b){ if (a >= INF - b) return INF; return a + b; }

int hfun(ll w, ll c){
    if (c > CM) c = CM;
    if (c < 0) c = 0;
    for (int q = 0; q <= QM; q++) if (G[q][c] >= w) return q;
    return QM + 1;
}
ll credit(ll lo, ll hi, ll B){
    if (hi <= 0) return CM;
    ll c = B / hi;
    if (c > CM) c = CM;
    return c;
}
int est(ll lo, ll hi, ll B){
    if (lo >= hi) return 0;
    return hfun(hi - lo + 1, credit(lo, hi, B));
}

bool ask(ll x){
    printf("? %lld\n", x);
    fflush(stdout);
    string s;
    if (!(cin >> s)) exit(0);
    if (s == "Lucky!") return true;
    if (s == "Fraudster!") return false;
    exit(0);
}

int main(){
    for (int c = 0; c <= CM; c++) G[0][c] = 1;
    for (int q = 1; q <= QM; q++){
        for (int c = 0; c <= CM; c++){
            ll v = G[q-1][min(c+1, CM)];
            if (c >= 1){
                ll cand = sat(G[q-1][min(c+1, CM)], G[q-1][c-1]);
                v = max(v, cand);
            }
            G[q][c] = v;
        }
    }
    ll lo = 0, hi = 100000000000000LL, B = 1;
    while (2 * B <= hi){
        ll x = B;
        if (ask(x)){ lo = x; B += x; }
        else { hi = x - 1; B -= x; break; }
    }
    while (lo < hi){
        ll ha = min(hi, B);
        int best = INT_MAX; ll bx = -1;
        if (ha >= lo + 1){
            auto hf = [&](ll x){ return est(lo, x - 1, B - x); };
            auto hs = [&](ll x){ return est(x, hi, B + x); };
            ll L = lo + 1, R = ha, pos = ha;
            while (L <= R){
                ll m = L + (R - L) / 2;
                if (hf(m) >= hs(m)){ pos = m; R = m - 1; }
                else L = m + 1;
            }
            for (ll x : {pos, pos - 1, pos + 1}){
                if (x < lo + 1 || x > ha) continue;
                int v = max(hf(x), hs(x));
                if (v < best){ best = v; bx = x; }
            }
        }
        int rc = est(lo, hi, B + lo);
        if (bx == -1 || rc < best){
            // recharge
            if (ask(lo)) B += lo;
            else { return 0; } // impossible
        } else {
            if (ask(bx)){ lo = bx; B += bx; }
            else { hi = bx - 1; B -= bx; }
        }
    }
    printf("! %lld\n", lo);
    fflush(stdout);
    return 0;
}
