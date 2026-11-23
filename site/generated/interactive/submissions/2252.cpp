#include <bits/stdc++.h>
using namespace std;

struct Solver {
    int n;
    map<pair<int,int>, string> ballCache;

    Solver(int n) : n(n) {}

    string rawQuery(const vector<int>& d) {
        cout << "?";
        for (int x : d) cout << ' ' << x;
        cout << '\n' << flush;

        string s;
        if (!(cin >> s)) exit(0);
        if (s == "-1") exit(0);
        return s;
    }

    string ball(int c, int r) {
        if (r == 0) return string(n, '0');

        if (r >= n - 1) {
            string s(n, '1');
            s[c] = '0';
            return s;
        }

        auto key = make_pair(c, r);
        auto it = ballCache.find(key);
        if (it != ballCache.end()) return it->second;

        vector<int> d(n, 0);
        d[c] = r;
        string s = rawQuery(d);
        ballCache[key] = s;
        return s;
    }

    string smartQuery(const vector<int>& d) {
        int cnt = 0, last = -1;
        for (int i = 0; i < n; i++) {
            if (d[i] > 0) {
                cnt++;
                last = i;
            }
        }
        if (cnt == 0) return string(n, '0');
        if (cnt == 1) return ball(last, d[last]);
        return rawQuery(d);
    }

    int eccentricity(int c) {
        int lo = 1, hi = n - 1;
        while (lo < hi) {
            int mid = (lo + hi) / 2;
            string s = ball(c, mid);
            if (count(s.begin(), s.end(), '1') == n - 1) hi = mid;
            else lo = mid + 1;
        }
        return lo;
    }

    int farthestVertex(int c, int e) {
        string s = ball(c, e - 1);
        for (int i = 0; i < n; i++) {
            if (i != c && s[i] == '0') return i;
        }
        return -1;
    }

    vector<int> exactDistance(int c, int r) {
        if (r == 0) return {c};
        string cur = ball(c, r);
        string prev = ball(c, r - 1);
        vector<int> res;
        for (int i = 0; i < n; i++) {
            if (cur[i] == '1' && prev[i] == '0') res.push_back(i);
        }
        return res;
    }

    struct Interval {
        int a, b, L;
        vector<int> s;
        int m = -1;
        vector<int> le, ri;
    };

    vector<pair<int,int>> solvePath(int u, int v) {
        vector<int> all(n);
        iota(all.begin(), all.end(), 0);

        vector<Interval> cur;
        cur.push_back({u, v, n - 1, all});

        while (true) {
            bool unfinished = false;
            for (const auto& in : cur) {
                if (in.L > 1) {
                    unfinished = true;
                    break;
                }
            }
            if (!unfinished) break;

            for (auto& in : cur) {
                if (in.L == 2) {
                    for (int x : in.s) {
                        if (x != in.a && x != in.b) {
                            in.m = x;
                            break;
                        }
                    }
                }
            }

            for (int color = 0; color < 2; color++) {
                vector<int> ids;
                vector<int> d(n, 0);

                for (int i = 0; i < (int)cur.size(); i++) {
                    if (i % 2 != color || cur[i].L < 3) continue;
                    ids.push_back(i);
                    int k = cur[i].L / 2;
                    d[cur[i].a] = max(d[cur[i].a], k - 1);
                    d[cur[i].b] = max(d[cur[i].b], cur[i].L - k - 1);
                }

                if (ids.empty()) continue;
                string resp = smartQuery(d);
                vector<int> bad;

                for (int id : ids) {
                    auto& in = cur[id];
                    vector<int> off;
                    for (int x : in.s) {
                        if (x != in.a && x != in.b && resp[x] == '0') {
                            off.push_back(x);
                        }
                    }
                    if (off.size() == 1) in.m = off[0];
                    else bad.push_back(id);
                }

                for (int id : bad) {
                    auto& in = cur[id];
                    int k = in.L / 2;
                    vector<int> candidates = exactDistance(in.a, k);
                    for (int x : candidates) {
                        if (x == in.a || x == in.b) continue;
                        if (find(in.s.begin(), in.s.end(), x) != in.s.end()) {
                            in.m = x;
                            break;
                        }
                    }
                }
            }

            for (auto& in : cur) {
                if (in.L > 1 && in.L <= 3) {
                    in.le.clear();
                    in.ri.clear();
                    for (int x : in.s) {
                        if (x != in.a && x != in.b && x != in.m) {
                            in.ri.push_back(x);
                        }
                    }
                }
            }

            for (int color = 0; color < 2; color++) {
                vector<int> ids;
                vector<int> d(n, 0);

                for (int i = 0; i < (int)cur.size(); i++) {
                    if (i % 2 != color || cur[i].L < 4) continue;
                    ids.push_back(i);
                    int k = cur[i].L / 2;
                    d[cur[i].a] = max(d[cur[i].a], k - 1);
                }

                if (ids.empty()) continue;
                string resp = smartQuery(d);
                vector<int> bad;

                for (int id : ids) {
                    auto& in = cur[id];
                    int k = in.L / 2;
                    vector<int> on, off;

                    for (int x : in.s) {
                        if (x == in.a || x == in.b || x == in.m) continue;
                        if (resp[x] == '1') on.push_back(x);
                        else off.push_back(x);
                    }

                    if ((int)on.size() == k - 1) {
                        in.le = on;
                        in.ri = off;
                    } else {
                        bad.push_back(id);
                    }
                }

                for (int id : bad) {
                    auto& in = cur[id];
                    int k = in.L / 2;
                    string isolated = ball(in.a, k - 1);
                    in.le.clear();
                    in.ri.clear();

                    for (int x : in.s) {
                        if (x == in.a || x == in.b || x == in.m) continue;
                        if (isolated[x] == '1') in.le.push_back(x);
                        else in.ri.push_back(x);
                    }
                }
            }

            vector<Interval> nxt;
            for (auto& in : cur) {
                if (in.L == 1) {
                    nxt.push_back(in);
                    continue;
                }

                int k = in.L / 2;
                vector<int> sl, sr;
                sl.push_back(in.a);
                sl.insert(sl.end(), in.le.begin(), in.le.end());
                sl.push_back(in.m);

                sr.push_back(in.m);
                sr.insert(sr.end(), in.ri.begin(), in.ri.end());
                sr.push_back(in.b);

                nxt.push_back({in.a, in.m, k, sl});
                nxt.push_back({in.m, in.b, in.L - k, sr});
            }
            cur.swap(nxt);
        }

        vector<pair<int,int>> edges;
        for (const auto& in : cur) edges.push_back({in.a, in.b});
        return edges;
    }

    vector<pair<int,int>> solveGeneral(int u, int v, int D) {
        int k = D / 2;
        vector<int> A = exactDistance(u, k);
        vector<int> B = exactDistance(v, D - k);

        int root = -1;
        for (int x : A) {
            if (find(B.begin(), B.end(), x) != B.end()) {
                root = x;
                break;
            }
        }

        int R = D - k;
        vector<int> depth(n, -1);
        depth[root] = 0;

        for (int r = 1; r < R; r++) {
            string s = ball(root, r);
            for (int i = 0; i < n; i++) {
                if (depth[i] == -1 && s[i] == '1') depth[i] = r;
            }
        }
        for (int i = 0; i < n; i++) {
            if (depth[i] == -1) depth[i] = R;
        }

        vector<vector<int>> layers(R + 1);
        for (int i = 0; i < n; i++) layers[depth[i]].push_back(i);

        vector<int> parent(n, -1), code(n, 0);

        for (int t = 1; t <= R; t++) {
            if (layers[t - 1].size() == 1) {
                for (int x : layers[t]) parent[x] = layers[t - 1][0];
            }
        }

        for (int rem = 0; rem < 3; rem++) {
            vector<int> ps;
            int bits = 0;

            for (int p = rem; p < R; p += 3) {
                int sz = layers[p].size();
                if (sz <= 1 || layers[p + 1].empty()) continue;
                ps.push_back(p);

                int b = 0;
                while ((1 << b) < sz) b++;
                bits = max(bits, b);
            }

            for (int b = 0; b < bits; b++) {
                vector<int> d(n, 0);

                for (int p : ps) {
                    for (int i = 0; i < (int)layers[p].size(); i++) {
                        if ((i >> b) & 1) d[layers[p][i]] = 1;
                    }
                }

                string resp = smartQuery(d);
                for (int p : ps) {
                    for (int x : layers[p + 1]) {
                        if (resp[x] == '1') code[x] |= (1 << b);
                    }
                }
            }

            for (int p : ps) {
                for (int x : layers[p + 1]) {
                    parent[x] = layers[p][code[x]];
                }
            }
        }

        vector<pair<int,int>> edges;
        for (int i = 0; i < n; i++) {
            if (i != root) edges.push_back({parent[i], i});
        }
        return edges;
    }

    void run() {
        if (n == 2) {
            cout << "!\n1 2\n" << flush;
            return;
        }

        int e0 = eccentricity(0);
        int u = farthestVertex(0, e0);
        vector<pair<int,int>> edges;

        if (e0 == n - 1) {
            edges = solvePath(0, u);
        } else {
            int D = eccentricity(u);
            int v = farthestVertex(u, D);

            if (D == n - 1) edges = solvePath(u, v);
            else edges = solveGeneral(u, v, D);
        }

        cout << "!\n";
        for (auto [a, b] : edges) {
            cout << a + 1 << ' ' << b + 1 << '\n';
        }
        cout << flush;
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n)) return 0;

    Solver solver(n);
    solver.run();
    return 0;
}
