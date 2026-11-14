#include <bits/stdc++.h>
using namespace std;
typedef long long ll; typedef unsigned long long ull;
static char buf[1<<25]; size_t blen=0,bpos=0;
inline int gc(){ if(bpos==blen){ blen=fread(buf,1,sizeof(buf),stdin); bpos=0; if(!blen) return -1;} return buf[bpos++]; }
inline ll rd(){ int c=gc(); while(c!='-'&&(c<'0'||c>'9')) c=gc(); ll x=0; while(c>='0'&&c<='9'){x=x*10+(c-'0');c=gc();} return x; }
int par[100005];
int fnd(int x){ while(par[x]!=x){par[x]=par[par[x]];x=par[x];} return x; }
int main(){
    int n=rd(), m=rd(), k=rd();
    vector<int> eu(m), ev(m);
    vector<ull> evs; evs.reserve((size_t)m+(size_t)n*k);
    for(int i=0;i<m;i++){ eu[i]=rd()-1; ev[i]=rd()-1; ull w=rd(); evs.push_back((w<<22)|i); }
    vector<ll> c(k);
    for(int j=0;j<k;j++){ c[j]=rd(); for(int i=0;i<n;i++){ ull w=rd(); evs.push_back((w<<22)|(ull)(m+j*n+i)); } }
    sort(evs.begin(),evs.end());
    int F=1<<k;
    vector<ll> g(F,0), Q(F,0);
    vector<int> comp(F,0), nb(F,0);
    int adj[16]={0};
    auto recompute=[&](){
        nb[0]=0;
        for(int s=1;s<F;s++){ int lb=__builtin_ctz(s); nb[s]=nb[s&(s-1)]|adj[lb]; }
        comp[0]=0;
        for(int s=1;s<F;s++){
            int r=s&-s;
            while(true){ int nr=r|(nb[r]&s); if(nr==r) break; r=nr; }
            comp[s]=comp[s&~r]+1;
        }
    };
    for(int j=0;j<k;j++) adj[j]=1<<j; // self loops harmless
    recompute();
    ll lastQ=0;
    vector<int> mask(n,0); vector<ll> last(n,0);
    for(int i=0;i<n;i++) par[i]=i;
    auto addEdges=[&](int A,int B,ll t){
        // connect all towns in A with all in B
        bool ch=false;
        for(int a=A;a;a&=a-1){ int j=__builtin_ctz(a); if(B&~adj[j]) { ch=true; break; } }
        if(!ch) return;
        for(int s=1;s<F;s++) Q[s]+=(t-lastQ)*comp[s];
        lastQ=t;
        for(int a=A;a;a&=a-1){ int j=__builtin_ctz(a); adj[j]|=B; }
        for(int b=B;b;b&=b-1){ int j=__builtin_ctz(b); adj[j]|=A; }
        recompute();
    };
    for(ull e:evs){
        ll t=(ll)(e>>22); int id=(int)(e&((1u<<22)-1));
        if(id<m){
            int a=fnd(eu[id]), b=fnd(ev[id]);
            if(a==b) continue;
            g[mask[a]]+=t-last[a]; g[mask[b]]+=t-last[b];
            int A=mask[a], B=mask[b];
            par[b]=a; mask[a]=A|B; last[a]=t;
            if(A&&B) addEdges(A,B,t);
        } else {
            int j=(id-m)/n, i=(id-m)%n;
            int a=fnd(i);
            if(mask[a]>>j&1) continue;
            g[mask[a]]+=t-last[a];
            int A=mask[a];
            mask[a]|=1<<j; last[a]=t;
            if(A) addEdges(A,1<<j,t);
        }
    }
    ll H=evs.empty()?1:(ll)(evs.back()>>22)+1;
    for(int i=0;i<n;i++) if(fnd(i)==i) g[mask[i]]+=H-last[i];
    for(int s=1;s<F;s++) Q[s]+=(H-lastQ)*comp[s];
    for(int b=0;b<k;b++) for(int s=0;s<F;s++) if(s>>b&1) g[s]+=g[s^(1<<b)];
    ll best=LLONG_MAX;
    for(int s=0;s<F;s++){
        ll cs=0; for(int j=0;j<k;j++) if(s>>j&1) cs+=c[j];
        best=min(best,cs+g[(F-1)&~s]+Q[s]-H);
    }
    printf("%lld\n",best);
}
