#include <bits/stdc++.h>
using namespace std;
typedef unsigned long long u64;

int N, M, K;
u64 adj[64];
mt19937_64 rng(12345);

struct Plan {
    vector<int> c;            // spines
    vector<int> g;            // gaps per step
    vector<vector<int>> A, C, B;
    int D() const { int d = c.size(); for (int x : g) d += x; return d; }
};

static inline int dlo(int s) { return max(0, s - K + 1); }
static inline int dhi(int s) { return min(s, K - 1); }
static inline int dlen(int s) { if (s < 0 || s > 2 * K - 2) return 0; return dhi(s) - dlo(s) + 1; }

// capacity of step i given position of its first free diagonal f
int stepCap(const Plan& P, int i, int g, int f) {
    int a = P.c[i], b = P.c[i + 1];
    if (g == 0) return 0;
    if (f < 0 || f + g - 1 > 2 * K - 2) return -1000;
    if (g == 1) return dlen(f) - ((a != b && (adj[a] >> b & 1)) ? 1 : 0);
    // g==2
    int lo1 = dlo(f), hi2 = dhi(f + 1);
    return hi2 - lo1 - 1;
}

// compute free positions for each step given offset p
void positions(const Plan& P, int p, vector<int>& fpos) {
    int m = P.c.size() - 1;
    fpos.assign(m, 0);
    int s = p;
    for (int i = 0; i < m; i++) {
        fpos[i] = s + 1;
        s += 1 + P.g[i];
    }
}

bool checkCaps(const Plan& P, int p) {
    vector<int> fp;
    positions(P, p, fp);
    int m = P.c.size() - 1;
    for (int i = 0; i < m; i++) {
        if (P.g[i] == 0) continue;
        int used = P.A[i].size() + P.B[i].size() + P.C[i].size();
        if (used > stepCap(P, i, P.g[i], fp[i])) return false;
    }
    return true;
}

vector<vector<int>> build(const Plan& P, int p) {
    vector<vector<int>> G(K, vector<int>(K, 0));
    int m = P.c.size() - 1;
    int L = 2 * K - 1;
    vector<int> diagCol(L, -1);
    // fill spines
    int s = p;
    for (int d = 0; d < p; d++) diagCol[d] = P.c[0];
    vector<int> fp(m);
    for (int i = 0; i <= m; i++) {
        diagCol[s] = P.c[i];
        if (i < m) {
            fp[i] = s + 1;
            for (int t = 0; t < P.g[i]; t++) diagCol[s + 1 + t] = -2;
            s += 1 + P.g[i];
        }
    }
    for (int d = s + 1; d < L; d++) diagCol[d] = P.c[m];
    for (int d = 0; d < L; d++)
        if (diagCol[d] >= 0)
            for (int r = dlo(d); r <= dhi(d); r++) G[r][d - r] = diagCol[d];
    for (int i = 0; i < m; i++) {
        if (P.g[i] == 0) continue;
        int a = P.c[i], b = P.c[i + 1];
        int f = fp[i];
        if (P.g[i] == 1) {
            int r = dlo(f);
            for (int x : P.C[i]) { G[r][f - r] = x; r++; }
            bool adjab = (a == b) || (adj[a] >> b & 1);
            for (; r <= dhi(f); r++) G[r][f - r] = adjab ? a : P.C[i][0];
        } else {
            int lo1 = dlo(f), hi1 = dhi(f), lo2 = dlo(f + 1), hi2 = dhi(f + 1);
            int m1 = lo1 + P.A[i].size();
            int m2 = m1 + P.C[i].size();
            // F1
            for (int r = lo1; r <= hi1; r++) {
                int col;
                if (r < m1) col = P.A[i][r - lo1];
                else if (r < m2) col = P.C[i][r - m1];
                else col = b;
                G[r][f - r] = col;
            }
            for (int r = lo2; r <= hi2; r++) {
                int col;
                if (r <= m1) col = a;
                else if (r <= m2) col = b;
                else {
                    int idx = r - m2 - 1;
                    col = idx < (int)P.B[i].size() ? P.B[i][idx] : b;
                }
                G[r][f + 1 - r] = col;
            }
        }
    }
    return G;
}

bool validate(const vector<vector<int>>& G) {
    int k = G.size();
    u64 seen = 0;
    vector<u64> got(N, 0);
    for (int i = 0; i < k; i++)
        for (int j = 0; j < k; j++) {
            int x = G[i][j];
            if (x < 0 || x >= N) return false;
            seen |= 1ULL << x;
            int dx[2] = {0, 1}, dy[2] = {1, 0};
            for (int d = 0; d < 2; d++) {
                int ni = i + dx[d], nj = j + dy[d];
                if (ni >= k || nj >= k) continue;
                int y = G[ni][nj];
                if (y < 0 || y >= N) return false;
                if (x == y) continue;
                if (!(adj[x] >> y & 1)) return false;
                got[x] |= 1ULL << y;
                got[y] |= 1ULL << x;
            }
        }
    if (seen != ((N == 64) ? ~0ULL : ((1ULL << N) - 1))) return false;
    for (int x = 0; x < N; x++)
        if (got[x] != adj[x]) return false;
    return true;
}

// greedy hosting given walk plan with initial g/hosts
int stepGain(const Plan& P, int i, int ng, int R, const vector<u64>& unc) {
    int a = P.c[i], b = P.c[i + 1];
    if (R <= 0) return 0;
    if (a == b) return min(R, __builtin_popcountll(unc[a]));
    u64 two = unc[a] & unc[b];
    int c2 = __builtin_popcountll(two);
    u64 one;
    if (ng == 1) one = (unc[a] ^ unc[b]) & adj[a] & adj[b];
    else one = (unc[a] | unc[b]) & ~two;
    int c1 = __builtin_popcountll(one);
    int t2 = min(R, c2);
    return 2 * t2 + min(R - t2, c1);
}
void applyStep(Plan& P, int i, int bg, int R, vector<u64>& unc) {
    int a = P.c[i], b = P.c[i + 1];
    P.g[i] = bg;
    auto cov = [&](int x, int y) { unc[x] &= ~(1ULL << y); unc[y] &= ~(1ULL << x); };
    if (a == b) {
        u64 s = unc[a];
        while (s && R > 0) {
            int x = __builtin_ctzll(s); s &= s - 1;
            P.C[i].push_back(x); cov(a, x); R--;
        }
        return;
    }
    u64 two = unc[a] & unc[b];
    while (two && R > 0) {
        int x = __builtin_ctzll(two); two &= two - 1;
        P.C[i].push_back(x); cov(a, x); cov(b, x); R--;
    }
    u64 one;
    if (bg == 1) one = (unc[a] ^ unc[b]) & adj[a] & adj[b];
    else one = (unc[a] | unc[b]);
    while (one && R > 0) {
        int x = __builtin_ctzll(one); one &= one - 1;
        bool ua = unc[a] >> x & 1, ub = unc[b] >> x & 1;
        if (!ua && !ub) continue;
        if (bg == 1) P.C[i].push_back(x);
        else if (ua) P.A[i].push_back(x);
        else P.B[i].push_back(x);
        if (ua) cov(a, x);
        if (ub) cov(b, x);
        R--;
    }
}
bool greedy(Plan& P, vector<u64>& unc) {
    int Lmax = 2 * K - 1;
    while (true) {
        int m = P.c.size() - 1;
        bool any = false;
        for (int x = 0; x < N; x++) if (unc[x]) { any = true; break; }
        if (!any) break;
        int D = P.D();
        if (D > Lmax) return false;
        int p = max(0, (Lmax - D) / 2);
        vector<int> fp;
        positions(P, p, fp);
        double best = 0; int bi = -1, bg = 0, bR = 0; bool dup = false;
        for (int i = 0; i < m; i++) {
            int a = P.c[i], b = P.c[i + 1];
            bool adjab = (a == b) || (adj[a] >> b & 1);
            int used = P.A[i].size() + P.B[i].size() + P.C[i].size();
            for (int ng = max(1, P.g[i]); ng <= 2; ng++) {
                if (ng == 2 && (a == b || !adjab)) continue;
                int cost = ng - P.g[i];
                if (D + cost > Lmax) continue;
                int cap = stepCap(P, i, ng, fp[i]);
                int R = cap - used;
                int gain = stepGain(P, i, ng, R, unc);
                if (gain <= 0) continue;
                double sc = cost == 0 ? 1e9 + gain : (double)gain / cost + (rng() % 1000) * 1e-6;
                if (sc > best) { best = sc; bi = i; bg = ng; bR = R; dup = false; }
            }
        }
        if (D + 2 <= Lmax && best < 1e9) {
            for (int j = 0; j <= m; j++) {
                int a = P.c[j];
                int f = (j < m) ? fp[j] : p + D;  // position after spine j
                int cap = dlen(f);
                int gain = min(cap, __builtin_popcountll(unc[a]));
                if (gain <= 0) continue;
                double sc = gain / 2.0 + (rng() % 1000) * 1e-6;
                if (sc > best) { best = sc; bi = j; dup = true; bR = cap; }
            }
        }
        if (bi < 0) return false;
        if (dup) {
            int j = bi;
            P.c.insert(P.c.begin() + j + 1, P.c[j]);
            P.g.insert(P.g.begin() + j, 0);
            P.A.insert(P.A.begin() + j, vector<int>());
            P.B.insert(P.B.begin() + j, vector<int>());
            P.C.insert(P.C.begin() + j, vector<int>());
            applyStep(P, j, 1, bR, unc);
        } else {
            applyStep(P, bi, bg, bR, unc);
        }
    }
    return true;
}

vector<vector<int>> tree;
vector<int> tour;
void dfsTour(int u) {
    tour.push_back(u);
    for (int v : tree[u]) { dfsTour(v); tour.push_back(u); }
}

bool finish(Plan& P, vector<u64>& unc, vector<vector<int>>& out);
bool attemptB(vector<vector<int>>& out) {
    // independent set I, walk over W = V \ I
    vector<int> ord(N);
    iota(ord.begin(), ord.end(), 0);
    shuffle(ord.begin(), ord.end(), rng);
    int noise = rng() % 3;
    vector<int> key(N);
    for (int x = 0; x < N; x++) key[x] = __builtin_popcountll(adj[x]) + (noise ? (int)(rng() % (noise + 1)) : 0);
    stable_sort(ord.begin(), ord.end(), [&](int x, int y) { return key[x] < key[y]; });
    u64 I = 0, blocked = 0;
    for (int x : ord) if (!(blocked >> x & 1)) { I |= 1ULL << x; blocked |= adj[x] | (1ULL << x); }
    u64 Wm = ((1ULL << N) - 1) & ~I;
    if (!Wm) return false;
    // H: adjacency among W: direct or via common neighbor in I
    vector<int> Wv;
    for (int x = 0; x < N; x++) if (Wm >> x & 1) Wv.push_back(x);
    vector<u64> h1(N, 0), h2(N, 0);
    for (int x : Wv) {
        h1[x] = adj[x] & Wm;
        u64 r = 0;
        u64 s = adj[x] & I;
        while (s) { int y = __builtin_ctzll(s); s &= s - 1; r |= adj[y]; }
        h2[x] = r & Wm & ~h1[x] & ~(1ULL << x);
    }
    // DFS on H, preferring direct edges
    tree.assign(N, {});
    vector<int> vis(N, 0);
    int root = Wv[rng() % Wv.size()];
    vector<int> st = {root};
    vis[root] = 1;
    while (!st.empty()) {
        int u = st.back();
        vector<int> c1, c2;
        for (int v : Wv) if (!vis[v]) { if (h1[u] >> v & 1) c1.push_back(v); else if (h2[u] >> v & 1) c2.push_back(v); }
        int v;
        if (!c1.empty() && (c2.empty() || rng() % 4)) v = c1[rng() % c1.size()];
        else if (!c2.empty()) v = c2[rng() % c2.size()];
        else { st.pop_back(); continue; }
        vis[v] = 1; tree[u].push_back(v); st.push_back(v);
    }
    for (int x : Wv) if (!vis[x]) return false;
    vector<int> hgt(N, 0);
    function<int(int)> h = [&](int u) { int r = 0; for (int v : tree[u]) r = max(r, h(v) + 1); return hgt[u] = r; };
    h(root);
    for (int u : Wv) {
        auto& ch = tree[u];
        if (ch.empty()) continue;
        int bi = 0;
        for (int j = 1; j < (int)ch.size(); j++) if (hgt[ch[j]] > hgt[ch[bi]]) bi = j;
        swap(ch[bi], ch.back());
        if (ch.size() > 2) shuffle(ch.begin(), ch.end() - 1, rng);
    }
    tour.clear();
    dfsTour(root);
    {
        vector<int> first(N, -1);
        for (int i = 0; i < (int)tour.size(); i++) if (first[tour[i]] < 0) first[tour[i]] = i;
        int last = 0;
        for (int x : Wv) last = max(last, first[x]);
        tour.resize(last + 1);
    }
    Plan P;
    vector<u64> unc(adj, adj + N);
    for (int x : tour) {
        if (!P.c.empty()) {
            int u = P.c.back();
            P.A.push_back({}); P.B.push_back({}); P.C.push_back({});
            if (adj[u] >> x & 1) {
                P.g.push_back(0);
                unc[u] &= ~(1ULL << x); unc[x] &= ~(1ULL << u);
            } else {
                P.g.push_back(1);
                u64 cn = adj[u] & adj[x] & I;
                int y = __builtin_ctzll(cn);
                P.C.back().push_back(y);
                unc[u] &= ~(1ULL << y); unc[y] &= ~(1ULL << u);
                unc[x] &= ~(1ULL << y); unc[y] &= ~(1ULL << x);
            }
        }
        P.c.push_back(x);
    }
    return finish(P, unc, out);
}

bool attempt(int mode, vector<vector<int>>& out) {
    if (mode == 2) return attemptB(out);
    // build DFS tree
    tree.assign(N, {});
    vector<int> vis(N, 0);
    int root = rng() % N;
    // iterative DFS with choice heuristics
    vector<int> st;
    st.push_back(root);
    vis[root] = 1;
    vector<int> par(N, -1);
    while (!st.empty()) {
        int u = st.back();
        // choose next unvisited neighbor
        vector<int> cand;
        for (int v = 0; v < N; v++) if ((adj[u] >> v & 1) && !vis[v]) cand.push_back(v);
        if (cand.empty()) { st.pop_back(); continue; }
        int v;
        if (mode == 0) v = cand[rng() % cand.size()];
        else {
            // Warnsdorff: fewest unvisited neighbors
            int bestc = INT_MAX; vector<int> bs;
            for (int w : cand) {
                int cnt = 0;
                for (int z = 0; z < N; z++) if ((adj[w] >> z & 1) && !vis[z]) cnt++;
                if (cnt < bestc) { bestc = cnt; bs = {w}; }
                else if (cnt == bestc) bs.push_back(w);
            }
            v = bs[rng() % bs.size()];
        }
        vis[v] = 1;
        par[v] = u;
        tree[u].push_back(v);
        st.push_back(v);
    }
    // order children: put the deepest subtree last to reduce tour
    vector<int> depthSub(N, 0);
    // compute heights
    function<int(int)> h = [&](int u) {
        int r = 0;
        for (int v : tree[u]) r = max(r, h(v) + 1);
        return depthSub[u] = r;
    };
    h(root);
    for (int u = 0; u < N; u++) {
        auto& ch = tree[u];
        if (ch.empty()) continue;
        int bi = 0;
        for (int j = 1; j < (int)ch.size(); j++) if (depthSub[ch[j]] > depthSub[ch[bi]]) bi = j;
        swap(ch[bi], ch.back());
        if (ch.size() > 2) shuffle(ch.begin(), ch.end() - 1, rng);
    }
    tour.clear();
    dfsTour(root);
    // trim trailing returns
    {
        vector<int> first(N, -1);
        for (int i = 0; i < (int)tour.size(); i++) if (first[tour[i]] < 0) first[tour[i]] = i;
        int last = 0;
        for (int x = 0; x < N; x++) last = max(last, first[x]);
        tour.resize(last + 1);
    }
    Plan P;
    vector<u64> unc(adj, adj + N);
    // convert leaf excursions u c u
    int T = tour.size();
    for (int i = 0; i < T; i++) {
        int x = tour[i];
        if (i > 0 && i + 1 < T && tour[i - 1] == tour[i + 1] && tree[x].empty()) {
            // x is a leaf; previous spine is tour[i-1] already pushed
            P.g.push_back(1);
            P.A.push_back({}); P.B.push_back({}); P.C.push_back({x});
            int u = tour[i - 1];
            unc[u] &= ~(1ULL << x); unc[x] &= ~(1ULL << u);
            P.c.push_back(u);
            i++;  // skip the return u (already pushed)
            continue;
        }
        if (!P.c.empty()) {
            P.g.push_back(0);
            P.A.push_back({}); P.B.push_back({}); P.C.push_back({});
            int u = P.c.back();
            unc[u] &= ~(1ULL << x); unc[x] &= ~(1ULL << u);
        }
        P.c.push_back(x);
    }
    return finish(P, unc, out);
}
bool finish(Plan& P, vector<u64>& unc, vector<vector<int>>& out) {
    if (!greedy(P, unc)) return false;
    int D = P.D();
    int L = 2 * K - 1;
    if (D > L) return false;
    int p0 = (L - D) / 2;
    vector<int> ps;
    ps.push_back(p0);
    for (int p = 0; p <= L - D; p++) if (p != p0) ps.push_back(p);
    for (int p : ps) {
        if (!checkCaps(P, p)) continue;
        auto G = build(P, p);
        if (validate(G)) { out = G; return true; }
    }
    return false;
}

static chrono::steady_clock::time_point T0;
static double elapsed() { return chrono::duration<double>(chrono::steady_clock::now() - T0).count(); }
int main() {
    T0 = chrono::steady_clock::now();
    int T;
    scanf("%d", &T);
    while (T--) {
        scanf("%d %d", &N, &M);
        for (int i = 0; i < N; i++) adj[i] = 0;
        for (int i = 0; i < M; i++) {
            int a, b;
            scanf("%d %d", &a, &b);
            a--; b--;
            adj[a] |= 1ULL << b;
            adj[b] |= 1ULL << a;
        }
        vector<vector<int>> ans;
        int Kmax = 5 * N / 4;
        if (N == 1) {
            ans = {{0}};
        } else {
            bool ok = false;
            // try smallest K? just use Kmax
            K = Kmax;
            double now = elapsed();
            double budget = max(0.004, (0.40 - now) / (T + 1));
            for (int it = 0; !ok; it++) {
                ok = attempt(it % 3, ans);
                if ((it & 15) == 15 && elapsed() - now > budget && it >= 60) break;
            }
            if (!ok) {
                fprintf(stderr, "FAIL N=%d M=%d\n", N, M);
                // fallback (exceeds bound, but well-formed)
                for (K = Kmax + 1; !ok; K++)
                    for (int it = 0; it < 30 && !ok; it++) ok = attempt(it % 3, ans);
            }
        }
        printf("%d\n", (int)ans.size());
        for (auto& row : ans) {
            for (int j = 0; j < (int)row.size(); j++) printf("%d%c", row[j] + 1, j + 1 == (int)row.size() ? '\n' : ' ');
        }
    }
}
