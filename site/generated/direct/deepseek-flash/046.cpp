#include <bits/stdc++.h>
using namespace std;

using int64 = long long;
using u64 = unsigned long long;

struct Interval {
    int64 l, r;
};

struct Seg {
    int col;
    int64 l, r, len;
};

static const int64 MAXV = 1000000000000000000LL;

static u64 splitmix64(u64 x) {
    x += 0x9e3779b97f4a7c15ULL;
    x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9ULL;
    x = (x ^ (x >> 27)) * 0x94d049bb133111ebULL;
    return x ^ (x >> 31);
}

struct PNode {
    int lc = 0, rc = 0;
    int cnt = 0;
    u64 h1 = 0, h2 = 0;
};

vector<PNode> pst(1);

u64 rnd1(int64 x) {
    return splitmix64((u64)x + 0x123456789abcdef0ULL);
}
u64 rnd2(int64 x) {
    return splitmix64((u64)x + 0xfedcba9876543210ULL);
}

int pstUpdate(int root, int64 L, int64 R, int64 pos) {
    int nw = (int)pst.size();
    pst.push_back(pst[root]);

    if (L == R) {
        pst[nw].cnt++;
        pst[nw].h1 += rnd1(pos);
        pst[nw].h2 += rnd2(pos);
        return nw;
    }

    int64 mid = (L + R) >> 1;
    if (pos <= mid) {
        pst[nw].lc = pstUpdate(pst[root].lc, L, mid, pos);
    } else {
        pst[nw].rc = pstUpdate(pst[root].rc, mid + 1, R, pos);
    }

    int lc = pst[nw].lc;
    int rc = pst[nw].rc;
    pst[nw].cnt = pst[lc].cnt + pst[rc].cnt;
    pst[nw].h1 = pst[lc].h1 + pst[rc].h1;
    pst[nw].h2 = pst[lc].h2 + pst[rc].h2;
    return nw;
}

int addVectorToRoot(int root, const vector<int64>& v) {
    for (int64 x : v) {
        if (x > 0 && x <= MAXV) {
            root = pstUpdate(root, 1, MAXV, x);
        }
    }
    return root;
}

int comparePST(int a, int b, int64 L, int64 R) {
    if (a == b) return 0;

    const PNode &na = pst[a];
    const PNode &nb = pst[b];

    if (na.h1 == nb.h1 && na.h2 == nb.h2) return 0;

    if (L == R) {
        if (na.cnt == nb.cnt) return 0;
        return na.cnt > nb.cnt ? 1 : -1;
    }

    int64 mid = (L + R) >> 1;

    u64 ra1 = pst[na.rc].h1;
    u64 ra2 = pst[na.rc].h2;
    u64 rb1 = pst[nb.rc].h1;
    u64 rb2 = pst[nb.rc].h2;

    if (ra1 != rb1 || ra2 != rb2) {
        return comparePST(na.rc, nb.rc, mid + 1, R);
    } else {
        return comparePST(na.lc, nb.lc, L, mid);
    }
}

int compareRoots(int a, int b) {
    if (a == b) return 0;
    return comparePST(a, b, 1, MAXV);
}

void collectPST(int node, int64 L, int64 R, vector<int64>& out) {
    if (node == 0) return;

    if (L == R) {
        int c = pst[node].cnt;
        while (c-- > 0) out.push_back(L);
        return;
    }

    int64 mid = (L + R) >> 1;
    collectPST(pst[node].rc, mid + 1, R, out);
    collectPST(pst[node].lc, L, mid, out);
}

struct Key {
    int64 L, R;
    bool operator==(const Key& o) const {
        return L == o.L && R == o.R;
    }
};

struct KeyHash {
    size_t operator()(const Key& k) const {
        return splitmix64((u64)k.L) ^ (splitmix64((u64)k.R + 0x9e3779b97f4a7c15ULL) << 1);
    }
};

vector<Interval> blackCells[2];

unordered_map<Key, vector<int64>, KeyHash> memoSolve;

vector<Seg> getSegments(int64 L, int64 R) {
    vector<Seg> res;

    for (int col = 0; col < 2; ++col) {
        const auto& v = blackCells[col];

        int idx = int(lower_bound(v.begin(), v.end(), Interval{L, LLONG_MIN},
            [](const Interval& a, const Interval& b) {
                return a.l < b.l;
            }) - v.begin());

        int start = idx;
        if (idx > 0 && v[idx - 1].r >= L) start = idx - 1;

        for (int i = start; i < (int)v.size() && v[i].l <= R; ++i) {
            if (v[i].r < L) continue;

            int64 l = max(v[i].l, L);
            int64 r = min(v[i].r, R);
            if (l <= r) {
                res.push_back(Seg{col, l, r, r - l + 1});
            }
        }
    }

    return res;
}

const vector<int64>& solveRange(int64 L, int64 R) {
    if (L < 1) L = 1;
    if (R > MAXV) R = MAXV;

    static const vector<int64> EMPTY;

    if (L > R) return EMPTY;

    Key key{L, R};
    auto it = memoSolve.find(key);
    if (it != memoSolve.end()) return it->second;

    vector<Seg> segs = getSegments(L, R);

    if (segs.empty()) {
        auto ret = memoSolve.emplace(key, vector<int64>{});
        return ret.first->second;
    }

    int64 M = 0;
    for (const auto& s : segs) M = max(M, s.len);

    vector<Seg> mx;
    for (const auto& s : segs) {
        if (s.len == M) mx.push_back(s);
    }

    sort(mx.begin(), mx.end(), [](const Seg& a, const Seg& b) {
        if (a.l != b.l) return a.l < b.l;
        if (a.r != b.r) return a.r < b.r;
        return a.col < b.col;
    });

    int k = (int)mx.size();

    vector<int64> starts(k);
    for (int i = 0; i < k; ++i) starts[i] = mx[i].l;

    vector<int> bestCnt(k);
    vector<int> bestRoot(k, 0);

    for (int i = k - 1; i >= 0; --i) {
        int64 ri = mx[i].r;

        int p = int(upper_bound(starts.begin(), starts.end(), ri) - starts.begin());

        if (p == k) {
            bestCnt[i] = 1;

            const vector<int64>& suffix = solveRange(ri + 1, R);
            bestRoot[i] = addVectorToRoot(0, suffix);
        } else {
            int bestCountHere = -1;
            int bestRootHere = 0;

            {
                int count = 1 + bestCnt[p];

                const vector<int64>& gap = solveRange(ri + 1, starts[p] - 1);
                int root = bestRoot[p];
                root = addVectorToRoot(root, gap);

                bestCountHere = count;
                bestRootHere = root;
            }

            if (p + 1 < k && starts[p + 1] <= mx[p].r) {
                int count = 1 + bestCnt[p + 1];

                const vector<int64>& gap = solveRange(ri + 1, starts[p + 1] - 1);
                int root = bestRoot[p + 1];
                root = addVectorToRoot(root, gap);

                if (count > bestCountHere) {
                    bestCountHere = count;
                    bestRootHere = root;
                } else if (count == bestCountHere) {
                    if (compareRoots(root, bestRootHere) > 0) {
                        bestRootHere = root;
                    }
                }
            }

            bestCnt[i] = bestCountHere;
            bestRoot[i] = bestRootHere;
        }
    }

    int chosenFirst = 0;
    int chosenCount = -1;
    int chosenRoot = 0;

    for (int f = 0; f < k && f <= 1; ++f) {
        if (f == 1 && starts[1] > mx[0].r) {
            // For a maximum independent set, the first chosen interval
            // can be the second one only if it overlaps the first one.
            continue;
        }

        int count = bestCnt[f];

        const vector<int64>& prefix = solveRange(L, starts[f] - 1);
        int root = bestRoot[f];
        root = addVectorToRoot(root, prefix);

        if (count > chosenCount) {
            chosenCount = count;
            chosenRoot = root;
            chosenFirst = f;
        } else if (count == chosenCount) {
            if (compareRoots(root, chosenRoot) > 0) {
                chosenRoot = root;
                chosenFirst = f;
            }
        }
    }

    vector<int64> gaps;
    collectPST(chosenRoot, 1, MAXV, gaps);

    vector<int64> answer;
    answer.reserve(chosenCount + gaps.size());

    for (int i = 0; i < chosenCount; ++i) answer.push_back(M);
    for (int64 x : gaps) answer.push_back(x);

    sort(answer.begin(), answer.end(), greater<int64>());

    auto ret = memoSolve.emplace(key, move(answer));
    return ret.first->second;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int T;
    cin >> T;

    while (T--) {
        int n, m;
        cin >> n >> m;

        blackCells[0].clear();
        blackCells[1].clear();
        memoSolve.clear();
        pst.assign(1, PNode{});

        blackCells[0].reserve(n);
        blackCells[1].reserve(m);

        for (int i = 0; i < n; ++i) {
            int64 l, r;
            cin >> l >> r;
            blackCells[0].push_back({l, r});
        }

        for (int i = 0; i < m; ++i) {
            int64 l, r;
            cin >> l >> r;
            blackCells[1].push_back({l, r});
        }

        sort(blackCells[0].begin(), blackCells[0].end(),
             [](const Interval& a, const Interval& b) { return a.l < b.l; });
        sort(blackCells[1].begin(), blackCells[1].end(),
             [](const Interval& a, const Interval& b) { return a.l < b.l; });

        const vector<int64>& ans = solveRange(1, MAXV);

        cout << ans.size() << '\n';
        for (size_t i = 0; i < ans.size(); ++i) {
            if (i) cout << ' ';
            cout << ans[i];
        }
        cout << '\n';
    }

    return 0;
}
