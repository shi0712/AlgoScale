#include <bits/stdc++.h>
using namespace std;
typedef unsigned long long ull;
typedef long long ll;
static const ull MOD=1000000007ULL;
ull pw(ull b,ull e){ull r=1;b%=MOD;while(e){if(e&1)r=r*b%MOD;b=b*b%MOD;e>>=1;}return r;}
struct Mat{ull a[9];};
static inline void mul(const Mat&x,const Mat&y,Mat&z){
    for(int i=0;i<3;i++){
        ull x0=x.a[i*3],x1=x.a[i*3+1],x2=x.a[i*3+2];
        for(int j=0;j<3;j++){
            z.a[i*3+j]=(x0*y.a[j]+x1*y.a[3+j]+x2*y.a[6+j])%MOD;
        }
    }
}
int main(){
    int T; if(scanf("%d",&T)!=1) return 0;
    while(T--){
        int n; scanf("%d",&n);
        vector<ll> l(n),r(n);
        for(auto&v:l)scanf("%lld",&v);
        for(auto&v:r)scanf("%lld",&v);
        ll M=*max_element(r.begin(),r.end());
        ull total=1; for(int i=0;i<n;i++) total=total*((r[i]-l[i]+1)%MOD)%MOD;
        vector<ll> P; P.push_back(1); P.push_back(M+1);
        for(int i=0;i<n;i++){ if(l[i]+1<=M+1)P.push_back(l[i]+1); P.push_back(r[i]+1);}
        sort(P.begin(),P.end()); P.erase(unique(P.begin(),P.end()),P.end());
        int K=(int)P.size()-1; // segments [P[k],P[k+1])
        // regime of i at segment k: 0 big (P<=l),1 mixed,2 small (P>r)
        // events: at segment index where l[i]+1 == P[k] -> mixed ; r[i]+1==P[k] -> small
        vector<vector<pair<int,int>>> ev(K);
        vector<int> reg0(n);
        int D=1;
        {
            vector<int> dcnt(K+1,0);
            for(int i=0;i<n;i++){
                ll x=P[0];
                reg0[i]= x<=l[i]?0:(x<=r[i]?1:2);
                int a=lower_bound(P.begin(),P.end(),l[i]+1)-P.begin();
                int b=lower_bound(P.begin(),P.end(),r[i]+1)-P.begin();
                // mixed for segments [a,b)
                if(a>=1 && a<K && a<b) ev[a].push_back({i,1});
                if(b>=1 && b<K) ev[b].push_back({i,2});
                if(a<b){dcnt[max(a,0)]++; dcnt[min(b,K)]--;}
            }
            int cur=0; for(int k=0;k<K;k++){cur+=dcnt[k]; D=max(D,cur+1);}
        }
        // vals
        vector<ull> vals((size_t)K*D);
        int sz=1; while(sz<n) sz<<=1;
        vector<Mat> tr(2*sz);
        Mat I; for(int q=0;q<9;q++) I.a[q]=(q%4==0);
        vector<ull> c(n), lm(n), rp(n);
        for(int i=0;i<n;i++){c[i]=(r[i]-l[i]+1)%MOD; lm[i]=(ull)(l[i]%MOD); rp[i]=(ull)((r[i]+1)%MOD);}
        vector<int> reg(n);
        for(int t=0;t<D;t++){
            auto setleaf=[&](int i){
                ull s,g;
                if(reg[i]==0){s=0;g=c[i];}
                else if(reg[i]==2){s=c[i];g=0;}
                else {s=(t+MOD-lm[i])%MOD; g=(rp[i]+MOD-t)%MOD;}
                Mat &m=tr[sz+i];
                m.a[0]=0;m.a[1]=s;m.a[2]=0;
                m.a[3]=0;m.a[4]=0;m.a[5]=s;
                m.a[6]=g;m.a[7]=0;m.a[8]=s;
            };
            for(int i=0;i<n;i++){reg[i]=reg0[i]; setleaf(i);}
            for(int i=n;i<sz;i++) tr[sz+i]=I;
            for(int v=sz-1;v>=1;v--) mul(tr[2*v],tr[2*v+1],tr[v]);
            for(int k=0;k<K;k++){
                for(auto [i,rg]:ev[k]){
                    reg[i]=rg; setleaf(i);
                    for(int v=(sz+i)>>1;v>=1;v>>=1) mul(tr[2*v],tr[2*v+1],tr[v]);
                }
                const Mat&R=tr[1];
                vals[(size_t)k*D+t]=(R.a[6]+R.a[7]+R.a[8])%MOD;
            }
        }
        // interpolation: Psi(X)=sum_{x<X} p(x), nodes 0..D
        int N=D+1;
        vector<ull> fact(N+1),ifact(N+1);
        fact[0]=1; for(int i=1;i<=N;i++) fact[i]=fact[i-1]*i%MOD;
        ifact[N]=pw(fact[N],MOD-2); for(int i=N;i>0;i--) ifact[i-1]=ifact[i]*i%MOD;
        vector<ull> psi(N), pre(N+1), suf(N+1);
        auto evalAt=[&](ll X)->ull{
            if(X>=0 && X<N) return psi[X];
            ull xm=(ull)(X%MOD);
            pre[0]=1; for(int j=0;j<N;j++) pre[j+1]=pre[j]*((xm+MOD-j)%MOD)%MOD;
            suf[N]=1; for(int j=N-1;j>=0;j--) suf[j]=suf[j+1]*((xm+MOD-j)%MOD)%MOD;
            ull res=0;
            for(int j=0;j<N;j++){
                ull term=psi[j]*pre[j]%MOD*suf[j+1]%MOD*ifact[j]%MOD*ifact[N-1-j]%MOD;
                if((N-1-j)&1) res=(res+MOD-term)%MOD; else res=(res+term)%MOD;
            }
            return res;
        };
        ull sumbad=0;
        for(int k=0;k<K;k++){
            psi[0]=0;
            for(int j=1;j<N;j++) psi[j]=(psi[j-1]+vals[(size_t)k*D+j-1])%MOD;
            ull v=(evalAt(P[k+1])+MOD-evalAt(P[k]))%MOD;
            sumbad=(sumbad+v)%MOD;
        }
        ull ans=((ull)(M%MOD)*total%MOD+MOD-sumbad)%MOD;
        printf("%llu\n",ans);
    }
}
