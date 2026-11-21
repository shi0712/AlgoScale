#include <bits/stdc++.h>
using namespace std;

int n;

struct Component {
    vector<int> A, B;        // current coloring: A = color 0, B = color 1
    vector<int> verts;       // all vertices in this component
    vector<int> oldComps;    // components merged when newVertex was added
    int f;                   // number of edges inside this component
    int newVertex;
};

vector<Component> comps;

int ask(const vector<int>& s) {
    cout << "? " << s.size() << '\n';
    for (int i = 0; i < (int)s.size(); ++i) {
        if (i) cout << ' ';
        cout << s[i];
    }
    cout << '\n';
    cout.flush();

    int res;
    cin >> res;
    if (res == -1) exit(0);
    if (!cin) exit(0);
    return res;
}

// candidates must form an independent set in the graph.
// There must be at least one edge from x to candidates.
int findNeighborInIndependent(int x, const vector<int>& candidates) {
    int l = 0, r = (int)candidates.size();
    while (r - l > 1) {
        int mid = (l + r) >> 1;
        vector<int> q;
        q.reserve(mid - l + 1);
        q.push_back(x);
        for (int i = l; i < mid; ++i) q.push_back(candidates[i]);

        int cnt = ask(q); // since candidates[l..mid) are independent
        if (cnt > 0) r = mid;
        else l = mid;
    }
    return candidates[l];
}

vector<int> getTreePath(int cid, int u, int v) {
    vector<int> color(n + 1, -1);
    for (int x : comps[cid].A) color[x] = 0;
    for (int x : comps[cid].B) color[x] = 1;

    vector<vector<int>> tree(n + 1);

    function<void(int)> build = [&](int id) {
        Component& C = comps[id];
        if (C.oldComps.empty()) return;

        int x = C.newVertex;

        for (int old : C.oldComps) {
            vector<int> candidates;
            for (int y : comps[old].verts) {
                if (color[y] != color[x]) {
                    candidates.push_back(y);
                }
            }

            int y = findNeighborInIndependent(x, candidates);
            tree[x].push_back(y);
            tree[y].push_back(x);

            build(old);
        }
    };

    build(cid);

    vector<int> parent(n + 1, -2);
    queue<int> q;
    q.push(u);
    parent[u] = 0;

    while (!q.empty()) {
        int x = q.front();
        q.pop();
        if (x == v) break;

        for (int y : tree[x]) {
            if (parent[y] == -2) {
                parent[y] = x;
                q.push(y);
            }
        }
    }

    vector<int> path;
    int cur = v;
    while (true) {
        path.push_back(cur);
        if (cur == u) break;
        cur = parent[cur];
    }
    reverse(path.begin(), path.end());
    return path;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    cin >> n;

    comps.reserve(n + 5);

    vector<int> order(n);
    iota(order.begin(), order.end(), 1);

    mt19937 rng(chrono::steady_clock::now().time_since_epoch().count());
    shuffle(order.begin(), order.end(), rng);

    vector<int> active;
    vector<int> stamp(n + 5, 0);
    int curStamp = 1;

    for (int pos = 0; pos < n; ++pos) {
        int i = order[pos];

        vector<int> prefix(active.size() + 1, 0);
        for (int j = 0; j < (int)active.size(); ++j) {
            prefix[j + 1] = prefix[j] + comps[active[j]].f;
        }

        auto countRange = [&](int l, int r) -> int {
            if (l >= r) return 0;

            vector<int> q;
            q.push_back(i);

            for (int idx = l; idx < r; ++idx) {
                int cid = active[idx];
                for (int v : comps[cid].verts) {
                    q.push_back(v);
                }
            }

            int val = ask(q);
            return val - (prefix[r] - prefix[l]);
        };

        int total = 0;
        if (!active.empty()) {
            total = countRange(0, (int)active.size());
        }

        vector<pair<int, int>> adjacent;

        if (total > 0) {
            function<void(int, int, int)> dfs = [&](int l, int r, int tot) {
                if (tot == 0) return;

                if (r - l == 1) {
                    adjacent.push_back({active[l], tot});
                    return;
                }

                int mid = (l + r) >> 1;
                int left = countRange(l, mid);
                int right = tot - left;

                if (left > 0) dfs(l, mid, left);
                if (right > 0) dfs(mid, r, right);
            };

            dfs(0, (int)active.size(), total);
        }

        if (adjacent.empty()) {
            Component c;
            c.A = {i};
            c.B = {};
            c.verts = {i};
            c.oldComps = {};
            c.f = 0;
            c.newVertex = i;

            int id = (int)comps.size();
            comps.push_back(c);
            active.push_back(id);
        } else {
            for (auto& pr : adjacent) {
                int cid = pr.first;
                int t = pr.second;

                Component& C = comps[cid];

                bool aNonEmpty = !C.A.empty();
                bool bNonEmpty = !C.B.empty();

                int cntA;

                if (!aNonEmpty) {
                    cntA = 0;
                } else if (!bNonEmpty) {
                    cntA = t;
                } else {
                    vector<int> q;
                    q.reserve(C.A.size() + 1);
                    q.push_back(i);
                    for (int v : C.A) q.push_back(v);

                    cntA = ask(q); // C.A is independent
                }

                int cntB = t - cntA;

                if (cntA > 0 && cntB > 0) {
                    int u = findNeighborInIndependent(i, C.A);
                    int v = findNeighborInIndependent(i, C.B);

                    vector<int> path = getTreePath(cid, u, v);

                    vector<int> cycle;
                    cycle.push_back(i);
                    for (int x : path) cycle.push_back(x);

                    cout << "N " << cycle.size() << '\n';
                    for (int j = 0; j < (int)cycle.size(); ++j) {
                        if (j) cout << ' ';
                        cout << cycle[j];
                    }
                    cout << '\n';
                    cout.flush();
                    return 0;
                }

                // We want the neighbors of i inside C to have color 1,
                // while i itself has color 0.
                if (cntA > 0) {
                    swap(C.A, C.B);
                }
            }

            vector<int> newA, newB, newVerts, oldIds;
            newA.push_back(i);
            newVerts.push_back(i);

            int sumF = 0;

            for (auto& pr : adjacent) {
                int cid = pr.first;
                Component& C = comps[cid];

                sumF += C.f;

                newA.insert(newA.end(), C.A.begin(), C.A.end());
                newB.insert(newB.end(), C.B.begin(), C.B.end());
                newVerts.insert(newVerts.end(), C.verts.begin(), C.verts.end());

                oldIds.push_back(cid);
            }

            int newF = sumF + total;

            Component c;
            c.A = move(newA);
            c.B = move(newB);
            c.verts = move(newVerts);
            c.oldComps = move(oldIds);
            c.f = newF;
            c.newVertex = i;

            int newId = (int)comps.size();
            comps.push_back(c);

            ++curStamp;
            for (auto& pr : adjacent) {
                stamp[pr.first] = curStamp;
            }

            vector<int> newActive;
            newActive.reserve(active.size() + 1);

            for (int cid : active) {
                if (stamp[cid] != curStamp) {
                    newActive.push_back(cid);
                }
            }

            newActive.push_back(newId);
            active.swap(newActive);
        }
    }

    int finalCid = active[0];
    vector<int>& side = comps[finalCid].A;

    cout << "Y " << side.size() << '\n';
    for (int i = 0; i < (int)side.size(); ++i) {
        if (i) cout << ' ';
        cout << side[i];
    }
    cout << '\n';
    cout.flush();

    return 0;
}
