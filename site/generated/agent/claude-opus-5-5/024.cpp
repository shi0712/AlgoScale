#include <bits/stdc++.h>
using namespace std;
typedef long long ll; typedef unsigned long long ull;
const int MOD=998244353,G=3;
ll pw(ll b,ll e){ll r=1;b%=MOD;if(b<0)b+=MOD;while(e){if(e&1)r=r*b%MOD;b=b*b%MOD;e>>=1;}return r;}
vector<int> rev; vector<ll> rts{0,1};
void ntt(vector<ll>&a,bool inv){
    int n=a.size(); if((int)rev.size()!=n){rev.assign(n,0);int k=__builtin_ctz(n);for(int i=0;i<n;i++)rev[i]=(rev[i>>1]>>1)|((i&1)<<(k-1));}
    if((int)rts.size()<n){int k=__builtin_ctz(rts.size());rts.resize(n);
        while((1<<k)<n){ll z=pw(G,(MOD-1)>>(k+1));for(int i=1<<(k-1);i<(1<<k);i++){rts[2*i]=rts[i];rts[2*i+1]=rts[i]*z%MOD;}k++;}}
    for(int i=0;i<n;i++)if(i<rev[i])swap(a[i],a[rev[i]]);
    for(int k=1;k<n;k*=2)for(int i=0;i<n;i+=2*k)for(int j=0;j<k;j++){
        ll z=rts[j+k]*a[i+j+k]%MOD; a[i+j+k]=a[i+j]-z+(a[i+j]<z?MOD:0); a[i+j]=a[i+j]+z-(a[i+j]+z>=MOD?MOD:0);}
    if(inv){reverse(a.begin()+1,a.end());ll ni=pw(n,MOD-2);for(auto&x:a)x=x*ni%MOD;}
}
vector<ll> mul(vector<ll>a,vector<ll>b,int lim){
    if(a.empty()||b.empty())return {};
    int s=a.size()+b.size()-1,n=1;while(n<s)n<<=1;
    a.resize(n);b.resize(n);ntt(a,0);ntt(b,0);for(int i=0;i<n;i++)a[i]=a[i]*b[i]%MOD;ntt(a,1);
    a.resize(min(s,lim));return a;
}
vector<ll> inv(const vector<ll>&a,int n){
    vector<ll> b{pw(a[0],MOD-2)};
    for(int m=1;m<n;m*=2){
        int m2=2*m; vector<ll> f(a.begin(),a.begin()+min((int)a.size(),m2));
        vector<ll> t=mul(f,b,m2); for(auto&x:t)x=(MOD-x)%MOD; t[0]=(t[0]+2)%MOD;
        b=mul(b,t,m2);
    }
    b.resize(n);return b;
}
vector<ll> invs;
vector<ll> plog(const vector<ll>&a,int n){
    vector<ll> d(n,0); for(int i=1;i<(int)a.size()&&i<=n;i++) d[i-1]=a[i]*i%MOD;
    vector<ll> r=mul(d,inv(a,n),n); r.resize(n);
    vector<ll> res(n,0); for(int i=n-1;i>=1;i--) res[i]=r[i-1]*invs[i]%MOD;
    return res;
}
vector<ll> pexp(const vector<ll>&a,int n){
    vector<ll> b{1};
    for(int m=1;m<n;m*=2){
        int m2=2*m; vector<ll> l=plog(b,m2);
        vector<ll> t(m2); for(int i=0;i<m2;i++){ll v=(i<(int)a.size()?a[i]:0)-l[i];if(v<0)v+=MOD;t[i]=v;}
        t[0]=(t[0]+1)%MOD; b.resize(m2); b=mul(b,t,m2);
    }
    b.resize(n);return b;
}
int main(){
    int n; if(scanf("%d",&n)!=1)return 0;
    vector<ll> cnt(n+2,0); ll m=0;
    for(int i=0;i<n;i++){int x;scanf("%d",&x);cnt[x]++;m+=x;}
    int L=m/2+1;
    invs.assign(max<ll>(L+2,m+2),1);
    for(int i=2;i<(int)invs.size();i++) invs[i]=(MOD-(MOD/i)*invs[MOD%i]%MOD)%MOD;
    vector<ll> lg(L,0);
    for(int j=1;j<L;j++) lg[j]=(ll)n%MOD*invs[j]%MOD;
    for(int s=0;s<=n;s++) if(cnt[s]){
        ll step=s+1;
        for(ll j=1;j*step<L;j++){ lg[j*step]=(lg[j*step]-cnt[s]%MOD*invs[j])%MOD; if(lg[j*step]<0)lg[j*step]+=MOD; }
    }
    vector<ll> N=pexp(lg,L);
    // binomials C(m,k)
    vector<ll> fact(m+1),ifact(m+1); fact[0]=1; for(ll i=1;i<=m;i++)fact[i]=fact[i-1]*i%MOD;
    ifact[m]=pw(fact[m],MOD-2); for(ll i=m;i>0;i--)ifact[i-1]=ifact[i]*i%MOD;
    auto C=[&](ll a,ll b)->ll{if(b<0||b>a)return 0;return fact[a]*ifact[b]%MOD*ifact[a-b]%MOD;};
    ll ans=0;
    for(int k=0;k<L;k++){
        ll K=(N[k]-(k?N[k-1]:0)+MOD)%MOD;
        ll f=(C(m,k)-C(m,k-1)+MOD)%MOD;
        ans=(ans+K*f)%MOD;
    }
    printf("%lld\n",ans);
}
