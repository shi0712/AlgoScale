#include <bits/stdc++.h>
using namespace std;
static constexpr int MOD = 998244353;
int power(int a, int b) { int r=1; for (;b;b>>=1,a=(long long)a*a%MOD) if(b&1) r=(long long)r*a%MOD; return r; }
int main(){
    ios::sync_with_stdio(false); cin.tie(nullptr);
    int T; cin>>T;
    while(T--){
        int n; cin>>n;
        vector<int> pos(n+1);
        bool sorted=true;
        for(int i=1,v;i<=n;i++){cin>>v; pos[v]=i; if(v!=i) sorted=false;}
        if(sorted){cout<<0<<'\n'; continue;}
        int inv=power(n-1,MOD-2), stride=n+1;
        vector<int> dp((n+1)*stride), col(n+1);
        for(int x=1;x<n;x++){
            int row=0;
            for(int y=1;y<=n-x;y++){
                int s=1+row; if(s>=MOD) s-=MOD;
                s+=col[y]; if(s>=MOD) s-=MOD;
                int v=(long long)s*inv%MOD;
                dp[x*stride+y]=v;
                row+=v; if(row>=MOD) row-=MOD;
                col[y]+=v; if(col[y]>=MOD) col[y]-=MOD;
            }
        }
        int ans=1;
        for(int cut=1;cut<n;cut++){
            int x=cut,y=0,prev=0;
            for(int value=1;value<n;value++){
                if(pos[value]<=cut) --x; else ++y;
                if(pos[value]>cut && pos[value+1]<=cut){
                    ans+=dp[x*stride+y]; if(ans>=MOD) ans-=MOD;
                    if(prev){ans-=dp[x*stride+prev]; if(ans<0) ans+=MOD;}
                    prev=y;
                }
            }
        }
        cout<<ans<<'\n';
    }
}
