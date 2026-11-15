#include <bits/stdc++.h>
using namespace std;
typedef unsigned int u32; typedef unsigned long long u64;
static const u32 MOD=998244353;
u32 pw(u32 a,u64 e){u64 r=1,b=a;while(e){if(e&1)r=r*b%MOD;b=b*b%MOD;e>>=1;}return r;}

void ntt(vector<u32>&a){
    int n=a.size(),L=__builtin_ctz(n);
    static vector<u32> rt(2,1);
    for(static int k=2,s=2;k<n;k*=2,s++){
        rt.resize(n);
        u32 z[2]={1,pw(3,MOD>>s)};
        for(int i=k;i<2*k;i++) rt[i]=(u64)rt[i/2]*z[i&1]%MOD;
    }
    vector<int> rev(n);
    for(int i=0;i<n;i++) rev[i]=(rev[i/2]|(i&1)<<L)/2;
    for(int i=0;i<n;i++) if(i<rev[i]) swap(a[i],a[rev[i]]);
    for(int k=1;k<n;k*=2)
        for(int i=0;i<n;i+=2*k)
            for(int j=0;j<k;j++){
                u32 z=(u64)rt[j+k]*a[i+j+k]%MOD;
                u32 ai=a[i+j];
                a[i+j+k]=ai>=z?ai-z:ai+MOD-z;
                u32 s=ai+z; a[i+j]=s>=MOD?s-MOD:s;
            }
}
void intt(vector<u32>&a){
    ntt(a); reverse(a.begin()+1,a.end());
    u32 inv=pw(a.size(),MOD-2);
    for(auto&x:a) x=(u64)x*inv%MOD;
}
vector<u32> mul(vector<u32> a,vector<u32> b){
    if(a.empty()||b.empty()) return {};
    size_t need=a.size()+b.size()-1;
    if(min(a.size(),b.size())<=16){
        vector<u32> r(need,0);
        for(size_t i=0;i<a.size();i++) if(a[i])
            for(size_t j=0;j<b.size();j++) r[i+j]=(r[i+j]+(u64)a[i]*b[j])%MOD;
        return r;
    }
    size_t L=1; while(L<need) L<<=1;
    a.resize(L); b.resize(L);
    ntt(a); ntt(b);
    for(size_t i=0;i<L;i++) a[i]=(u64)a[i]*b[i]%MOD;
    intt(a); a.resize(need); return a;
}

vector<u32> invf, fct;

pair<vector<u32>,vector<u32>> bm(int m0){
    int m=m0, dxD=1, dxN=0;
    vector<u32> D((size_t)2*(m+1),0), Nt((size_t)(m+1),0), Ng((size_t)(m+1),0);
    D[0]=1;
    for(int a=2;a<=m;a++){
        u32 ua=(u64)(a-1)*invf[a]%MOD;
        D[(m+1)+a]=(MOD-ua)%MOD;
    }
    Nt[0]=1;
    for(int a=0;a<=m;a++) Ng[a]=invf[a];
    while(m>0){
        int Sy=2*m+2;
        int bmax=max(2*dxD,dxN+dxD);
        size_t need=(size_t)bmax*Sy+2*m+1;
        size_t L=1; while(L<need) L<<=1;
        vector<u32> fD(L,0),fT(L,0),fG(L,0);
        for(int b=0;b<=dxD;b++) for(int a=0;a<=m;a++) fD[(size_t)b*Sy+a]=D[(size_t)b*(m+1)+a];
        for(int b=0;b<=dxN;b++) for(int a=0;a<=m;a++){
            fT[(size_t)b*Sy+a]=Nt[(size_t)b*(m+1)+a];
            fG[(size_t)b*Sy+a]=Ng[(size_t)b*(m+1)+a];
        }
        ntt(fD); ntt(fT); ntt(fG);
        size_t h=L>>1;
        for(size_t k=0;k<h;k++){
            u32 a0=fD[k],a1=fD[k+h];
            u32 p=(u64)a0*a1%MOD;
            fD[k]=fD[k+h]=p;
            fT[k]=(u64)fT[k]*a1%MOD; fT[k+h]=(u64)fT[k+h]*a0%MOD;
            fG[k]=(u64)fG[k]*a1%MOD; fG[k+h]=(u64)fG[k+h]*a0%MOD;
        }
        intt(fD); intt(fT); intt(fG);
        int m2=m/2, par=m&1;
        int ndD=2*dxD, ndN=dxN+dxD;
        vector<u32> nD((size_t)(ndD+1)*(m2+1)), nT((size_t)(ndN+1)*(m2+1)), nG((size_t)(ndN+1)*(m2+1));
        for(int b=0;b<=ndD;b++) for(int j=0;j<=m2;j++) nD[(size_t)b*(m2+1)+j]=fD[(size_t)b*Sy+2*j];
        for(int b=0;b<=ndN;b++) for(int j=0;j<=m2;j++){
            nT[(size_t)b*(m2+1)+j]=fT[(size_t)b*Sy+2*j+par];
            nG[(size_t)b*(m2+1)+j]=fG[(size_t)b*Sy+2*j+par];
        }
        auto trimRows=[&](vector<u32>&v,int &dx,int rowlen,bool two,vector<u32>*w){
            int top=0;
            for(int b=dx;b>=1;b--){
                bool nz=false;
                for(int j=0;j<rowlen&&!nz;j++){
                    if(v[(size_t)b*rowlen+j]) nz=true;
                    if(two&&(*w)[(size_t)b*rowlen+j]) nz=true;
                }
                if(nz){top=b;break;}
            }
            dx=top;
            v.resize((size_t)(dx+1)*rowlen);
            if(two) w->resize((size_t)(dx+1)*rowlen);
        };
        dxD=ndD; trimRows(nD,dxD,m2+1,false,nullptr);
        dxN=ndN; trimRows(nT,dxN,m2+1,true,&nG);
        D.swap(nD); Nt.swap(nT); Ng.swap(nG);
        m=m2;
    }
    vector<u32> T(m0/2+1,0),G(m0/2+1,0);
    for(int b=0;b<=dxN&&b<=m0/2;b++){T[b]=Nt[b];G[b]=Ng[b];}
    return {T,G};
}

vector<u32> subv(const vector<u32>&a,const vector<u32>&b){
    vector<u32> r(max(a.size(),b.size()),0);
    for(size_t i=0;i<a.size();i++) r[i]=a[i];
    for(size_t i=0;i<b.size();i++) r[i]=(r[i]+MOD-b[i])%MOD;
    return r;
}
vector<vector<u32>> polys;
vector<u32> prodRange(int l,int r){
    if(l==r) return polys[l];
    int mid=(l+r)/2;
    return mul(prodRange(l,mid),prodRange(mid+1,r));
}

int main(){
    int n; string s;
    cin>>n>>s;
    fct.assign(n+5,1); invf.assign(n+5,1);
    for(int i=1;i<n+5;i++) fct[i]=(u64)fct[i-1]*i%MOD;
    invf[n+4]=pw(fct[n+4],MOD-2);
    for(int i=n+4;i>0;i--) invf[i-1]=(u64)invf[i]*i%MOD;
    vector<pair<int,int>> segs;
    int i=0;
    while(i<n){
        int A=0,B=0;
        while(i<n&&s[i]=='R'){A++;i++;}
        while(i<n&&s[i]=='L'){B++;i++;}
        segs.push_back({A,B});
    }
    map<int,pair<vector<u32>,vector<u32>>> mp;
    for(auto&sg:segs){
        for(int v:{sg.first,sg.second}) if(v>0 && !mp.count(v)) mp[v]=bm(v);
    }
    for(auto&sg:segs){
        int A=sg.first,B=sg.second;
        vector<u32> P;
        if(A==0||B==0){
            int m=A?A:B;
            auto&T=mp[m].first; auto&G=mp[m].second;
            // P = x*G + (1-x)*T
            P.assign(T.size()+2,0);
            for(size_t k=0;k<G.size();k++) P[k+1]=(P[k+1]+G[k])%MOD;
            for(size_t k=0;k<T.size();k++){
                P[k]=(P[k]+T[k])%MOD;
                P[k+1]=(P[k+1]+MOD-T[k])%MOD;
            }
        } else {
            auto&TA=mp[A].first; auto&GA=mp[A].second;
            auto&TB=mp[B].first; auto&GB=mp[B].second;
            vector<u32> ZA=subv(GA,TA), ZB=subv(GB,TB);
            vector<u32> P1=mul(GA,GB), W=mul(ZA,ZB);
            P.assign(max(P1.size(),W.size()+1),0);
            for(size_t k=0;k<P1.size();k++) P[k]=P1[k];
            for(size_t k=0;k<W.size();k++){
                P[k+1]=(P[k+1]+W[k])%MOD;
                P[k]=(P[k]+MOD-W[k])%MOD;
            }
        }
        while(P.size()>1&&P.back()==0) P.pop_back();
        polys.push_back(P);
    }
    vector<u32> R=prodRange(0,(int)polys.size()-1);
    R.resize(max((size_t)n+1,R.size()),0);
    string out;
    for(int k=1;k<=n;k++){
        u32 v=(u64)R[k]*fct[n]%MOD;
        out+=to_string(v);
        out+=(k==n?'\n':' ');
    }
    fputs(out.c_str(),stdout);
    return 0;
}
