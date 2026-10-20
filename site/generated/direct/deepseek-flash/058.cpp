#include <bits/stdc++.h>
using namespace std;

struct FenwickMax {
    int n;
    vector<int> bit;
    FenwickMax(int n = 0) : n(n), bit(n + 2, 0) {}

    void update(int idx, int val) {
        while (idx <= n) {
            bit[idx] = max(bit[idx], val);
            idx += idx & -idx;
        }
    }

    int query(int idx) const {
        int res = 0;
        while (idx > 0) {
            res = max(res, bit[idx]);
            idx -= idx & -idx;
        }
        return res;
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    cin >> n;
    int N = 2 * n;
    vector<int> a(N + 1);
    for (int i = 1; i <= N; ++i) cin >> a[i];

    vector<int> firstPos(n + 1, 0), secondPos(n + 1, 0);
    vector<int> firstOrder;
    firstOrder.reserve(n);

    for (int i = 1; i <= N; ++i) {
        int x = a[i];
        if (firstPos[x] == 0) {
            firstPos[x] = i;
            firstOrder.push_back(x);
        } else {
            secondPos[x] = i;
        }
    }

    // rank[x] = length of the longest containment chain ending at interval x
    FenwickMax fw(N);
    vector<int> rankv(n + 1, 0);
    int D = 0;

    for (int x : firstOrder) {
        int r = secondPos[x];
        int idx = N - r + 1;          // reverse index
        int best = fw.query(idx - 1); // previous intervals with second > r
        rankv[x] = best + 1;
        D = max(D, rankv[x]);
        fw.update(idx, rankv[x]);
    }

    // All unordered pairs of stacks, lexicographic order.
    vector<pair<int, int>> pairsLex;
    pairsLex.reserve(180 * 179 / 2);
    for (int i = 1; i <= 180; ++i) {
        for (int j = i + 1; j <= 180; ++j) {
            pairsLex.emplace_back(i, j);
        }
    }

    vector<pair<int, int>> pairsColex;
    pairsColex.reserve(180 * 179 / 2);
    for (int j = 2; j <= 180; ++j) {
        for (int i = 1; i < j; ++i) {
            pairsColex.emplace_back(i, j);
        }
    }

    vector<int> last(181, 0);
    vector<int> indeg(n + 1, 0);
    vector<int> head(n + 1, -1);
    vector<int> nxt;
    vector<int> que(n + 1);
    vector<int> color(N + 1, 0);
    vector<int> from, to;
    from.reserve(N + 5);
    to.reserve(N + 5);

    auto run_check = [&](const vector<pair<int, int>>& rankPair, int orient) -> bool {
        fill(last.begin(), last.end(), 0);
        fill(indeg.begin(), indeg.end(), 0);

        from.clear();
        to.clear();

        for (int i = 1; i <= N; ++i) {
            int x = a[i];
            int r = rankv[x] - 1;

            pair<int, int> p;
            if (r >= 0 && r < (int)rankPair.size()) {
                p = rankPair[r];
            } else {
                p = rankPair[r % (int)rankPair.size()];
            }

            int c;
            bool isFirst = (i == firstPos[x]);
            if (isFirst) {
                c = (orient == 0 ? p.first : p.second);
            } else {
                c = (orient == 0 ? p.second : p.first);
            }

            color[i] = c;

            int y = last[c];
            if (y != 0) {
                // x occurs later on this stack, so x must be removed before y.
                from.push_back(x);
                to.push_back(y);
                indeg[y]++;
            }
            last[c] = x;
        }

        int E = (int)from.size();
        fill(head.begin(), head.end(), -1);
        nxt.resize(E);

        for (int e = 0; e < E; ++e) {
            int u = from[e];
            nxt[e] = head[u];
            head[u] = e;
        }

        int ql = 0, qr = 0;
        for (int v = 1; v <= n; ++v) {
            if (indeg[v] == 0) que[qr++] = v;
        }

        while (ql < qr) {
            int u = que[ql++];
            for (int e = head[u]; e != -1; e = nxt[e]) {
                int v = to[e];
                if (--indeg[v] == 0) {
                    que[qr++] = v;
                }
            }
        }

        return qr == n;
    };

    vector<pair<int, int>> rankPair(D);
    mt19937 rng(712367821);

    auto fill_pairs = [&](int type) {
        int sz = (int)pairsLex.size();
        if (type == 0) {
            // lexicographic
            for (int i = 0; i < D; ++i) {
                rankPair[i] = pairsLex[i % sz];
            }
        } else if (type == 1) {
            // reverse of the first D lexicographic pairs
            for (int i = 0; i < D; ++i) {
                int idx = (D - 1 - i) % sz;
                if (idx < 0) idx += sz;
                rankPair[i] = pairsLex[idx];
            }
        } else if (type == 2) {
            // colexicographic
            for (int i = 0; i < D; ++i) {
                rankPair[i] = pairsColex[i % sz];
            }
        } else if (type == 3) {
            // reverse colexicographic
            for (int i = 0; i < D; ++i) {
                int idx = (D - 1 - i) % sz;
                if (idx < 0) idx += sz;
                rankPair[i] = pairsColex[idx];
            }
        } else {
            // random shuffle
            vector<pair<int, int>> tmp = pairsLex;
            shuffle(tmp.begin(), tmp.end(), rng);
            for (int i = 0; i < D; ++i) {
                rankPair[i] = tmp[i % sz];
            }
        }
    };

    bool ok = false;

    // Deterministic candidates first.
    for (int type = 0; type < 4 && !ok; ++type) {
        fill_pairs(type);
        for (int orient = 0; orient < 2 && !ok; ++orient) {
            ok = run_check(rankPair, orient);
        }
    }

    // Random fallbacks, normally not needed.
    for (int it = 0; it < 25 && !ok; ++it) {
        fill_pairs(4);
        int orient = it & 1;
        ok = run_check(rankPair, orient);
    }

    for (int i = 1; i <= N; ++i) {
        if (i > 1) cout << ' ';
        cout << color[i];
    }
    cout << '\n';

    return 0;
}
