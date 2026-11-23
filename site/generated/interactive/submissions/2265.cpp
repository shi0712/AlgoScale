#include <bits/stdc++.h>
using namespace std;

static int n, t;
static vector<int> sm, fl;
static vector<char> kn;

static int ask(int l, int r){
    printf("? %d %d\n", l, r);
    fflush(stdout);
    int x;
    if(scanf("%d", &x) != 1) exit(0);
    if(x < 0) exit(0);
    return x;
}

int main(){
    if(scanf("%d %d", &n, &t) != 2) return 0;
    int sz = 4*(n+2);
    sm.assign(sz, 0);
    fl.assign(sz, 0);
    kn.assign(sz, 0);

    if(n > 1){
        sm[1] = ask(1, n);
        kn[1] = 1;
    } else {
        // single element must be zero (at least k>=1 zeros exist)
        sm[1] = 0; kn[1] = 1;
    }

    for(int round = 0; round < t; round++){
        int k;
        if(scanf("%d", &k) != 1) return 0;
        if(k < 0) return 0;

        int node = 1, l = 1, r = n;
        while(l < r){
            int mid = (l + r) >> 1;
            int lc = node << 1, rc = lc | 1;
            if(!kn[lc] || !kn[rc]){
                if(kn[node] && sm[node] == fl[node]){
                    // all ones inside are exactly the flipped positions:
                    // every other position is zero -> whole subtree known
                    sm[lc] = fl[lc]; kn[lc] = 1;
                    sm[rc] = fl[rc]; kn[rc] = 1;
                } else {
                    if(!kn[lc]){
                        sm[lc] = ask(l, mid);
                        kn[lc] = 1;
                    }
                    if(!kn[rc]){
                        if(kn[node]) sm[rc] = sm[node] - sm[lc];
                        else sm[rc] = ask(mid+1, r);
                        kn[rc] = 1;
                    }
                }
            }
            int zl = (mid - l + 1) - sm[lc];
            if(k <= zl){
                node = lc; r = mid;
            } else {
                k -= zl; node = rc; l = mid + 1;
            }
        }

        printf("! %d\n", l);
        fflush(stdout);

        // flip position l : update flip counts and known sums on the root path
        int nd = 1, a = 1, b = n;
        while(true){
            fl[nd]++;
            if(kn[nd]) sm[nd]++;
            if(a == b) break;
            int mid = (a + b) >> 1;
            if(l <= mid){ nd <<= 1; b = mid; }
            else { nd = (nd << 1) | 1; a = mid + 1; }
        }
    }
    return 0;
}
