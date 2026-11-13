#include <bits/stdc++.h>
using namespace std;

// ---------------------------------------------------------------------------
// Per arena i the (at most) three meaningful actions are:
//   home  : b_i = 0        cost 0        -> default winner d_i (or nobody)
//   block : b_i = u_i      cost u_i      -> invalidate d_i, winner becomes e_i
//                                           (or nobody when e_i does not exist)
//   win   : b_i = c_i      cost c_i      -> we win
// where
//   u_i = largest positive value that occurs exactly once among a_{i,*}
//   d_i = its owner           (none when u_i does not exist)
//   e_i = owner of the second largest such value (none when absent)
//   c_i = smallest integer > u_i that does not occur among a_{i,*}
//
// For a fixed K the problem "everybody wins at most K, we win at least K" is a
// pure min-cost flow (integral).  Its value Q(K) is convex in K, therefore the
// minimum over K can be found by binary search on the discrete slope.
// ---------------------------------------------------------------------------

struct MCMF {
    int N, ecnt = 0;
    vector<int> head, to, nxt, cap;
    vector<long long> cost, pot, dist;
    vector<int> pv, pe;

    explicit MCMF(int n) : N(n), head(n, -1), pot(n, 0), dist(n), pv(n), pe(n) {}

    int add(int u, int v, int c, long long w) {
        int id = ecnt;
        to.push_back(v); cap.push_back(c); cost.push_back(w); nxt.push_back(head[u]); head[u] = ecnt++;
        to.push_back(u); cap.push_back(0); cost.push_back(-w); nxt.push_back(head[v]); head[v] = ecnt++;
        return id;
    }

    // min cost flow of value f ; returns {achieved flow, cost}
    pair<int, long long> run(int s, int t, int f) {
        const long long INF = LLONG_MAX / 4;
        int flow = 0;
        long long tot = 0;
        vector<int> hp, pos(N, -1);
        hp.reserve(4 * N);
        while (flow < f) {
            for (int v = 0; v < N; v++) dist[v] = INF;
            dist[s] = 0;
            hp.clear();
            fill(pos.begin(), pos.end(), -1);
            auto pushNode = [&](int v) {
                if (pos[v] < 0) { pos[v] = (int)hp.size(); hp.push_back(v); }
                int i = pos[v];
                while (i > 0) {
                    int p = (i - 1) >> 1;
                    if (dist[hp[p]] <= dist[hp[i]]) break;
                    swap(hp[p], hp[i]); pos[hp[p]] = p; pos[hp[i]] = i; i = p;
                }
            };
            auto popNode = [&]() {
                int r = hp[0];
                int last = hp.back(); hp.pop_back(); pos[r] = -2;
                if (!hp.empty()) {
                    hp[0] = last; pos[last] = 0;
                    int i = 0, sz = (int)hp.size();
                    while (true) {
                        int l = 2 * i + 1, rr = 2 * i + 2, b = i;
                        if (l < sz && dist[hp[l]] < dist[hp[b]]) b = l;
                        if (rr < sz && dist[hp[rr]] < dist[hp[b]]) b = rr;
                        if (b == i) break;
                        swap(hp[b], hp[i]); pos[hp[b]] = b; pos[hp[i]] = i; i = b;
                    }
                }
                return r;
            };
            pushNode(s);
            while (!hp.empty()) {
                int u = popNode();
                if (dist[u] >= INF) break;
                for (int e = head[u]; e != -1; e = nxt[e]) {
                    if (cap[e] <= 0) continue;
                    int v = to[e];
                    long long nd = dist[u] + cost[e] + pot[u] - pot[v];
                    if (nd < dist[v]) {
                        dist[v] = nd;
                        pv[v] = u; pe[v] = e;
                        pushNode(v);
                    }
                }
            }
            if (dist[t] >= INF) break;
            for (int v = 0; v < N; v++) if (dist[v] < INF) pot[v] += dist[v];
            int addf = f - flow;
            for (int v = t; v != s; v = pv[v]) addf = min(addf, cap[pe[v]]);
            for (int v = t; v != s; v = pv[v]) { cap[pe[v]] -= addf; cap[pe[v] ^ 1] += addf; }
            flow += addf;
            tot += (long long)addf * pot[t];
        }
        return {flow, tot};
    }
};

int n, m;
vector<vector<int>> a;
vector<int> dd, ee;      // default winner / runner-up  (-1 = none)
vector<long long> uu, cc;

void buildOptions() {
    dd.assign(n, -1); ee.assign(n, -1); uu.assign(n, 0); cc.assign(n, 0);
    vector<pair<int, int>> v;
    vector<int> vals;
    for (int i = 0; i < n; i++) {
        v.clear();
        v.reserve(m);
        for (int j = 0; j < m; j++) v.push_back({a[i][j], j});
        sort(v.begin(), v.end(), greater<pair<int, int>>());
        int p = 0;
        while (p < m) {
            int q = p;
            while (q < m && v[q].first == v[p].first) q++;
            if (q - p == 1 && v[p].first > 0) {
                if (dd[i] < 0) { dd[i] = v[p].second; uu[i] = v[p].first; }
                else if (ee[i] < 0) { ee[i] = v[p].second; }
            }
            p = q;
        }
        vals.clear();
        for (int j = 0; j < m; j++) vals.push_back(a[i][j]);
        sort(vals.begin(), vals.end());
        vals.erase(unique(vals.begin(), vals.end()), vals.end());
        // smallest integer > u_i that does not occur in this arena
        long long c = uu[i] + 1;
        int idx = (int)(lower_bound(vals.begin(), vals.end(), (int)c) - vals.begin());
        while (idx < (int)vals.size() && vals[idx] == (int)c) { c++; idx++; }
        cc[i] = c;
    }
}

// build and solve the network for a fixed K, optionally recover the bids
long long solveQ(int K, bool wantChoices, vector<long long>& outB) {
    int S = 0, base = 1, US = base + n + m, NOB = US + 1, COLL = NOB + 1, T = COLL + 1;
    int N = T + 1;
    MCMF mf(N);
    vector<array<int, 3>> optEdge(n, array<int, 3>{-1, -1, -1});
    for (int i = 0; i < n; i++) {
        int A = base + i;
        mf.add(S, A, 1, 0);
        if (dd[i] >= 0) optEdge[i][0] = mf.add(A, base + n + dd[i], 1, 0);
        if (ee[i] >= 0) optEdge[i][1] = mf.add(A, base + n + ee[i], 1, uu[i]);
        else if (dd[i] >= 0) optEdge[i][1] = mf.add(A, NOB, 1, uu[i]);
        else optEdge[i][1] = mf.add(A, NOB, 1, 0);
        optEdge[i][2] = mf.add(A, US, 1, cc[i]);
    }
    for (int j = 0; j < m; j++) mf.add(base + n + j, COLL, K, 0);
    mf.add(NOB, COLL, n, 0);
    mf.add(COLL, T, max(0, n - K), 0);
    mf.add(US, T, n, 0);
    auto res = mf.run(S, T, n);
    if (res.first < n) return LLONG_MAX / 4;
    if (wantChoices) {
        outB.assign(n, 0);
        for (int i = 0; i < n; i++) {
            int chosen = -1;
            for (int o = 0; o < 3; o++) {
                int e = optEdge[i][o];
                if (e >= 0 && mf.cap[e] == 0) { chosen = o; break; }
            }
            if (chosen == 0) outB[i] = 0;
            else if (chosen == 1) outB[i] = uu[i];
            else if (chosen == 2) outB[i] = cc[i];
            else outB[i] = 0; // should not happen
        }
    }
    return res.second;
}

// sanity check of a bid vector: everybody's wins, and the total price
bool validBids(const vector<long long>& b, long long& cost) {
    cost = 0;
    vector<int> w(m + 1, 0);
    vector<long long> v;
    for (int i = 0; i < n; i++) {
        cost += b[i];
        v.clear();
        for (int j = 0; j < m; j++) v.push_back(a[i][j]);
        v.push_back(b[i]);
        sort(v.begin(), v.end(), greater<long long>());
        for (int p = 0; p < (int)v.size();) {
            int q = p;
            while (q < (int)v.size() && v[q] == v[p]) q++;
            if (q - p == 1) {
                if (v[p] > 0) {
                    if (v[p] == b[i]) w[m]++;
                    else for (int j = 0; j < m; j++) if (a[i][j] == v[p]) w[j]++;
                }
                break;
            }
            p = q;
        }
    }
    for (int j = 0; j < m; j++) if (w[j] > w[m]) return false;
    return true;
}

int main() {
    if (scanf("%d %d", &n, &m) != 2) return 0;
    a.assign(n, vector<int>(m));
    for (int i = 0; i < n; i++)
        for (int j = 0; j < m; j++)
            if (scanf("%d", &a[i][j]) != 1) a[i][j] = 0;

    buildOptions();

    vector<long long> cache(n + 2, -1);
    vector<long long> dummy;
    auto Qeval = [&](int K) -> long long {
        if (cache[K] < 0) cache[K] = solveQ(K, false, dummy);
        return cache[K];
    };

    int lo = 0, hi = n;
    while (lo < hi) {
        int mid = (lo + hi) >> 1;
        long long q1 = Qeval(mid), q2 = Qeval(mid + 1);
        if (q1 <= q2) hi = mid; else lo = mid + 1;
    }
    // safety: pick the best among the found K and a few neighbours
    int bestK = lo;
    long long bestVal = Qeval(lo);
    for (int K = max(0, lo - 2); K <= min(n, lo + 2); K++) {
        long long v = Qeval(K);
        if (v < bestVal) { bestVal = v; bestK = K; }
    }

    vector<long long> b;
    solveQ(bestK, true, b);
    long long cost = 0;
    if (!validBids(b, cost) || cost != bestVal)  // should never trigger
        b = cc;
    for (int i = 0; i < n; i++) {
        if (b[i] < 0) b[i] = 0;
        if (b[i] > 1000000000LL) b[i] = 1000000000LL;
    }
    for (int i = 0; i < n; i++) printf("%lld%c", b[i], i + 1 == n ? '\n' : ' ');
    return 0;
}
