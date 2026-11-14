#pragma GCC optimize("O3,unroll-loops")
#include <bits/stdc++.h>
using namespace std;
typedef long long ll;
static const ll INF = LLONG_MAX / 4;
int main(){
    int n, c;
    if(scanf("%d %d",&n,&c)!=2) return 0;
    vector<ll> t(n);
    for(auto &x:t) if(scanf("%lld",&x)!=1) return 0;
    sort(t.begin(), t.end());
    if(n<=c){ printf("%lld\n", t[n-1]); return 0; }
    vector<ll> P(n+1,0);
    for(int i=0;i<n;i++) P[i+1]=P[i]+t[i];
    int Dcap = n/c + 3;
    // FG[m][D]: min cost of fast groups {1..j}, 2<=j<=m, paying sum(j-1)=D
    vector<vector<ll>> FG(c+1, vector<ll>(Dcap+1, INF));
    FG[1][0]=0;
    for(int m=2;m<=c;m++){
        FG[m]=FG[m-1];
        ll w=t[m-1]+P[m]; int s=m-1;
        for(int D=s; D<=Dcap; D++) if(FG[m][D-s]<INF) FG[m][D]=min(FG[m][D], FG[m][D-s]+w);
    }
    // a(pos) = t[n-pos], pos 1-indexed from the top
    vector<ll> dp(Dcap+2, INF), g(Dcap+2, INF), nx(Dcap+2, INF);
    dp[0]=0;
    ll ans=INF; int prevDn=0;
    for(long long k=0; ; k++){
        long long base = k*(ll)(c-1);
        if(base>=n) break;
        // valid D: D<=k, q=base+D, m=n-q>=1
        int Dmax = (int)min<long long>({k, (long long)n-1-base, (long long)Dcap});
        bool any=false;
        for(int D=0; D<=Dmax; D++){
            if(dp[D]>=INF) continue;
            any=true;
            long long m = n-base-D;
            if(m<=c){ ll f=FG[m][D]; if(f<INF) ans=min(ans, dp[D]+t[m-1]+f); }
        }
        if(!any) break;
        // transitions: need m>=c (y>=1) or m>=c+1 (y==0) => D <= n-base-c-[y==0]
        long long lim = (long long)n-base-c;
        int Dh = (int)min<long long>(Dmax, lim);
        if(Dh<0) break;
        for(int D=0; D<=Dh; D++) g[D]=dp[D]+t[n-1-(base+D)];
        int Dn = min(Dh+1, Dcap);
        for(int D=0; D<=Dn; D++) nx[D]=INF;
        int ymax=min(c-1, Dh+1);
        for(int y=0;y<=ymax;y++){
            int lo=max(0,y-1);
            int hi = (y==0)? (int)min<long long>(Dh, lim-1) : Dh;
            ll py=P[y];
            int off = 1-y;
            for(int D=lo; D<=hi; D++){
                nx[D+off]=min(nx[D+off], g[D]+py);
            }
        }
        for(int D=0; D<=Dn; D++) dp[D]=nx[D];
        for(int D=Dn+1; D<=prevDn; D++) dp[D]=INF;
        prevDn=Dn;
    }
    printf("%lld\n", ans);
}
