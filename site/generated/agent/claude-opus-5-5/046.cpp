#include <bits/stdc++.h>
using namespace std;
typedef long long ll; typedef unsigned long long ull;
static int L, M_;
static vector<int> beta_; static vector<ll> off_, xs; static vector<char> isLeaf;
static vector<vector<pair<int,ll>>> rho;
static ull *slopeA, *icptA;
static int LEAFN;
void build(int v,int lo,int hi,const vector<ll>&X){
    if(hi-lo==1){isLeaf[v]=1;xs[v]=X[lo];return;}
    int mid=(lo+hi)/2;build(2*v,lo,mid,X);build(2*v+1,mid,hi,X);
}
static inline ll evalFrom(int v,ll l){
    ll acc=0;
    while(!isLeaf[v]){ v = (l<beta_[v])?2*v:2*v+1; acc+=off_[v]; }
    return l*acc - xs[v];
}
int computeBeta(int v){
    int lo=1,hi=L+1;
    int a=2*v,b=2*v+1; ll ca=off_[a],cb=off_[b];
    while(true){
        if(lo==hi) return lo;
        while(!isLeaf[a] && (beta_[a]<=lo || beta_[a]>=hi)){ a = beta_[a]<=lo?2*a+1:2*a; ca+=off_[a]; }
        while(!isLeaf[b] && (beta_[b]<=lo || beta_[b]>=hi)){ b = beta_[b]<=lo?2*b+1:2*b; cb+=off_[b]; }
        if(isLeaf[a]&&isLeaf[b]){
            ll d=cb-ca, dx=xs[b]-xs[a];
            if(d<=0) return hi;
            ll t=dx/d+1;
            if(t<lo) t=lo; if(t>hi) t=hi;
            return (int)t;
        }
        int m = !isLeaf[a]?beta_[a]:beta_[b];
        ll g = evalFrom(b,m)+(ll)m*cb - evalFrom(a,m)-(ll)m*ca;
        if(g>0) hi=m; else lo=m+1;
    }
}
static inline void give(int c,int p,ll d){
    if(p>L||d==0) return;
    if(off_[c]) slopeA[p]+=(ull)d*(ull)off_[c];
    if(isLeaf[c]) icptA[p]-=(ull)xs[c]*(ull)d;
    else rho[c].push_back({p,d});
}
void push(int v){
    if(rho[v].empty()) return;
    vector<pair<int,ll>> lst; lst.swap(rho[v]);
    int b=beta_[v]; ll A=0;
    for(auto&[p,d]:lst){ if(p<b){A+=d; give(2*v,p,d);} else give(2*v+1,p,d); }
    if(A!=0 && b<=L){ give(2*v,b,-A); give(2*v+1,b,A); }
}
void flushAll(int v){
    if(isLeaf[v]) return;
    push(v); flushAll(2*v); flushAll(2*v+1);
}
static char buf[1<<25]; int bufpos=0,buflen=0;
inline int gc(){ if(bufpos==buflen){buflen=fread(buf,1,sizeof(buf),stdin);bufpos=0; if(buflen<=0) return -1;} return buf[bufpos++]; }
inline ll rd(){ int c=gc(); while(c!='-'&&(c<'0'||c>'9')) c=gc(); bool neg=false; if(c=='-'){neg=true;c=gc();} ll r=0; while(c>='0'&&c<='9'){r=r*10+(c-'0');c=gc();} return neg?-r:r; }
int main(){
    int n=rd(); L=rd(); ll T=rd();
    vector<ll> S(n),H(n),P(n);
    for(int i=0;i<n;i++){S[i]=rd();H[i]=rd();P[i]=rd();}
    vector<ll> sv(S.begin(),S.end()); sort(sv.begin(),sv.end()); sv.erase(unique(sv.begin(),sv.end()),sv.end());
    // leaves sorted by x = T - s ascending: origin(s=T), then s descending
    int m=sv.size()+1;
    vector<ll> X(m); X[0]=0; for(int j=1;j<m;j++) X[j]=T-sv[m-1-j];
    auto leafIdx=[&](ll s){ int pos=lower_bound(sv.begin(),sv.end(),s)-sv.begin(); return m-1-pos; };
    int sz=4*m+4;
    beta_.assign(sz,L+1); off_.assign(sz,0); xs.assign(sz,0); isLeaf.assign(sz,0); rho.assign(sz,{});
    build(1,0,m,X);
    slopeA=(ull*)calloc(L+2,sizeof(ull)); icptA=(ull*)calloc(L+2,sizeof(ull));
    vector<int> ord(n); iota(ord.begin(),ord.end(),0);
    sort(ord.begin(),ord.end(),[&](int a,int b){return P[a]>P[b];});
    vector<int> path; path.reserve(64);
    for(int t=0;t<n;t++){
        int i=ord[t]; int k=leafIdx(S[i]); ll h=H[i];
        int v=1,lo=0,hi=m; path.clear();
        while(!isLeaf[v]){
            push(v); path.push_back(v);
            int mid=(lo+hi)/2;
            if(k<mid){ off_[2*v+1]+=h; v=2*v; hi=mid; }
            else { v=2*v+1; lo=mid; }
        }
        off_[v]+=h;
        for(int j=(int)path.size()-1;j>=0;j--) beta_[path[j]]=computeBeta(path[j]);
        ll w=P[i]-(t+1<n?P[ord[t+1]]:0);
        if(w>0) rho[1].push_back({1,w});
    }
    flushAll(1);
    // compute f
    ull s1=0,s2=0;
    for(int l=1;l<=L;l++){ s1+=slopeA[l]; s2+=icptA[l]; slopeA[l]=(ull)l*s1+s2; }
    int q=rd();
    string out; out.reserve((size_t)q*9);
    int cur=0; // max l with f(l)<=M
    char tmp[24];
    for(int j=0;j<q;j++){
        ll Mq=rd();
        while(cur<L && (ll)slopeA[cur+1]<=Mq) cur++;
        int len=0; int x=cur; if(x==0) tmp[len++]='0'; while(x){tmp[len++]='0'+x%10;x/=10;}
        while(len) out.push_back(tmp[--len]); out.push_back('\n');
    }
    fwrite(out.data(),1,out.size(),stdout);
}
