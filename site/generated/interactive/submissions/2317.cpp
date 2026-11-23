#include <bits/stdc++.h>
using namespace std;
typedef unsigned long long u64;

int n;
static u64 H[2002][32];

int main(){
    if(scanf("%d",&n)!=1) return 0;
    vector<int> rk(n+2,0);
    vector<int> F(n+2,0); // F[j] = f(j, k-1)
    rk[1]=0; F[1]=0;
    for(int k=2;k<=n;k++){
        int m=k-1;
        int C=m+1; // candidates 0..m
        int W=(C+63)/64;
        // build H
        for(int w=0;w<W;w++) H[m+1][w]=0;
        for(int j=m;j>=1;j--){
            int t=rk[j]; // set bits 0..t
            int fw=t/64; // full words below fw
            for(int w=0;w<W;w++){
                u64 x=H[j+1][w];
                if(w<fw) x^=~0ULL;
                else if(w==fw){
                    int b=t%64;
                    u64 mk = (b==63)?~0ULL:((1ULL<<(b+1))-1);
                    x^=mk;
                }
                H[j][w]=x;
            }
        }
        vector<u64> S(W,0);
        for(int r=0;r<C;r++) S[r/64]|=1ULL<<(r%64);
        int sz=C;
        while(sz>1){
            int lo=W,hi=-1;
            for(int w=0;w<W;w++) if(S[w]){ if(lo==W) lo=w; hi=w; }
            int best=-1,bs=INT_MAX;
            for(int j=1;j<=m;j++){
                int c=0;
                for(int w=lo;w<=hi;w++) c+=__builtin_popcountll(H[j][w]&S[w]);
                int sc=abs(2*c-sz);
                if(sc<bs){bs=sc;best=j; if(sc<=1) break;}
            }
            printf("? %d %d\n",best,k);
            fflush(stdout);
            int a;
            if(scanf("%d",&a)!=1) return 0;
            if(a<0) return 0;
            int bit=a^F[best];
            int ns=0;
            for(int w=0;w<W;w++){
                if(bit) S[w]&=H[best][w]; else S[w]&=~H[best][w];
                ns+=__builtin_popcountll(S[w]);
            }
            sz=ns;
        }
        int r=0;
        for(int w=0;w<W;w++) if(S[w]){ r=w*64+__builtin_ctzll(S[w]); break; }
        // update F
        int cnt=0;
        for(int j=m;j>=1;j--){
            if(rk[j]>=r) cnt++;
            F[j]^=(cnt&1);
        }
        F[k]=0;
        for(int i=1;i<=m;i++) if(rk[i]>=r) rk[i]++;
        rk[k]=r;
    }
    printf("!");
    for(int i=1;i<=n;i++) printf(" %d",rk[i]+1);
    printf("\n");
    fflush(stdout);
    return 0;
}
