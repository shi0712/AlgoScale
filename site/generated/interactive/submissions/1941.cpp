#include <bits/stdc++.h>
using namespace std;

pair<int,int> ask(int d) {
    cout << "? " << d << endl;
    int a,b;
    if (!(cin >> a) || a < 0 || !(cin >> b) || b < 0) exit(0);
    return {a,b};
}

void answer(const vector<int>& p) {
    cout << "!";
    for (int x:p) cout << ' ' << x;
    cout << endl;
    exit(0);
}

// For small instances, optimize the decision tree over the exact set of
// graphs consistent with the transcript.
struct Small {
    int n, id[6][6]{};
    vector<int> graphs, pm;
    vector<vector<int>> paths;
    vector<unsigned char> replies;
    map<pair<int,vector<int>>,pair<int,int>> memo;
    static constexpr int INF=100000000;
    int idx(int s,int g,int d) {
        return (s*(int)graphs.size()+g)*n+d;
    }
    int path(const vector<int>& gs) {
        int forbidden=0;
        for(int g:gs) forbidden |= graphs[g];
        for(int i=0;i<(int)pm.size();i++)
            if(!(pm[i]&forbidden)) return i;
        return -1;
    }
    pair<int,int> solve(int s,const vector<int>& gs) {
        if(path(gs)>=0) return {0,-1};
        auto key=make_pair(s,gs);
        auto it=memo.find(key);
        if(it!=memo.end()) return it->second;
        pair<int,int> best={INF,-1};
        int m=__builtin_popcount((unsigned)s);
        for(int d=0;d<m;d++) {
            vector<int> bucket[49];
            for(int g:gs) bucket[replies[idx(s,g,d)]].push_back(g);
            if(bucket[0].size()==gs.size()) continue;
            int cost=gs.size();
            for(int r=0;r<49 && cost<best.first;r++) if(!bucket[r].empty()) {
                int a=r/7;
                int ns=a?(s^(1<<(a-1))):s;
                cost+=solve(ns,bucket[r]).first;
            }
            if(cost<best.first) best={cost,d};
            if(best.first==(int)gs.size()) break;
        }
        return memo[key]=best;
    }
    void run(int N) {
        n=N;
        int e=0;
        for(int i=0;i<n;i++) for(int j=i+1;j<n;j++)
            id[i][j]=id[j][i]=1<<e++;
        for(int t=0;t<(1<<e);t++)
            if(__builtin_popcount((unsigned)t)==n-2) graphs.push_back(t);
        vector<int> p(n);
        iota(p.begin(),p.end(),1);
        do {
            if(p.front()>p.back()) continue;
            int mask=0;
            for(int j=1;j<n;j++) mask|=id[p[j]-1][p[j-1]-1];
            paths.push_back(p); pm.push_back(mask);
        } while(next_permutation(p.begin(),p.end()));
        replies.resize((1<<n)*graphs.size()*n);
        for(int s=1;s<(1<<n);s++) for(int g=0;g<(int)graphs.size();g++) {
            int deg[6]{},w[6]{};
            for(int i=0;i<n;i++) if(s>>i&1)
                for(int j=0;j<n;j++) if(i!=j && (s>>j&1)) {
                    if(graphs[g]&id[i][j]) {
                        if(!w[i]) w[i]=j+1;
                    } else ++deg[i];
                }
            for(int d=0;d<n;d++) {
                int v=-1;
                for(int i=0;i<n;i++) if((s>>i&1) && deg[i]>=d &&
                    (v<0 || deg[i]<deg[v])) v=i;
                if(v>=0) replies[idx(s,g,d)]=(v+1)*7+w[v];
            }
        }
        int s=(1<<n)-1;
        vector<int> gs(graphs.size());
        iota(gs.begin(),gs.end(),0);
        while(true) {
            int i=path(gs);
            if(i>=0) answer(paths[i]);
            int d=solve(s,gs).second;
            if(d<0) exit(0);
            auto [a,b]=ask(d);
            vector<int> next;
            for(int g:gs) if(replies[idx(s,g,d)]==7*a+b) next.push_back(g);
            if(next.empty()) exit(0);
            gs.swap(next);
            if(a) s^=1<<(a-1);
        }
    }
};

struct Step {
    int v,ban;       // v is adjacent to everyone still present except ban
    int hard=0,w=0;  // optional min-degree query just after a universal v
};

int main(){
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    int n;
    if(!(cin>>n)) return 0;
    if(n<=6) Small().run(n);

    set<int> live;
    for(int i=1;i<=n;i++) live.insert(i);
    vector<Step> log;
    int freeSteps=0, first=-1, helpers=0;
    int nextTrial=0, used=0;

    // Work backwards with a forest of certified paths. A deleted vertex
    // can join two paths whenever it sees an endpoint of each.
    auto build=[&](bool clique) -> int {
        using P=pair<int,int>;
        set<P> comps;
        vector<array<int,2>> adj(n+1,{0,0});
        auto edge=[&](int a,int b){
            adj[a][adj[a][0]!=0]=b;
            adj[b][adj[b][0]!=0]=a;
        };
        if(clique){
            int last=0;
            for(int x:live){
                if(last) edge(last,x);
                last=x;
            }
            if(last) comps.emplace(*live.begin(),last);
        }else{
            for(int x:live) comps.emplace_hint(comps.end(),x,x);
        }
        int priority=first<0?0:log[first].ban;
        auto insertVertex=[&](int v,int ban,int limit){
            // Allowed endpoints are below limit and different from ban.
            int ends[2],cnt=0;
            auto take=[&](set<P>::iterator it){
                auto [a,b]=*it;
                if(a>=limit) return false;
                if(a==ban) swap(a,b);
                if(a==ban || a>=limit) return false;
                edge(v,a);
                ends[cnt++]=b;
                comps.erase(it);
                return true;
            };
            if(priority && priority<limit && priority!=ban){
                auto it=comps.find({priority,priority});
                if(it!=comps.end()) take(it);
            }
            for(auto it=comps.begin();cnt<2 && it!=comps.end();){
                if(it->first>=limit) break;
                auto cur=it++;
                take(cur);
            }
            while(cnt<2) ends[cnt++]=v;
            comps.emplace(min(ends[0],ends[1]),max(ends[0],ends[1]));
        };
        for(int i=(int)log.size()-1;i>=0;i--){
            auto [v,b,h,w]=log[i];
            if(h) insertVertex(h,0,w);
            insertVertex(v,b,n+1);
        }
        if(comps.size()==1){
            vector<int> out;
            int x=comps.begin()->first,prev=0;
            while(x){
                out.push_back(x);
                int y=adj[x][0]==prev?adj[x][1]:adj[x][0];
                prev=x;x=y;
            }
            answer(out);
        }
        return (int)comps.size()-1;
    };

    auto consider=[&](){
        int m=live.size();
        bool guaranteed=freeSteps>=m-1 && first>=0 &&
            (!live.count(log[first].ban) || helpers>0);
        bool blocked=first==0 && helpers==0 && first<(int)log.size()
                    && live.count(log[first].ban);
        if(guaranteed || (!blocked && used>=nextTrial && used>=m-1)){
            int deficit=build(false);
            nextTrial=used+max({1,n/48,deficit/3});
        }
    };

    auto addFree=[&](int v,int b){
        if(first<0) first=log.size();
        else if(b!=log[first].ban) ++helpers;
        log.push_back({v,b});
        ++freeSteps;
    };

    bool triedUniversal=false;
    while(true){
        int m=live.size();
        // At the start of each round at most m-2 edges are missing.
        if(m==2) build(true);
        if(!triedUniversal && first==0 && !helpers && live.count(log[0].ban)
           && freeSteps+1>=m-2){
            triedUniversal=true;
            auto [a,b]=ask(m-1);
            if(a){
                live.erase(a);
                addFree(a,0);
                build(false);
            }
        }
        auto [v,b]=ask(m-2);
        if(!v || !live.erase(v)) return 0;
        ++used;
        int oldFirst=first, oldHelpers=helpers;
        addFree(v,b);
        if(!b && m<=4) build(true); // no complement leaves, and at most two edges
        consider();
        if(b) continue;

        // No complement-degree-one vertex existed. Removing a maximum
        // complement-degree vertex now pays for both deletions.
        auto [h,w]=ask(0);
        if(!h || !live.count(h)) return 0;
        if(!w) build(true); // the entire remaining graph is complete
        live.erase(h);
        ++used;
        first=oldFirst; helpers=oldHelpers+(oldFirst>=0);
        --freeSteps;
        log.back().hard=h;
        log.back().w=w;
        consider();
    }
}
