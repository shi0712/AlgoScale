#include <bits/stdc++.h>
using namespace std;
static int n;
static vector<int> h, lc, rc, ds, hr;
static int meld(int a,int b){
    if(!a) return b; if(!b) return a;
    if(h[a]<h[b]) swap(a,b);
    rc[a]=meld(rc[a],b);
    if(ds[lc[a]]<ds[rc[a]]) swap(lc[a],rc[a]);
    ds[a]=ds[rc[a]]+1;
    return a;
}
static inline void pushv(int v,int x){ lc[x]=rc[x]=0; ds[x]=1; hr[v]=meld(hr[v],x); }
static inline void popv(int v){ int t=hr[v]; hr[v]=meld(lc[t],rc[t]); lc[t]=rc[t]=0; ds[t]=1; }
static vector<array<int,4>> st;
static int mergePath(int x,int y){
    st.clear();
    while(hr[x]&&hr[y]){
        int cx=hr[x],cy=hr[y];
        popv(x); popv(y);
        st.push_back({x,y,cx,cy});
        x=cx;y=cy;
    }
    int cur=x;
    while(!st.empty()){
        auto e=st.back(); st.pop_back();
        int px=e[0],py=e[1];
        hr[px]=meld(hr[px],hr[py]);
        hr[py]=0;
        pushv(px,cur);
        cur=px;
    }
    return cur;
}
int main(){
    int T; if(scanf("%d",&T)!=1) return 0;
    while(T--){
        if(scanf("%d",&n)!=1) return 0;
        vector<int> eu(max(0,n-1)),ev(max(0,n-1)),deg(n+2,0);
        for(int i=0;i<n-1;i++){
            if(scanf("%d %d",&eu[i],&ev[i])!=2) return 0;
            deg[eu[i]]++;deg[ev[i]]++;
        }
        if(n==1){puts("0");continue;}
        vector<int> start(n+2,0);
        for(int i=1;i<=n;i++) start[i+1]=start[i]+deg[i];
        vector<int> adj(2*(n-1)),pos(start.begin(),start.end());
        for(int i=0;i<n-1;i++){adj[pos[eu[i]]++]=ev[i];adj[pos[ev[i]]++]=eu[i];}
        h.assign(n+1,0);lc.assign(n+1,0);rc.assign(n+1,0);ds.assign(n+1,1);hr.assign(n+1,0);
        ds[0]=0;
        vector<int> order,par(n+1,0);
        order.reserve(n);order.push_back(1);par[1]=0;
        for(size_t i=0;i<order.size();i++){
            int v=order[i];
            for(int k=start[v];k<start[v+1];k++){
                int u=adj[k];
                if(u!=par[v]){par[u]=v;order.push_back(u);}
            }
        }
        for(int i=n-1;i>=1;i--){
            int v=order[i];
            h[v]=hr[v]?h[hr[v]]+1:0;
            pushv(par[v],v);
        }
        int root=1;
        long long folds=0;
        vector<int> path;
        while(hr[root]){
            int t=hr[root];
            int k1=h[t];
            int sec=0;
            if(lc[t]||rc[t]){
                int a=lc[t],b=rc[t];
                if(a&&b) sec=(h[a]>=h[b])?a:b; else sec=a?a:b;
            }
            int k2=sec?h[sec]:-1;
            if(k1-k2<=1){
                folds++;
                if(k1==k2){
                    int u1=hr[root]; popv(root);
                    int u2=hr[root]; popv(root);
                    int w=mergePath(u1,u2);
                    pushv(root,w);
                } else {
                    int b=t; popv(root);
                    if(!hr[root]){
                        hr[root]=hr[b];
                    } else {
                        int q=hr[root]; popv(root);
                        int bp=hr[b]; popv(b);
                        int w=mergePath(q,bp);
                        hr[root]=meld(hr[root],hr[b]);
                        hr[b]=0;
                        pushv(root,w);
                    }
                }
            } else {
                int r=k1+1;
                int D=(k2>=0)?r+k2+1:r;
                path.clear();
                int v=root,j=0;
                while(true){
                    path.push_back(v);
                    int tt=hr[v];
                    if(!tt)break;
                    if(j>=1){
                        int a=lc[tt],b=rc[tt];
                        int s=-1;
                        if(a)s=h[a];
                        if(b)s=max(s,h[b]);
                        int cand=(r-j)+(s>=0?s+1:0);
                        D=max(D,cand);
                    }
                    v=tt;j++;
                }
                int z;
                if(D%2==0) z=r-D/2; else z=r-(D+1)/2;
                if(z<=0) break; // cannot happen
                for(int i=0;i<z;i++) popv(path[i]);
                for(int i=0;i<z;i++){
                    int p=path[i];
                    h[p]=hr[p]?h[hr[p]]+1:0;
                    pushv(path[i+1],p);
                }
                root=path[z];
            }
        }
        printf("%lld\n",folds);
    }
    return 0;
}
