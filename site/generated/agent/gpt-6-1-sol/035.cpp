#include <bits/stdc++.h>
using namespace std;

static constexpr int INF = 1000000007;

/*
At a vertex, regard its own number/position as an extra, dummy port.
The order of incident edge operations is a cyclic order of all ports, cut
at the dummy port.  A number's route fixes consecutive pairs in these
local cycles.  Every collection of local orders is realizable on a tree
(remove a leaf inductively, then insert its edge into its neighbor's order).

Partially fixed pairs form disjoint chains.  An unused input can connect
to any unused output except the output of its own chain, unless that is
the last chain.  Thus a DSU maintains the single forbidden output for
each input.  Greedily choose the smallest reachable destination for each
number, then fix all transitions on its unique tree path.

For a fixed input, the minimum reachable position is the minimum among
the allowed outgoing branches.  Heavy-light decomposition maintains these
minima: light branches are stored in ordered sets, and each heavy-chain
transfer has the form min(constant, pass ? next : INF), in either direction.
Each port is consumed once, so there are O(n) local changes and queries;
their total time is O(n log^2 n), with O(n) memory.
*/

struct State {
    int down = INF, up = INF;
    bool downPass = true, upPass = true;
};

static inline State combine(const State &a, const State &b) {
    State c;
    c.down = min(a.down, a.downPass ? b.down : INF);
    c.downPass = a.downPass && b.downPass;
    c.up = min(b.up, b.upPass ? a.up : INF);
    c.upPass = a.upPass && b.upPass;
    return c;
}

struct SegmentTree {
    int size;
    vector<State> tree;
    void init(const vector<State> &a) {
        size = 1;
        while (size < (int)a.size()) size <<= 1;
        tree.assign(2 * size, State{});
        for (int i = 0; i < (int)a.size(); ++i) tree[size+i] = a[i];
        for (int i = size-1; i; --i) tree[i] = combine(tree[2*i], tree[2*i+1]);
    }
    void setValue(int p, State s) {
        p += size;
        tree[p] = s;
        while (p >>= 1) tree[p] = combine(tree[2*p], tree[2*p+1]);
    }
    State query(int l, int r) const { // Inclusive, in top-to-bottom order.
        State a, b;
        for (l += size, r += size+1; l < r; l >>= 1, r >>= 1) {
            if (l & 1) a = combine(a, tree[l++]);
            if (r & 1) b = combine(tree[--r], b);
        }
        return combine(a,b);
    }
};

struct Edge { int to, port; };

struct Solver {
    int n;
    vector<vector<Edge>> adj;
    vector<int> initial, parent, depth, heavy, sub, parentPort, childPort;
    vector<int> head, pos, tail, order, chainDP;
    vector<int> portNeighbor, uf, ufSize, firstPort, lastPort, active;
    vector<unsigned char> inFree, outFree;
    vector<set<pair<int,int>>> candidates;
    SegmentTree seg;

    int find(int x) {
        int r=x;
        while (uf[r]!=r) r=uf[r];
        while (uf[x]!=x) { int y=uf[x]; uf[x]=r; x=y; }
        return r;
    }
    // Every unfinished local chain has one free input and one free output.
    // Joining those two would close its cycle before all ports were included.
    int forbidden(int v, int input) {
        if (active[v] <= 1) return -1;
        return firstPort[find(input)];
    }
    int bestCandidate(int v, int banned) const {
        auto it = candidates[v].begin();
        if (it == candidates[v].end()) return INF;
        if (it->second == banned) ++it;
        return it == candidates[v].end() ? INF : it->first;
    }
    State localState(int v) {
        State s;
        s.downPass = s.upPass = false;
        int p = parent[v] == -1 ? v : parentPort[v];
        if (inFree[p]) {
            int ban = forbidden(v,p);
            s.down = bestCandidate(v,ban);
            s.downPass = heavy[v] != -1 && outFree[childPort[heavy[v]]] && ban != childPort[heavy[v]];
        }
        if (heavy[v] != -1) {
            int hport = childPort[heavy[v]];
            if (inFree[hport]) {
                int ban = forbidden(v,hport);
                s.up = bestCandidate(v,ban);
                s.upPass = parent[v] != -1 && outFree[parentPort[v]] && ban != parentPort[v];
            }
        }
        return s;
    }
    void refresh(int v) {
        // Only a changed chain minimum can affect its light-edge parent.
        while (true) {
            int h=head[v];
            seg.setValue(pos[v],localState(v));
            int value=seg.query(pos[h],tail[h]).down;
            int old=chainDP[h];
            if (value==old) break;
            chainDP[h]=value;
            int p=parent[h];
            if (p==-1 || !outFree[childPort[h]]) break;
            candidates[p].erase({old,childPort[h]});
            candidates[p].insert({value,childPort[h]});
            v=p;
        }
    }
    int minimumDestination(int source) {
        int answer=INF, v=source, input=source;
        while (true) {
            int ban=forbidden(v,input);
            answer=min(answer,bestCandidate(v,ban));
            if (heavy[v]!=-1) {
                int hp=childPort[heavy[v]];
                if (outFree[hp] && ban!=hp)
                    answer=min(answer,seg.query(pos[v]+1,tail[head[v]]).down);
            }
            if (parent[v]==-1 || !outFree[parentPort[v]] || ban==parentPort[v]) break;
            int c=v;
            v=parent[v];
            if (heavy[v]==c) {
                // Climb this entire chain, stopping at the first blocked exit.
                int h=head[v];
                State s=seg.query(pos[h],pos[v]);
                answer=min(answer,s.up);
                if (!s.upPass) break;
                c=h;
                v=parent[h];
            }
            input=childPort[c];
        }
        return answer-1;
    }
    void useTransition(int v, int input, int output) {
        int nei=portNeighbor[output];
        if (nei==-1) candidates[v].erase({v+1,output});
        else if (parent[nei]==v && heavy[v]!=nei)
            candidates[v].erase({chainDP[nei],output});

        int a=find(input),b=find(output);
        if (a!=b) {
            int first=firstPort[a], last=lastPort[b];
            if (ufSize[a]<ufSize[b]) swap(a,b);
            uf[b]=a;
            ufSize[a]+=ufSize[b];
            firstPort[a]=first;
            lastPort[a]=last;
        }
        inFree[input]=false;
        outFree[output]=false;
        --active[v];
        refresh(v);
    }
    int portTo(int v, int u) const {
        return parent[v]==u ? parentPort[v] : childPort[u];
    }
    void assignPath(int s,int t) {
        vector<int> path,right;
        int a=s,b=t;
        while (a!=b) {
            if (depth[a]>=depth[b]) {path.push_back(a);a=parent[a];}
            else {right.push_back(b);b=parent[b];}
        }
        path.push_back(a);
        for (auto it=right.rbegin();it!=right.rend();++it) path.push_back(*it);
        for (int i=0;i<(int)path.size();++i) {
            int v=path[i];
            int input=i==0 ? v : portTo(v,path[i-1]);
            int output=i+1==(int)path.size() ? v : portTo(v,path[i+1]);
            useTransition(v,input,output);
        }
    }
    void run() {
        cin>>n;
        initial.resize(n);
        for (int &v:initial) {cin>>v;--v;}
        adj.resize(n);
        int ports=3*n-2;
        portNeighbor.assign(ports,-1);
        for (int i=0;i<n-1;++i) {
            int a,b;cin>>a>>b;--a;--b;
            int p=n+2*i,q=p+1;
            adj[a].push_back({b,p});adj[b].push_back({a,q});
            portNeighbor[p]=b;portNeighbor[q]=a;
        }
        parent.assign(n,-1);depth.assign(n,0);heavy.assign(n,-1);
        sub.assign(n,1);parentPort.assign(n,-1);childPort.assign(n,-1);
        order.reserve(n);order.push_back(0);
        for (int k=0;k<n;++k) {
            int v=order[k];
            for (auto e:adj[v]) if (e.to!=parent[v]) {
                parent[e.to]=v;depth[e.to]=depth[v]+1;
                childPort[e.to]=e.port;
                parentPort[e.to]=(e.port-n)%2 ? e.port-1 : e.port+1;
                order.push_back(e.to);
            }
        }
        vector<int> dp(n);
        for (int k=n-1;k>=0;--k) {
            int v=order[k];dp[v]=v+1;
            for (auto e:adj[v]) if (parent[e.to]==v) {
                int u=e.to;
                sub[v]+=sub[u];dp[v]=min(dp[v],dp[u]);
                if (heavy[v]==-1 || sub[u]>sub[heavy[v]]) heavy[v]=u;
            }
        }
        head.resize(n);pos.resize(n);tail.resize(n);int timer=0;
        for (int v:order) if (parent[v]==-1 || heavy[parent[v]]!=v) {
            for (int u=v;u!=-1;u=heavy[u]) {head[u]=v;pos[u]=timer++;}
            tail[v]=timer-1;
        }
        uf.resize(ports);iota(uf.begin(),uf.end(),0);ufSize.assign(ports,1);
        firstPort=lastPort=uf;
        inFree.assign(ports,true);outFree.assign(ports,true);
        active.resize(n);candidates.resize(n);
        for (int v=0;v<n;++v) {
            active[v]=adj[v].size()+1;
            candidates[v].insert({v+1,v});
            for (auto e:adj[v]) if (parent[e.to]==v && heavy[v]!=e.to)
                candidates[v].insert({dp[e.to],e.port});
        }
        vector<State> states(n);
        for (int v=0;v<n;++v) states[pos[v]]=localState(v);
        seg.init(states);
        chainDP.assign(n,INF);
        for (int v=0;v<n;++v) if (head[v]==v) chainDP[v]=seg.query(pos[v],tail[v]).down;
        for (int i=0;i<n;++i) {
            int s=initial[i];
            int t=minimumDestination(s);
            cout<<t+1<<(i+1==n?'\n':' ');
            assignPath(s,t);
        }
    }
};

int main() {
    ios::sync_with_stdio(false);cin.tie(nullptr);
    int T;cin>>T;
    while (T--) {Solver solver;solver.run();}
    return 0;
}
