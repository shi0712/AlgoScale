#include <bits/stdc++.h>
using namespace std;

// Micali--Vazirani maximum-cardinality matching for a general graph.
class MicaliVazirani {
    static constexpr int INF = 1'000'000'000;
    enum class EdgeState : unsigned char { Fresh, Prop, Bridge };
    struct Arc { int to, rev; EdgeState state; };
    struct PetalDSU {
        vector<int> p, sz, bud, direct_parent;
        void init(int n) {
            p.resize(n); iota(p.begin(), p.end(), 0);
            sz.assign(n, 1); bud.resize(n); iota(bud.begin(), bud.end(), 0);
            direct_parent.assign(n, -1);
        }
        int find(int x) { return p[x] == x ? x : p[x] = find(p[x]); }
        int top(int x) { return bud[find(x)]; }
        void merge_into(int x, int y) {
            direct_parent[x] = y;
            int a = find(x), b = find(y);
            if (a == b) return;
            int wanted = bud[b];
            if (sz[a] < sz[b]) { p[a] = b; sz[b] += sz[a]; bud[b] = wanted; }
            else { p[b] = a; sz[a] += sz[b]; bud[a] = wanted; }
        }
    } dsu;
    int n;
    vector<vector<Arc>> g;
    vector<int> mate, pred_it, dead, even_level, odd_level, color;
    vector<vector<int>> pred;
    vector<vector<pair<int,int>>> ddfs_child;
    struct BridgeInfo { int a=-1,b=-1,ba=-1,bb=-1; };
    vector<BridgeInfo> bridge_of;
    int next_color = 1;
    queue<int> killed_queue;
    int min_level(int v) const { return min(even_level[v], odd_level[v]); }
    int tenacity(int u, int v) const {
        if (mate[u] == v) {
            if (odd_level[u] >= INF || odd_level[v] >= INF) return INF;
            long long z = 1LL * odd_level[u] + odd_level[v] + 1;
            return z >= INF ? INF : (int)z;
        }
        if (even_level[u] >= INF || even_level[v] >= INF) return INF;
        long long z = 1LL * even_level[u] + even_level[v] + 1;
        return z >= INF ? INF : (int)z;
    }
    int descend_one(vector<int>& A, int ca, vector<int>& B, int cb,
                    vector<int>& support) {
        int u = A.back();
        while (pred_it[u] < (int)pred[u].size()) {
            int raw = pred[u][pred_it[u]++];
            int v = dsu.top(raw);
            if (dead[raw]) continue;
            if (color[v] == 0) {
                A.push_back(v); support.push_back(v);
                ddfs_child[u].push_back({raw, v}); color[v] = ca;
                return -1;
            }
            if (!B.empty() && v == B.back()) ddfs_child[u].push_back({raw, v});
        }
        A.pop_back();
        if (A.empty()) {
            if (B.size() == 1) { int z = B.back(); color[z] = 0; return z; }
            A.push_back(B.back()); color[A.back()] = ca; B.pop_back();
        }
        return -1;
    }
    pair<int,int> double_dfs(int a, int b, vector<int>& support) {
        vector<int> A{dsu.top(a)}, B{dsu.top(b)};
        if (A[0] == B[0]) return {A[0], B[0]};
        support = {A[0], B[0]};
        int ca = ++next_color, cb = ++next_color;
        color[A[0]] = ca; color[B[0]] = cb;
        while (true) {
            if (min_level(A.back()) == 0 && min_level(B.back()) == 0)
                return {A.back(), B.back()};
            int bottleneck;
            if (min_level(A.back()) >= min_level(B.back()))
                bottleneck = descend_one(A, ca, B, cb, support);
            else bottleneck = descend_one(B, cb, A, ca, support);
            if (bottleneck != -1) return {bottleneck, bottleneck};
        }
    }
    void kill(int v) { if (!dead[v]) { dead[v] = 1; killed_queue.push(v); } }
    void take_unmatched_edge(int u, int v) {
        if (dead[u] || dead[v] || mate[u] == v) return;
        kill(u); kill(v); mate[u] = v; mate[v] = u;
    }
    void augment_segment(int u, int target, bool force_bridge = false) {
        if (u == target) return;
        if (!force_bridge && min_level(u) == even_level[u]) {
            int x = pred[u][0], k = 0;
            while (dsu.top(pred[x][k]) != dsu.top(x)) ++k;
            u = pred[x][k]; take_unmatched_edge(x, u);
            augment_segment(u, target, false); return;
        }
        BridgeInfo bi = bridge_of[u];
        int e0 = bi.a, e1 = bi.b, b0 = bi.ba, b1 = bi.bb;
        if (((color[b0] ^ 1) == color[u]) || color[b1] == color[u]) {
            swap(b0, b1); swap(e0, e1);
        }
        take_unmatched_edge(e0, e1);
        bool ok1 = open_petal(e0, b0, u); assert(ok1);
        int up = dsu.direct_parent[u];
        bool ok2 = open_petal(e1, b1, up); assert(ok2);
        augment_segment(up, target, false);
    }
    bool open_petal(int cur, int base_cur, int wanted) {
        if (base_cur == wanted) { augment_segment(cur, base_cur, false); return true; }
        for (auto [raw, b] : ddfs_child[base_cur]) {
            if ((b == wanted || color[b] == color[base_cur]) &&
                open_petal(raw, b, wanted)) {
                augment_segment(cur, base_cur, false);
                take_unmatched_edge(base_cur, raw); return true;
            }
        }
        return false;
    }
    bool phase() {
        vector<vector<int>> layer(n);
        vector<vector<pair<int,int>>> bridge_by_tenacity(2*n + 3);
        vector<int> lost_pred(n, 0);
        auto assign_level = [&](int v, int lv) {
            if (lv & 1) odd_level[v] = lv; else even_level[v] = lv;
            if (lv < n) layer[lv].push_back(v);
        };
        for (int v=0; v<n; ++v) if (mate[v] == -1) assign_level(v, 0);
        bool augmented = false;
        for (int lv=0; lv<n && !augmented; ++lv) {
            for (int u : layer[lv]) for (auto &e : g[u]) {
                bool parity_ok = ((odd_level[u] == lv) == (mate[u] == e.to));
                if (e.state != EdgeState::Fresh || !parity_ok) continue;
                int v = e.to;
                if (min_level(v) >= lv+1) {
                    e.state = EdgeState::Prop; g[v][e.rev].state = EdgeState::Prop;
                    if (min_level(v) > lv+1) assign_level(v, lv+1);
                    pred[v].push_back(u);
                } else {
                    e.state = EdgeState::Bridge; g[v][e.rev].state = EdgeState::Bridge;
                    int t = tenacity(u,v);
                    if (t < INF && t < (int)bridge_by_tenacity.size())
                        bridge_by_tenacity[t].push_back({u,v});
                }
            }
            int tneed = 2*lv+1;
            if (tneed >= (int)bridge_by_tenacity.size()) continue;
            for (auto [a,b] : bridge_by_tenacity[tneed]) {
                if (dead[dsu.top(a)] || dead[dsu.top(b)]) continue;
                vector<int> support; auto [x,y] = double_dfs(a,b,support);
                BridgeInfo info{a,b,dsu.top(a),dsu.top(b)};
                for (int v : support) {
                    if (v == y) continue;
                    bridge_of[v] = info; dsu.merge_into(v,y);
                    assign_level(v, tneed-min_level(v));
                    if (even_level[v] > odd_level[v])
                        for (auto const &e : g[v])
                            if (e.state == EdgeState::Bridge && mate[v] != e.to) {
                                int tt = tenacity(v,e.to);
                                if (tt < INF && tt < (int)bridge_by_tenacity.size())
                                    bridge_by_tenacity[tt].push_back({v,e.to});
                            }
                }
                if (x != y) {
                    augment_segment(x,y,true); augmented = true;
                    while (!killed_queue.empty()) {
                        int v=killed_queue.front(); killed_queue.pop();
                        for (auto const &e : g[v])
                            if (e.state == EdgeState::Prop &&
                                min_level(e.to) > min_level(v) && !dead[e.to])
                                if (++lost_pred[e.to] == (int)pred[e.to].size()) kill(e.to);
                    }
                }
            }
        }
        return augmented;
    }
    void reset_phase_state() {
        for (auto &adj:g) for (auto &e:adj) e.state=EdgeState::Fresh;
        pred.assign(n,{}); pred_it.assign(n,0); dead.assign(n,0);
        even_level.assign(n,INF); odd_level.assign(n,INF); color.assign(n,0);
        ddfs_child.assign(n,{}); bridge_of.assign(n,{}); next_color=1;
        dsu.init(n); while (!killed_queue.empty()) killed_queue.pop();
    }
public:
    explicit MicaliVazirani(int n=0): n(n), g(n), mate(n,-1) {}
    void add_edge(int u,int v) {
        int ru=(int)g[v].size(), rv=(int)g[u].size();
        g[u].push_back({v,ru,EdgeState::Fresh});
        g[v].push_back({u,rv,EdgeState::Fresh});
    }
    vector<int> solve() {
        fill(mate.begin(),mate.end(),-1);
        while (true) { reset_phase_state(); if (!phase()) break; }
        return mate;
    }
};

struct CenterInfo { int r,c,a,b; };

int main() {
    ios::sync_with_stdio(false); cin.tie(nullptr);
    int n,m; if (!(cin>>n>>m)) return 0;
    vector<string> board(n); for (auto &s:board) cin>>s;
    vector<vector<int>> dot_id(n,vector<int>(m,-1));
    vector<pair<int,int>> vertex_cell; int V=0;
    for (int i=0;i<n;++i) for (int j=0;j<m;++j)
        if (board[i][j]=='.') { dot_id[i][j]=V++; vertex_cell.push_back({i,j}); }
    vector<CenterInfo> centers;
    for (int i=0;i<n;++i) for (int j=0;j<m;++j) if (board[i][j]!='.') {
        int a=V++,b=V++; vertex_cell.push_back({i,j}); vertex_cell.push_back({i,j});
        centers.push_back({i,j,a,b});
    }
    MicaliVazirani matcher(V);
    auto inside=[&](int x,int y){return 0<=x&&x<n&&0<=y&&y<m;};
    const int dr[4]={-1,1,0,0}, dc[4]={0,0,-1,1};
    for (auto const &cen:centers) {
        int i=cen.r,j=cen.c; matcher.add_edge(cen.a,cen.b);
        for (int d=0;d<4;++d) {
            int x=i+dr[d],y=j+dc[d];
            if (!inside(x,y)||board[x][y]!='.') continue;
            int w=dot_id[x][y];
            if (board[i][j]=='+') {
                matcher.add_edge(cen.a,w); matcher.add_edge(cen.b,w);
            } else {
                if (d>=2) matcher.add_edge(cen.a,w); else matcher.add_edge(cen.b,w);
            }
        }
    }
    vector<int> mate=matcher.solve();
    for (auto const &cen:centers) {
        int a=cen.a,b=cen.b;
        if (mate[a]==b) continue;
        bool ea=mate[a]!=-1, eb=mate[b]!=-1;
        if (ea&&eb) continue;
        if (ea) { int w=mate[a]; mate[w]=mate[a]=-1; }
        if (eb) { int w=mate[b]; mate[w]=mate[b]=-1; }
        mate[a]=b; mate[b]=a;
    }
    struct Tromino { array<pair<int,int>,3> cell; };
    vector<Tromino> pieces;
    for (auto const &cen:centers) if (mate[cen.a]!=cen.b) {
        int x=mate[cen.a],y=mate[cen.b];
        if (x==-1||y==-1) continue;
        pieces.push_back({array<pair<int,int>,3>{
            pair<int,int>{cen.r,cen.c},vertex_cell[x],vertex_cell[y]}});
    }
    vector<string> ans=board;
    for (auto const &p:pieces) {
        bool used[26]={};
        for (auto [r,c]:p.cell) for (int d=0;d<4;++d) {
            int x=r+dr[d],y=c+dc[d];
            if (inside(x,y)&&ans[x][y]>='a'&&ans[x][y]<='z')
                used[ans[x][y]-'a']=true;
        }
        int col=0; while (col<26&&used[col]) ++col; assert(col<26);
        char ch=char('a'+col); for (auto [r,c]:p.cell) ans[r][c]=ch;
    }
    for (auto const &s:ans) cout<<s<<'\n';
    return 0;
}
