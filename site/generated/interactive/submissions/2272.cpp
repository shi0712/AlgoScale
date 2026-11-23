#include <bits/stdc++.h>
using namespace std;

static int N, M;
static vector<int> par, rel_, sz0, sz1, tot;

static int findp(int x){
    if(par[x]==x) return x;
    int p=par[x]; int r=findp(p);
    rel_[x]^=rel_[p]; par[x]=r; return r;
}
static void unite(int x,int y,int d){
    int rx=findp(x), ry=findp(y);
    int px=rel_[x], py=rel_[y];
    if(rx==ry) return;
    int L=px^py^d;
    if(tot[rx]<tot[ry]) swap(rx,ry);
    par[ry]=rx; rel_[ry]=L;
    if(L==0){ sz0[rx]+=sz0[ry]; sz1[rx]+=sz1[ry]; }
    else{ sz0[rx]+=sz1[ry]; sz1[rx]+=sz0[ry]; }
    tot[rx]+=tot[ry];
}
static int ask(int x,int y){
    printf("? %d %d\n", x+1, y+1); fflush(stdout);
    int r; if(scanf("%d",&r)!=1) exit(0);
    if(r!=0&&r!=1) exit(0); return r;
}
struct Res{ bool solved=false; int genuine=-1,counterfeit=-1;
    vector<int> undet,rep,repBig,forced; vector<char> bal; };
static Res deduce(){
    Res R; R.forced.assign(N,-1); R.rep.assign(N,-1); R.repBig.assign(N,-1); R.bal.assign(N,0);
    for(int i=0;i<N;i++){ int r=findp(i);
        if(R.rep[r]<0) R.rep[r]=i;
        int mine=rel_[i]==0?sz0[r]:sz1[r], other=rel_[i]==0?sz1[r]:sz0[r];
        if(mine>=other&&R.repBig[r]<0) R.repBig[r]=i; }
    vector<int> roots; for(int i=0;i<N;i++) if(par[i]==i) roots.push_back(i);
    long long base=0; vector<pair<int,int>> items; vector<int> sp(N,0);
    for(int r:roots){ int s0=sz0[r],s1=sz1[r];
        bool f0=(s0<=M&&s1<=N-M), f1=(s1<=M&&s0<=N-M);
        if(f0&&!f1){R.forced[r]=0;base+=s0;}
        else if(f1&&!f0){R.forced[r]=1;base+=s1;}
        else if(f0&&f1){ base+=min(s0,s1);
            if(s0==s1) R.bal[r]=1; else{ sp[r]=(s0<s1?0:1); items.push_back(make_pair(r,abs(s0-s1))); } } }
    int T=M-(int)base; if(T<0)T=0;
    vector<pair<int,int>> act;
    for(size_t i=0;i<items.size();i++){ if(items[i].second<=T) act.push_back(items[i]); else R.forced[items[i].first]=sp[items[i].first]; }
    int K=(int)act.size(), W=T+1;
    vector<unsigned char> pre((size_t)(K+1)*W,0), suf((size_t)(K+1)*W,0);
    pre[0]=1;
    for(int i=0;i<K;i++){ int d=act[i].second; const unsigned char* pi=&pre[(size_t)i*W]; unsigned char* po=&pre[(size_t)(i+1)*W];
        for(int t=0;t<=T;t++){ unsigned char v=pi[t]; if(!v&&t>=d&&pi[t-d]) v=1; po[t]=v; } }
    suf[(size_t)K*W]=1;
    for(int i=K-1;i>=0;i--){ int d=act[i].second; const unsigned char* si=&suf[(size_t)(i+1)*W]; unsigned char* so=&suf[(size_t)i*W];
        for(int t=0;t<=T;t++){ unsigned char v=si[t]; if(!v&&t>=d&&si[t-d]) v=1; so[t]=v; } }
    for(int i=0;i<K;i++){ int r=act[i].first,d=act[i].second;
        const unsigned char* pi=&pre[(size_t)i*W]; const unsigned char* si=&suf[(size_t)(i+1)*W];
        bool ex=false,in=false;
        for(int t=0;t<=T;t++) if(pi[t]&&si[T-t]){ex=true;break;}
        for(int t=0;t+d<=T;t++) if(pi[t]&&si[T-d-t]){in=true;break;}
        if(!ex) R.forced[r]=1-sp[r]; else if(!in) R.forced[r]=sp[r]; }
    for(size_t i=0;i<roots.size();i++) if(R.forced[roots[i]]<0) R.undet.push_back(roots[i]);
    R.solved=R.undet.empty();
    for(int i=0;i<N;i++){ int r=par[i];
        if(R.forced[r]>=0){ if(rel_[i]==R.forced[r]){ if(R.counterfeit<0)R.counterfeit=i; } else { if(R.genuine<0)R.genuine=i; } }
        if(R.genuine>=0&&R.counterfeit>=0) break; }
    return R;
}
static void answer(const Res&R){
    printf("!");
    for(int i=0;i<N;i++){ int r=par[i]; if(R.forced[r]>=0&&rel_[i]==R.forced[r]) printf(" %d", i+1); }
    printf("\n"); fflush(stdout);
}
int main(){
    if(scanf("%d %d",&N,&M)!=2) return 0;
    par.resize(N); iota(par.begin(),par.end(),0);
    rel_.assign(N,0); sz0.assign(N,1); sz1.assign(N,0); tot.assign(N,1);
    Res R=deduce();
    if(R.solved){ answer(R); return 0; }
    vector<int> ord(N); iota(ord.begin(),ord.end(),0);
    unsigned seed=(unsigned)chrono::steady_clock::now().time_since_epoch().count();
    mt19937 rng(seed); shuffle(ord.begin(),ord.end(),rng);
    int np=N/2;
    for(int i=0;i<np;i++){
        int x=ord[2*i],y=ord[2*i+1];
        int d=ask(x,y); unite(x,y,d);
        R=deduce();
        if(R.solved){ answer(R); return 0; }
        bool allDet=true;
        for(int j=2*i+2;j<N;j++) if(R.forced[par[ord[j]]]<0){ allDet=false; break; }
        if(allDet) break;
    }
    while(true){
        R=deduce();
        if(R.solved) break;
        int x=-1,y=-1;
        if(R.genuine>=0||R.counterfeit>=0){
            int ref=(R.genuine>=0?R.genuine:R.counterfeit);
            int best=-1; long long bs=-1;
            for(size_t i=0;i<R.undet.size();i++){ int r=R.undet[i];
                long long sc=(long long)tot[r]*2+(R.bal[r]?0:1);
                if(sc>bs){bs=sc;best=r;} }
            if(best<0) break;
            x=R.rep[best]; y=ref;
        } else {
            int b1=-1,b2=-1; long long s1=-1,s2=-1;
            for(size_t i=0;i<R.undet.size();i++){ int r=R.undet[i];
                long long sc=(long long)tot[r]*2+(R.bal[r]?0:1);
                if(sc>s1){ s2=s1;b2=b1;s1=sc;b1=r; }
                else if(sc>s2){ s2=sc;b2=r; } }
            if(b1<0||b2<0) break;
            x=R.repBig[b1]; y=R.repBig[b2];
        }
        if(x<0||y<0||x==y) break;
        int d=ask(x,y); unite(x,y,d);
    }
    answer(R);
    return 0;
}
