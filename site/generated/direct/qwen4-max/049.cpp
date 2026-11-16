#include <bits/stdc++.h>
using namespace std;
using u32 = unsigned int; using u64 = unsigned long long;
static const u32 MOD = 998244353;
static u32 mpow(u32 b, u32 e){ u64 r=1,x=b; while(e){ if(e&1) r=r*x%MOD; x=x*x%MOD; e>>=1;} return (u32)r; }

int main(){
    int T; if(scanf("%d",&T)!=1) return 0;
    while(T--){
        int n; scanf("%d",&n);
        vector<int> pos(n+1);
        bool srt=true;
        for(int i=1;i<=n;i++){ int x; scanf("%d",&x); pos[x]=i; if(x!=i) srt=false; }
        if(srt){ puts("0"); continue; }
        u32 alpha = mpow((u32)(n-1), MOD-2);
        u32 beta  = (u32)((u64)alpha*(u32)n%MOD);
        // triangular table F[A][t], A+t <= n-1
        vector<size_t> off(n+1); size_t tot=0;
        for(int A=0;A<n;A++){ off[A]=tot; tot += (size_t)(n-A); }
        off[n]=tot;
        vector<u32> F(tot,0), colSum(n,0);
        for(int A=0;A<n;A++){
            u32 rowSum=0; u32* Fa=F.data()+off[A];
            for(int B=0;B<=n-2-A;B++){
                u32 f;
                if(A==0 && B==0) f=1;
                else{
                    u32 s=colSum[B]+rowSum; if(s>=MOD) s-=MOD;
                    f=(u32)((u64)alpha*s%MOD);
                }
                rowSum+=f; if(rowSum>=MOD) rowSum-=MOD;
                colSum[B]+=f; if(colSum[B]>=MOD) colSum[B]-=MOD;
                Fa[B+1]=rowSum;
            }
        }
        vector<int> d(n+2);
        u32 sumW=0;
        for(int i=1;i<=n;i++){
            int zeros=0,cnt=0;
            for(int v=1;v<=n;v++){
                if(pos[v]>i) ++zeros;
                else if(zeros) d[++cnt]=zeros;
            }
            if(!cnt) continue;              // this split already sorts everything
            u32 w=0;
            for(int K=1;K<=cnt;K++){
                w += F[off[cnt-K]+(size_t)d[K]];
                if(w>=MOD) w-=MOD;
            }
            sumW += w; if(sumW>=MOD) sumW-=MOD;
        }
        u64 ans = (u64)beta*mpow((u32)n,MOD-2)%MOD;
        ans = ans*sumW%MOD;
        ans = (ans+1)%MOD;
        printf("%u\n",(unsigned)ans);
    }
    return 0;
}
