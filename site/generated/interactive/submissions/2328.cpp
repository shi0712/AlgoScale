#include <bits/stdc++.h>
using namespace std;

typedef bitset<608> B;
int n;
unordered_map<B,int> cache_;
B U;
vector<B> mask;

void die(){ exit(0); }

int ask(const B& s){
    int c = s.count();
    if(c<=1) return 0;
    auto it = cache_.find(s);
    if(it!=cache_.end()) return it->second;
    string out = "? " + to_string(c) + "\n";
    bool first=true;
    for(int i=1;i<=n;i++) if(s[i]){
        if(!first) out+=' ';
        out += to_string(i);
        first=false;
    }
    out+='\n';
    fputs(out.c_str(), stdout);
    fflush(stdout);
    int x;
    if(scanf("%d",&x)!=1) die();
    if(x<0) die();
    cache_[s]=x;
    return x;
}

bool isCached(const B& s){
    if(s.count()<=1) return true;
    return cache_.count(s)>0;
}

void build(int node,int l,int r){
    B m; for(int i=l;i<=r;i++) m[i]=1;
    mask[node]=m;
    if(l==r) return;
    int mid=(l+r)/2;
    build(node*2,l,mid); build(node*2+1,mid+1,r);
}

int findN(int v,int node,int l,int r,int k){
    while(true){
        B S = U & mask[node];
        if(S.count()==1){
            for(int i=l;i<=r;i++) if(S[i]) return i;
        }
        int mid=(l+r)/2;
        B A = U & mask[node*2], Bs = U & mask[node*2+1];
        if(A.none()){ node=node*2+1; l=mid+1; continue; }
        if(Bs.none()){ node=node*2; r=mid; continue; }
        B Av=A; Av[v]=1; B Bv=Bs; Bv[v]=1;
        int costA = (isCached(A)?0:1) + (isCached(Av)?0:1);
        int costB = (isCached(Bs)?0:1) + (isCached(Bv)?0:1);
        int kA,kB;
        if(costA<=costB){
            kA = ask(Av)-ask(A); kB=k-kA;
        } else {
            kB = ask(Bv)-ask(Bs); kA=k-kB;
        }
        bool goA;
        if(kA>0 && kB>0){
            bool cA=isCached(A), cB=isCached(Bs);
            if(cA && !cB) goA=false;
            else if(!cA && cB) goA=true;
            else goA = A.count()<=Bs.count();
        } else goA = kA>0;
        if(goA){ node=node*2; r=mid; k=kA; }
        else { node=node*2+1; l=mid+1; k=kB; }
    }
}

int par[605], dep[605];

int main(){
    if(scanf("%d",&n)!=1) return 0;
    if(n==1){ printf("Y 1\n1\n"); fflush(stdout); return 0; }
    mask.assign(4*n+5,B());
    build(1,1,n);
    B full; for(int i=1;i<=n;i++) full[i]=1;
    U=full; U[1]=0;
    par[1]=0; dep[1]=0;
    vector<int> st; st.push_back(1);
    while(!st.empty()){
        if(U.none()) break;
        int v=st.back();
        B Uv=U; Uv[v]=1;
        int k = ask(Uv)-ask(U);
        if(k==0){ st.pop_back(); continue; }
        int u=findN(v,1,1,n,k);
        par[u]=v; dep[u]=dep[v]+1;
        U[u]=0;
        st.push_back(u);
    }
    int m = ask(full);
    vector<int> As,Bv;
    for(int i=1;i<=n;i++){ if(dep[i]%2==0) As.push_back(i); else Bv.push_back(i); }
    auto printY=[&](){
        string out="Y "+to_string(As.size())+"\n";
        for(size_t i=0;i<As.size();i++){ if(i) out+=' '; out+=to_string(As[i]); }
        out+='\n';
        fputs(out.c_str(),stdout); fflush(stdout);
    };
    if(m==n-1){ printY(); return 0; }
    auto toB=[&](const vector<int>& xs,int cnt){
        B s; for(int i=0;i<cnt;i++) s[xs[i]]=1; return s;
    };
    vector<int> X;
    B Am=toB(As,As.size());
    if(ask(Am)>0) X=As;
    else {
        B Bm=toB(Bv,Bv.size());
        if(ask(Bm)>0) X=Bv;
        else { printY(); return 0; }
    }
    // smallest i such that prefix [0..i] has an edge
    int len=X.size();
    int lo=1, hi=len-1;
    while(lo<hi){
        int md=(lo+hi)/2;
        if(ask(toB(X,md+1))>0) hi=md; else lo=md+1;
    }
    int x=X[lo];
    vector<int> cand(X.begin(),X.begin()+lo);
    while(cand.size()>1){
        int h=cand.size()/2;
        vector<int> half(cand.begin(),cand.begin()+h);
        B s=toB(half,half.size()); s[x]=1;
        if(ask(s)>0) cand=half;
        else cand=vector<int>(cand.begin()+h,cand.end());
    }
    int y=cand[0];
    vector<int> a,b;
    int p=x,q=y;
    while(dep[p]>dep[q]){ a.push_back(p); p=par[p]; }
    while(dep[q]>dep[p]){ b.push_back(q); q=par[q]; }
    while(p!=q){ a.push_back(p); p=par[p]; b.push_back(q); q=par[q]; }
    a.push_back(p);
    reverse(b.begin(),b.end());
    for(int z:b) a.push_back(z);
    string out="N "+to_string(a.size())+"\n";
    for(size_t i=0;i<a.size();i++){ if(i) out+=' '; out+=to_string(a[i]); }
    out+='\n';
    fputs(out.c_str(),stdout); fflush(stdout);
    return 0;
}
