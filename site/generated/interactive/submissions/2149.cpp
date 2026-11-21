#include <bits/stdc++.h>
using namespace std;

struct Key {
    array<uint64_t, 10> w{};
    bool operator==(const Key& other) const {
        return w == other.w;
    }
};

struct KeyHash {
    static uint64_t splitmix64(uint64_t x) {
        x += 0x9e3779b97f4a7c15ULL;
        x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9ULL;
        x = (x ^ (x >> 27)) * 0x94d049bb133111ebULL;
        return x ^ (x >> 31);
    }

    size_t operator()(const Key& k) const {
        uint64_t h = 0x123456789abcdef0ULL;
        for (int i = 0; i < 10; ++i) {
            h ^= splitmix64(k.w[i] + 0x9e3779b97f4a7c15ULL + (h << 6) + (h >> 2));
        }
        return (size_t)h;
    }
};

static inline void setBit(Key& k, int v) {
    int i = v - 1;
    k.w[i >> 6] |= 1ULL << (i & 63);
}

class Solver {
public:
    int n;
    unordered_map<Key, int, KeyHash> cache;
    vector<int> mark;
    int timer;

    Solver(int n_) : n(n_), mark(n_ + 1, 0), timer(0) {
        cache.reserve(200000);
        cache.max_load_factor(0.7);
    }

    static int estimate(int m, int k) {
        if (k <= 0 || k >= m) return 0;
        if (m == 1) return 1;
        if (m == 2) return 1;

        int ratio = (m + k - 1) / k;
        int levels = 0;
        while ((1 << levels) < ratio) ++levels;

        long long nodes = 1LL * k * levels + k - 1;
        if (nodes > m) nodes = m;
        long long cost = 2 * nodes - k;
        if (cost < 0) cost = 0;
        return (int)cost;
    }

    Key makeKeyVec(const vector<int>& v) const {
        Key k;
        for (int x : v) setBit(k, x);
        return k;
    }

    Key makeKeyPair(int a, int b) const {
        Key k;
        setBit(k, a);
        setBit(k, b);
        return k;
    }

    Key makeKeyUnion(const vector<int>& a, const vector<int>& b) const {
        Key k;
        for (int x : a) setBit(k, x);
        for (int x : b) setBit(k, x);
        return k;
    }

    Key makeKeyUnionSingle(const vector<int>& a, int x) const {
        Key k;
        for (int v : a) setBit(k, v);
        setBit(k, x);
        return k;
    }

    int readAnswer() {
        int ans;
        if (!(cin >> ans)) exit(0);
        if (ans < 0) exit(0);
        return ans;
    }

    int queryVec(const vector<int>& v) {
        int sz = (int)v.size();
        if (sz <= 1) return 0;

        Key k = makeKeyVec(v);
        auto it = cache.find(k);
        if (it != cache.end()) return it->second;

        cout << "? " << sz << '\n';
        for (int i = 0; i < sz; ++i) {
            if (i) cout << ' ';
            cout << v[i];
        }
        cout << '\n';
        cout.flush();

        int ans = readAnswer();
        cache[k] = ans;
        return ans;
    }

    int queryPair(int a, int b) {
        if (a == b) return 0;

        Key k = makeKeyPair(a, b);
        auto it = cache.find(k);
        if (it != cache.end()) return it->second;

        cout << "? 2\n" << a << ' ' << b << '\n';
        cout.flush();

        int ans = readAnswer();
        cache[k] = ans;
        return ans;
    }

    int queryUnion(const vector<int>& a, const vector<int>& b) {
        int sz = (int)a.size() + (int)b.size();
        if (sz <= 1) return 0;

        Key k = makeKeyUnion(a, b);
        auto it = cache.find(k);
        if (it != cache.end()) return it->second;

        cout << "? " << sz << '\n';
        int idx = 0;
        for (int x : a) {
            if (idx++) cout << ' ';
            cout << x;
        }
        for (int x : b) {
            if (idx++) cout << ' ';
            cout << x;
        }
        cout << '\n';
        cout.flush();

        int ans = readAnswer();
        cache[k] = ans;
        return ans;
    }

    int queryUnionSingle(const vector<int>& a, int x) {
        int sz = (int)a.size() + 1;
        if (sz <= 1) return 0;

        Key k = makeKeyUnionSingle(a, x);
        auto it = cache.find(k);
        if (it != cache.end()) return it->second;

        cout << "? " << sz << '\n';
        int idx = 0;
        for (int v : a) {
            if (idx++) cout << ' ';
            cout << v;
        }
        if (idx++) cout << ' ';
        cout << x << '\n';
        cout.flush();

        int ans = readAnswer();
        cache[k] = ans;
        return ans;
    }

    vector<int> subtract(const vector<int>& p, const vector<int>& rem) {
        if (rem.empty()) return p;

        ++timer;
        if (timer == INT_MAX) {
            fill(mark.begin(), mark.end(), 0);
            timer = 1;
        }
        for (int v : rem) mark[v] = timer;

        vector<int> res;
        res.reserve(p.size() - rem.size());
        for (int v : p) {
            if (mark[v] != timer) res.push_back(v);
        }
        return res;
    }

    int computeCross(int r, const vector<int>& p) {
        int m = (int)p.size();
        if (m == 0) return 0;
        if (m == 1) return queryPair(r, p[0]);

        int fp = queryVec(p);
        int fpr = queryUnionSingle(p, r);
        return fpr - fp;
    }

    int findAnyNeighborIn(int r, const vector<int>& p) {
        vector<int> cur = p;
        while (cur.size() > 1) {
            int mid = (int)cur.size() / 2;
            vector<int> a(cur.begin(), cur.begin() + mid);
            int crossA = computeCross(r, a);
            if (crossA > 0) {
                cur = move(a);
            } else {
                cur = vector<int>(cur.begin() + mid, cur.end());
            }
        }
        return cur[0];
    }

    int findAnyNonNeighborIn(int r, const vector<int>& p) {
        vector<int> cur = p;
        while (cur.size() > 1) {
            int mid = (int)cur.size() / 2;
            vector<int> a(cur.begin(), cur.begin() + mid);
            int crossA = computeCross(r, a);
            if (crossA < 0) crossA = 0;
            if (crossA > (int)a.size()) crossA = (int)a.size();

            int nonA = (int)a.size() - crossA;
            if (nonA > 0) {
                cur = move(a);
            } else {
                cur = vector<int>(cur.begin() + mid, cur.end());
            }
        }
        return cur[0];
    }

    vector<int> scanAllNeighbors(int r, const vector<int>& p) {
        vector<int> res;
        for (int v : p) {
            if (queryPair(r, v)) res.push_back(v);
        }
        return res;
    }

    vector<int> scanAllNonNeighbors(int r, const vector<int>& p) {
        vector<int> res;
        for (int v : p) {
            if (!queryPair(r, v)) res.push_back(v);
        }
        return res;
    }

    vector<int> getNeighbors(int r, const vector<int>& p, int k) {
        int m = (int)p.size();
        if (k <= 0) return {};
        if (k >= m) return p;
        if (m == 1) return p;

        int d = m - k;

        if (k == 1) {
            int v = findAnyNeighborIn(r, p);
            return {v};
        }
        if (d == 1) {
            int v = findAnyNonNeighborIn(r, p);
            return subtract(p, vector<int>{v});
        }

        int costN = estimate(m, k);
        int costD = estimate(m, d);
        int costScan = m;

        vector<int> res;

        if (costN <= costScan && costN <= costD) {
            int mid = m / 2;
            vector<int> a(p.begin(), p.begin() + mid);
            vector<int> b(p.begin() + mid, p.end());

            int crossA = computeCross(r, a);
            if (crossA < 0) crossA = 0;
            if (crossA > (int)a.size()) crossA = (int)a.size();

            int crossB = k - crossA;
            if (crossB < 0) crossB = 0;
            if (crossB > (int)b.size()) crossB = (int)b.size();

            res = getNeighbors(r, a, crossA);
            vector<int> resB = getNeighbors(r, b, crossB);
            res.insert(res.end(), resB.begin(), resB.end());
        } else if (costD <= costScan && costD <= costN) {
            vector<int> non = getNonNeighbors(r, p, d);
            res = subtract(p, non);
        } else {
            if (k <= d) {
                res.reserve(k);
                for (int v : p) {
                    if ((int)res.size() == k) break;
                    if (queryPair(r, v)) res.push_back(v);
                }
            } else {
                vector<int> non;
                non.reserve(d);
                for (int v : p) {
                    if ((int)non.size() == d) break;
                    if (!queryPair(r, v)) non.push_back(v);
                }
                res = subtract(p, non);
            }
        }

        if ((int)res.size() != k) {
            res = scanAllNeighbors(r, p);
        }
        return res;
    }

    vector<int> getNonNeighbors(int r, const vector<int>& p, int d) {
        int m = (int)p.size();
        if (d <= 0) return {};
        if (d >= m) return p;
        if (m == 1) return p;

        int k = m - d;

        if (d == 1) {
            int v = findAnyNonNeighborIn(r, p);
            return {v};
        }
        if (k == 1) {
            int v = findAnyNeighborIn(r, p);
            return subtract(p, vector<int>{v});
        }

        int costD = estimate(m, d);
        int costN = estimate(m, k);
        int costScan = m;

        vector<int> res;

        if (costD <= costScan && costD <= costN) {
            int mid = m / 2;
            vector<int> a(p.begin(), p.begin() + mid);
            vector<int> b(p.begin() + mid, p.end());

            int crossA = computeCross(r, a);
            if (crossA < 0) crossA = 0;
            if (crossA > (int)a.size()) crossA = (int)a.size();

            int nonA = (int)a.size() - crossA;
            if (nonA < 0) nonA = 0;
            if (nonA > (int)a.size()) nonA = (int)a.size();

            int nonB = d - nonA;
            if (nonB < 0) nonB = 0;
            if (nonB > (int)b.size()) nonB = (int)b.size();

            res = getNonNeighbors(r, a, nonA);
            vector<int> resB = getNonNeighbors(r, b, nonB);
            res.insert(res.end(), resB.begin(), resB.end());
        } else if (costN <= costScan && costN <= costD) {
            vector<int> neigh = getNeighbors(r, p, k);
            res = subtract(p, neigh);
        } else {
            if (d <= k) {
                res.reserve(d);
                for (int v : p) {
                    if ((int)res.size() == d) break;
                    if (!queryPair(r, v)) res.push_back(v);
                }
            } else {
                vector<int> neigh;
                neigh.reserve(k);
                for (int v : p) {
                    if ((int)neigh.size() == k) break;
                    if (queryPair(r, v)) neigh.push_back(v);
                }
                res = subtract(p, neigh);
            }
        }

        if ((int)res.size() != d) {
            res = scanAllNonNeighbors(r, p);
        }
        return res;
    }

    int findVertexWithNeighborMulti(const vector<int>& c, const vector<int>& f, int ff) {
        vector<int> cur = c;
        while (cur.size() > 1) {
            int mid = (int)cur.size() / 2;
            vector<int> a(cur.begin(), cur.begin() + mid);

            int fa = (a.size() == 1 ? 0 : queryVec(a));
            int faf = queryUnion(a, f);
            int crossA = faf - fa - ff;

            if (crossA > 0) {
                cur = move(a);
            } else {
                cur = vector<int>(cur.begin() + mid, cur.end());
            }
        }
        return cur[0];
    }

    pair<int, int> findEdgeBetween(const vector<int>& a, const vector<int>& b) {
        const vector<int>* f = &a;
        const vector<int>* c = &b;
        if (a.size() > b.size()) swap(f, c);

        if (f->size() == 1) {
            int fv = (*f)[0];
            int cv = findAnyNeighborIn(fv, *c);
            return {cv, fv};
        } else {
            int ff = queryVec(*f);
            int cv = findVertexWithNeighborMulti(*c, *f, ff);
            int fv = findAnyNeighborIn(cv, *f);
            return {cv, fv};
        }
    }

    pair<int, int> findEdgeWithin(vector<int> s, int fs) {
        while (true) {
            if (s.size() == 2) return {s[0], s[1]};

            int mid = (int)s.size() / 2;
            vector<int> a(s.begin(), s.begin() + mid);
            vector<int> b(s.begin() + mid, s.end());

            int fa = (a.size() == 1 ? 0 : queryVec(a));
            int fb = (b.size() == 1 ? 0 : queryVec(b));

            if (fa > 0) {
                s = move(a);
                fs = fa;
                continue;
            }
            if (fb > 0) {
                s = move(b);
                fs = fb;
                continue;
            }
            return findEdgeBetween(a, b);
        }
    }
};

static vector<int> getPath(int u, int v, const vector<int>& parent, const vector<int>& depth) {
    vector<int> left, right;
    int a = u, b = v;

    while (depth[a] > depth[b]) {
        left.push_back(a);
        a = parent[a];
    }
    while (depth[b] > depth[a]) {
        right.push_back(b);
        b = parent[b];
    }
    while (a != b) {
        left.push_back(a);
        right.push_back(b);
        a = parent[a];
        b = parent[b];
    }

    vector<int> path;
    for (int x : left) path.push_back(x);
    path.push_back(a);
    for (int i = (int)right.size() - 1; i >= 0; --i) path.push_back(right[i]);
    return path;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n)) return 0;

    if (n == 1) {
        cout << "Y 1\n1\n";
        cout.flush();
        return 0;
    }

    Solver sol(n);

    vector<int> outside;
    outside.reserve(n - 1);
    for (int i = 2; i <= n; ++i) outside.push_back(i);

    vector<int> q;
    q.reserve(n);
    q.push_back(1);

    vector<char> inTree(n + 1, 0);
    inTree[1] = 1;

    vector<vector<int>> adj(n + 1);

    for (size_t qi = 0; qi < q.size() && !outside.empty(); ++qi) {
        int r = q[qi];
        if (outside.empty()) break;

        int k = sol.computeCross(r, outside);
        if (k <= 0) continue;

        vector<int> neigh = sol.getNeighbors(r, outside, k);
        if ((int)neigh.size() != k) {
            neigh = sol.scanAllNeighbors(r, outside);
        }

        for (int v : neigh) {
            if (v < 1 || v > n || inTree[v]) continue;
            inTree[v] = 1;
            adj[r].push_back(v);
            adj[v].push_back(r);
            q.push_back(v);
        }

        vector<int> newOutside;
        newOutside.reserve(outside.size());
        for (int v : outside) {
            if (!inTree[v]) newOutside.push_back(v);
        }
        outside.swap(newOutside);
    }

    // Safety fallback (should not be needed for valid connected graphs).
    if (!outside.empty()) {
        vector<int> tree;
        for (int i = 1; i <= n; ++i) {
            if (inTree[i]) tree.push_back(i);
        }

        while (!outside.empty()) {
            auto [u, v] = sol.findEdgeBetween(tree, outside);
            int oldU, newV;
            if (inTree[u]) {
                oldU = u;
                newV = v;
            } else {
                oldU = v;
                newV = u;
            }

            adj[oldU].push_back(newV);
            adj[newV].push_back(oldU);
            inTree[newV] = 1;
            tree.push_back(newV);

            for (size_t i = 0; i < outside.size(); ++i) {
                if (outside[i] == newV) {
                    outside[i] = outside.back();
                    outside.pop_back();
                    break;
                }
            }
        }
    }

    vector<int> color(n + 1, -1), parent(n + 1, 0), depth(n + 1, 0);
    queue<int> cq;

    for (int i = 1; i <= n; ++i) {
        if (color[i] != -1) continue;
        color[i] = 0;
        parent[i] = 0;
        depth[i] = 0;
        cq.push(i);

        while (!cq.empty()) {
            int u = cq.front();
            cq.pop();

            for (int v : adj[u]) {
                if (color[v] == -1) {
                    color[v] = color[u] ^ 1;
                    parent[v] = u;
                    depth[v] = depth[u] + 1;
                    cq.push(v);
                }
            }
        }
    }

    vector<int> X, Y;
    for (int i = 1; i <= n; ++i) {
        if (color[i] == 0) X.push_back(i);
        else Y.push_back(i);
    }

    vector<int> S1 = (X.size() <= Y.size() ? X : Y);
    vector<int> S2 = (X.size() <= Y.size() ? Y : X);

    int f1 = (S1.empty() ? 0 : sol.queryVec(S1));

    vector<int> badSet;
    int badF = 0;

    if (f1 > 0) {
        badSet = S1;
        badF = f1;
    } else {
        int f2 = (S2.empty() ? 0 : sol.queryVec(S2));
        if (f2 > 0) {
            badSet = S2;
            badF = f2;
        } else {
            vector<int> side = S1.empty() ? S2 : S1;
            cout << "Y " << side.size() << '\n';
            for (int i = 0; i < (int)side.size(); ++i) {
                if (i) cout << ' ';
                cout << side[i];
            }
            cout << '\n';
            cout.flush();
            return 0;
        }
    }

    auto [u, v] = sol.findEdgeWithin(badSet, badF);
    vector<int> cycle = getPath(u, v, parent, depth);

    cout << "N " << cycle.size() << '\n';
    for (int i = 0; i < (int)cycle.size(); ++i) {
        if (i) cout << ' ';
        cout << cycle[i];
    }
    cout << '\n';
    cout.flush();

    return 0;
}
