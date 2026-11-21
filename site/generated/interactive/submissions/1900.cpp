#include <bits/stdc++.h>
using namespace std;

struct DSU {
    vector<int> p;
    DSU(int n = 0) : p(n, -1) {}
    int root(int x) {
        while (p[x] >= 0) {
            if (p[p[x]] >= 0) p[x] = p[p[x]];
            x = p[x];
        }
        return x;
    }
    bool join(int x, int y) {
        x = root(x); y = root(y);
        if (x == y) return false;
        if (p[x] > p[y]) swap(x,y);
        p[x] += p[y]; p[y] = x;
        return true;
    }
};

int n,m,ones,unknowns;
vector<int> u,v,state,base,orderE;
mt19937 rng(23809417);

[[noreturn]] void finish(const vector<int>& a) {
    cout << "!";
    for (int e : a) cout << ' ' << e;
    cout << '\n' << flush;
    exit(0);
}
void solved() {
    vector<int> a;
    for (int i=0;i<m;++i) if (state[i]==1) a.push_back(i);
    finish(a);
}
void mark(int e,int x) {
    if (state[e]!=-1) return;
    state[e]=x; ones+=x; --unknowns;
    if (ones==n-1) solved();
}
int ask(const vector<int>& a) {
    cout << "?";
    for (int e:a) cout << ' ' << e;
    cout << '\n' << flush;
    int r;
    if (!(cin>>r) || r<0) exit(0);
    if (r==n-1) finish(a);
    return r;
}

// Contract known royal edges. Loops cannot be royal, and bridges must be.
void graphRules() {
    DSU d(n);
    for(int e=0;e<m;++e) if(state[e]==1) d.join(u[e],v[e]);
    vector<vector<pair<int,int>>> adj(n);
    for(int e=0;e<m;++e) if(state[e]<0) {
        int a=d.root(u[e]), b=d.root(v[e]);
        if(a==b) mark(e,0);
        else {adj[a].emplace_back(b,e); adj[b].emplace_back(a,e);}
    }
    vector<int> tin(n),lo(n);
    int tim=0;
    auto dfs=[&](auto&& dfs,int a,int pe)->void {
        tin[a]=lo[a]=++tim;
        for(auto [b,e]:adj[a]) if(e!=pe) {
            if(!tin[b]) {
                dfs(dfs,b,e);
                lo[a]=min(lo[a],lo[b]);
                if(lo[b]>tin[a]) mark(e,1);
            } else lo[a]=min(lo[a],tin[b]);
        }
    };
    for(int i=0;i<n;++i) if(!tin[i]) dfs(dfs,i,-1);
}

// Learn a reference tree whose edges can later be used as known padding.
void referenceTree() {
    int best=n;
    int lb=ones;
    for(int t=0;t<(m>6*n?8:1);++t) {
        shuffle(orderE.begin(),orderE.end(),rng);
        DSU d(n);
        vector<int> a;
        for(int e:orderE) if(d.join(u[e],v[e])) a.push_back(e);
        int r=ask(a);
        if(r<best) { best=r; base=a; }
        if(best==lb) break;
    }
    vector<int> idx(m,-1),par(n,-1),up(n),dep(n);
    vector<vector<pair<int,int>>> adj(n);
    for(int i=0;i<n-1;++i) {
        int e=base[i]; idx[e]=i;
        adj[u[e]].emplace_back(v[e],e);
        adj[v[e]].emplace_back(u[e],e);
    }
    auto dfs=[&](auto&& dfs,int a)->void {
        for(auto [b,e]:adj[a]) if(b!=par[a]) {
            par[b]=a; up[b]=e; dep[b]=dep[a]+1; dfs(dfs,b);
        }
    };
    dfs(dfs,0);
    int remain,need;
    auto quotas=[&]() {
        remain=0; need=best;
        for(int e:base) {
            remain+=state[e]<0;
            need-=state[e]==1;
        }
        if(need==0 || remain==need) {
            for(int e:base) if(state[e]<0) mark(e,need!=0);
            remain=need=0;
        }
        int cnt=unknowns-remain, k=n-1-ones-need;
        if(cnt && (!k || k==cnt))
            for(int e:orderE) if(idx[e]<0 && state[e]<0) mark(e,k!=0);
    };
    quotas();
    for(int e:orderE) {
        if(!remain) break;
        if(idx[e]>=0) continue;
        int a=u[e], b=v[e], anchor=-1;
        vector<int> todo;
        while(a!=b) {
            if(dep[a]<dep[b]) swap(a,b);
            int f=up[a]; a=par[a];
            if(state[f]<0) todo.push_back(f); else anchor=f;
        }
        if(todo.empty()) continue;
        auto diff=[&](int f) {
            base[idx[f]]=e;
            int r=ask(base)-best;
            base[idx[f]]=f;
            return r;
        };
        if(state[e]<0 && anchor>=0) mark(e,state[anchor]+diff(anchor));
        vector<int> eq;
        for(int f:todo) {
            int z=diff(f);
            if(z && state[e]<0) mark(e,z==1);
            if(state[e]>=0) mark(f,state[e]-z);
            else eq.push_back(f);
        }
        // In the all-equal case the whole cycle cannot be royal.
        if(state[e]<0) mark(e,0);
        for(int f:eq) mark(f,state[e]);
        quotas();
    }
}

// Count on any forest by extending it using only reference edges.
int forestCount(vector<int> a) {
    DSU d(n);
    for(int e:a) d.join(u[e],v[e]);
    int pad=0;
    for(int e:base) if(d.join(u[e],v[e])) {
        a.push_back(e); pad+=state[e];
    }
    return ask(a)-pad;
}

struct Group { vector<int> a; int k; };
void reduce(vector<Group>& g) {
    for(;;) {
        int before=unknowns;
        graphRules();
        vector<Group> h;
        for(auto &s:g) {
            int j=0;
            for(int e:s.a) {
                if(state[e]<0) s.a[j++]=e;
                else s.k-=state[e];
            }
            s.a.resize(j);
            if(!s.k || s.k==j) {
                for(int e:s.a) mark(e,s.k!=0);
            } else h.push_back(move(s));
        }
        g.swap(h);
        if(before==unknowns) return;
    }
}

// Adaptive quantitative weighing of up to ten binary variables.
// Each decision chooses a subset whose sum has high entropy.
struct Node {
    vector<int> poss;
    int mask=0;
    array<int,11> child{};
};
vector<Node> nodes(1);
int roots[11];
int newNode(vector<int> a) {
    Node z; z.poss=move(a);
    nodes.push_back(move(z));
    return (int)nodes.size()-1;
}
void prepare(int id,int b) {
    if(nodes[id].mask || nodes[id].poss.size()<=1) return;
    const auto &p=nodes[id].poss;
    static double cost[1025];
    if(!cost[2]) for(int i=1;i<=1024;++i) cost[i]=i*log2(i);
    double best=1e100;
    int opt=0;
    for(int s=1;s<(1<<b);++s) {
        int hist[11]={};
        for(int x:p) ++hist[__builtin_popcount(unsigned(x&s))];
        double score=0;
        for(int j=0;j<=b;++j) score+=cost[hist[j]];
        if(score<best-1e-8) {best=score;opt=s;}
    }
    nodes[id].mask=opt;
    vector<vector<int>> subs(b+1);
    for(int x:p) subs[__builtin_popcount(unsigned(x&opt))].push_back(x);
    for(int i=0;i<=b;++i) if(!subs[i].empty()) {
        int c=newNode(move(subs[i]));
        nodes[id].child[i]=c;
    }
}

void recover() {
    graphRules();
    vector<DSU> forests;
    vector<Group> g;
    shuffle(orderE.begin(),orderE.end(),rng);
    for(int e:orderE) if(state[e]<0) {
        int j=0;
        while(j<(int)g.size() && !forests[j].join(u[e],v[e])) ++j;
        if(j==(int)g.size()) {
            g.push_back({{},0}); forests.emplace_back(n);
            forests.back().join(u[e],v[e]);
        }
        g[j].a.push_back(e);
    }
    int left=n-1-ones, total=unknowns;
    // Leave a large group to be counted by subtraction.
    reverse(g.begin(),g.end());
    for(auto &s:g) {
        int sz=s.a.size();
        int low=max(0,left-(total-sz)), high=min(left,sz);
        s.k=low==high?low:forestCount(s.a);
        total-=sz; left-=s.k;
        if(s.k==0 || s.k==sz) for(int e:s.a) mark(e,s.k!=0);
    }
    for(;;) {
        reduce(g);
        vector<Group> h;
        vector<int> batch;
        DSU d(n);
        auto split=[&](int i,int k) {
            Group &s=g[i];
            int mid=s.a.size()/2;
            Group r{vector<int>(s.a.begin()+mid,s.a.end()),s.k-k};
            s.a.resize(mid); s.k=k;
            auto add=[&](Group &t) {
                if(!t.k || t.k==(int)t.a.size()) {
                    for(int e:t.a) mark(e,t.k!=0);
                } else h.push_back(move(t));
            };
            add(s); add(r);
        };
        auto flushBatch=[&]() {
            int b=batch.size();
            if(!b) return;
            if(!roots[b]) {
                vector<int> p(1<<b);
                iota(p.begin(),p.end(),0);
                roots[b]=newNode(move(p));
            }
            int id=roots[b];
            while(nodes[id].poss.size()>1) {
                prepare(id,b);
                int mask=nodes[id].mask, offset=0;
                vector<int> a;
                for(int j=0;j<b;++j) if(mask>>j&1) {
                    Group &s=g[batch[j]];
                    int mid=s.a.size()/2;
                    offset+=max(0,s.k-((int)s.a.size()-mid));
                    a.insert(a.end(),s.a.begin(),s.a.begin()+mid);
                }
                int r=forestCount(move(a))-offset;
                id=nodes[id].child[r];
            }
            int x=nodes[id].poss[0];
            for(int j=0;j<b;++j) {
                Group &s=g[batch[j]];
                int low=max(0,s.k-(int)(s.a.size()-s.a.size()/2));
                split(batch[j],low+((x>>j)&1));
            }
            batch.clear(); d=DSU(n);
        };
        for(int i=0;i<(int)g.size();++i) {
            auto &s=g[i];
            int mid=s.a.size()/2, low=max(0,s.k-((int)s.a.size()-mid));
            if(min(mid,s.k)-low>1) {
                split(i,forestCount(vector<int>(s.a.begin(),s.a.begin()+mid)));
                continue;
            }
            bool ok=true;
            for(int j=0;j<mid;++j) {
                int e=s.a[j];
                if(!d.join(u[e],v[e])) {ok=false;break;}
            }
            if(!ok) {
                flushBatch();
                for(int j=0;j<mid;++j) {
                    int e=s.a[j]; d.join(u[e],v[e]);
                }
            }
            batch.push_back(i);
            if(batch.size()==10) flushBatch();
        }
        flushBatch(); g.swap(h);
    }
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    if(!(cin>>n>>m)) return 0;
    u.resize(m); v.resize(m); orderE.resize(m);
    state.assign(m,-1); unknowns=m;
    for(int i=0;i<m;++i) {
        if(!(cin>>u[i]>>v[i])) return 0;
        orderE[i]=i;
    }
    graphRules();
    referenceTree();
    recover();
}
