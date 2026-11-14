#include <bits/stdc++.h>
using namespace std;
const long long MOD=998244353;
long long pw(long long b,long long e){long long r=1;b%=MOD;while(e){if(e&1)r=r*b%MOD;b=b*b%MOD;e>>=1;}return r;}
static unsigned F[2002][2002],H[2002][2002];
int main(){
    int T;scanf("%d",&T);
    while(T--){
        int n;scanf("%d",&n);vector<int>p(n);for(auto&x:p)scanf("%d",&x);
        bool sorted=true;for(int i=0;i<n;i++)if(p[i]!=i+1)sorted=false;
        if(sorted){puts("0");continue;}
        long long inv=pw(n-1,MOD-2),nn=n,n1=n+1;
        for(int A=0;A<=n;A++)for(int B=0;B<=n;B++){
            long long v=0;
            if(A<=1&&B<=1) v=((A+B)&1)?MOD-nn:nn;
            if(A) v+=nn*F[A-1][B];
            if(B) v+=nn*F[A][B-1];
            if(A&&B) v+=(MOD-n1)*F[A-1][B-1]%MOD;
            F[A][B]=v%MOD*inv%MOD;
        }
        for(int A=0;A<=n;A++){H[A][0]=0;for(int c=0;c<=n;c++)H[A][c+1]=(H[A][c]+F[A][c])%MOD;}
        vector<char> in(n+1,0);
        long long tot=0;
        vector<int> cs;
        for(int j=1;j<n;j++){
            in[p[j-1]]=1;
            cs.clear();int cnt=0;
            for(int v=n;v>=1;v--){ if(in[v])cnt++; else cs.push_back(cnt);}
            int m=cs.size();
            for(int k=0;k<m;k++){int A=m-1-k; if(cs[k]) tot+=H[A][cs[k]];}
            tot%=MOD;
        }
        long long ans=(1+tot%MOD*pw(n,MOD-2))%MOD;
        printf("%lld\n",ans);
    }
}
