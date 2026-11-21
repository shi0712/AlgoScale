#include <bits/stdc++.h>
using namespace std;

constexpr int N = 1000;
using B = bitset<N>;
using U = uint64_t;

int n;
mt19937 rng(73192657);
vector<B> poss, knownE;
vector<vector<B>> history, clauses;
vector<pair<int,int>> edges;
vector<int> parent0, sz;
vector<B> component;

int root(int a) {
    while (parent0[a] != a) a = parent0[a] = parent0[parent0[a]];
    return a;
}
void join0(int a, int b) {
    a = root(a); b = root(b);
    if (a == b) return;
    if (sz[a] < sz[b]) swap(a,b);
    parent0[b] = a;
    sz[a] += sz[b];
    component[a] |= component[b];
}
void confirm(int a, int b) {
    if (knownE[a][b]) return;
    knownE[a].set(b); knownE[b].set(a);
    edges.emplace_back(a,b);
    join0(a,b);
}

int smallrand(int k) { return (uint64_t(uint32_t(rng())) * k) >> 32; }

int pick(const B& s) {
    int c = s.count();
    if (c > n/8) {
        for (;;) {
            int j = smallrand(n);
            if (s[j]) return j;
        }
    }
    int k = smallrand(c);
    int j = s._Find_first();
    while (k--) j = s._Find_next(j);
    return j;
}

void reduceGraph() {
    while ((int)edges.size() < n-1) {
        for (int i=0;i<n;i++) poss[i] &= ~component[root(i)];
        clauses.assign(n,{});
        vector<pair<int,int>> force;
        vector<int> lb(n);
        for (int i=0;i<n;i++) {
            auto &v = clauses[i];
            vector<pair<int,B>> t;
            for (const B& q: history[i]) {
                if ((q & knownE[i]).any()) continue;
                B s = q & poss[i];
                t.emplace_back(s.count(),s);
            }
            if (knownE[i].none()) t.emplace_back(poss[i].count(),poss[i]);
            sort(t.begin(),t.end(),[](const auto& x,const auto& y){return x.first<y.first;});
            B used, common = poss[i];
            for (auto &[c,s]: t) {
                if (c == 1) {
                    force.emplace_back(i,s._Find_first());
                    break;
                }
                bool redundant=false;
                for (const B& h:v) if ((h & ~s).none()) {redundant=true;break;}
                if (redundant) continue;
                v.push_back(s);
                common &= s;
                if ((used & s).none()) { ++lb[i]; used |= s; }
            }
            if (!v.empty() && common.none()) lb[i]=max(lb[i],2);
        }
        if (!force.empty()) {
            for (auto [i,j]:force) confirm(i,j);
            continue;
        }

        int slack=2*(n-1-(int)edges.size())-accumulate(lb.begin(),lb.end(),0);
        bool changed=false;
        if (slack<=2) {
            for (int i=0;i<n;i++) {
                int cap=lb[i]+slack;
                B bad;
                if (cap==0) bad=poss[i];
                if (cap==1)
                    for (auto &s:clauses[i]) bad |= poss[i] & ~s;
                if (cap==2 && clauses[i].size()>=2) {
                    for (int j=poss[i]._Find_first();j<n;j=poss[i]._Find_next(j)) {
                        B p=poss[i];
                        for (auto &s:clauses[i]) if (!s[j]) p &= s;
                        if (p.none()) bad.set(j);
                    }
                }
                if (bad.any()) {
                    changed=true;
                    poss[i] &= ~bad;
                    for (int j=bad._Find_first();j<n;j=bad._Find_next(j)) poss[j].reset(i);
                }
            }
        }
        if (changed) continue;
        vector<int> tin(n),low(n);
        vector<B> adj(n);
        for (int i=0;i<n;i++) adj[i]=poss[i]|knownE[i];
        int timer=0;
        auto dfs=[&](auto&& self,int u,int p)->void {
            tin[u]=low[u]=++timer;
            for (int v=adj[u]._Find_first();v<n;v=adj[u]._Find_next(v)) {
                if (v==p) continue;
                if (tin[v]) low[u]=min(low[u],tin[v]);
                else {
                    self(self,v,u);
                    low[u]=min(low[u],low[v]);
                    if (low[v]>tin[u] && !knownE[u][v]) force.emplace_back(u,v);
                }
            }
        };
        dfs(dfs,0,-1);
        if (force.empty()) break;
        for (auto [i,j]:force) confirm(i,j);
    }
}

// Construct a collection of plausible completions, used ONLY to choose probes.
B plan() {
    vector<vector<int>> to(n);
    vector<U> masks(n*n);
    auto add=[&](int u,int v,U m) {
        U &w=masks[u*n+v];
        if (!w) to[u].push_back(v);
        w |= m;
    };
    for (auto [u,v]:edges) {add(u,v,~U(0));add(v,u,~U(0));}
    vector<int> order(n);
    iota(order.begin(),order.end(),0);
    auto origP=parent0,origS=sz;
    auto origC=component;
    for (int h=0;h<48;h++) {
        U bit=U(1)<<h;
        vector<B> chosen(n);
        int left=n-1-edges.size();
        auto take=[&](int u,int v) {
            chosen[u].set(v); chosen[v].set(u);
            add(u,v,bit); add(v,u,bit);
            join0(u,v); --left;
        };
        shuffle(order.begin(),order.end(),rng);
        for (int u:order) {
            if (!left) break;
            auto &cs=clauses[u];
            if (cs.empty()) continue;
            if (cs.size()>1) shuffle(cs.begin(),cs.end(),rng);
            for (;;) {
                B s=poss[u] & ~component[root(u)];
                bool need=false;
                for (const B& c:cs) {
                    if ((chosen[u]&c).any()) continue;
                    B z=s & c;
                    if (z.any()) { s=z; need=true; }
                }
                if (!need) break;
                take(u,pick(s));
            }
        }
        while (left) {
            shuffle(order.begin(),order.end(),rng);
            for (int u:order) {
                B s=poss[u] & ~component[root(u)];
                if (s.any()) take(u,pick(s));
                if (!left) break;
            }
        }
        parent0=origP; sz=origS; component=origC;
    }
    // Extra one-edge alternatives reward ruling out candidates even when
    // the sampled completions already agree on most of the tree.
    for (int i=0;i<n;i++) if (poss[i].any())
        for (int h=48;h<62;h++) add(pick(poss[i]),i,U(1)<<h);

    vector<vector<U>> suf(n);
    for (int i=0;i<n;i++) suf[i].resize(to[i].size());
    int val[65];
    for (int i=0;i<=64;i++) val[i]=i*(64-i);
    B best;
    int bestScore=-1;
    for (int attempt=0;attempt<2;attempt++) {
        B active;
        if (attempt) for (int i=0;i<n;i++) if (smallrand(4)==0) active.set(i);
        for (int step=0;step<6;step++) {
            shuffle(order.begin(),order.end(),rng);
            vector<U> cur(n);
            for (int k=n-1;k>=0;k--) {
                int u=order[k];
                for (int j=0;j<(int)to[u].size();j++) {
                    int v=to[u][j];
                    suf[u][j]=cur[v];
                    if (active[u]) cur[v] |= masks[u*n+v];
                }
            }
            fill(cur.begin(),cur.end(),0);
            bool moved=false;
            for (int u:order) {
                int gain=0;
                for (int j=0;j<(int)to[u].size();j++) {
                    int v=to[u][j]; U x=cur[v]|suf[u][j];
                    gain+=val[popcount(x|masks[u*n+v])]-val[popcount(x)];
                }
                bool use=gain>0;
                moved |= use!=active[u];
                active[u]=use;
                if (use) for (int v:to[u]) cur[v] |= masks[u*n+v];
            }
            int score=0;
            for (U x:cur) score+=val[popcount(x)];
            if (score>bestScore || (score==bestScore && active.count()<best.count())) {
                best=active; bestScore=score;
            }
            if (!moved) break;
        }
    }
    return best;
}

string ask(const vector<int>& d) {
    cout<<"?";
    for (int x:d) cout<<' '<<x;
    cout<<endl;
    string s;
    if (!(cin>>s) || (int)s.size()!=n || s.find_first_not_of("01")!=string::npos) exit(0);
    return s;
}

// For very small trees, partition the exact hypothesis set.
void tiny() {
    using E=vector<pair<int,int>>;
    vector<E> trees;
    vector<array<array<int,5>,5>> dist;
    int total=1;
    for(int i=0;i<n-2;i++) total*=n;
    for(int code=0;code<total;code++) {
        vector<int> deg(n,1),p;
        int z=code;
        for(int i=0;i<n-2;i++){p.push_back(z%n);deg[z%n]++;z/=n;}
        E e;
        for(int x:p){
            int v=find(deg.begin(),deg.end(),1)-deg.begin();
            e.emplace_back(x,v);deg[x]--;deg[v]--;
        }
        int a=find(deg.begin(),deg.end(),1)-deg.begin();
        int b=find(deg.begin()+a+1,deg.end(),1)-deg.begin();
        e.emplace_back(a,b);
        array<array<int,5>,5> ds{};
        for(int i=0;i<n;i++)for(int j=0;j<n;j++) ds[i][j]=i==j?0:n;
        for(auto [x,y]:e)ds[x][y]=ds[y][x]=1;
        for(int k=0;k<n;k++)for(int i=0;i<n;i++)for(int j=0;j<n;j++)
            ds[i][j]=min(ds[i][j],ds[i][k]+ds[k][j]);
        trees.push_back(e);dist.push_back(ds);
    }
    vector<int> live(total);iota(live.begin(),live.end(),0);
    while(live.size()>1) {
        int best=INT_MAX;
        vector<int> bd,br;
        for(int q=1;q<total*n*n;q++) {
            vector<int> d(n),r;int z=q;
            for(int &x:d){x=z%n;z/=n;}
            int f[32]{},cost=0;
            for(int t:live) {
                int m=0;
                for(int j=0;j<n;j++)for(int i=0;i<n;i++)
                    if(i!=j && dist[t][i][j]<=d[j]) m|=1<<i;
                r.push_back(m);cost+=2*f[m]+++1;
                if(cost>=best)break;
            }
            if(cost<best){best=cost;bd=d;br=r;}
        }
        string s=ask(bd);int m=0;
        for(int i=0;i<n;i++)if(s[i]=='1')m|=1<<i;
        vector<int> nxt;
        for(int i=0;i<(int)live.size();i++)if(br[i]==m)nxt.push_back(live[i]);
        if(nxt.empty())exit(0);
        live.swap(nxt);
    }
    edges=trees[live[0]];
}

int main(){
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    if(!(cin>>n) || n<2 || n>N)return 0;
    if(n<=5) tiny();
    else {
        poss.resize(n);knownE.resize(n);history.resize(n);
        component.resize(n);parent0.resize(n);sz.assign(n,1);
        for(int i=0;i<n;i++){
            parent0[i]=i;component[i].set(i);
            for(int j=0;j<n;j++)if(i!=j)poss[i].set(j);
        }
        vector<B> asked;
        int probe=-1;
        for(int round=0;;round++){
            reduceGraph();
            if((int)edges.size()==n-1) break;
            B q;
            if (!round) {
                vector<int> p(n);iota(p.begin(),p.end(),0);
                shuffle(p.begin(),p.end(),rng);
                for(int k=0;k<max(1,n/5);k++)q.set(p[k]);
            } else if(probe>=0 && poss[probe].any()) q.set(probe);
            else q=plan();
            probe=-1;
            bool informative=false;
            for(int i=0;i<n && !informative;i++){
                if((q & knownE[i]).any() || (q & poss[i]).none())continue;
                bool fresh=true;
                for(auto &s:clauses[i])if((s & ~q).none()){fresh=false;break;}
                informative=fresh;
            }
            if(!informative || find(asked.begin(),asked.end(),q)!=asked.end()){
                int v=0;
                for(int i=1;i<n;i++)if(poss[i].count()>poss[v].count())v=i;
                q.reset();q.set(v);
            }
            asked.push_back(q);
            vector<int>d(n);
            for(int i=0;i<n;i++)d[i]=q[i];
            string s=ask(d);
            B off;
            for(int i=0;i<n;i++){
                if(s[i]=='0')off.set(i);
                else history[i].push_back(q);
            }
            for(int i=0;i<n;i++){
                if(q[i])poss[i]&=~off;
                if(off[i])poss[i]&=~q;
            }
            if(off.count()==size_t(n-1))probe=s.find('1');
            else if(round==0 && off.none()){
                for(int i=0;i<n;i++)if(!q[i]){probe=i;break;}
            }
        }
    }
    cout<<"!"<<endl;
    for(auto [u,v]:edges)cout<<u+1<<' '<<v+1<<endl;
    return 0;
}
