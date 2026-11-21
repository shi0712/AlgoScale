// Menji, We Miss You!
//
// We maintain the exact set S of vertices consistent with all answers.
// A query (u,k) keeps S <- S & ball(u,k)  or  S <- S \ ball(u,k).
// While |S| > 1 we greedily choose a ball that minimizes
// max(|S & ball|, |S \ ball|), so the larger possible remaining set is
// roughly halved each turn.
//
// For large S we take a few centers: the two endpoints of a diameter of S,
// samples and the weighted median on that diameter path, plus some spread
// out candidates.  For small S we simply try every candidate as a center.
// For a fixed center all radii are examined in O(n) with one BFS.
#include <bits/stdc++.h>
using namespace std;

static int n;
static vector<vector<int>> adj;
static vector<char> inS;
static int m;
static vector<int> dist_, par_, qbuf, cntAt;

// BFS from src. If needPar, fills par_. Returns max distance.
static int bfs(int src, bool needPar) {
    fill(dist_.begin(), dist_.end(), -1);
    if (needPar) fill(par_.begin(), par_.end(), -1);
    int head = 0, tail = 0;
    qbuf[tail++] = src;
    dist_[src] = 0;
    int maxd = 0;
    while (head < tail) {
        int x = qbuf[head++];
        int dx = dist_[x] + 1;
        for (int y : adj[x]) {
            if (dist_[y] < 0) {
                dist_[y] = dx;
                if (needPar) par_[y] = x;
                if (dx > maxd) maxd = dx;
                qbuf[tail++] = y;
            }
        }
    }
    return maxd;
}

static bool ask(int u, int k) {
    cout << "? " << (u + 1) << ' ' << k << '\n';
    cout.flush();
    int o;
    if (!(cin >> o)) exit(0);
    return o == 1;
}

struct BestQuery {
    int mx;
    int cnt;     // candidates inside ball
    int center;
    int k;
};

static BestQuery bestQ;

static inline void consider(int mxv, int acc, int u, int k) {
    if (mxv < bestQ.mx ||
        (mxv == bestQ.mx && acc > bestQ.cnt)) {
        bestQ.mx = mxv;
        bestQ.cnt = acc;
        bestQ.center = u;
        bestQ.k = k;
    }
}

// Evaluate all radii for one center and update bestQ.
static void evalCenter(int u) {
    int maxd = bfs(u, false);
    if ((int)cntAt.size() < maxd + 1) cntAt.resize(maxd + 1);
    fill(cntAt.begin(), cntAt.begin() + maxd + 1, 0);
    for (int v = 0; v < n; ++v)
        if (inS[v]) ++cntAt[dist_[v]];
    int acc = 0;
    for (int k = 0; k <= maxd; ++k) {
        acc += cntAt[k];
        if (acc == 0 || acc == m) continue;
        int mxv = max(acc, m - acc);
        if (mxv <= bestQ.mx) consider(mxv, acc, u, k);
    }
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    if (!(cin >> n)) return 0;
    adj.assign(n, {});
    for (int i = 1; i < n; ++i) {
        int p;
        cin >> p;
        --p;
        adj[i].push_back(p);
        adj[p].push_back(i);
    }
    dist_.assign(n, -1);
    par_.assign(n, -1);
    qbuf.assign(n, 0);
    cntAt.assign(n, 0);
    inS.assign(n, 1);
    m = n;

    vector<int> candidates;
    vector<int> centers;
    vector<char> used;

    while (m > 1) {
        candidates.clear();
        for (int v = 0; v < n; ++v)
            if (inS[v]) candidates.push_back(v);

        centers.clear();
        const int ALL_THRESH = 220;
        if (m <= ALL_THRESH) {
            centers = candidates;
        } else {
            // Find a diameter (farthest pair of candidates) of the candidate set.
            int c0 = candidates[0];
            bfs(c0, false);
            int a = c0;
            for (int v : candidates)
                if (dist_[v] > dist_[a]) a = v;
            bfs(a, true);
            int b = a;
            for (int v : candidates)
                if (dist_[v] > dist_[b]) b = v;

            // Path a -> b (parent tree is rooted at a).
            vector<int> path;
            for (int x = b;; x = par_[x]) {
                path.push_back(x);
                if (x == a) break;
            }
            reverse(path.begin(), path.end());
            int L = (int)path.size();

            centers.push_back(a);
            centers.push_back(b);

            int K = min(L, 12);
            if (K >= 2) {
                for (int i = 0; i < K; ++i) {
                    int idx = (int)((long long)i * (L - 1) / (K - 1));
                    centers.push_back(path[idx]);
                }
            }
            // weighted median along the diameter path
            {
                long long acc = 0;
                int med = path[0];
                for (int v : path) {
                    if (inS[v]) ++acc;
                    if (acc * 2 >= m) { med = v; break; }
                }
                centers.push_back(med);
            }
            // spread candidates (by id order)
            int nc = (int)candidates.size();
            for (int i = 1; i <= 8; ++i) {
                int idx = (int)((long long)i * (nc - 1) / 8);
                centers.push_back(candidates[idx]);
            }
        }

        // Deduplicate centers.
        used.assign(n, 0);
        {
            int w = 0;
            for (int u : centers) {
                if (!used[u]) { used[u] = 1; centers[w++] = u; }
            }
            centers.resize(w);
        }

        bestQ.mx = m + 1;
        bestQ.cnt = -1;
        bestQ.center = -1;
        bestQ.k = 0;

        for (int u : centers) evalCenter(u);

        // If the split is poor, try more centers.
        if (bestQ.mx * 5 > m * 3 && m > 1) {
            // Add many evenly spaced candidates.
            int nc = (int)candidates.size();
            int extra = (int)centers.size();
            int want = min(nc, 60);
            for (int i = 0; i < want; ++i) {
                int idx = (int)((long long)i * (nc - 1) / max(1, want - 1));
                int u = candidates[idx];
                if (!used[u]) { used[u] = 1; centers.push_back(u); }
            }
            for (int i = extra; i < (int)centers.size(); ++i) evalCenter(centers[i]);
        }
        if (bestQ.mx * 4 > m * 3 && m <= 1200) {
            // Last resort for medium sizes: look at every candidate.
            int base = (int)centers.size();
            for (int u : candidates)
                if (!used[u]) { used[u] = 1; centers.push_back(u); }
            for (int i = base; i < (int)centers.size(); ++i) evalCenter(centers[i]);
        }

        int u = bestQ.center;
        int k = bestQ.k;
        bool yes = ask(u, k);

        bfs(u, false);
        int nm = 0;
        for (int v = 0; v < n; ++v) {
            if (inS[v]) {
                bool inside = (dist_[v] <= k);
                if (inside != yes) inS[v] = 0;
                else ++nm;
            }
        }
        m = nm;
    }

    int ans = -1;
    for (int v = 0; v < n; ++v)
        if (inS[v]) { ans = v; break; }
    if (ans < 0) return 0;
    cout << "! " << (ans + 1) << '\n';
    cout.flush();
    return 0;
}
