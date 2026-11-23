#include <bits/stdc++.h>
using namespace std;

// directions: 0=N,1=E,2=S,3=W
static const int DR[4] = {-1, 0, 1, 0};
static const int DC[4] = {0, 1, 0, -1};
static const int DF[4] = {1, 0, -1, 0};
static const int DS[4] = {0, 1, 0, -1};

#ifndef PARAM_A
#define PARAM_A 1.0
#endif
#ifndef PARAM_B
#define PARAM_B 1.0
#endif
#ifndef PARAM_NAVW
#define PARAM_NAVW 1.0
#endif
#ifndef PARAM_SLACK
#define PARAM_SLACK 3
#endif
#ifndef PARAM_MAXF
#define PARAM_MAXF 400
#endif
#ifndef PARAM_ROLLOUT
#define PARAM_ROLLOUT 1
#endif
#ifndef PARAM_TIME
#define PARAM_TIME 4.0
#endif
#ifndef PARAM_K
#define PARAM_K 8
#endif
#ifndef PARAM_K2
#define PARAM_K2 4
#endif
#ifndef PARAM_LEVEL
#define PARAM_LEVEL 1
#endif
#ifndef PARAM_RMAXF
#define PARAM_RMAXF 400
#endif
#ifndef PARAM_STEPT
#define PARAM_STEPT 1.0
#endif
#ifndef PARAM_WORK
#define PARAM_WORK 200000000LL
#endif

static double nowSec() {
    using namespace chrono;
    return duration<double>(steady_clock::now().time_since_epoch()).count();
}

struct Solver {
    int R, C;
    vector<string> g;
    vector<int> obs;  // state -> observation (states = cell*4+d), -1 if wall cell
    vector<int> comp;
    vector<vector<int>> compCells;
    vector<int> cls;  // state -> equivalence class id
    vector<int> S;
    deque<int> plan;
    double A = PARAM_A, B = PARAM_B, NAVW = PARAM_NAVW;
    double timeBudget = PARAM_TIME, t0 = 0;
    int K = PARAM_K;
    bool useRollout = PARAM_ROLLOUT;

    static const int OFF = 100, W = 201;
    vector<int> vis, par, pact, distv;
    int stamp = 0;

    bool open(int r, int c) const { return r >= 0 && r < R && c >= 0 && c < C && g[r][c] == '.'; }

    static pair<int, int> rot(int a, int b, int r) {
        for (int i = 0; i < r; i++) { int t = a; a = b; b = -t; }
        return {a, b};
    }

    void init(int r, int c, const vector<string>& grid) {
        t0 = nowSec();
        R = r; C = c; g = grid;
        int N = R * C;
        obs.assign(N * 4, -1);
        for (int i = 0; i < R; i++)
            for (int j = 0; j < C; j++) {
                if (g[i][j] != '.') continue;
                for (int d = 0; d < 4; d++) {
                    int k = 0, ii = i + DR[d], jj = j + DC[d];
                    while (open(ii, jj)) { k++; ii += DR[d]; jj += DC[d]; }
                    obs[(i * C + j) * 4 + d] = k;
                }
            }
        comp.assign(N, -1);
        for (int i = 0; i < N; i++) {
            if (g[i / C][i % C] != '.' || comp[i] != -1) continue;
            int id = compCells.size();
            compCells.push_back({});
            vector<int> st = {i};
            comp[i] = id;
            while (!st.empty()) {
                int x = st.back(); st.pop_back();
                compCells[id].push_back(x);
                int xr = x / C, xc = x % C;
                for (int d = 0; d < 4; d++) {
                    int yr = xr + DR[d], yc = xc + DC[d];
                    if (open(yr, yc) && comp[yr * C + yc] == -1) { comp[yr * C + yc] = id; st.push_back(yr * C + yc); }
                }
            }
        }
        cls.assign(N * 4, -1);
        map<vector<int>, int> shapeId;
        map<tuple<int, int, int>, int> classId;
        for (int id = 0; id < (int)compCells.size(); id++) {
            auto& cells = compCells[id];
            for (int d = 0; d < 4; d++) {
                int rr = (4 - d) % 4;
                vector<pair<int, int>> pts;
                pts.reserve(cells.size());
                int mr = INT_MAX, mc = INT_MAX;
                for (int x : cells) {
                    auto q = rot(x / C, x % C, rr);
                    pts.push_back(q);
                    mr = min(mr, q.first); mc = min(mc, q.second);
                }
                vector<int> key;
                key.reserve(cells.size() * 2);
                vector<pair<int, int>> sp = pts;
                for (auto& q : sp) { q.first -= mr; q.second -= mc; }
                sort(sp.begin(), sp.end());
                for (auto& q : sp) { key.push_back(q.first); key.push_back(q.second); }
                auto it = shapeId.find(key);
                int sid;
                if (it == shapeId.end()) { sid = shapeId.size(); shapeId[key] = sid; }
                else sid = it->second;
                for (size_t k = 0; k < cells.size(); k++) {
                    auto t = make_tuple(sid, pts[k].first - mr, pts[k].second - mc);
                    auto it2 = classId.find(t);
                    int cid;
                    if (it2 == classId.end()) { cid = classId.size(); classId[t] = cid; }
                    else cid = it2->second;
                    cls[cells[k] * 4 + d] = cid;
                }
            }
        }
        vis.assign(W * W * 4, 0);
        par.assign(W * W * 4, -1);
        pact.assign(W * W * 4, -1);
        distv.assign(W * W * 4, 0);
        clsMark.assign(classId.size() + 1, 0);
        clsGroup.assign(classId.size() + 1, -1);
    }

    inline int applyAct(int s, int a) const {
        int cell = s >> 2, d = s & 3;
        if (a == 0) return cell * 4 + ((d + 3) & 3);
        if (a == 1) return cell * 4 + ((d + 1) & 3);
        int r = cell / C + DR[d], c = cell % C + DC[d];
        return (r * C + c) * 4 + d;
    }

    inline int relState(int st, int f, int sd, int dr) const {
        int cell = st >> 2, d = st & 3;
        int r = cell / C + f * DR[d] + sd * DR[(d + 1) & 3];
        int c = cell % C + f * DC[d] + sd * DC[(d + 1) & 3];
        return (r * C + c) * 4 + ((d + dr) & 3);
    }

    static bool allSamePos(const vector<int>& v) {
        for (int x : v) if ((x >> 2) != (v[0] >> 2)) return false;
        return true;
    }

    // Kv: mutually equivalent states (same cls). returns fixed cell, -1 if none, -2 if all same pos
    int fixedPoint(const vector<int>& Kv) const {
        if (allSamePos(Kv)) return -2;
        int s = Kv[0], ps = s >> 2, ds = s & 3;
        int psr = ps / C, psc = ps % C;
        int idx = -1;
        for (size_t i = 1; i < Kv.size(); i++) {
            int r = ((Kv[i] & 3) - ds) & 3;
            if (r == 0) return -1;
            if (idx < 0) idx = i;
        }
        // solve T(q) = q with T(q) = rot(q - ps, r) + pt
        int t = Kv[idx], pt = t >> 2;
        int r = ((t & 3) - ds) & 3;
        auto rp = rot(psr, psc, r);
        int ua = pt / C - rp.first, ub = pt % C - rp.second;  // (I - Rot) q = u
        int qa2, qb2;  // 2*q
        if (r == 2) { qa2 = ua; qb2 = ub; }
        else if (r == 1) { qa2 = ua + ub; qb2 = ub - ua; }
        else { qa2 = ua - ub; qb2 = ua + ub; }
        if ((qa2 & 1) || (qb2 & 1)) return -1;
        int qr = qa2 / 2, qc = qb2 / 2;
        if (!open(qr, qc)) return -1;
        if (comp[qr * C + qc] != comp[ps]) return -1;
        for (size_t i = 1; i < Kv.size(); i++) {
            int tt = Kv[i], ptt = tt >> 2;
            int rr = ((tt & 3) - ds) & 3;
            auto v = rot(qr - psr, qc - psc, rr);
            if (v.first + ptt / C != qr || v.second + ptt % C != qc) return -1;
        }
        return qr * C + qc;
    }

    vector<int> clsMark, clsGroup;
    int clsStamp = 0;
    void groupBelief(const vector<int>& Bv, vector<int>& grp, vector<vector<int>>& members) {
        clsStamp++;
        members.clear();
        grp.resize(Bv.size());
        for (size_t i = 0; i < Bv.size(); i++) {
            int c = cls[Bv[i]];
            if (clsMark[c] != clsStamp) { clsMark[c] = clsStamp; clsGroup[c] = members.size(); members.push_back({}); }
            grp[i] = clsGroup[c];
            members[grp[i]].push_back(Bv[i]);
        }
    }

    // 0 continue, 1 yes, 2 no
    int statusOf(const vector<int>& Bv) {
        if (allSamePos(Bv)) return 1;
        vector<int> grp;
        vector<vector<int>> members;
        groupBelief(Bv, grp, members);
        for (auto& m : members) if (m.size() == 1 || fixedPoint(m) != -1) return 0;
        return 2;
    }

    inline int nodeIdx(int f, int s, int dr) const { return (((f + OFF) * W) + (s + OFF)) * 4 + dr; }

    // distances to targetCell, stored compactly per component: index compIdx[cell]*4+d
    unordered_map<int, vector<int>> revCache;
    vector<int> compIdx;
    inline int dAt(const vector<int>& dd, int st) const { return dd[compIdx[st >> 2] * 4 + (st & 3)]; }
    const vector<int>& distTo(int targetCell) {
        auto it = revCache.find(targetCell);
        if (it != revCache.end()) return it->second;
        if (compIdx.empty()) {
            compIdx.assign(R * C, -1);
            for (auto& cc : compCells)
                for (size_t k = 0; k < cc.size(); k++) compIdx[cc[k]] = k;
        }
        int M = compCells[comp[targetCell]].size() * 4;
        vector<int> dd(M, 1 << 20);
        auto at = [&](int st) -> int& { return dd[compIdx[st >> 2] * 4 + (st & 3)]; };
        vector<int> q;
        for (int d = 0; d < 4; d++) { at(targetCell * 4 + d) = 0; q.push_back(targetCell * 4 + d); }
        for (size_t h = 0; h < q.size(); h++) {
            int u = q[h];
            int du = at(u);
            int cell = u >> 2, d = u & 3;
            int p1 = cell * 4 + ((d + 1) & 3), p2 = cell * 4 + ((d + 3) & 3);
            if (at(p1) > du + 1) { at(p1) = du + 1; q.push_back(p1); }
            if (at(p2) > du + 1) { at(p2) = du + 1; q.push_back(p2); }
            int pr = cell / C - DR[d], pc = cell % C - DC[d];
            if (open(pr, pc)) {
                int p = (pr * C + pc) * 4 + d;
                if (at(p) > du + 1) { at(p) = du + 1; q.push_back(p); }
            }
        }
        return revCache[targetCell] = move(dd);
    }

    vector<int> navPath(int s, int targetCell) {
        const vector<int>& dd = distTo(targetCell);
        vector<int> acts;
        int u = s;
        while ((u >> 2) != targetCell) {
            for (int a = 0; a < 3; a++) {
                if (a == 2 && obs[u] <= 0) continue;
                int v = applyAct(u, a);
                if (dAt(dd, v) == dAt(dd, u) - 1) { acts.push_back(a); u = v; break; }
            }
        }
        return acts;
    }

    struct Cand { double score; vector<int> acts; int dist; };

    // candidate probes for belief Bv, sorted by heuristic score
    vector<Cand> candidates(const vector<int>& Bv, int keep, int maxf) {
        stamp++;
        vector<int> q;
        q.reserve(1024);
        int start = nodeIdx(0, 0, 0);
        vis[start] = stamp; par[start] = -1; distv[start] = 0;
        q.push_back(start);
        vector<int> frontier;
        int dmin = INT_MAX;
        const int SLACK = PARAM_SLACK;
        int mergeNode = -1;
        size_t n = Bv.size();
        for (size_t h = 0; h < q.size(); h++) {
            int u = q[h];
            if (dmin != INT_MAX && distv[u] > dmin + SLACK) break;
            if (mergeNode != -1 && distv[u] >= distv[mergeNode]) break;
            int dr = u & 3, rest = u >> 2;
            int f = rest / W - OFF, sd = rest % W - OFF;
            int s0 = relState(Bv[0], f, sd, dr);
            int o0 = obs[s0];
            bool agree = true, sameCell = true;
            for (size_t i = 1; i < n; i++) {
                int si = relState(Bv[i], f, sd, dr);
                if (obs[si] != o0) { agree = false; break; }
                if ((si >> 2) != (s0 >> 2)) sameCell = false;
            }
            if (!agree) {
                if (u != start) {
                    frontier.push_back(u);
                    dmin = min(dmin, distv[u]);
                    if ((int)frontier.size() >= maxf) break;
                }
                continue;
            }
            if (sameCell && u != start) {
                if (mergeNode == -1) mergeNode = u;
                continue;
            }
            for (int a = 0; a < 3; a++) {
                int nf = f, ns = sd, ndr = dr;
                if (a == 0) ndr = (dr + 3) & 3;
                else if (a == 1) ndr = (dr + 1) & 3;
                else {
                    if (o0 <= 0) continue;
                    nf = f + DF[dr]; ns = sd + DS[dr];
                    if (nf < -OFF || nf > OFF || ns < -OFF || ns > OFF) continue;
                }
                int v = nodeIdx(nf, ns, ndr);
                if (vis[v] == stamp) continue;
                vis[v] = stamp; par[v] = u; pact[v] = a; distv[v] = distv[u] + 1;
                q.push_back(v);
            }
        }
        vector<Cand> res;
        auto mk = [&](int u, double sc) {
            Cand c;
            c.score = sc;
            for (int x = u; par[x] != -1; x = par[x]) c.acts.push_back(pact[x]);
            reverse(c.acts.begin(), c.acts.end());
            c.dist = distv[u];
            return c;
        };
        if (mergeNode != -1) res.push_back(mk(mergeNode, distv[mergeNode]));
        if (frontier.empty()) {
            if (res.empty()) {
                int fp = fixedPoint(Bv);
                Cand c;
                c.acts = navPath(Bv[0], fp);
                c.score = c.acts.size();
                c.dist = c.acts.size();
                res.push_back(c);
            }
            return res;
        }
        vector<int> grp;
        vector<vector<int>> members;
        groupBelief(Bv, grp, members);
        int G = members.size();
        vector<int> gfp(G);
        vector<const vector<int>*> gdist(G, nullptr);
        for (int gi = 0; gi < G; gi++) {
            if (members[gi].size() == 1) gfp[gi] = -2;
            else {
                int fp = fixedPoint(members[gi]);
                gfp[gi] = fp;
                if (fp >= 0) gdist[gi] = &distTo(fp);
            }
        }
        if ((int)mark.size() < G) mark.assign(G, 0), mtag = 0;
        struct P { int cnt, cell, groups, good; bool same; double nav; };
        P parts[128];
        int touched[128];
        vector<pair<double, int>> scored;
        vector<int> rs(n);
        double cutoff = mergeNode != -1 ? distv[mergeNode] : 1e18;
        for (int u : frontier) {
            if (distv[u] >= cutoff) continue;
            int dr = u & 3, rest = u >> 2;
            int f = rest / W - OFF, sd = rest % W - OFF;
            int nt = 0;
            for (size_t i = 0; i < n; i++) {
                int st = relState(Bv[i], f, sd, dr);
                rs[i] = st;
                int o = obs[st];
                P& p = parts[o];
                bool fresh = true;
                for (int k = 0; k < nt; k++) if (touched[k] == o) { fresh = false; break; }
                if (fresh) { touched[nt++] = o; p.cnt = 0; p.cell = st >> 2; p.groups = 0; p.good = 0; p.same = true; p.nav = 0; }
                p.cnt++;
                if (p.cell != (st >> 2)) p.same = false;
            }
            if (mtag > INT_MAX - 1000) { fill(mark.begin(), mark.end(), 0); mtag = 0; }
            int baseTag = mtag;
            mtag += 128;
            for (size_t i = 0; i < n; i++) {
                int st = rs[i];
                int o = obs[st];
                P& p = parts[o];
                int gi = grp[i];
                int t = baseTag + o + 1;
                if (mark[gi] != t) {
                    mark[gi] = t;
                    p.groups++;
                    if (gfp[gi] != -1) p.good++;
                }
                if (gfp[gi] >= 0) p.nav += dAt(*gdist[gi], st);
            }
            double tot = 0;
            for (int k = 0; k < nt; k++) {
                P& p = parts[touched[k]];
                if (p.same || p.good == 0) continue;
                if (p.groups == 1) { tot += p.nav; continue; }
                tot += p.cnt * (B + A * log2((double)p.groups)) + NAVW * p.nav;
            }
            double sc = distv[u] + tot / n;
            scored.push_back({sc, u});
        }
        int kk = min<int>(keep, scored.size());
        partial_sort(scored.begin(), scored.begin() + kk, scored.end());
        for (int i = 0; i < kk; i++) res.push_back(mk(scored[i].second, scored[i].first));
        sort(res.begin(), res.end(), [](const Cand& a, const Cand& b) { return a.score < b.score; });
        return res;
    }
    vector<int> mark;
    int mtag = 0;

    void applyActs(vector<int>& Bv, const vector<int>& acts) const {
        for (int a : acts)
            for (auto& x : Bv) x = applyAct(x, a);
    }

    // exact total cost (sum over states of rounds incl. final report) of base policy from belief Bv.
    // returns -1 if the work budget is exhausted.
    long long work = 0, workLimit = 0, calls = 0;
    double deadline = 0;
    long long treeCost(vector<int>& Bv, int level) {
        int st = statusOf(Bv);
        if (st != 0) return Bv.size();
        work += Bv.size() + 50;
        if (work > workLimit) return -1;
        if ((++calls & 63) == 0 && nowSec() > deadline) { work = workLimit + 1; return -1; }
        auto cs = candidates(Bv, level > 0 ? PARAM_K2 : 1, PARAM_RMAXF);
        int ch = 0;
        if (level > 0 && cs.size() > 1) {
            long long best = LLONG_MAX;
            for (size_t ci = 0; ci < cs.size(); ci++) {
                vector<int> B2 = Bv;
                applyActs(B2, cs[ci].acts);
                long long r = addParts(B2, (long long)Bv.size() * cs[ci].acts.size(), level - 1);
                if (r < 0) return -1;
                if (r < best) { best = r; ch = ci; }
            }
        }
        auto& acts = cs[ch].acts;
        long long cost = (long long)Bv.size() * acts.size();
        applyActs(Bv, acts);
        return addParts(Bv, cost, level);
    }
    long long addParts(vector<int>& Bv, long long cost, int level) {
        // split by observation
        int cntBuf[128];
        memset(cntBuf, 0, sizeof cntBuf);
        vector<int> keys;
        for (int x : Bv) { int o = obs[x]; if (cntBuf[o]++ == 0) keys.push_back(o); }
        if (keys.size() == 1) {
            cntBuf[keys[0]] = 0;
            long long r = treeCost(Bv, level);
            return r < 0 ? -1 : cost + r;
        }
        vector<vector<int>> parts(keys.size());
        int idx[128];
        for (size_t k = 0; k < keys.size(); k++) { idx[keys[k]] = k; parts[k].reserve(cntBuf[keys[k]]); cntBuf[keys[k]] = 0; }
        for (int x : Bv) parts[idx[obs[x]]].push_back(x);
        vector<int>().swap(Bv);
        for (auto& p : parts) {
            long long r = treeCost(p, level);
            if (r < 0) return -1;
            cost += r;
        }
        return cost;
    }

    void makePlan() {
        plan.clear();
        double now = nowSec();
        double elapsed = now - t0;
        bool roll = useRollout && elapsed < timeBudget;
        int keep = roll ? K : 1;
        auto cs = candidates(S, keep, PARAM_MAXF);
        int choice = 0;
        if (roll && cs.size() > 1) {
            deadline = now + max(0.0, min(PARAM_STEPT, (timeBudget - elapsed) * 0.3));
            long long best = LLONG_MAX;
            int bestI = -1;
            for (size_t ci = 0; ci < cs.size(); ci++) {
                vector<int> Bv = S;
                applyActs(Bv, cs[ci].acts);
                work = 0; workLimit = PARAM_WORK;
                long long r = addParts(Bv, (long long)S.size() * cs[ci].acts.size(), PARAM_LEVEL);
                if (r < 0) { if (ci == 0) break; continue; }
                if (r < best) { best = r; bestI = ci; }
                if (nowSec() > deadline) break;
            }
            if (bestI >= 0) choice = bestI;
        }
        for (int a : cs[choice].acts) plan.push_back(a);
    }

    void start(int d0) {
        for (int st = 0; st < R * C * 4; st++)
            if (obs[st] == d0) S.push_back(st);
    }

    // returns action: 0 left, 1 right, 2 step, 3 yes, 4 no
    int decide(int& yi, int& yj) {
        int st = statusOf(S);
        if (st == 1) { yi = (S[0] >> 2) / C + 1; yj = (S[0] >> 2) % C + 1; return 3; }
        if (st == 2) return 4;
        if (plan.empty()) makePlan();
        int a = plan.front();
        plan.pop_front();
        for (auto& x : S) x = applyAct(x, a);
        return a;
    }

    void observe(int d) {
        size_t before = S.size();
        vector<int> ns;
        ns.reserve(S.size());
        for (int x : S) if (obs[x] == d) ns.push_back(x);
        S.swap(ns);
        if (S.size() != before) plan.clear();
    }
};

#ifndef LOCAL
int main() {
    int r, c;
    if (!(cin >> r >> c)) return 0;
    vector<string> grid(r);
    for (auto& s : grid) cin >> s;
    Solver sol;
    sol.init(r, c, grid);
    int d;
    if (!(cin >> d)) return 0;
    sol.start(d);
    while (true) {
        int yi, yj;
        int a = sol.decide(yi, yj);
        if (a == 3) { printf("yes %d %d\n", yi, yj); fflush(stdout); return 0; }
        if (a == 4) { printf("no\n"); fflush(stdout); return 0; }
        static const char* names[3] = {"left", "right", "step"};
        printf("%s\n", names[a]);
        fflush(stdout);
        if (!(cin >> d)) return 0;
        sol.observe(d);
    }
}
#endif
