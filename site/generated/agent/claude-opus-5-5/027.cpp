#include <bits/stdc++.h>
using namespace std;
typedef long long ll;
typedef unsigned long long ull;
const int MOD=998244353;
ll pw(ll b,ll e){ll r=1;b%=MOD;if(b<0)b+=MOD;while(e){if(e&1)r=r*b%MOD;b=b*b%MOD;e>>=1;}return r;}
vector<int> rts; vector<int> rev;
void ntt(vector<int>&a,bool inv){
    int n=a.size();
    if((int)rev.size()!=n){rev.assign(n,0);for(int i=1;i<n;i++)rev[i]=(rev[i>>1]>>1)|((i&1)?n>>1:0);}
    for(int i=0;i<n;i++)if(i<rev[i])swap(a[i],a[rev[i]]);
    if((int)rts.size()<n){
        rts.assign(max(2,n),0);rts[1]=1;
        for(int k=1;(1<<k)<n;k++){
            ll z=pw(3,(MOD-1)>>(k+1));
            for(int i=1<<(k-1);i<(1<<k);i++){rts[2*i]=rts[i];rts[2*i+1]=(ll)rts[i]*z%MOD;}
        }
    }
    for(int k=1;k<n;k<<=1)for(int i=0;i<n;i+=2*k)for(int j=0;j<k;j++){
        int z=(ll)rts[j+k]*a[i+j+k]%MOD;int &ai=a[i+j];
        a[i+j+k]=ai-z<0?ai-z+MOD:ai-z; ai=ai+z>=MOD?ai+z-MOD:ai+z;
    }
    if(inv){reverse(a.begin()+1,a.end());ll ni=pw(n,MOD-2);for(auto&x:a)x=x*ni%MOD;}
}
vector<int> mul(vector<int> a,vector<int> b){
    if(a.empty()||b.empty())return {};
    int s=a.size()+b.size()-1;
    if(min(a.size(),b.size())<=32){
        vector<ull> r(s,0);
        for(size_t i=0;i<a.size();i++){for(size_t j=0;j<b.size();j++){r[i+j]=(r[i+j]+(ull)a[i]*b[j])%MOD;}}
        vector<int> o(s);for(int i=0;i<s;i++)o[i]=r[i];return o;
    }
    int n=1;while(n<s)n<<=1;
    a.resize(n);b.resize(n);ntt(a,0);ntt(b,0);
    for(int i=0;i<n;i++)a[i]=(ll)a[i]*b[i]%MOD;
    ntt(a,1);a.resize(s);return a;
}
vector<int> inverse(const vector<int>&a,int m){ // a[0]!=0, mod x^m
    vector<int> b={(int)pw(a[0],MOD-2)};
    int k=1;
    while(k<m){
        k*=2;
        vector<int> ac(a.begin(),a.begin()+min((int)a.size(),k));
        vector<int> t=mul(mul(b,b),ac);t.resize(k);
        vector<int> nb(k);
        for(int i=0;i<k;i++){ll v=(i<(int)b.size()?2LL*b[i]:0)-t[i];v%=MOD;if(v<0)v+=MOD;nb[i]=v;}
        b=nb;
    }
    b.resize(m);return b;
}
vector<ll> fact,ifact;
// returns for N: [x^N] num_k(x)/(1-zG(x)) for nums: 1, e^x, x e^x ; as polys in z of length D+1
array<vector<int>,3> solveN(int N){
    int D=N/2+1;
    int nx=N,dy=1;
    vector<int> Q((nx+1)*(dy+1),0);
    Q[0]=1;
    for(int i=2;i<=N;i++){ // G_i=(i-1)/i!
        ll g=(ll)(i-1)*ifact[i]%MOD; Q[i*(dy+1)+1]=(MOD-g)%MOD;
    }
    array<vector<int>,3> P;
    for(int k=0;k<3;k++)P[k].assign((nx+1)*(dy+1),0);
    for(int i=0;i<=N;i++){
        P[0][i*(dy+1)]=(i==0);
        P[1][i*(dy+1)]=ifact[i];
        P[2][i*(dy+1)]=(i>=1?ifact[i-1]:0);
    }
    while(N>0){
        int Y=2*dy+1;
        int len=2*nx*Y+2*dy+1;
        int sz=1;while(sz<len)sz<<=1;
        vector<int> A(sz,0),B(sz,0);
        for(int i=0;i<=nx;i++)for(int j=0;j<=dy;j++){
            int v=Q[i*(dy+1)+j];A[i*Y+j]=v;B[i*Y+j]=(i&1)?(v?MOD-v:0):v;
        }
        ntt(A,0);ntt(B,0);
        int Nn=N/2,par=N&1;
        int nnx=min(Nn,nx),ndy=min(2*dy,D);
        array<vector<int>,3> NP;
        for(int k=0;k<3;k++){
            vector<int> C(sz,0);
            for(int i=0;i<=nx;i++)for(int j=0;j<=dy;j++)C[i*Y+j]=P[k][i*(dy+1)+j];
            ntt(C,0);
            for(int t=0;t<sz;t++)C[t]=(ll)C[t]*B[t]%MOD;
            ntt(C,1);
            NP[k].assign((nnx+1)*(ndy+1),0);
            for(int i=0;i<=nnx;i++){int src=2*i+par; if(src>2*nx)break;
                for(int j=0;j<=ndy;j++)NP[k][i*(ndy+1)+j]=C[src*Y+j];}
        }
        for(int t=0;t<sz;t++)A[t]=(ll)A[t]*B[t]%MOD;
        ntt(A,1);
        vector<int> NQ((nnx+1)*(ndy+1),0);
        for(int i=0;i<=nnx;i++)for(int j=0;j<=ndy;j++)NQ[i*(ndy+1)+j]=A[2*i*Y+j];
        Q=NQ;P=NP;nx=nnx;dy=ndy;N=Nn;
    }
    vector<int> q0(Q.begin(),Q.begin()+dy+1);
    vector<int> qi=inverse(q0,D+1);
    array<vector<int>,3> R;
    for(int k=0;k<3;k++){
        vector<int> p0(P[k].begin(),P[k].begin()+dy+1);
        R[k]=mul(p0,qi);R[k].resize(D+1);
    }
    return R;
}
vector<int> add(const vector<int>&a,const vector<int>&b,ll cb=1){
    vector<int> r(max(a.size(),b.size()),0);
    for(size_t i=0;i<a.size();i++)r[i]=a[i];
    for(size_t i=0;i<b.size();i++)r[i]=(r[i]+cb%MOD*b[i]%MOD+MOD)%MOD;
    return r;
}
vector<int> shiftz(const vector<int>&a){vector<int> r(a.size()+1,0);for(size_t i=0;i<a.size();i++)r[i+1]=a[i];return r;}
int main(){
    int n;string s;cin>>n>>s;
    fact.assign(n+2,1);ifact.assign(n+2,1);
    for(int i=1;i<=n+1;i++)fact[i]=fact[i-1]*i%MOD;
    ifact[n+1]=pw(fact[n+1],MOD-2);for(int i=n+1;i>0;i--)ifact[i-1]=ifact[i]*i%MOD;
    // mountains
    vector<pair<int,int>> mts;
    int i=0;
    while(i<n){
        int a=0,b=0;
        while(i<n&&s[i]=='R'){a++;i++;}
        while(i<n&&s[i]=='L'){b++;i++;}
        mts.push_back({a,b});
    }
    auto get=[&](int N)->array<vector<int>,3>{
        if(N==0){array<vector<int>,3> r;r[0]={1};r[1]={1};r[2]={0};return r;}
        return solveN(N);
    };
    // U(N-1) = r[2] at N
    vector<vector<int>> polys;
    for(auto [a,b]:mts){
        vector<int> P;
        if(a==0){auto r=get(b);P=shiftz(r[2]);}
        else if(b==0){auto r=get(a);P=shiftz(r[2]);}
        else{
            auto ra=get(a),rb=get(b);
            // P=(z-1)T(a)T(b) - z(U(a)U(b)-U(a-1)U(b)-U(a)U(b-1))
            vector<int> TT=mul(ra[0],rb[0]);
            vector<int> X=add(shiftz(TT),TT,-1);
            vector<int> UU=mul(ra[1],rb[1]);
            UU=add(UU,mul(ra[2],rb[1]),-1);
            UU=add(UU,mul(ra[1],rb[2]),-1);
            P=add(X,shiftz(UU),-1);
        }
        while(P.size()>1&&P.back()==0)P.pop_back();
        polys.push_back(P);
    }
    // D&C product
    priority_queue<pair<int,int>,vector<pair<int,int>>,greater<>> pq;
    for(int k=0;k<(int)polys.size();k++)pq.push({(int)polys[k].size(),k});
    while(pq.size()>1){
        int x=pq.top().second;pq.pop();int y=pq.top().second;pq.pop();
        polys[x]=mul(polys[x],polys[y]);polys[y].clear();polys[y].shrink_to_fit();
        pq.push({(int)polys[x].size(),x});
    }
    vector<int>&res=polys[pq.top().second];
    string out;
    for(int k=1;k<=n;k++){
        ll v=k<(int)res.size()?res[k]:0;
        v=v*fact[n]%MOD;
        out+=to_string(v);out+=(k==n?'\n':' ');
    }
    fputs(out.c_str(),stdout);
}
