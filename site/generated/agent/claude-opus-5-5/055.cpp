#include <bits/stdc++.h>
using namespace std;
int main(){
    int T;
    if(scanf("%d",&T)!=1) return 0;
    static char buf[200005];
    while(T--){
        int K; if(scanf("%s %d",buf,&K)!=2) return 0;
        int n=strlen(buf); const char* s=buf;
        // precompute windows starting at p: (len, letter)
        vector<array<pair<int,char>,6>> win(n); vector<int> wc(n,0);
        for(int p=0;p<n;p++){
            for(int L=3;L<=5;L++){int r=p+L-1;if(r>=n)break;
                for(int i=p;i<=r;i++){ if(i-p>2||r-i>2)continue;
                    char c=s[i==p?p+1:p]; if(s[i]==c) continue;
                    bool ok=true; for(int j=p;j<=r&&ok;j++) if(j!=i&&s[j]!=c) ok=false;
                    if(ok) win[p][wc[p]++]={L,c};
                }
            }
        }
        const long long NEG=LLONG_MIN/4;
        long long best=LLONG_MAX;
        vector<array<long long,3>> dp(n+1);
        for(int lam=0;lam<=5;lam++){
            for(int p=0;p<=n;p++) dp[p]={NEG,NEG,NEG};
            dp[0][0]=0;
            for(int p=0;p<n;p++){
                for(int st=0;st<3;st++){
                    long long v=dp[p][st]; if(v==NEG) continue;
                    if(st!=2) dp[p+1][0]=max(dp[p+1][0],v);
                    for(int w=0;w<wc[p];w++){
                        int L=win[p][w].first; char c=win[p][w].second; int r=p+L-1;
                        if(st==0&&p>0&&s[p-1]==c) continue;
                        if(st==2&&s[p-1]==c) continue;
                        int ns=(r+1==n)?1:(s[r+1]==c?2:1);
                        dp[r+1][ns]=max(dp[r+1][ns],v+L-lam);
                    }
                }
            }
            long long g=max(dp[n][0],dp[n][1]);
            best=min(best,g+(long long)lam*K);
        }
        printf("%lld\n",best);
    }
}
