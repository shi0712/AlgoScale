#include <bits/stdc++.h>
using namespace std;

// ---------------------------------------------------------------- oracle
#ifdef LOCAL
namespace judge {
int N;
vector<bitset<608>> adj;
long long queries = 0;
long long ask(const vector<int>& s) {
    queries++;
    bitset<608> m;
    for (int x : s) m[x] = 1;
    long long c = 0;
    for (int x : s) c += (adj[x] & m).count();
    return c / 2;
}
}  // namespace judge
static long long rawAsk(const vector<int>& s) { return judge::ask(s); }
#else
static long long rawAsk(const vector<int>& s) {
    string out = "? " + to_string(s.size()) + "\n";
    for (size_t i = 0; i < s.size(); i++) {
        if (i) out += ' ';
        out += to_string(s[i]);
    }
    out += '\n';
    fputs(out.c_str(), stdout);
    fflush(stdout);
    long long r;
    if (scanf("%lld", &r) != 1 || r < 0) exit(0);
    return r;
}
#endif

static map<vector<int>, long long> qcache;
static long long ask(const vector<int>& s0) {
    vector<int> s = s0;
    sort(s.begin(), s.end());
    s.erase(unique(s.begin(), s.end()), s.end());
    if (s.size() <= 1) return 0;
    auto it = qcache.find(s);
    if (it != qcache.end()) return it->second;
    long long r = rawAsk(s);
    qcache[s] = r;
    return r;
}

// ---------------------------------------------------------------- solver
struct Result {
    bool bip;
    vector<int> seq;
};

struct Params {
    double lazyEnter = 0.25;
    double ratio = 1.5;
    int spot = 1000000;
    double k0 = 4;
    bool pairFrontier = true, pool = true;
    int sample = 1024, maxRestarts = 3;
    bool exactDefer = true, evict = true;
    int deferMin = 8, evictSize = 6;
    double evictFrac = 1.5, evD = 1e9, evFrBig = 1;
    int evSzBig = 1000;
    double wA = 1.3, wG = 1, wMin = 0.3;
    double coinLimit = 16384, batchC = 4.0;
    int batchMax = 32;
    double guessRisk = 0.3;
    int window = 40;
};

struct WrongGuess {
    int w;
};
long long dbgItemQ, dbgItemCoins, dbgDQ, dbgDCoins; double dbgItemH, dbgH, dbgHn, dbgC, dbgA, dbgOver, dbgBatches;
#ifdef LOCAL
static double envd(const char* k, double d) { const char* v = getenv(k); return v ? atof(v) : d; }
#endif

struct Solver {
    struct Child {
        int id;
        vector<int> snap;  // side of child adjacent to creator, sorted by time
        long long cnt;     // number of edges creator -> snap (-1 unknown)
        int nb = -1;       // cached earliest neighbour
    };
    struct Ver {
        vector<int> mem;  // sorted by time
        int creator = -1;
        vector<Child> ch;
        long long E = 0;
        bitset<608> bs;
    };
    struct St {
        vector<Ver> ver;
        vector<int> color, tim;
        vector<int> active;
        long long qW = 0;
        int qWt = 0;
        vector<int> W;
        int idx = 0;
        bool lazy = false;
        int ec = 0;                 // exact side colour in lazy mode
        vector<int> dirtyBase;      // dirty side at lazy entry (clean)
        vector<int> dirtyList;      // vertices added to dirty side
        vector<int> dirtyPrev;      // version before that vertex
        vector<long long> dirtyA;   // edges to exact side
        vector<char> obs;           // isolation observations
        int checkAt = 0, clock = 0, aZero = 0, forceSingle = 0;
        deque<int> pend;
        vector<char> deferred;
    } S;
    int n;
    vector<int> order;
    Params P;
#ifdef LOCAL
    Solver() {
        P.lazyEnter = envd("LAZY", P.lazyEnter); P.guessRisk = envd("GRISK", P.guessRisk);
        P.ratio = envd("RATIO", P.ratio); P.spot = (int)envd("SPOT", P.spot); P.k0 = envd("K0", P.k0);
        P.deferMin = (int)envd("DMIN", P.deferMin); P.coinLimit = envd("CLIM", P.coinLimit);
        P.batchC = envd("BC", P.batchC); P.batchMax = (int)envd("BMAX", P.batchMax);
        P.window = (int)envd("WIN", P.window); P.spot = (int)envd("SPOT", P.spot); P.exactDefer = envd("EDEF", 1); P.evictSize = (int)envd("ESZ", P.evictSize); P.pool = envd("POOL", 1); P.sample = (int)envd("SAMP", P.sample); P.maxRestarts = (int)envd("MAXR", P.maxRestarts); P.evictFrac = envd("EFR", P.evictFrac); P.evict = envd("EVICT", 1); P.wA = envd("WA", P.wA); P.wG = envd("WG", P.wG); P.wMin = envd("WMIN", P.wMin); P.evD = envd("EVD", P.evD); P.evFrBig = envd("EVFB", P.evFrBig); P.evSzBig = (int)envd("EVSB", P.evSzBig);
    }
#endif
    bool noGuess = false;
    int restarts = 0;
    vector<char> badGuess = vector<char>(700, 0);
    bool haveCk = false;
    St ck;
    Result res;

    // ------------------------------------------------ helpers
    long long edgesTo(int w, const vector<int>& Sv, size_t l, size_t r) {
        vector<int> q;
        q.reserve(r - l + 1);
        q.push_back(w);
        for (size_t i = l; i < r; i++) q.push_back(Sv[i]);
        return ask(q);
    }
    // earliest neighbour of w in independent list (time sorted); c known count>0 or -1 (unknown, but >0)
    int earliestNb(int w, const vector<int>& Sv, long long c) {
        size_t l = 0, r = Sv.size();
        while (r - l > 1) {
            if (c == (long long)(r - l)) return Sv[l];
            size_t m = (l + r) / 2;
            long long x = edgesTo(w, Sv, l, m);
            if (x > 0) r = m, c = x;
            else {
                l = m;
                // c unchanged (if known)
            }
        }
        return Sv[l];
    }
    vector<int> path(int x, int y, int id) {
        if (x == y) return {x};
        Ver& V = S.ver[id];
        int w = V.creator;
        int cx = -1, cy = -1;
        for (int i = 0; i < (int)V.ch.size(); i++) {
            if (S.ver[V.ch[i].id].bs[x]) cx = i;
            if (S.ver[V.ch[i].id].bs[y]) cy = i;
        }
        if (cx >= 0 && cx == cy) return path(x, y, V.ch[cx].id);
        auto nbOf = [&](int ci) {
            Child& c = S.ver[id].ch[ci];
            if (c.nb < 0) {
                if (c.cnt < 0) {
                    c.cnt = edgesTo(w, c.snap, 0, c.snap.size());
                    if (c.cnt == 0) throw WrongGuess{w};
                }
                c.nb = earliestNb(w, c.snap, c.cnt);
            }
            return c.nb;
        };
        vector<int> resv;
        if (x != w) {
            int nb = nbOf(cx);
            resv = path(x, nb, S.ver[id].ch[cx].id);
        }
        resv.push_back(w);
        if (y != w) {
            int nb = nbOf(cy);
            vector<int> p = path(y, nb, S.ver[id].ch[cy].id);
            reverse(p.begin(), p.end());
            resv.insert(resv.end(), p.begin(), p.end());
        }
        return resv;
    }

    double degEst() {
        int t = S.qWt;
        if (t < 2) return 2.0;
        double e = (double)S.qW * (double)n * (n - 1) / ((double)t * (t - 1));
        return max(2.0, 2.0 * e / n);
    }
    double pIso() {
        double f = (double)S.W.size() / n;
        double model = pow(max(0.0, 1.0 - f), degEst());
        int cnt = 0, iso = 0;
        for (int i = (int)S.obs.size() - 1; i >= 0 && cnt < P.window; i--, cnt++) iso += S.obs[i];
        return (iso + P.k0 * model) / (cnt + P.k0);
    }

    // ------------------------------------------------ exact mode (batched)
    struct Item {
        int comp, col;
        double w;
        long long sz;
        vector<int> mem;
    };
    struct HN {
        int l = -1, r = -1, par = -1, a = 0, b = 0;
        vector<int> items;
        long long cap = 0;
        double w = 0;
    };
    vector<Item> items;
    vector<HN> hn;
    int hroot = -1;

    void buildItems() {
        double D = degEst();
        items.clear();
        for (int id : S.active) {
            vector<int> sd[2];
            for (int u : S.ver[id].mem) sd[S.color[u]].push_back(u);
            for (int c = 0; c < 2; c++)
                if (!sd[c].empty()) {
                    double w = max(P.wMin, sd[c].size() * D * P.wA - (double)S.ver[id].E + P.wG);
                    items.push_back({id, c, w, (long long)sd[c].size(), sd[c]});
                }
        }
        hn.clear();
        priority_queue<pair<double, int>, vector<pair<double, int>>, greater<>> pq;
        for (int i = 0; i < (int)items.size(); i++) {
            HN h;
            h.items = {i};
            h.cap = items[i].sz;
            h.w = items[i].w;
            hn.push_back(h);
            pq.push({items[i].w, i});
        }
        while (pq.size() > 1) {
            auto [w1, x] = pq.top();
            pq.pop();
            auto [w2, y] = pq.top();
            pq.pop();
            HN h;
            h.l = x, h.r = y;
            h.items = hn[x].items;
            h.items.insert(h.items.end(), hn[y].items.begin(), hn[y].items.end());
            h.cap = hn[x].cap + hn[y].cap;
            h.w = w1 + w2;
            hn.push_back(h);
            int id = hn.size() - 1;
            hn[x].par = hn[y].par = id;
            pq.push({w1 + w2, id});
        }
        hroot = pq.top().second;
        int ctr = 0;
        auto dfs = [&](auto&& self, int x) -> void {
            if (hn[x].l < 0) {
                hn[x].a = ctr++;
                hn[x].b = ctr;
                return;
            }
            self(self, hn[x].l);
            self(self, hn[x].r);
            hn[x].a = hn[hn[x].l].a;
            hn[x].b = hn[hn[x].r].b;
        };
        dfs(dfs, hroot);
    }

    // generic adaptive coin weighing. coins have ranges and priors.
    // closure(set) -> closed set of coin indices actually included by the query realising `set`
    // query(closedSet, vals, resolved) -> sum of unresolved coin values in closedSet
    struct Coin {
        long long lo, hi;
        vector<double> pr;
    };
    static vector<double> binomPrior(long long c, double p, long long lo, long long hi) {
        vector<double> v;
        p = min(0.98, max(0.02, p));
        for (long long x = lo; x <= hi; x++) {
            double lg = lgamma(c + 1.0) - lgamma(x + 1.0) - lgamma(c - x + 1.0) + x * log(p) + (c - x) * log(1 - p);
            v.push_back(exp(lg));
        }
        return v;
    }
    template <class CL, class QF>
    vector<long long> resolveCoins(const vector<Coin>& coins, CL closure, QF query, long long total = -1) {
        int K = coins.size();
        vector<long long> vals(K, 0);
        vector<char> res(K, 0);
        for (int i = 0; i < K; i++)
            if (coins[i].lo == coins[i].hi) vals[i] = coins[i].lo, res[i] = 1;
        while (true) {
            vector<int> chunk;
            double prod = 1;
            for (int i = 0; i < K; i++)
                if (!res[i]) {
                    double r = coins[i].hi - coins[i].lo + 1;
                    if (total < 0 && !chunk.empty() && prod * r > P.coinLimit) continue;
                    chunk.push_back(i);
                    prod *= r;
                }
            if (chunk.empty()) break;
            int C = chunk.size();
            vector<int> pos(K, -1);
            for (int j = 0; j < C; j++) pos[chunk[j]] = j;
            // enumerate (values stored as offsets from lo)
            vector<int> asg;  // flat A*C
            vector<double> wt{1.0};
            if (total >= 0) {
                long long rem = total;
                for (int i = 0; i < K; i++)
                    if (res[i]) rem -= vals[i];
                    else rem -= coins[i].lo;
                vector<int> cur(C);
                wt.clear();
                auto dfs = [&](auto&& self, int j, long long left, double w) -> void {
                    if (j == C) {
                        if (left == 0) {
                            asg.insert(asg.end(), cur.begin(), cur.end());
                            wt.push_back(w);
                        }
                        return;
                    }
                    const Coin& cn = coins[chunk[j]];
                    int R = cn.hi - cn.lo + 1;
                    for (int x = 0; x < R && x <= left; x++) {
                        cur[j] = x;
                        self(self, j + 1, left - x, w * max(1e-12, cn.pr[x]));
                    }
                };
                dfs(dfs, 0, rem, 1.0);
                total = -1;
            } else {
                asg.reserve((size_t)prod * C);
                vector<int> curA;  // A*j
                size_t A = 1;
                for (int j = 0; j < C; j++) {
                    const Coin& cn = coins[chunk[j]];
                    int R = cn.hi - cn.lo + 1;
                    vector<int> na;
                    na.reserve(A * R * (j + 1));
                    vector<double> nw;
                    nw.reserve(A * R);
                    for (size_t a2 = 0; a2 < A; a2++)
                        for (int x = 0; x < R; x++) {
                            for (int t = 0; t < j; t++) na.push_back(curA[a2 * j + t]);
                            na.push_back(x);
                            nw.push_back(wt[a2] * max(1e-12, cn.pr[x]));
                        }
                    curA.swap(na);
                    wt.swap(nw);
                    A *= R;
                }
                asg.swap(curA);
            }
            size_t A = wt.size();
            vector<long long> sm(A), sm2(A);
            vector<double> bucket;
            while (A > 1) {
                vector<int> cand;
                for (int j = 0; j < C; j++) {
                    bool var = false;
                    for (size_t a2 = 1; a2 < A && !var; a2++)
                        if (asg[a2 * C + j] != asg[j]) var = true;
                    if (var) cand.push_back(chunk[j]);
                }
                vector<int> cur;
                fill(sm.begin(), sm.begin() + A, 0);
                double bestH = 0;
                while (true) {
                    vector<int> bestCl;
                    double bh = bestH + 1e-9;
                    for (int c : cand) {
                        if (find(cur.begin(), cur.end(), c) != cur.end()) continue;
                        vector<int> t = cur;
                        t.push_back(c);
                        vector<int> cl = closure(t);
                        bool ok = true;
                        vector<int> add;
                        for (int i : cl) {
                            if (!res[i] && pos[i] < 0) ok = false;
                            if (pos[i] >= 0 && find(cur.begin(), cur.end(), i) == cur.end()) add.push_back(pos[i]);
                        }
                        if (!ok) continue;
                        long long mx = 0;
                        for (size_t a2 = 0; a2 < A; a2++) {
                            long long v = sm[a2];
                            for (int j : add) v += asg[a2 * C + j];
                            sm2[a2] = v;
                            mx = max(mx, v);
                        }
                        bucket.assign(mx + 1, 0.0);
                        double tot = 0;
                        for (size_t a2 = 0; a2 < A; a2++) bucket[sm2[a2]] += wt[a2], tot += wt[a2];
                        double h = 0;
                        for (double w : bucket)
                            if (w > 0) h -= w / tot * log(w / tot);
                        if (h > bh) bh = h, bestCl = cl;
                    }
                    if (bestCl.empty()) break;
                    for (int i : bestCl)
                        if (pos[i] >= 0 && find(cur.begin(), cur.end(), i) == cur.end())
                            for (size_t a2 = 0; a2 < A; a2++) sm[a2] += asg[a2 * C + pos[i]];
                    cur = bestCl;
                    bestH = bh;
                }
                long long obs = query(cur, vals, res);
                long long base = 0;
                for (int i : cur)
                    if (pos[i] >= 0) base += coins[i].lo;
                obs -= base;
                size_t w2 = 0;
                for (size_t a2 = 0; a2 < A; a2++)
                    if (sm[a2] == obs) {
                        if (w2 != a2) {
                            for (int j = 0; j < C; j++) asg[w2 * C + j] = asg[a2 * C + j];
                            wt[w2] = wt[a2];
                        }
                        w2++;
                    }
                A = w2;
                if (A == 0) throw runtime_error("inconsistent");
            }
            for (int j = 0; j < C; j++) vals[chunk[j]] = coins[chunk[j]].lo + asg[j], res[chunk[j]] = 1;
        }
        return vals;
    }

    void newSingleton(int v) {
        Ver V;
        V.mem = {v};
        V.bs[v] = 1;
        S.color[v] = 0;
        S.ver.push_back(V);
        S.active.push_back(S.ver.size() - 1);
    }

    // d-phase: determine d for each vertex of independent batch B given total T
    static double logBinom(long long a, long long b) { return lgamma(a + 1.0) - lgamma(b + 1.0) - lgamma(a - b + 1.0); }
    vector<long long> dPhase(const vector<int>& B, long long T) {
        int k = B.size();
        vector<long long> d(k, 0);
        // groups (l,r,total); split while compositions too many
        vector<array<long long, 3>> st{{0, k, T}};
        while (!st.empty()) {
            auto [l, r, c] = st.back();
            st.pop_back();
            if (c == 0) continue;
            if (r - l == 1) {
                d[l] = c;
                continue;
            }
            if (logBinom(c + r - l - 1, r - l - 1) > log(P.coinLimit)) {
                long long m = (l + r) / 2;
                vector<int> q = S.W;
                for (long long j = l; j < m; j++) q.push_back(B[j]);
                long long x = ask(q) - S.qW;
                st.push_back({l, m, x});
                st.push_back({m, r, c - x});
                continue;
            }
            vector<Coin> coins;
            for (long long j = l; j < r; j++) {
                Coin cn{0, c, {}};
                double f = 1;
                for (long long x = 0; x <= c; x++) {
                    if (x > 0) f *= x;
                    cn.pr.push_back(1.0 / f);
                }
                coins.push_back(cn);
            }
            auto closure = [&](const vector<int>& t) { return t; };
            auto query = [&](const vector<int>& cl, const vector<long long>& vals, const vector<char>& res) {
                vector<int> q = S.W;
                long long known = 0;
                for (int i : cl) {
                    q.push_back(B[l + i]);
                    if (res[i]) known += vals[i];
                }
                return ask(q) - S.qW - known;
            };
            vector<long long> x = resolveCoins(coins, closure, query, c);
            for (long long j = l; j < r; j++) d[j] = x[j - l];
        }
        return d;
    }

    // item phase: for each vertex, list of (item, count)
    vector<vector<pair<int, long long>>> itemPhase(const vector<int>& B, const vector<long long>& d) {
        int k = B.size();
        int H = hn.size();
        vector<vector<char>> st(k, vector<char>(H, 0));  // 1 zero, 2 sat, 3 coin, 4 leaf
        vector<vector<pair<int, long long>>> found(k);
        struct En {
            int u, node;
            long long c;
        };
        vector<En> cur;
        auto norm = [&](int u, int N, long long c, vector<En>& out) {
            if (c == 0) {
                st[u][N] = 1;
                return;
            }
            if (c == hn[N].cap) {
                st[u][N] = 2;
                for (int i : hn[N].items) found[u].push_back({i, items[i].sz});
                return;
            }
            if (hn[N].l < 0) {
                st[u][N] = 4;
                found[u].push_back({hn[N].items[0], c});
                return;
            }
            st[u][N] = 3;
            out.push_back({u, N, c});
        };
        for (int u = 0; u < k; u++)
            if (d[u] > 0) norm(u, hroot, d[u], cur);
        while (!cur.empty()) {
            int K = cur.size();
            vector<Coin> coins;
            for (auto& e : cur) {
                int L = hn[e.node].l, R = hn[e.node].r;
                long long lo = max(0LL, e.c - hn[R].cap), hi = min(e.c, hn[L].cap);
                coins.push_back({lo, hi, binomPrior(e.c, hn[L].w / hn[e.node].w, lo, hi)});
            }
            map<pair<int, int>, int> coinAt;
            for (int i = 0; i < K; i++) coinAt[{cur[i].u, cur[i].node}] = i;
            auto closure = [&](const vector<int>& t) {
                set<int> us, ns;
                for (int i : t) us.insert(cur[i].u), ns.insert(cur[i].node);
                vector<int> cl;
                for (int i = 0; i < K; i++)
                    if (us.count(cur[i].u) && ns.count(cur[i].node)) cl.push_back(i);
                return cl;
            };
            auto query = [&](const vector<int>& cl, const vector<long long>& vals, const vector<char>& res) {
                set<int> us, ns;
                for (int i : cl) us.insert(cur[i].u), ns.insert(cur[i].node);
                vector<int> q;
                map<int, int> mask;
                for (int u : us) q.push_back(B[u]);
                for (int N : ns)
                    for (int it : hn[hn[N].l].items) {
                        q.insert(q.end(), items[it].mem.begin(), items[it].mem.end());
                        mask[items[it].comp] |= 1 << items[it].col;
                    }
                long long known = 0;
                for (auto [c, m] : mask)
                    if (m == 3) known += S.ver[c].E;
                for (int u : us)
                    for (int N : ns) {
                        auto itc = coinAt.find({u, N});
                        if (itc != coinAt.end()) {
                            if (res[itc->second]) known += vals[itc->second];
                            continue;
                        }
                        // walk up
                        int x = N;
                        while (x >= 0 && st[u][x] != 1 && st[u][x] != 2) x = hn[x].par;
                        if (x >= 0 && st[u][x] == 2) known += hn[hn[N].l].cap;
                    }
                return ask(q) - known;
            };
#ifdef DBG
            long long q0 = judge::queries;
#endif
            vector<long long> x = resolveCoins(coins, closure, query);
#ifdef DBG
            dbgItemQ += judge::queries - q0; dbgItemCoins += K;
            for (auto& cn : coins) { double t = 0, h = 0; for (double p : cn.pr) t += p; for (double p : cn.pr) if (p > 0) h -= p / t * log2(p / t); dbgItemH += h; }
#endif
            vector<En> next;
            for (int i = 0; i < K; i++) {
                auto e = cur[i];
                st[e.u][e.node] = 0;
                norm(e.u, hn[e.node].l, x[i], next);
                norm(e.u, hn[e.node].r, e.c - x[i], next);
            }
            cur = next;
        }
        return found;
    }

    // ---------------- asynchronous coin pool (d-values and item splits together)
    struct PCoin {
        int type;  // 0: d of vertex u ; 1: split of node for vertex u
        int u, node;
        long long c, lo, hi;
        vector<double> pr;
    };
    void poolPhase(const vector<int>& B, long long T, vector<long long>& d,
                   vector<vector<pair<int, long long>>>& found) {
        int k = B.size();
        int H = hn.size();
        d.assign(k, -1);
        found.assign(k, {});
        vector<vector<char>> st(k, vector<char>(H, 0));
        vector<vector<long long>> cnt(k, vector<long long>(H, 0));
        vector<vector<int>> coinAt(k, vector<int>(H, -1));
        vector<PCoin> pc;
        // queue of groups: (coin ids, sum constraint or -1)
        deque<pair<vector<int>, long long>> qu;
        vector<char> inTable;
        auto addCoin = [&](PCoin c) {
            pc.push_back(c);
            inTable.push_back(0);
            return (int)pc.size() - 1;
        };
        auto norm = [&](int u, int N, long long c) {
            cnt[u][N] = c;
            if (c == 0) {
                st[u][N] = 1;
                return;
            }
            if (c == hn[N].cap) {
                st[u][N] = 2;
                for (int i : hn[N].items) found[u].push_back({i, items[i].sz});
                return;
            }
            if (hn[N].l < 0) {
                st[u][N] = 4;
                found[u].push_back({hn[N].items[0], c});
                return;
            }
            st[u][N] = 3;
            int L = hn[N].l, R = hn[N].r;
            long long lo = max(0LL, c - hn[R].cap), hi = min(c, hn[L].cap);
            int id = addCoin({1, u, N, c, lo, hi, binomPrior(c, hn[L].w / hn[N].w, lo, hi)});
            coinAt[u][N] = id;
            qu.push_back({{id}, -1});
        };
        // d groups
        {
            vector<array<long long, 3>> stk{{0, k, T}};
            while (!stk.empty()) {
                auto [l, r, c] = stk.back();
                stk.pop_back();
                if (c == 0) {
                    for (long long j = l; j < r; j++) d[j] = 0;
                    continue;
                }
                if (r - l == 1) {
                    d[l] = c;
                    continue;
                }
                if (logBinom(c + r - l - 1, r - l - 1) > log(P.coinLimit)) {
                    long long m = (l + r) / 2;
                    vector<int> q = S.W;
                    for (long long j = l; j < m; j++) q.push_back(B[j]);
                    long long x = ask(q) - S.qW;
                    stk.push_back({l, m, x});
                    stk.push_back({m, r, c - x});
                    continue;
                }
                vector<int> ids;
                for (long long j = l; j < r; j++) {
                    PCoin cn{0, (int)j, -1, c, 0, c, {}};
                    double f = 1;
                    for (long long x = 0; x <= c; x++) {
                        if (x > 0) f *= x;
                        cn.pr.push_back(1.0 / f);
                    }
                    ids.push_back(addCoin(cn));
                }
                qu.push_back({ids, c});
            }
        }
        for (int u = 0; u < k; u++)
            if (d[u] > 0) norm(u, hroot, d[u]);
        // table
        vector<int> cols;
        vector<int> tab;  // A x C
        vector<double> wt{1.0};
        size_t A = 1;
        auto C = [&]() { return (int)cols.size(); };
        auto enumGroup = [&](const vector<int>& ids, long long sum, vector<int>& out, vector<double>& ow) {
            int g = ids.size();
            vector<int> cur(g);
            auto dfs = [&](auto&& self, int j, long long left, double w) -> void {
                if (j == g) {
                    if (sum < 0 || left == 0) {
                        out.insert(out.end(), cur.begin(), cur.end());
                        ow.push_back(w);
                    }
                    return;
                }
                const PCoin& cn = pc[ids[j]];
                int R = cn.hi - cn.lo + 1;
                for (int x = 0; x < R; x++) {
                    if (sum >= 0 && x > left) break;
                    cur[j] = x;
                    self(self, j + 1, sum >= 0 ? left - x : 0, w * max(1e-12, cn.pr[x]));
                }
            };
            long long s0 = sum;
            if (sum >= 0)
                for (int id : ids) s0 -= pc[id].lo;
            dfs(dfs, 0, s0, 1.0);
        };
        // cached enumeration for the queue front
        vector<int> fe;
        vector<double> fw;
        bool feValid = false;
        auto fill = [&]() {
            while (!qu.empty()) {
                auto& [ids, sum] = qu.front();
                if (!feValid) {
                    fe.clear();
                    fw.clear();
                    enumGroup(ids, sum, fe, fw);
                    feValid = true;
                }
                vector<int>& e = fe;
                vector<double>& ew = fw;
                size_t E = ew.size();
                if (C() > 0 && A * E > (size_t)P.coinLimit) break;
                int g = ids.size(), c0 = C();
                vector<int> nt;
                nt.reserve(A * E * (c0 + g));
                vector<double> nw;
                nw.reserve(A * E);
                for (size_t a2 = 0; a2 < A; a2++)
                    for (size_t e2 = 0; e2 < E; e2++) {
                        for (int j = 0; j < c0; j++) nt.push_back(tab[a2 * c0 + j]);
                        for (int j = 0; j < g; j++) nt.push_back(e[e2 * g + j]);
                        nw.push_back(wt[a2] * ew[e2]);
                    }
                for (int id : ids) cols.push_back(id), inTable[id] = 1;
                tab.swap(nt);
                wt.swap(nw);
                A *= E;
                qu.pop_front();
                feValid = false;
            }
        };
        // known edges of vertex u into node Z (-1 unknown)
        auto known = [&](auto&& self, int u, int Z) -> long long {
            char s0 = st[u][Z];
            if (s0 == 1) return 0;
            if (s0 == 2) return hn[Z].cap;
            if (s0 == 3 || s0 == 4) return cnt[u][Z];
            int X = hn[Z].par;
            while (X >= 0 && st[u][X] == 0) X = hn[X].par;
            if (X >= 0) {
                if (st[u][X] == 1) return 0;
                if (st[u][X] == 2) return hn[Z].cap;
                return -1;
            }
            if (hn[Z].l < 0) return -1;
            long long a = self(self, u, hn[Z].l);
            if (a < 0) return -1;
            long long b = self(self, u, hn[Z].r);
            if (b < 0) return -1;
            return a + b;
        };
        // closure for item query: returns false if invalid; fills included table cols & known offset
        auto closureI = [&](const vector<int>& chosen, vector<int>& incl, long long& off) {
            vector<int> us, ns;
            for (int id : chosen) {
                if (find(us.begin(), us.end(), pc[id].u) == us.end()) us.push_back(pc[id].u);
                if (find(ns.begin(), ns.end(), pc[id].node) == ns.end()) ns.push_back(pc[id].node);
            }
            for (size_t i = 0; i < ns.size(); i++)
                for (size_t j = i + 1; j < ns.size(); j++) {
                    const HN &x = hn[hn[ns[i]].l], &y = hn[hn[ns[j]].l];
                    if (x.a < y.b && y.a < x.b) return false;
                }
            incl.clear();
            off = 0;
            for (int u : us)
                for (int N : ns) {
                    int id = coinAt[u][N];
                    if (id >= 0 && st[u][N] == 3) {
                        if (!inTable[id]) return false;
                        incl.push_back(id);
                        continue;
                    }
                    long long kv = known(known, u, hn[N].l);
                    if (kv < 0) return false;
                    off += kv;
                }
            return true;
        };
        auto retire = [&]() {
            int c0 = C();
            vector<char> cst(c0, 1);
            for (size_t a2 = 1; a2 < A; a2++)
                for (int j = 0; j < c0; j++)
                    if (cst[j] && tab[a2 * c0 + j] != tab[j]) cst[j] = 0;
            vector<int> keep, gone;
            for (int j = 0; j < c0; j++) (cst[j] ? gone : keep).push_back(j);
            if (gone.empty()) return;
            vector<long long> gv;
            for (int j : gone) gv.push_back(pc[cols[j]].lo + tab[j]);
            vector<int> goneIds;
            for (int j : gone) goneIds.push_back(cols[j]);
            int c1 = keep.size();
            vector<int> nt(A * c1);
            for (size_t a2 = 0; a2 < A; a2++)
                for (int t = 0; t < c1; t++) nt[a2 * c1 + t] = tab[a2 * c0 + keep[t]];
            tab.swap(nt);
            vector<int> nc;
            for (int j : keep) nc.push_back(cols[j]);
            cols.swap(nc);
            for (size_t i = 0; i < goneIds.size(); i++) {
                int id = goneIds[i];
                long long v = gv[i];
                inTable[id] = 0;
                PCoin cn = pc[id];
                if (cn.type == 0) {
                    d[cn.u] = v;
                    if (v > 0) norm(cn.u, hroot, v);
                } else {
                    st[cn.u][cn.node] = 0;
                    coinAt[cn.u][cn.node] = -1;
                    norm(cn.u, hn[cn.node].l, v);
                    norm(cn.u, hn[cn.node].r, cn.c - v);
                }
            }
        };
        vector<long long> sm, sm2;
        vector<double> bucket;
        while (true) {
            fill();
            retire();
            fill();
            if (C() == 0) {
                if (qu.empty()) break;
                continue;
            }
            int c0 = C();
            sm.assign(A, 0);
            sm2.assign(A, 0);
            vector<int> chosen, incl;
            long long off = 0;
            int type = -1;
            double bestH = 0;
            bool fullScan = false;
            while (true) {
                int bestJ = -1;
                vector<int> bestIncl;
                long long bestOff = 0;
                double bh = bestH + 1e-9;
                for (int j = 0; j < c0; j++) {
                    int id = cols[j];
                    if (find(chosen.begin(), chosen.end(), id) != chosen.end()) continue;
                    if (find(incl.begin(), incl.end(), id) != incl.end()) continue;
                    if (type >= 0 && pc[id].type != type) continue;
                    vector<int> ch2 = chosen;
                    ch2.push_back(id);
                    vector<int> in2;
                    long long of2 = 0;
                    if (pc[id].type == 0) in2 = ch2;
                    else if (!closureI(ch2, in2, of2)) continue;
                    // new columns
                    vector<int> add;
                    for (int x : in2)
                        if (find(incl.begin(), incl.end(), x) == incl.end())
                            add.push_back(find(cols.begin(), cols.end(), x) - cols.begin());
                    long long mx = 0;
                    size_t stp = fullScan ? 1 : max<size_t>(1, A / P.sample);
                    for (size_t a2 = 0; a2 < A; a2 += stp) {
                        long long v = sm[a2];
                        for (int t : add) v += tab[a2 * c0 + t];
                        sm2[a2] = v;
                        mx = max(mx, v);
                    }
                    bucket.assign(mx + 1, 0.0);
                    double tot = 0;
                    for (size_t a2 = 0; a2 < A; a2 += stp) bucket[sm2[a2]] += wt[a2], tot += wt[a2];
                    double h = 0;
                    for (double w : bucket)
                        if (w > 0) h -= w / tot * log(w / tot);
                    if (h > bh) bh = h, bestJ = j, bestIncl = in2, bestOff = of2;
                }
                if (bestJ < 0 && chosen.empty() && !fullScan) {
                    fullScan = true;
                    continue;
                }
                if (bestJ < 0 && chosen.empty()) {
                    // fallback: any coin that is valid alone
                    for (int j = 0; j < c0 && bestJ < 0; j++) {
                        int id = cols[j];
                        vector<int> ch1{id}, in2{id};
                        long long of2 = 0;
                        if (pc[id].type == 1 && !closureI(ch1, in2, of2)) continue;
                        bestJ = j, bestIncl = in2, bestOff = of2;
                    }
                }
                if (bestJ < 0) break;
                int id = cols[bestJ];
                type = pc[id].type;
                for (int x : bestIncl)
                    if (find(incl.begin(), incl.end(), x) == incl.end()) {
                        int t = find(cols.begin(), cols.end(), x) - cols.begin();
                        for (size_t a2 = 0; a2 < A; a2++) sm[a2] += tab[a2 * c0 + t];
                    }
                chosen.push_back(id);
                incl = bestIncl;
                off = bestOff;
                bestH = bh;
            }
            if (chosen.empty()) throw runtime_error("no query");
#ifdef DBG
            dbgH += bestH / log(2.0); dbgHn++; dbgC += c0; dbgA += log2((double)A);
#endif
            // execute query
            long long obs;
            if (type == 0) {
                vector<int> q = S.W;
                for (int id : incl) q.push_back(B[pc[id].u]);
                obs = ask(q) - S.qW;
            } else {
                vector<int> us, ns;
                for (int id : chosen) {
                    if (find(us.begin(), us.end(), pc[id].u) == us.end()) us.push_back(pc[id].u);
                    if (find(ns.begin(), ns.end(), pc[id].node) == ns.end()) ns.push_back(pc[id].node);
                }
                vector<int> q;
                map<int, int> mask;
                for (int u : us) q.push_back(B[u]);
                for (int N : ns)
                    for (int it : hn[hn[N].l].items) {
                        q.insert(q.end(), items[it].mem.begin(), items[it].mem.end());
                        mask[items[it].comp] |= 1 << items[it].col;
                    }
                long long kE = 0;
                for (auto [cc, m] : mask)
                    if (m == 3) kE += S.ver[cc].E;
                obs = ask(q) - kE - off;
            }
            for (int id : incl) obs -= pc[id].lo;
            size_t w2 = 0;
            for (size_t a2 = 0; a2 < A; a2++)
                if (sm[a2] == obs) {
                    if (w2 != a2) {
                        for (int j = 0; j < c0; j++) tab[w2 * c0 + j] = tab[a2 * c0 + j];
                        wt[w2] = wt[a2];
                    }
                    w2++;
                }
            A = w2;
            tab.resize(A * c0);
            wt.resize(A);
            if (A == 0) throw runtime_error("inconsistent pool");
        }
    }

    int currentComp(int v) {
        for (int id : S.active)
            if (S.ver[id].bs[v]) return id;
        return -1;
    }

    // merge placed vertex v with found items; returns 1 if odd cycle found
    int mergeVertex(int v, long long d, const vector<pair<int, long long>>& fnd) {
        struct Acc {
            long long cnt[2] = {0, 0};
            vector<int> snap[2];
        };
        map<int, Acc> by;
        for (auto [i, c] : fnd) {
            const Item& it = items[i];
            int id = currentComp(it.mem[0]);
            int col = S.color[it.mem[0]];
            Acc& A = by[id];
            A.cnt[col] += c;
            A.snap[col].insert(A.snap[col].end(), it.mem.begin(), it.mem.end());
        }
        auto bt = [&](int a, int b) { return S.tim[a] < S.tim[b]; };
        for (auto& [id, A] : by) {
            sort(A.snap[0].begin(), A.snap[0].end(), bt);
            sort(A.snap[1].begin(), A.snap[1].end(), bt);
            if (A.cnt[0] > 0 && A.cnt[1] > 0) {
                int x = earliestNb(v, A.snap[0], A.cnt[0]);
                int y = earliestNb(v, A.snap[1], A.cnt[1]);
                vector<int> p = path(x, y, id);
                p.push_back(v);
                res = {false, p};
                return 1;
            }
        }
        Ver NV;
        NV.creator = v;
        NV.E = d;
        int vcol = -1;
        vector<int> ids;
        for (auto& [id, A] : by) ids.push_back(id);
        sort(ids.begin(), ids.end(), [&](int a, int b) { return S.ver[a].mem.size() > S.ver[b].mem.size(); });
        for (int id : ids) {
            Acc& A = by[id];
            int s = A.cnt[0] > 0 ? 0 : 1;
            Ver& V = S.ver[id];
            if (vcol < 0) vcol = 1 - s;
            else if (s == vcol)
                for (int u : V.mem) S.color[u] ^= 1;
            NV.E += V.E;
            NV.ch.push_back({id, A.snap[s], A.cnt[s], -1});
            NV.mem.insert(NV.mem.end(), V.mem.begin(), V.mem.end());
            NV.bs |= V.bs;
        }
        S.color[v] = vcol;
        NV.mem.push_back(v);
        NV.bs[v] = 1;
        sort(NV.mem.begin(), NV.mem.end(), bt);
        vector<int> na;
        for (int id : S.active)
            if (!by.count(id)) na.push_back(id);
        S.ver.push_back(move(NV));
        na.push_back(S.ver.size() - 1);
        S.active = na;
        return 0;
    }

    // process batch (already removed from pend). returns 0 ok, 1 result
    int processBatch(vector<int> B) {
#ifdef DBG
        fprintf(stderr, "[PB k=%zu W=%zu m=%zu q=%lld]\n", B.size(), S.W.size(), S.active.size(), judge::queries);
#endif
        if (S.W.empty()) {
            int v = B[0];
            place(v);
            newSingleton(v);
            for (int i = (int)B.size() - 1; i >= 1; i--) S.pend.push_front(B[i]);
            return 0;
        }
        int k = B.size();
        vector<int> q = S.W;
        q.insert(q.end(), B.begin(), B.end());
#ifdef DBG
        long long qb0 = judge::queries;
#endif
        long long r = ask(q);
        long long T = r - S.qW;
        if (T == 0) {
            for (int v : B) {
                S.obs.push_back(1);
                if (k == 1 && !S.deferred[v] && P.exactDefer && S.active.size() == 1 &&
                    (int)S.ver[S.active[0]].mem.size() >= P.deferMin) {
                    S.deferred[v] = 1;
                    S.pend.push_back(v);
                    continue;
                }
                place(v);
                newSingleton(v);
            }
            S.qWt = S.W.size();
            return 0;
        }
        if (k >= 2) {
            if (ask(B) > 0) {
                // longest independent prefix
                int lo = 1, hi = k - 1;
                while (lo < hi) {
                    int mid = (lo + hi) / 2;
                    vector<int> pb(B.begin(), B.begin() + mid + 1);
                    if (ask(pb) > 0) hi = mid;
                    else lo = mid + 1;
                }
                for (int i = k - 1; i >= lo; i--) S.pend.push_front(B[i]);
                B.resize(lo);
                k = lo;
                q = S.W;
                q.insert(q.end(), B.begin(), B.end());
                r = ask(q);
                T = r - S.qW;
                if (T == 0) {
                    for (int v : B) {
                        S.obs.push_back(1);
                        place(v);
                        newSingleton(v);
                    }
                    S.qWt = S.W.size();
                    return 0;
                }
            }
        }
#ifdef DBG
        long long q0d = judge::queries;
        dbgOver += q0d - qb0; dbgBatches++;
#endif
        vector<long long> d;
        vector<vector<pair<int, long long>>> found;
        if (P.pool) {
            buildItems();
            poolPhase(B, T, d, found);
        } else {
            d = k == 1 ? vector<long long>{T} : dPhase(B, T);
            buildItems();
            found = itemPhase(B, d);
        }
#ifdef DBG
        dbgDQ += judge::queries - q0d; dbgDCoins += k;
#endif
        for (int i = 0; i < k; i++) S.obs.push_back(d[i] == 0);
        for (int i = 0; i < k; i++) {
            int v = B[i];
            place(v);
            if (d[i] == 0) {
                newSingleton(v);
                continue;
            }
            if (mergeVertex(v, d[i], found[i])) return 1;
        }
        S.qW = r;
        S.qWt = S.W.size();
        return 0;
    }

    chrono::steady_clock::time_point t0 = chrono::steady_clock::now();
    void timeGuard() {
        double el = chrono::duration<double>(chrono::steady_clock::now() - t0).count();
        if (el > 3.0) P.coinLimit = min(P.coinLimit, 2048.0), P.sample = min(P.sample, 256);
        if (el > 6.0) P.coinLimit = min(P.coinLimit, 256.0), P.sample = min(P.sample, 64), P.batchMax = min(P.batchMax, 8);
    }
    int batchSize() {
        timeGuard();
        if (S.forceSingle > 0) {
            S.forceSingle--;
            return 1;
        }
        if (S.active.size() < 2) return 1;
        double D = degEst();
        int k = (int)floor(sqrt(P.batchC * n / D));
        return max(1, min(P.batchMax, k));
    }

    // ------------------------------------------------ lazy mode
    void enterLazy() {
        int K = S.active[0];
        long long s[2] = {0, 0};
        for (int u : S.ver[K].mem) s[S.color[u]]++;
        S.ec = s[0] <= s[1] ? 0 : 1;
        S.lazy = true;
        S.dirtyBase.clear();
        for (int u : S.ver[K].mem)
            if (S.color[u] != S.ec) S.dirtyBase.push_back(u);
        S.dirtyList.clear();
        S.dirtyPrev.clear();
        S.dirtyA.clear();
        S.checkAt = max<int>(S.W.size() + 2, (int)ceil(S.W.size() * P.ratio));
    }
    void sides(int K, vector<int>& Ex, vector<int>& Dy) {
        Ex.clear(), Dy.clear();
        for (int u : S.ver[K].mem) (S.color[u] == S.ec ? Ex : Dy).push_back(u);
    }
    void place(int v) {
        S.tim[v] = S.clock++;
        S.W.push_back(v);
    }
    void addLazyVer(int v, int K, int col, vector<int> snap, long long cnt) {
        place(v);
        Ver NV;
        NV.creator = v;
        NV.mem = S.ver[K].mem;
        NV.mem.push_back(v);
        NV.bs = S.ver[K].bs;
        NV.bs[v] = 1;
        NV.ch.push_back({K, move(snap), cnt, -1});
        S.color[v] = col;
        S.ver.push_back(move(NV));
        S.active = {(int)S.ver.size() - 1};
    }
    void markClean(const vector<int>& Dy) {
        S.dirtyBase = Dy;
        S.dirtyList.clear();
        S.dirtyPrev.clear();
        S.dirtyA.clear();
    }
    // dirty side found non-independent: locate and produce odd cycle. returns 1 result, 2 restart
    int diagnose() {
        int lo = 0, hi = S.dirtyList.size() - 1;
        while (lo < hi) {
            int mid = (lo + hi) / 2;
            vector<int> q = S.dirtyBase;
            q.insert(q.end(), S.dirtyList.begin(), S.dirtyList.begin() + mid + 1);
            if (ask(q) > 0) hi = mid;
            else lo = mid + 1;
        }
        int v = S.dirtyList[lo];
        int prev = S.dirtyPrev[lo];
        vector<int> Ex, Dy;
        sides(prev, Ex, Dy);
        try {
            int x = earliestNb(v, Ex, S.dirtyA[lo]);
            int y = earliestNb(v, Dy, -1);
            vector<int> p = path(x, y, prev);
            p.push_back(v);
            res = {false, p};
            return 1;
        } catch (WrongGuess& e) {
            badGuess[e.w] = 1;
            return 2;
        }
    }
    int checkDirty() {
        if (S.dirtyList.empty()) return 0;
        vector<int> Ex, Dy;
        sides(S.active[0], Ex, Dy);
        if (ask(Dy) == 0) {
            markClean(Dy);
            return 0;
        }
        return diagnose();
    }
    // 0 placed, 1 result, 2 restart, 3 deferred
    int processLazy(int v) {
#ifdef DBG
        fprintf(stderr, "[PL v=%d W=%zu q=%lld pend=%zu]\n", v, S.W.size(), judge::queries, S.pend.size());
#endif
        int K = S.active[0];
        vector<int> Ex, Dy;
        sides(K, Ex, Dy);
        vector<int> q = Ex;
        q.push_back(v);
        long long a = ask(q);
        if (a > 0) {
            S.dirtyList.push_back(v);
            S.dirtyPrev.push_back(K);
            S.dirtyA.push_back(a);
            addLazyVer(v, K, 1 - S.ec, Ex, a);
            return 0;
        }
        S.aZero++;
        double p = pIso();
        bool guess = !noGuess && !badGuess[v] && (S.aZero % P.spot != 0) &&
                     p * 0.5 * (1.5 * (n - (int)S.W.size()) + 20) < P.guessRisk;
        if (guess) {
            if (!haveCk) {
                haveCk = true;
                ck = S;
            }
            addLazyVer(v, K, S.ec, Dy, -1);
            return 0;
        }
        q = Dy;
        q.push_back(v);
        long long b = ask(q);
        S.obs.push_back(b == 0);
        if (b > 0) {
            addLazyVer(v, K, S.ec, Dy, b);
            return 0;
        }
        // isolated; dirty side is verified clean by this answer
        markClean(Dy);
        if (!S.deferred[v]) {
            S.deferred[v] = 1;
            S.pend.push_back(v);
            return 3;
        }
        long long r = ask(S.ver[K].mem);
        S.ver[K].E = r;
        S.qW = r;
        S.lazy = false;
        place(v);
        S.qWt = S.W.size();
        newSingleton(v);
        return 0;
    }

    void tryEvict() {
        int K = -1;
        for (int id : S.active)
            if (K < 0 || S.ver[id].mem.size() > S.ver[K].mem.size()) K = id;
        if ((int)S.ver[K].mem.size() < P.deferMin) return;
        int cnt = 0;
        bool dense = degEst() >= P.evD;
        int esz = dense ? P.evSzBig : P.evictSize;
        double efr = dense ? P.evFrBig : P.evictFrac;
        for (int id : S.active) {
            if (id == K) continue;
            if ((int)S.ver[id].mem.size() > esz) return;
            for (int u : S.ver[id].mem)
                if (S.deferred[u]) return;
            cnt += S.ver[id].mem.size();
        }
        if (cnt > efr * S.ver[K].mem.size()) return;
        for (int id : S.active)
            if (id != K)
                for (int u : S.ver[id].mem) {
                    S.deferred[u] = 1;
                    S.pend.push_back(u);
                }
        S.active = {K};
        S.W = S.ver[K].mem;
        S.qW = S.ver[K].E;
    }

    Result run(int n_, const vector<int>& ord) {
        n = n_;
        S.color.assign(n + 1, 0);
        S.tim.assign(n + 1, -1);
        S.deferred.assign(n + 1, 0);
        S.pend.assign(ord.begin(), ord.end());
        while (true) {
            int st = 0;
            try {
            while (!S.pend.empty()) {
                if (!S.lazy) {
                    int k = min<int>(batchSize(), S.pend.size());
                    vector<int> B(S.pend.begin(), S.pend.begin() + k);
                    S.pend.erase(S.pend.begin(), S.pend.begin() + k);
#ifdef DBG
                    long long q0 = judge::queries;
#endif
                    st = processBatch(B);
#ifdef DBG
                    fprintf(stderr, "B%d:%lld(m%zu) ", k, judge::queries - q0, S.active.size());
#endif
                    if (st == 1) break;
                    if (!S.lazy && S.active.size() > 1 && P.evict) tryEvict();
                    if (!S.lazy && S.active.size() == 1 && S.ver[S.active[0]].mem.size() >= 2 && !S.pend.empty() &&
                        pIso() < P.lazyEnter)
                        enterLazy();
                    continue;
                }
                int v = S.pend.front();
#ifdef DBG
                bool wl = S.lazy; (void)wl;
#endif
#ifdef DBG
                long long q0 = judge::queries;
#endif
                st = processLazy(v);
#ifdef DBG
                fprintf(stderr, "L%lld ", judge::queries - q0);
#endif
#ifdef DBG

#endif
                if (st == 1 || st == 2) break;
                S.pend.pop_front();
                if (st == 3) {
                    st = 0;
                    continue;
                }
                if (S.lazy && (int)S.W.size() >= S.checkAt) {
                    S.checkAt = max<int>(S.W.size() + 1, (int)ceil(S.W.size() * P.ratio));
                    st = checkDirty();
                    if (st) break;
                }
                if (!S.lazy && S.active.size() > 1 && P.evict) tryEvict();
                if (!S.lazy && S.active.size() == 1 && S.ver[S.active[0]].mem.size() >= 2 && !S.pend.empty() &&
                    pIso() < P.lazyEnter)
                    enterLazy();
            }
            if (st == 0) st = S.lazy ? checkDirty() : 0;
            } catch (WrongGuess& e) {
                badGuess[e.w] = 1;
                if (!haveCk) throw;
                st = 2;
            }
            if (st == 1) return res;
            if (st == 0) break;
            // restart
#ifdef DBG
            fprintf(stderr, "\nRESTART at q=%lld\n", judge::queries);
#endif
            S = ck;
            if (++restarts >= P.maxRestarts) noGuess = true;
            haveCk = false;
        }
        vector<int> A;
        for (int v = 1; v <= n; v++)
            if (S.color[v] == 0) A.push_back(v);
        if (A.empty())
            for (int v = 1; v <= n; v++) A.push_back(v);
        return {true, A};
    }
};

#ifdef LOCAL
#include "harness.h"
#else
int main() {
    int n;
    if (scanf("%d", &n) != 1) return 0;
    vector<int> order(n);
    iota(order.begin(), order.end(), 1);
    Result r;
    try {
        Solver S;
        r = S.run(n, order);
    } catch (...) {
        // safety net: conservative settings (cached queries are reused)
        Solver S;
        S.P.batchMax = 1;
        S.P.lazyEnter = -1;
        S.P.pool = false;
        S.noGuess = true;
        r = S.run(n, order);
    }
    string out;
    if (r.bip) out = "Y " + to_string(r.seq.size()) + "\n";
    else out = "N " + to_string(r.seq.size()) + "\n";
    for (size_t i = 0; i < r.seq.size(); i++) {
        if (i) out += ' ';
        out += to_string(r.seq[i]);
    }
    out += '\n';
    fputs(out.c_str(), stdout);
    fflush(stdout);
    return 0;
}
#endif
