#include <bits/stdc++.h>
using namespace std;
typedef long long ll; typedef unsigned long long ull; typedef unsigned int u32;
static ll pw(ll b,ll e,ll m){ll r=1;b%=m;if(b<0)b+=m;while(e){if(e&1)r=r*b%m;b=b*b%m;e>>=1;}return r;}
template<u32 MOD> struct NTT{
    vector<u32> rt; // roots
    void prep(int n){
        if((int)rt.size()>=n) return;
        rt.assign(max(n,2),0); rt[1]=1;
        for(int k=1;(1<<k)<n;k++){
            u32 z=pw(3,(MOD-1)>>(k+1),MOD);
            for(int i=1<<(k-1);i<(1<<k);i++){rt[2*i]=rt[i];rt[2*i+1]=(ull)rt[i]*z%MOD;}
        }
    }
    void ntt(vector<u32>&a,int n){
        prep(n);
        // bit reverse
        for(int i=1,j=0;i<n;i++){int bit=n>>1;for(;j&bit;bit>>=1)j^=bit;j^=bit;if(i<j)swap(a[i],a[j]);}
        for(int k=1;k<n;k*=2)for(int i=0;i<n;i+=2*k)for(int j=0;j<k;j++){
            u32 z=(ull)rt[j+k]*a[i+j+k]%MOD; u32 &ai=a[i+j];
            a[i+j+k]= ai>=z? ai-z: ai+MOD-z; ai = ai+z>=MOD? ai+z-MOD: ai+z;
        }
    }
    void intt(vector<u32>&a,int n){
        reverse(a.begin()+1,a.begin()+n); ntt(a,n);
        u32 inv=pw(n,MOD-2,MOD); for(int i=0;i<n;i++)a[i]=(ull)a[i]*inv%MOD;
    }
};
const u32 P1=998244353,P2=167772161,P3=469762049;
NTT<P1> n1; NTT<P2> n2; NTT<P3> n3;
template<u32 MOD,class T> void mulp(T&nt,const vector<ll>&c,const vector<const vector<ll>*>&xs,int sz,vector<vector<u32>>&out){
    vector<u32> fc(sz,0); for(size_t i=0;i<c.size();i++)fc[i]=c[i]%MOD; nt.ntt(fc,sz);
    out.resize(xs.size());
    for(size_t t=0;t<xs.size();t++){
        vector<u32> f(sz,0); for(size_t i=0;i<xs[t]->size();i++)f[i]=(*xs[t])[i]%MOD;
        nt.ntt(f,sz); for(int i=0;i<sz;i++)f[i]=(ull)f[i]*fc[i]%MOD; nt.intt(f,sz); out[t]=move(f);
    }
}
// returns c*x for each x in xs, mod M, full length
vector<vector<ll>> multi(const vector<ll>&c,const vector<const vector<ll>*>&xs,ll M){
    size_t mx=0; for(auto x:xs)mx=max(mx,x->size());
    size_t L=c.size()+mx-1; int sz=1; while((size_t)sz<L)sz<<=1;
    vector<vector<u32>> o1,o2,o3;
    mulp<P1>(n1,c,xs,sz,o1); mulp<P2>(n2,c,xs,sz,o2); mulp<P3>(n3,c,xs,sz,o3);
    const ull r12=pw(P1,P2-2,P2), r123=pw((ull)P1*P2%P3,P3-2,P3);
    const ull p12m=(ull)P1*P2%M;
    vector<vector<ll>> res(xs.size());
    for(size_t t=0;t<xs.size();t++){
        size_t Lt=c.size()+xs[t]->size()-1; res[t].resize(Lt);
        for(size_t i=0;i<Lt;i++){
            ull a1=o1[t][i],a2=o2[t][i],a3=o3[t][i];
            ull x1=a1;
            ull x2=(a2+P2-a1%P2)%P2*r12%P2;
            ull v=(x1+(ull)P1*x2)%P3;
            ull x3=(a3+P3-v)%P3*r123%P3;
            res[t][i]=(ll)((x1%M+ (ull)P1%M*x2%M + p12m*x3%M)%M);
        }
    }
    return res;
}
vector<ll> conv(const vector<ll>&a,const vector<ll>&b,ll M){ return multi(a,{&b},M)[0]; }

int main(){
    int T; scanf("%d",&T);
    while(T--){
        ll n,M; scanf("%lld %lld",&n,&M);
        int F=2*n+2; vector<ll> fa(F+1),ifa(F+1); fa[0]=1; for(int i=1;i<=F;i++)fa[i]=fa[i-1]*i%M;
        ifa[F]=pw(fa[F],M-2,M); for(int i=F;i>0;i--)ifa[i-1]=ifa[i]*i%M;
        auto C=[&](int a,int b)->ll{ if(b<0||b>a)return 0; return fa[a]*ifa[b]%M*ifa[a-b]%M;};
        vector<ll> H(n+1),R(n+1);
        for(int m=0;m<=n;m++){ ll c=C(2*m,m)*pw(m+1,M-2,M)%M; H[m]=c*c%M; R[m]=H[m]*(1+2*m)%M; }
        // Bostan-Mori: [z^n] R/(1-yP), P=zH. bivariate arrays indexed [zi][yj]
        int N=n, dy=1;
        vector<vector<ll>> Pz(N+1,vector<ll>(dy+1,0)), Qz(N+1,vector<ll>(dy+1,0));
        for(int i=0;i<=N;i++)Pz[i][0]=R[i];
        Qz[0][0]=1; for(int i=1;i<=N;i++)Qz[i][1]=(M-H[i-1])%M;
        while(N>0){
            int W=2*dy+1;
            vector<ll> fp((N+1)*W,0),fq((N+1)*W,0),fm((N+1)*W,0);
            for(int i=0;i<=N;i++)for(int j=0;j<=dy;j++){
                fp[i*W+j]=Pz[i][j]; fq[i*W+j]=Qz[i][j]; fm[i*W+j]=(i&1)?(M-Qz[i][j])%M:Qz[i][j];
            }
            auto res=multi(fm,{&fp,&fq},M);
            int nd=min<ll>(2*dy,n); int NN=N/2; int par=N&1;
            vector<vector<ll>> nP(NN+1,vector<ll>(nd+1,0)), nQ(NN+1,vector<ll>(nd+1,0));
            for(int i=0;i<=NN;i++)for(int j=0;j<=nd;j++){
                size_t ip=(size_t)(2*i+par)*W+j, iq=(size_t)(2*i)*W+j;
                if(2*i+par<=N && ip<res[0].size()) nP[i][j]=res[0][ip];
                if(iq<res[1].size()) nQ[i][j]=res[1][iq];
            }
            Pz=move(nP);Qz=move(nQ);N=NN;dy=nd;
        }
        // Q(0,y)=1
        vector<ll> c(n+1,0); for(int j=0;j<=min<ll>(dy,n);j++)c[j]=Pz[0][j];
        ll inv2n1=pw(2*n+1,M-2,M);
        vector<ll> a(n+1),b(n+1);
        for(int j=0;j<=n;j++){ ll G=C(2*n+1,j)*inv2n1%M*c[j]%M; a[n-j]=G*fa[j]%M; b[j]=(j&1)?(M-ifa[j])%M:ifa[j]; }
        vector<ll> d=conv(a,b,M);
        for(int k=0;k<=n;k++){ ll v=d[n-k]*ifa[k]%M; printf("%lld%c",v,k==n?'\n':' '); }
    }
}
