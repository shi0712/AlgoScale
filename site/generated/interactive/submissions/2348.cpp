#include <bits/stdc++.h>
using namespace std;
int n;
vector<vector<int>> adj;
vector<long long> f0;
vector<int> cand;
vector<char> alive;

long long ask(const vector<int>& S){
    printf("? 1 %d",(int)S.size());
    for(int x:S) printf(" %d",x);
    printf("\n"); fflush(stdout);
    long long r; if(scanf("%lld",&r)!=1) exit(0);
    if(r==-1) exit(0);
    return r;
}
int main(){
    if(scanf("%d",&n)!=1) return 0;
    adj.assign(n+1,{});
    for(int i=0;i<n-1;i++){int a,b;scanf("%d %d",&a,&b);adj[a].push_back(b);adj[b].push_back(a);}
    f0.assign(n+1,0);cand.assign(n+1,0);
    for(int u=1;u<=n;u++){f0[u]=ask({u}); cand[u]=(llabs(f0[u])==1);}
    alive.assign(n+1,1);
    vector<int> toggled;
    int root=-1;
    int start=1;
    while(true){
        // collect component from start
        vector<int> comp; vector<int> par(n+1,0);
        {
            vector<int> st={start}; par[start]=-1;
            vector<char> vis(n+1,0); vis[start]=1;
            while(!st.empty()){int u=st.back();st.pop_back();comp.push_back(u);
                for(int v:adj[u]) if(alive[v]&&!vis[v]){vis[v]=1;par[v]=u;st.push_back(v);}}
        }
        int total=0; for(int u:comp) total+=cand[u];
        if(total==1){ for(int u:comp) if(cand[u]) root=u; break; }
        // subtree sums in reverse order
        vector<int> sub(n+1,0);
        for(int i=(int)comp.size()-1;i>=0;i--){int u=comp[i]; sub[u]+=cand[u]; if(par[u]>0) sub[par[u]]+=sub[u];}
        int best=-1,bestVal=INT_MAX;
        for(int u:comp){
            int mx=total-sub[u];
            for(int v:adj[u]) if(alive[v]&&par[v]==u) mx=max(mx,sub[v]);
            if(mx<bestVal){bestVal=mx;best=u;}
        }
        int c=best;
        // options
        vector<pair<int,int>> opts; // (weight, neighbor)
        for(int v:adj[c]) if(alive[v]){
            int w = (par[v]==c)? sub[v] : total-sub[c];
            if(w>0) opts.push_back({w,v});
        }
        bool cRoot = cand[c];
        int nopt=(int)opts.size()+(cRoot?1:0);
        int chosen=-1; // neighbor or 0 means c root
        if(nopt==1){
            if(cRoot) chosen=0; else chosen=opts[0].second;
        } else {
            printf("? 2 %d\n",c); fflush(stdout);
            toggled.push_back(c);
            while(true){
                int no=(int)opts.size()+(cRoot?1:0);
                if(no==1){ chosen = cRoot?0:opts[0].second; break; }
                sort(opts.rbegin(),opts.rend());
                int tw=(cRoot?1:0); for(auto&p:opts) tw+=p.first;
                vector<pair<int,int>> S,R; int sw=0;
                for(size_t i=0;i<opts.size();i++){
                    bool canAdd = true;
                    // ensure complement nonempty
                    if(!cRoot && i==opts.size()-1 && R.empty()) canAdd=false;
                    if(canAdd && (S.empty() || sw + opts[i].first <= tw/2 + (tw%2))){
                        if(S.empty() || 2*(sw+opts[i].first) <= tw+opts[i].first){ S.push_back(opts[i]); sw+=opts[i].first; continue;}
                    }
                    R.push_back(opts[i]);
                }
                if(!cRoot && R.empty()){ R.push_back(S.back()); S.pop_back(); }
                vector<int> q; long long base=0;
                for(auto&p:S){q.push_back(p.second); base+=f0[p.second];}
                long long r=ask(q);
                long long cnt=llabs(r-base)/2;
                if(cnt==(long long)S.size()-1){ opts=S; cRoot=false; }
                else { opts=R; }
            }
        }
        if(chosen==0){ root=c; break; }
        // restrict alive to component of chosen excluding c
        alive[c]=0;
        // mark all comp nodes dead except those in chosen's component
        {
            vector<char> keep(n+1,0);
            vector<int> st={chosen}; keep[chosen]=1;
            while(!st.empty()){int u=st.back();st.pop_back();
                for(int v:adj[u]) if(alive[v]&&!keep[v]){keep[v]=1;st.push_back(v);}}
            for(int u:comp) if(!keep[u]) alive[u]=0;
        }
        start=chosen;
    }
    // compute values
    vector<int> par(n+1,0), val(n+1,0);
    vector<int> st={root}; par[root]=-1;
    vector<char> vis(n+1,0); vis[root]=1;
    val[root]=(int)f0[root];
    while(!st.empty()){int u=st.back();st.pop_back();
        for(int v:adj[u]) if(!vis[v]){vis[v]=1;par[v]=u;val[v]=(int)(f0[v]-f0[u]);st.push_back(v);}}
    for(int t:toggled) val[t]=-val[t];
    printf("!");
    for(int u=1;u<=n;u++) printf(" %d",val[u]);
    printf("\n"); fflush(stdout);
    return 0;
}
