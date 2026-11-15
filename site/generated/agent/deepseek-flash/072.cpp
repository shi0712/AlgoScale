#include <bits/stdc++.h>
using namespace std;

/* ------------------------------------------------------------------
   Menji, We Miss You!
   Adaptive search with ball queries dist(u,X) <= k:
   maintain the set S of vertices consistent with all answers.
   Choose a ball that splits S as evenly as possible and query it.

   To find a good ball we sweep the diameter path of S (a 1D problem
   solved with two Fenwick trees and a moving k-th order statistic),
   and, when that is not balanced enough, we also sweep a few other
   scattered paths and test some random central vertices.
   ------------------------------------------------------------------ */

struct Fenwick {
    int n;
    vector<int> bit;
    void init(int n_) { n = n_; bit.assign(n + 1, 0); }
    void add(int i, int v) { for (++i; i <= n; i += i & -i) bit[i] += v; }
    int sumPrefix(int i) const {          // sum on [0..i]
        if (i < 0) return 0;
        if (i >= n) i = n - 1;
        int s = 0;
        for (++i; i > 0; i -= i & -i) s += bit[i];
        return s;
    }
};

struct Solver {
    int n;
    vector<vector<int>> adj;
    vector<int> distA, distB, distTmp, par, distC;
    vector<int> qbuf;

    void init(int n_, const vector<int>& parent) {
        n = n_;
        adj.assign(n + 1, {});
        for (int i = 2; i <= n; ++i) {
            adj[i].push_back(parent[i]);
            adj[parent[i]].push_back(i);
        }
        distA.assign(n + 1, 0);
        distB.assign(n + 1, 0);
        distTmp.assign(n + 1, 0);
        distC.assign(n + 1, 0);
        par.assign(n + 1, 0);
        qbuf.reserve(n + 1);
    }

    void bfs(int s, vector<int>& d, vector<int>* parOut = nullptr) {
        fill(d.begin(), d.end(), -1);
        qbuf.clear();
        qbuf.push_back(s);
        d[s] = 0;
        if (parOut) (*parOut)[s] = 0;
        for (size_t qi = 0; qi < qbuf.size(); ++qi) {
            int u = qbuf[qi];
            for (int v : adj[u]) if (d[v] < 0) {
                d[v] = d[u] + 1;
                if (parOut) (*parOut)[v] = u;
                qbuf.push_back(v);
            }
        }
    }

    // Best (largest smaller side) ball whose centre is on the path a--b.
    // Returns {M, centre vertex, radius}.  M = max over radii of min(cnt, N-cnt).
    tuple<int,int,int> sweepPath(const vector<int>& S, int a, int b) {
        int N = (int)S.size();
        bfs(a, distA, &par);
        int bestB = S[0];
        for (int x : S) if (distA[x] > distA[bestB]) bestB = x;
        if (b < 0) b = bestB;
        for (int x : S) if (distA[x] > distA[b]) b = x;
        bfs(b, distB);
        int L = distA[b];

        vector<int> path;
        path.reserve(L + 1);
        {
            int cur = b;
            while (cur != a) { path.push_back(cur); cur = par[cur]; }
            path.push_back(a);
            reverse(path.begin(), path.end());
        }

        vector<vector<int>> byPos(L + 1);
        vector<int> posOf(n + 1), hOf(n + 1);
        for (int x : S) {
            int h = (distA[x] + distB[x] - L) / 2;
            int p = (distA[x] + L - distB[x]) / 2;
            posOf[x] = p;
            hOf[x] = h;
            byPos[p].push_back(x);
        }

        Fenwick fL, fR;
        int SZ = 4 * n + 20;
        int OFF = n + 5;
        fL.init(SZ);
        fR.init(SZ);
        for (int x : S) {
            int bv = hOf[x] - posOf[x] + OFF;
            int av = hOf[x] + posOf[x];
            if (posOf[x] <= 0) fL.add(bv, 1);
            else fR.add(av, 1);
        }
        auto countLE = [&](int t, int v) -> int {
            return fL.sumPrefix(v - t + OFF) + fR.sumPrefix(v + t);
        };

        int k = max(1, N / 2);
        int lo = 0, hi = n;
        while (lo < hi) {
            int mid = (lo + hi) >> 1;
            if (countLE(0, mid) >= k) hi = mid;
            else lo = mid + 1;
        }
        int v = lo;                        // k-th smallest distance at t=0
        int bestM = -1, bestT = -1, bestR = -1;
        for (int t = 0; t <= L; ++t) {
            while (v > 0 && countLE(t, v - 1) >= k) --v;
            while (countLE(t, v) < k) ++v;
            int F1 = countLE(t, v);
            int F0 = (v > 0 ? countLE(t, v - 1) : 0);
            int m1 = min(F1, N - F1);
            int m0 = min(F0, N - F0);
            int M = max(m1, m0);
            if (M > bestM) {
                bestM = M;
                bestT = t;
                bestR = (m0 > m1 ? v - 1 : v);
            }
            if (t + 1 <= L) {
                for (int x : byPos[t + 1]) {
                    fR.add(hOf[x] + posOf[x], -1);
                    fL.add(hOf[x] - posOf[x] + OFF, 1);
                }
            }
        }
        return {bestM, path[bestT], bestR};
    }

    tuple<int,int,int> pathBest(const vector<int>& S) {
        int N = (int)S.size();
        if (N <= 1) return {N, S.empty() ? -1 : S[0], 0};
        bfs(S[0], distA);
        int a = S[0];
        for (int x : S) if (distA[x] > distA[a]) a = x;
        return sweepPath(S, a, -1);
    }

    // Exact best ball centred at c.  Returns {M, radius}.
    pair<int,int> evalCenter(const vector<int>& S, int c) {
        int N = (int)S.size();
        bfs(c, distTmp);
        static vector<int> cnt;            // static: avoid reallocation
        cnt.assign(n + 1, 0);
        int mx = 0;
        for (int x : S) {
            int d = distTmp[x];
            ++cnt[d];
            if (d > mx) mx = d;
        }
        int cum = 0, bestM = 0, bestR = -1;
        for (int r = 0; r <= mx; ++r) {
            int F1 = cum + cnt[r], F0 = cum;
            int m1 = min(F1, N - F1), m0 = min(F0, N - F0);
            int M = max(m1, m0);
            if (M > bestM) {
                bestM = M;
                bestR = (m0 > m1 ? r - 1 : r);
            }
            cum = F1;
        }
        return {bestM, bestR};
    }

    // Choose a query.  Returns {M, centre, radius}.
    tuple<int,int,int> bestSplit(const vector<int>& S, mt19937& rng) {
        int N = (int)S.size();
        auto best = pathBest(S);
        int bestM = get<0>(best);

        // If the diameter path already gives a split at least as good as
        // "half minus a few", additional work is unnecessary.
        int accept = N / 2 - 2;
        if (N <= 64 || bestM >= accept) return best;

        // A few more paths: from scattered vertices towards their farthest
        // candidate.  These catch optima lying in side branches.
        vector<int> starts;
        int extra = min(8, (int)S.size());
        starts.reserve(extra);
        for (int j = 0; j < extra; ++j)
            starts.push_back(S[(long long)j * N / extra]);
        shuffle(starts.begin(), starts.end(), rng);

        for (int a : starts) {
            auto cand = sweepPath(S, a, -1);
            if (get<0>(cand) > bestM) { bestM = get<0>(cand); best = cand; }
            if (bestM >= accept) return best;
        }

        // A small number of direct centre evaluations, including vertices
        // outside S (the optimum may be in an off-path branch).
        int samples = min(24, (int)S.size());
        for (int j = 0; j < samples; ++j) {
            int c = (j & 1) ? (int)(rng() % n + 1) : S[(int)(rng() % N)];
            auto [M, r] = evalCenter(S, c);
            if (M > bestM) {
                bestM = M;
                best = {M, c, r};
            }
            if (bestM >= accept) return best;
        }

        // Short hill-climb from the current best centre.
        int c = get<1>(best);
        for (int step = 0; step < 6 && bestM < accept; ++step) {
            int nb = -1, nM = -1, nr = -1;
            for (int v : adj[c]) {
                auto [M, r] = evalCenter(S, v);
                if (M > nM) { nM = M; nb = v; nr = r; }
            }
            if (nb < 0 || nM <= bestM) break;
            c = nb; bestM = nM; best = {nM, nb, nr};
        }
        return best;
    }

    // After a query, keep the consistent vertices.
    void filter(vector<int>& S, int c, int r, int answer) {
        bfs(c, distC);
        vector<int> keep;
        keep.reserve(S.size());
        for (int x : S) {
            bool inside = (distC[x] <= r);
            if ((inside && answer == 1) || (!inside && answer == 0))
                keep.push_back(x);
        }
        S.swap(keep);
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int T;
    if (!(cin >> T)) return 0;
    for (int tc = 0; tc < T; ++tc) {
        int n;
        cin >> n;
        vector<int> parent(n + 1, 0);
        for (int i = 2; i <= n; ++i) cin >> parent[i];

        Solver sol;
        sol.init(n, parent);
        vector<int> S(n);
        iota(S.begin(), S.end(), 1);
        mt19937 rng(712367u + (unsigned)tc * 1000003u);

        int used = 0;
        while (S.size() > 1) {
            if (used >= 21) break;            // should never happen
            auto best = sol.bestSplit(S, rng);
            int c = get<1>(best), r = get<2>(best);

            // Safety: make sure the chosen ball really splits S.
            sol.bfs(c, sol.distC);
            int cnt = 0;
            for (int x : S) cnt += (sol.distC[x] <= r);
            if (cnt == 0 || cnt == (int)S.size()) {
                c = S[0];
                r = 0;
                sol.bfs(c, sol.distC);
            }

            cout << "? " << c << ' ' << r << '\n';
            cout.flush();
            int o;
            if (!(cin >> o)) return 0;
            ++used;

            vector<int> keep;
            keep.reserve(S.size());
            for (int x : S) {
                bool inside = (sol.distC[x] <= r);
                if ((inside && o == 1) || (!inside && o == 0)) keep.push_back(x);
            }
            S.swap(keep);
            if (S.empty()) return 0;
        }
        cout << "! " << S[0] << '\n';
        cout.flush();
    }
    return 0;
}
