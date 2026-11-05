#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdint>
#include <vector>
#include <cassert>
using namespace std;
using i64 = long long;

// All traversal is iterative. No stored answers or shape-specific formulas.
class FastInput {
    static constexpr int S = 1 << 16;
    char a[S]; int p=0,z=0;
    int gc(){if(p==z){z=(int)fread(a,1,S,stdin);p=0;if(!z)return EOF;}return a[p++];}
public:
    bool read(int &x){int c;do{c=gc();if(c==EOF)return false;}while(c<=32);x=0;do{x=x*10+c-'0';c=gc();}while(c>32&&c!=EOF);return true;}
};

class Solver {
    int n,H,B,L;
    vector<int> par,head,nxt,sz,ht,dep,heavy,pos;
    vector<i64> base;
    // The high-layer skeleton: a region consists of a unary big-node chain
    // (including its bottom branching node) and attached small subtrees.
    int R=0, stride=0;
    vector<int> euler,tin,rid,roots,rend,bottom,jump;
    vector<i64> jumpSum, ftab, gtab, potential;
    i64 &ff(int r,int j){return ftab[(size_t)r*stride+j];}
    i64 &gg(int r,int j){return gtab[(size_t)r*stride+j];}
    i64 &pp(int r,int j){return potential[(size_t)r*stride+j];}
    i64 getG(int u,int j) {
        const int b=jump[u];
        if(!b)return base[u];
        const int k=j+dep[b]-dep[u];
        if(k>H)return base[u];
        return max(base[u],jumpSum[u]+gg(rid[b],k));
    }
    void highLayers(vector<i64> &nextG) {
        vector<int> bigCount(n+1), bigSon(n+1);
        for(int u=n;u>=1;--u)
            for(int v=head[u];v;v=nxt[v])if(sz[v]>=B){++bigCount[u];bigSon[u]=v;}
        euler.reserve(n);tin.resize(n+1);rid.resize(n+1);
        vector<int> st(1,1);
        while(!st.empty()){
            int u=st.back();st.pop_back();tin[u]=(int)euler.size();euler.push_back(u);
            if(u==1||(sz[u]>=B&&bigCount[par[u]]>=2)){
                rid[u]=R++;roots.push_back(u);bottom.push_back(0);
            }else rid[u]=rid[par[u]];
            if(sz[u]>=B&&bigCount[u]!=1)bottom[rid[u]]=u;
            for(int v=head[u];v;v=nxt[v])st.push_back(v);
        }
        vector<int> rootTin(R);
        for(int r=0;r<R;++r)rootTin[r]=tin[roots[r]];
        rend.resize(R);
        for(int r=0;r<R;++r)rend[r]=(int)(lower_bound(rootTin.begin(),rootTin.end(),tin[roots[r]]+sz[roots[r]])-rootTin.begin());
        jump.assign(n+1,0);jumpSum.assign(n+1,0);
        for(int u=n;u>=1;--u){
            if(bigCount[u]>=2)jump[u]=u;
            else if(bigCount[u]==1){int v=bigSon[u];jump[u]=jump[v];jumpSum[u]=(i64)sz[v]+jumpSum[v];}
        }
        stride=H+2;
        ftab.assign((size_t)R*stride,0);
        gtab.assign((size_t)R*stride,0);
        potential.assign((size_t)R*stride,0);
        vector<int> children;vector<i64> sumF(stride);
        for(int r=R-1;r>=0;--r){
            int b=bottom[r];assert(b!=0);
            children.clear();i64 smallBest=0;
            int maxh1=0,maxh2=0,who=-1;
            for(int v=head[b];v;v=nxt[v]){
                if(sz[v]>=B){
                    children.push_back(v);
                    int h=ht[v]+1;
                    if(h>maxh1){maxh2=maxh1;maxh1=h;who=v;}else maxh2=max(maxh2,h);
                }else smallBest=max(smallBest,(i64)sz[v]+base[v]);
            }
            fill(sumF.begin(),sumF.end(),0);
            for(int v:children)for(int j=L+1;j<=ht[v]+1;++j)sumF[j]+=ff(rid[v],j);
            for(int j=L+1;j<=H;++j){
                i64 best=smallBest;
                for(int v:children)best=max(best,(i64)sz[v]+getG(v,j+1)-ff(rid[v],j));
                gg(r,j)=sumF[j]+best;
            }
            // Add sibling potentials to all descendant regions. For a fixed
            // destination region, the sibling subtrees charged by its ancestors
            // are pairwise disjoint. Total work is O(n*R), not O(H*R^2).
            for(int v:children){
                int c=rid[v],last=(v==who?maxh2:maxh1);
                for(int s=c;s<rend[c];++s)
                    for(int j=L+1;j<=last;++j)pp(s,j)+=sumF[j]-ff(c,j);
            }
            const int u=roots[r], end=tin[u]+sz[u];
            for(int i=tin[u];i<end;++i){
                int x=euler[i],j=dep[x]-dep[u]+1;
                if(j>L)ff(r,j)=max(ff(r,j),getG(x,j)+pp(rid[x],j));
            }
        }
        for(int u=1;u<=n;++u)nextG[u]=getG(u,L+1);
        // High-layer tables are not needed in the low-layer phase.
        vector<i64>().swap(ftab);vector<i64>().swap(gtab);vector<i64>().swap(potential);
    }
public:
    explicit Solver(vector<int> p):n((int)p.size()-1),par(move(p)),head(n+1),nxt(n+1),sz(n+1,1),ht(n+1),dep(n+1),heavy(n+1),pos(n+1),base(n+1){
        for(int u=2;u<=n;++u){nxt[u]=head[par[u]];head[par[u]]=u;dep[u]=dep[par[u]]+1;}
        for(int u=n;u>=1;--u){
            for(int v=head[u];v;v=nxt[v]){
                sz[u]+=sz[v];ht[u]=max(ht[u],ht[v]+1);
                base[u]=max(base[u],(i64)sz[v]+base[v]);
                if(!heavy[u]||ht[v]>ht[heavy[u]])heavy[u]=v;
            }
        }
        H=ht[1]; B=1;while(1LL*B*B<n)++B;
#ifdef TEST_BLOCK
        B=TEST_BLOCK; // Test builds only; forces high-layer coverage.
#endif
        L=min(H,2*B);
        int used=0;
        for(int u=1;u<=n;++u)if(u==1||heavy[par[u]]!=u)
            for(int v=u;v;v=heavy[v])pos[v]=used++;
        assert(used==n);
    }
    i64 solve(){
        vector<i64> nextG=base;
        if(H>L)highLayers(nextG);
        vector<i64> pool(n),delta(n+1),F(n+1),curG(n+1);
        // Aggregate leaf children, which all have F=G=0 and size 1.
        vector<int> nh(n+1), nn(n+1), internal;
        vector<unsigned char> leaf(n+1),lightLeaf(n+1);
        internal.reserve(n);
        for(int u=n;u>=1;--u)if(head[u]){
            internal.push_back(u);
            for(int v=head[u];v;v=nxt[v]){
                if(head[v]){nn[v]=nh[u];nh[u]=v;}
                else{leaf[u]=1;if(v!=heavy[u])lightLeaf[u]=1;}
            }
        }
        for(int j=L;j>=1;--j){
            fill(pool.begin(),pool.end(),0);
            for(int u:internal){
                i64 sum=0,best=leaf[u]?1:0;
                for(int v=nh[u];v;v=nn[v]){
                    sum+=F[v];best=max(best,(i64)sz[v]+nextG[v]-F[v]);
                }
                i64 gu=sum+best;curG[u]=gu;
                int s=heavy[u];
                delta[u]=delta[s]+sum-F[s];
                pool[pos[u]]=gu-delta[u];
                if(lightLeaf[u])pool[pos[u]+1]=max(pool[pos[u]+1],sum-delta[u]);
                for(int v=nh[u];v;v=nn[v])if(v!=s){
                    i64 off=delta[v]+sum-F[v]-delta[u];
                    for(int k=0;k<=ht[v];++k)
                        pool[pos[u]+k+1]=max(pool[pos[u]+k+1],pool[pos[v]+k]+off);
                }
                F[u]=j-1<=ht[u]?max<i64>(0,pool[pos[u]+j-1]+delta[u]):0;
            }
            nextG.swap(curG);
        }
        return (i64)n+nextG[1];
    }
};

#ifndef TREE_VALUE_NO_MAIN
int main(){
    FastInput in;int T;if(!in.read(T))return 0;
    while(T--){int n=0,m=0;in.read(n);in.read(m);vector<int> p(n+1);for(int u=2;u<=n;++u)in.read(p[u]);printf("%lld\n",Solver(move(p)).solve());}
    return 0;
}
#endif
