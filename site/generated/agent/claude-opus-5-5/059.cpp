#include <bits/stdc++.h>
using namespace std;
typedef unsigned long long u64;
typedef unsigned __int128 u128;
typedef vector<u64> Big;

static void trim(Big& a){ while(!a.empty() && a.back()==0) a.pop_back(); }
static Big mulSmall(const Big& a, u64 b){
    Big r(a.size()+1); u64 c=0;
    for(size_t i=0;i<a.size();i++){ u128 t=(u128)a[i]*b+c; r[i]=(u64)t; c=(u64)(t>>64); }
    r[a.size()]=c; trim(r); return r;
}
static Big divSmall(const Big& a, u64 b){
    Big r(a.size()); u128 rem=0;
    for(size_t i=a.size();i-->0;){ u128 cur=(rem<<64)|a[i]; r[i]=(u64)(cur/b); rem=cur%b; }
    trim(r); return r;
}
static Big mulBig(const Big& a, const Big& b){
    if(a.empty()||b.empty()) return Big();
    Big r(a.size()+b.size(),0);
    for(size_t i=0;i<a.size();i++){
        u64 c=0;
        for(size_t j=0;j<b.size();j++){ u128 t=(u128)a[i]*b[j]+r[i+j]+c; r[i+j]=(u64)t; c=(u64)(t>>64); }
        size_t k=i+b.size();
        while(c){ u128 t=(u128)r[k]+c; r[k]=(u64)t; c=(u64)(t>>64); k++; }
    }
    trim(r); return r;
}
static void addTo(Big& a, const Big& b){
    if(a.size()<b.size()) a.resize(b.size(),0);
    u64 c=0;
    for(size_t i=0;i<a.size();i++){
        u64 bv = i<b.size()?b[i]:0;
        if(i>=b.size() && !c) break;
        u128 t=(u128)a[i]+bv+c; a[i]=(u64)t; c=(u64)(t>>64);
    }
    if(c) a.push_back(c);
}
// a - b, assumes a>=b
static Big subBig(const Big& a, const Big& b){
    Big r(a); u64 br=0;
    for(size_t i=0;i<r.size();i++){
        u64 bv = i<b.size()?b[i]:0;
        u128 t=(u128)r[i]-bv-br; r[i]=(u64)t; br=(u64)(t>>64)&1;
    }
    trim(r); return r;
}
static int cmpBig(const Big& a, const Big& b){
    if(a.size()!=b.size()) return a.size()<b.size()?-1:1;
    for(size_t i=a.size();i-->0;) if(a[i]!=b[i]) return a[i]<b[i]?-1:1;
    return 0;
}
// returns value as mantissa * 2^(64*e)
static long double toLD(const Big& a, long long& e){
    if(a.empty()){ e=0; return 0; }
    size_t k=a.size(); long double v=0;
    size_t lo = k>=3?k-3:0;
    for(size_t i=k;i-->lo;) v = v*18446744073709551616.0L + (long double)a[i];
    e=(long long)lo; return v;
}

int main(){
    long long n,m;
    if(!(cin>>n>>m)) return 0;
    // Cbig[a] = C(m-n+a+1, a+1), a=0..n
    vector<Big> Cb(n+1);
    {
        Big c{(u64)(m-n+1)};
        Cb[0]=c;
        for(long long a=1;a<=n;a++){
            c=mulSmall(c,(u64)(m-n+a+1));
            c=divSmall(c,(u64)(a+1));
            Cb[a]=c;
        }
    }
    vector<int> len(n+1);
    for(int a=0;a<=n;a++) len[a]=(int)Cb[a].size()+1;
    int LMAX=len[n];
    vector<Big> N(n+1);
    vector<u64> bufA((size_t)n*LMAX), bufB((size_t)n*LMAX);
    vector<u64> pre(LMAX), W(LMAX), base(LMAX);
    for(long long s=1;s<=n;s++){
        u64* cur=bufA.data(); u64* nxt=bufB.data();
        memset(cur,0,sizeof(u64)*s*LMAX);
        memset(nxt,0,sizeof(u64)*s*LMAX);
        long long X0=m-n;
        for(long long r=0;r<s;r++){
            cur[r*LMAX] = (r<=X0)? (u64)((X0-r)/s+1) : 0;
        }
        for(long long a=1;a<=n;a++){
            int L=len[a];
            long long X=m-n+a;
            long long r0=(X+1)%s;
            fill(pre.begin(),pre.begin()+L,0ULL);
            fill(W.begin(),W.begin()+L,0ULL);
            long long rho=r0;
            for(long long k=0;k<s;k++){
                u64* dst=nxt+rho*LMAX;
                const u64* cv=cur+rho*LMAX;
                unsigned long long c1=0,c2=0;
                for(int i=0;i<L;i++){
                    u64 p=pre[i];
                    dst[i]=p;
                    W[i]=__builtin_addcll(W[i],p,c1,&c1);
                    pre[i]=__builtin_addcll(p,cv[i],c2,&c2);
                }
                rho++; if(rho==s) rho=0;
            }
            // base = (Cb[a]-W)/s
            {
                Big w(W.begin(),W.begin()+L); trim(w);
                Big b=subBig(Cb[a],w);
                b=divSmall(b,(u64)s);
                fill(base.begin(),base.begin()+L,0ULL);
                for(size_t i=0;i<b.size();i++) base[i]=b[i];
            }
            for(long long r=0;r<s;r++){
                u64* dst=nxt+r*LMAX; unsigned long long c=0;
                for(int i=0;i<L;i++) dst[i]=__builtin_addcll(dst[i],base[i],c,&c);
            }
            swap(cur,nxt);
        }
        long long cls=m%s;
        Big v(cur+cls*LMAX, cur+cls*LMAX+LMAX); trim(v);
        N[s]=v;
    }
    // binomials small
    vector<vector<Big>> Cs(n+1, vector<Big>(n+1));
    for(int a=0;a<=n;a++){
        Cs[a][0]=Big{1};
        for(int b=1;b<=a;b++){ Cs[a][b]=Cs[a-1][b-1]; if(b<=a-1) addTo(Cs[a][b],Cs[a-1][b]); }
    }
    const Big& Cmn = Cb[n-1];
    long long eC; long double vC=toLD(Cmn,eC);
    vector<long double> eb(n+1);
    for(int t=1;t<=n;t++){
        Big pos, neg;
        for(int s=t;s<=n;s++){
            Big coef=mulBig(Cs[s-1][t-1],Cs[n][s]);
            Big term=mulBig(coef,N[s]);
            if((s-t)%2==0) addTo(pos,term); else addTo(neg,term);
        }
        trim(pos); trim(neg);
        Big T = cmpBig(pos,neg)>=0 ? subBig(pos,neg) : Big();
        long long eT; long double vT=toLD(T,eT);
        long double r = vT/vC;
        r = ldexpl(r, (int)(64*(eT-eC)));
        eb[t]=r;
    }
    // i-th smallest corresponds to t=n-i+1
    long double acc=0;
    string out;
    char buf[64];
    for(int i=1;i<=n;i++){
        acc+=eb[n-i+1];
        snprintf(buf,sizeof buf,"%.10Lf\n",acc);
        out+=buf;
    }
    fputs(out.c_str(),stdout);
}
