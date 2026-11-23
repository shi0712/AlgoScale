#include <bits/stdc++.h>
using namespace std;

int n, Utot;

int ask(int l, int r, int k){
    if(l == r) return 1;                 // single element -> always 1 distinct
    printf("? %d %d %d\n", l, r, k);
    fflush(stdout);
    int x;
    if(scanf("%d", &x) != 1) exit(0);
    if(x == -1) exit(0);
    return x;
}

int main(){
    if(scanf("%d", &n) != 1) return 0;
    bool even = (n % 2 == 0);
    Utot = even ? 2 : 1;
    int lo = 1, hi = n, zlo = 1, zhi = n;
    int prevCut = -1, prevU = -1;
    long long prevS = -1;
    while(lo < hi){
        int t = (lo + hi) / 2;
        int bit = 0, zt = 0;
        bool handled = false;
        if(even && hi == lo + 1 && zlo == lo && zhi == hi){
            int a = lo;                  // {y,z} = {a,a+1}
            if(a >= 2){
                int v = ask(a-1, a, n);
                if(v == 2){ zt = 0; bit = 1; } else { zt = 1; bit = 0; }
            } else {
                int v = ask(a+1, a+2, n);
                if(v == 2){ zt = 1; bit = 0; } else { zt = 0; bit = 1; }
            }
            handled = true;
        }
        if(!handled){
            int QA = ask(1, t, 2);
            long long FA = 2LL*QA - t;
            int cnt = 0, Uv = -1;
            for(int U = 0; U <= Utot; U++){
                if(even){
                    if(zhi <= t && U == 0) continue;
                    if(zlo > t && U == Utot) continue;
                }
                long long s = FA - U;
                if(s < 0) continue;
                long long cap = min((long long)(t - U), (long long)(n - t - (Utot - U)));
                if(s > cap) continue;
                if(prevCut == t - 1){
                    if(U == prevU){ if(s == prevS) continue; }
                    else if(U == prevU + 1){ if(s != prevS) continue; }
                    else continue;
                } else if(prevCut == t + 1){
                    if(U == prevU){ if(s == prevS) continue; }
                    else if(U == prevU - 1){ if(s != prevS) continue; }
                    else continue;
                }
                cnt++; Uv = U;
            }
            int U;
            if(cnt == 1) U = Uv;
            else {
                int QB = ask(t+1, n, 2);
                long long GB = 2LL*QB - (n - t);
                U = (int)((FA - GB + Utot) / 2);
            }
            long long st = FA - U;
            int S = Utot - U;
            if(!even){ bit = S; zt = 0; }
            else if(S == 0){ bit = 0; zt = 0; }
            else if(S == 2){ bit = 1; zt = 1; }
            else {
                if(zhi <= t) zt = 0;
                else if(zlo > t) zt = 1;
                else {
                    if(t >= 2){ int v = ask(1, t, n); zt = (v == 2) ? 0 : 1; }
                    else { int v = ask(t+1, n, n); zt = (v == 2) ? 1 : 0; }
                }
                bit = S - zt;
            }
            prevCut = t; prevU = U; prevS = st;
        }
        if(bit) lo = t + 1; else hi = t;
        if(even){
            if(zt) zlo = max(zlo, t + 1);
            else zhi = min(zhi, t);
        }
    }
    printf("! %d\n", lo);
    fflush(stdout);
    return 0;
}
