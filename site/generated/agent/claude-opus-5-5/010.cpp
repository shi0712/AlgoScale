#include <bits/stdc++.h>
using namespace std;
typedef long long ll;
typedef unsigned long long ull;
const int MOD=998244353;
ll pw(ll b,ll e){b%=MOD;if(b<0)b+=MOD;ll r=1;while(e){if(e&1)r=r*b%MOD;b=b*b%MOD;e>>=1;}return r;}
void ntt(vector<int>&a,bool inv){
    int n=a.size();
    for(int i=1,j=0;i<n;i++){int bit=n>>1;for(;j&bit;bit>>=1)j^=bit;j^=bit;if(i<j)swap(a[i],a[j]);}
    for(int len=2;len<=n;len<<=1){
        ll w=pw(3,(MOD-1)/len); if(inv) w=pw(w,MOD-2);
        int h=len/2;
        static vector<int> ws; ws.assign(h,1);
        for(int i=1;i<h;i++) ws[i]=(ll)ws[i-1]*w%MOD;
        for(int i=0;i<n;i+=len)for(int j=0;j<h;j++){
            int u=a[i+j],v=(ll)a[i+j+h]*ws[j]%MOD;
            a[i+j]=u+v>=MOD?u+v-MOD:u+v; a[i+j+h]=u-v<0?u-v+MOD:u-v;
        }
    }
    if(inv){ll ni=pw(n,MOD-2);for(auto&x:a)x=(ll)x*ni%MOD;}
}
vector<int> conv(vector<int> a,vector<int> b){
    int need=a.size()+b.size()-1;
    if(min(a.size(),b.size())<=32){
        vector<ull> r(need,0);
        vector<int> res(need);
        for(size_t i=0;i<a.size();i++){for(size_t j=0;j<b.size();j++){r[i+j]+=(ull)a[i]*b[j]%MOD;} if((i&15)==15) for(auto&x:r) x%=MOD;}
        for(int i=0;i<need;i++) res[i]=r[i]%MOD;
        return res;
    }
    int sz=1;while(sz<need)sz<<=1;
    a.resize(sz);b.resize(sz);ntt(a,false);ntt(b,false);
    for(int i=0;i<sz;i++)a[i]=(ll)a[i]*b[i]%MOD;
    ntt(a,true);a.resize(need);return a;
}
vector<ll> fact,ifact;
inline ll md(ll x){x%=MOD;if(x<0)x+=MOD;return x;}
// given f(0..d), return f(m+k) for k=0..L-1; requires m+k not in [0,d]
vector<int> shiftv(const vector<int>&f,ll m,int L){
    int d=f.size()-1;
    vector<int> A(d+1);
    for(int i=0;i<=d;i++){
        ll v=(ll)f[i]*ifact[i]%MOD*ifact[d-i]%MOD;
        if((d-i)&1) v=(MOD-v)%MOD;
        A[i]=v;
    }
    int nb=d+L;
    vector<int> vals(nb),B(nb);
    // vals[l]=m-d+l
    vector<ll> pre(nb+1);pre[0]=1;
    for(int l=0;l<nb;l++){vals[l]=md(m-d+l);pre[l+1]=pre[l]*vals[l]%MOD;}
    ll iv=pw(pre[nb],MOD-2);
    for(int l=nb-1;l>=0;l--){B[l]=iv*pre[l]%MOD;iv=iv*vals[l]%MOD;}
    vector<int> C=conv(A,B);
    vector<int> res(L);
    // prod_{j=0}^{d}(m+k-j) for k=0
    ll pr=1;for(int j=0;j<=d;j++) pr=pr*md(m-j)%MOD;
    for(int k=0;k<L;k++){
        res[k]=(ll)C[k+d]*pr%MOD;
        // next: multiply by (m+k+1), divide by (m+k-d) -> inv is B[k]
        pr=pr*md(m+k+1)%MOD*B[k]%MOD;
    }
    return res;
}
int n;
vector<ll> fac; // factor constant at t: value C - x ; has[t]
vector<char> has;
vector<ll> P;
const int TH=64;
// returns values of node product at x=lo..hi
vector<int> build(int lo,int hi){
    int len=hi-lo;
    if(len<=TH){
        vector<int> v(len+1,1);
        for(int t=lo;t<hi;t++) if(has[t]){
            for(int x=lo;x<=hi;x++) v[x-lo]=(ll)v[x-lo]*md(fac[t]-x)%MOD;
            for(int x=t;x<hi;x++) P[x]=P[x]*md(fac[t]-x)%MOD;
        }
        return v;
    }
    int mid=(lo+hi)/2;
    vector<int> VL=build(lo,mid),VR=build(mid,hi);
    int dL=mid-lo,dR=hi-mid;
    vector<int> e1=shiftv(VL,dL+1,hi-mid);
    P[mid]=P[mid]*VL[dL]%MOD;
    for(int t=mid+1;t<hi;t++) P[t]=P[t]*e1[t-mid-1]%MOD;
    vector<int> e2=shiftv(VR,-(ll)dL,dL);
    vector<int> res(len+1);
    for(int x=lo;x<=hi;x++){
        ll a = x<=mid? VL[x-lo] : e1[x-mid-1];
        ll b = x>=mid? VR[x-mid] : e2[x-lo];
        res[x-lo]=a*b%MOD;
    }
    return res;
}
int main(){
    int N=700000;fact.assign(N+1,1);ifact.assign(N+1,1);
    for(int i=1;i<=N;i++)fact[i]=fact[i-1]*i%MOD;
    ifact[N]=pw(fact[N],MOD-2);for(int i=N;i>0;i--)ifact[i-1]=ifact[i]*i%MOD;
    int T;scanf("%d",&T);
    while(T--){
        int m;scanf("%d %d",&n,&m);
        vector<int> a(m);for(auto&x:a)scanf("%d",&x);
        vector<int> nx(m,-1),last(2*n+2,-1);
        for(int i=m-1;i>=0;i--){nx[i]=last[a[i]];last[a[i]]=i;}
        vector<ll> D(m,0);
        for(int i=m-2;i>=0;i--){D[i]=1+D[i+1];if(nx[i]>=0)D[i]=max(D[i],(ll)n+1+D[nx[i]]);}
        vector<char> seen(2*n+2,0);
        fac.assign(n+2,0);has.assign(n+2,0);P.assign(n+2,1);
        vector<int> act(n+2,0); // number of activations at t
        int j=0;
        for(int i=0;i<m;i++){
            if(seen[a[i]])continue;seen[a[i]]=1;j++;
            ll h=D[0]-D[i];
            if(h>=n)break;
            ll g=h-(j-1);
            int t=h+1;
            has[t]=1;fac[t]=2LL*n+g;act[t]=j;
        }
        for(int t=1;t<=n;t++) if(has[t]) P[t]=md(fac[t]-t);
        // P[t] above counts only leaf; build multiplies contributions from earlier activations
        // reset and let build handle all
        for(int t=1;t<=n;t++) P[t]=1;
        build(1,n+1);
        // denominators
        ll sum=0; int r=0; ll den=1;
        for(int t=1;t<=n;t++){
            if(has[t]){r++; den=den*pw(2LL*n-r+1,MOD-2)%MOD;}
            sum=(sum+P[t]*den)%MOD;
        }
        ll ans=md(D[0]%MOD+1+n-sum);
        printf("%lld\n",ans);
    }
}
