#include <bits/stdc++.h>
using namespace std;

using Mask = uint64_t;

int n;
vector<vector<int>> adj;
vector<int> parentOf, sub, tmpPar;
vector<char> removed;

struct Entry {
    int all, branch, dist;
};
vector<vector<Entry>> pathInfo;
vector<vector<int>> hist;

int newHist(int length) {
    hist.emplace_back(length, 0);
    return (int)hist.size() - 1;
}

void decompose(int start) {
    vector<int> nodes{start};
    tmpPar[start] = 0;
    for (size_t i = 0; i < nodes.size(); ++i) {
        int v = nodes[i];
        sub[v] = 1;
        for (int w : adj[v])
            if (!removed[w] && w != tmpPar[v]) {
                tmpPar[w] = v;
                nodes.push_back(w);
            }
    }
    int sz = nodes.size(), c = start;
    for (int i = sz - 1; i > 0; --i)
        sub[tmpPar[nodes[i]]] += sub[nodes[i]];
    for (int v : nodes) {
        int largest = sz - sub[v];
        for (int w : adj[v])
            if (!removed[w] && tmpPar[w] == v)
                largest = max(largest, sub[w]);
        if (2 * largest <= sz) {
            c = v;
            break;
        }
    }
    removed[c] = true;
    int all = newHist(1), far = 0;
    pathInfo[c].push_back({all, -1, 0});
    for (int w : adj[c]) if (!removed[w]) {
        int b = newHist(1), mx = 0;
        vector<array<int, 3>> st{{w, c, 1}};
        while (!st.empty()) {
            auto [v, p, d] = st.back();
            st.pop_back();
            mx = max(mx, d);
            pathInfo[v].push_back({all, b, d});
            for (int x : adj[v])
                if (x != p && !removed[x]) st.push_back({x, v, d + 1});
        }
        hist[b].resize(mx + 1);
        far = max(far, mx);
    }
    hist[all].resize(far + 1);
    for (int w : adj[c]) if (!removed[w]) decompose(w);
}

int ballCount(int v, int r) {
    int ans = 0;
    for (const Entry &e : pathInfo[v]) {
        int d = r - e.dist;
        if (d < 0) continue;
        const auto &a = hist[e.all];
        ans += a[min(d, (int)a.size() - 1)];
        if (e.branch >= 0) {
            const auto &b = hist[e.branch];
            ans -= b[min(d, (int)b.size() - 1)];
        }
    }
    return ans;
}

pair<int,int> balancedBall(const vector<int>& alive) {
    for (auto &a : hist) fill(a.begin(), a.end(), 0);
    for (int v : alive)
        for (const Entry &e : pathInfo[v]) {
            ++hist[e.all][e.dist];
            if (e.branch >= 0) ++hist[e.branch][e.dist];
        }
    for (auto &a : hist) partial_sum(a.begin(), a.end(), a.begin());

    int m = alive.size(), need = (m + 1) / 2;
    int lo = 0, hi = n - 1;
    while (lo < hi) {
        int mid = (lo + hi) / 2;
        if (ballCount(1, mid) >= need) hi = mid;
        else lo = mid + 1;
    }
    vector<int> median(n + 1);
    int best = m + 1, u = 1, k = 0;
    auto consider = [&](int v, int r, int count) {
        int score = max(count, m - count);
        if (r >= 0 && score < best) {
            best = score;
            u = v;
            k = r;
        }
    };
    median[0] = lo;
    for (int v = 1; v <= n; ++v) {
        // Moving the center one edge changes the median distance by at most one.
        int r = median[parentOf[v]];
        int a = ballCount(v, r - 1);
        consider(v, r - 1, a);
        if (a >= need) {
            median[v] = r - 1;
            consider(v, r - 2, ballCount(v, r - 2));
        } else {
            int b = ballCount(v, r);
            consider(v, r, b);
            if (b >= need) median[v] = r;
            else {
                median[v] = r + 1;
                consider(v, r + 1, ballCount(v, r + 1));
            }
        }
        if (best == need) break;
    }
    return {u, k};
}

void distances(int u, vector<int>& d) {
    vector<int> q{u};
    q.reserve(n);
    d[u] = 0;
    for (size_t i = 0; i < q.size(); ++i) {
        int v = q[i];
        for (int w : adj[v]) {
            if (v != u && d[w] == d[v] - 1) {
                // Distances of unvisited vertices must not be consulted.
                // The traversal below instead uses the rooted tree structure.
                break;
            }
        }
        break;
    }
    // First mark the root-to-u path, then propagate in parent order.
    fill(d.begin(), d.end(), -1);
    int len = 0;
    for (int v = u; v; v = parentOf[v]) d[v] = len++;
    for (int v = 1; v <= n; ++v)
        if (d[v] < 0) d[v] = d[parentOf[v]] + 1;
}

int ask(int u, int k) {
    cout << "? " << u << ' ' << k << endl;
    int o;
    if (!(cin >> o) || (o != 0 && o != 1)) exit(0);
    return o;
}

struct Cut {
    Mask mask;
    int u, k;
};
vector<Cut> cuts;
// Memoized height-constrained decision trees for the final candidate set.
unordered_map<Mask,int> memo[10][10];
int budget;
constexpr int NO = -1, UNKNOWN = -2, LEAF = -3;

int plan(Mask s, int h, int low) {
    int m = popcount(s);
    if (m < (1 << low) || m > (1 << h)) return NO;
    if (m == 1) return LEAF;
    auto &cache = memo[h][low];
    auto it = cache.find(s);
    if (it != cache.end()) return it->second;
    if (--budget < 0) return UNKNOWN;

    int nl = max(0, low - 1);
    int least = max(1 << nl, m - (1 << (h - 1)));
    array<vector<int>,33> options;
    for (int i = 0; i < (int)cuts.size(); ++i) {
        int a = popcount(cuts[i].mask & s);
        int z = min(a, m - a);
        if (z >= least) options[z].push_back(i);
    }
    for (int z = m / 2; z >= least; --z) {
        for (int i : options[z]) {
            Mask a = cuts[i].mask & s, b = s ^ a;
            if (popcount(a) < popcount(b)) swap(a, b);
            int p = plan(a, h - 1, nl);
            if (p == UNKNOWN) return p;
            if (p == NO) continue;
            p = plan(b, h - 1, nl);
            if (p == UNKNOWN) return p;
            if (p != NO) return cache[s] = i;
        }
    }
    return cache[s] = NO;
}

int finishSmall(const vector<int>& alive) {
    int m = alive.size();
    if (m == 1) return alive[0];
    Mask full = ~Mask(0) >> (64 - m);
    vector<unsigned short> ds((size_t)(n + 1) * m);
    vector<int> d(n + 1);
    for (int j = 0; j < m; ++j) {
        distances(alive[j], d);
        for (int v = 1; v <= n; ++v) ds[(size_t)v*m+j] = d[v];
    }
    unordered_set<Mask> seen;
    array<unsigned,64> order;
    for (int v = 1; v <= n; ++v) {
        auto row = &ds[(size_t)v*m];
        for (int j = 0; j < m; ++j) order[j] = unsigned(row[j])*64+j;
        sort(order.begin(), order.begin()+m);
        Mask a = 0;
        for (int j = 0; j+1 < m; ++j) {
            a |= Mask(1) << (order[j] % 64);
            if (order[j]/64 == order[j+1]/64) continue;
            Mask key = (a & 1) ? full ^ a : a;
            if (seen.insert(key).second)
                cuts.push_back({a, v, (int)(order[j]/64)});
        }
    }
    Mask s = full;
    int inherited = -1, lower = 0;
    while (popcount(s) > 1) {
        int cnt = popcount(s);
        int h = bit_width((unsigned)(cnt - 1));
        int chosen = NO, usedH = -1, usedL = 0;
        auto attempt = [&](int height, int low, int allowance) {
            if (chosen >= 0) return;
            budget = allowance;
            int p = plan(s, height, low);
            if (p >= 0) {
                chosen = p;
                usedH = height;
                usedL = low;
            }
        };
        // First try to attain both the entropy bound on average depth
        // and the binary information bound on maximum depth.
        attempt(h, h - 1, 350);
        attempt(h, 0, 500);
        if (inherited >= h) attempt(inherited, lower, 5);
        attempt(h + 1, max(0,h-2), 250);
        if (chosen < 0) {
            int best = cnt;
            for (int i = 0; i < (int)cuts.size(); ++i) {
                int a = popcount(s & cuts[i].mask);
                int worst = max(a, cnt-a);
                if (worst < best) best = worst, chosen = i;
            }
        }
        const Cut &c = cuts[chosen];
        s &= ask(c.u,c.k) ? c.mask : ~c.mask;
        inherited = usedH - 1;
        lower = max(0,usedL - 1);
    }
    return alive[countr_zero(s)];
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    if (!(cin >> n)) return 0;
    adj.resize(n + 1);
    parentOf.resize(n + 1);
    for (int i = 2; i <= n; ++i) {
        if (!(cin >> parentOf[i])) return 0;
        adj[parentOf[i]].push_back(i);
        adj[i].push_back(parentOf[i]);
    }
    vector<int> alive(n);
    iota(alive.begin(), alive.end(), 1);
    if (n > 64) {
        pathInfo.resize(n + 1);
        removed.resize(n + 1);
        tmpPar.resize(n + 1);
        sub.resize(n + 1);
        decompose(1);
        vector<int> d(n + 1);
        while (alive.size() > 64) {
            auto [u,k] = balancedBall(alive);
            int o = ask(u,k);
            distances(u,d);
            erase_if(alive,[&](int v){ return (d[v] <= k) != o; });
        }
    }
    int result = finishSmall(alive);
    cout << "! " << result << endl;
    return 0;
}
