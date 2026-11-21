#include <bits/stdc++.h>
using namespace std;

constexpr int M = 10, K = M + 2, T = K * (K - 1) / 2;
int pw[M + 1];
using Order = array<unsigned char, K>;

struct Node {
    int test = -1, ch[2] = {-1, -1};
    int gain = 0, rep = 0;
};

struct Stats {
    int deepest[M + 1]{}, mass[M + 1]{};
    long long cost = 0;
};

int heightLB(int s) {
    return s <= 1 ? 0 : 32 - __builtin_clz(unsigned(s - 1));
}
int costLB(int s) {
    return s * (heightLB(s) + 1) - (1 << heightLB(s));
}

struct Tree {
    int m, nt = 0, root;
    int x[T], y[T];
    vector<Order> orders;
    vector<array<unsigned char, T>> bits;
    vector<Node> nodes;
    Stats st[2];

    void generate(int d, Order a) {
        if (d == m) {
            orders.push_back(a);
            Order rank{};
            for (int i = 0; i < m + 2; ++i) rank[a[i]] = i;
            array<unsigned char, T> b{};
            for (int q = 0; q < nt; ++q) b[q] = rank[x[q]] > rank[y[q]];
            bits.push_back(b);
            return;
        }
        for (int j = d; j <= d + 2; ++j) {
            Order b = a;
            for (int i = d + 2; i > j; --i) b[i] = b[i - 1];
            b[j] = d + 2;
            generate(d + 1, b);
        }
    }

    int makeNode(int s, int lo, int hi) {
        int id = nodes.size();
        nodes.emplace_back();
        nodes[id].rep = lo;
        // A complete aligned ternary subtree means that exactly a prefix
        // of the reverse insertions has been determined.
        if (hi - lo + 1 == s && lo % s == 0)
            for (int t = 0; t < m; ++t)
                if (pw[t] == s) nodes[id].gain = m - t;
        return id;
    }

    struct Result { int h, c, q; };
    struct Small {
        Tree &tr;
        const vector<int> &v;
        unsigned mask[T]{};
        unordered_map<unsigned, Result> memo;
        int extras = 120;
        Small(Tree &t, const vector<int> &vv) : tr(t), v(vv) {
            for (int i = 0; i < (int)v.size(); ++i)
                for (int q = 0; q < tr.nt; ++q)
                    mask[q] |= unsigned(tr.bits[v[i]][q]) << i;
        }
        Result solve(unsigned s) {
            if (!(s & (s - 1))) return {0, 0, -1};
            auto it = memo.find(s);
            if (it != memo.end()) return it->second;
            int n = __builtin_popcount(s);
            // Identical bipartitions require only one trial.
            array<tuple<int, unsigned, int>, T> opt;
            int cnt = 0;
            for (int q = 0; q < tr.nt; ++q) {
                unsigned b = mask[q] & s;
                if (!b || b == s) continue;
                if (b & (s & -s)) b ^= s;
                opt[cnt++] = {abs(2 * __builtin_popcount(b) - n), b, q};
            }
            sort(opt.begin(), opt.begin() + cnt);
            Result best{1000, 1000000, -1};
            unsigned prev = 0;
            for (int i = 0; i < cnt; ++i) {
                auto [unused, b, q] = opt[i];
                if (prev == b) continue;
                prev = b;
                int u = __builtin_popcount(b), w = n - u;
                int h = 1 + heightLB(max(u, w)), c = n + costLB(u) + costLB(w);
                if (make_pair(h, c) >= make_pair(best.h, best.c)) continue;
                if (best.q >= 0) {
                    if (!extras) break;
                    --extras;
                }
                Result l = solve(b), r = solve(s ^ b);
                h = 1 + max(l.h, r.h);
                c = n + l.c + r.c;
                if (make_pair(h, c) < make_pair(best.h, best.c))
                    best = {h, c, q};
                if (best.h == heightLB(n) && best.c == costLB(n)) break;
            }
            memo.emplace(s, best);
            return best;
        }
        int emit(unsigned s) {
            int id = tr.makeNode(__builtin_popcount(s), v[__builtin_ctz(s)],
                                 v[31 - __builtin_clz(s)]);
            if (s & (s - 1)) {
                int q = memo.at(s).q;
                tr.nodes[id].test = q;
                unsigned b = mask[q] & s;
                int l = emit(s ^ b), r = emit(b);
                tr.nodes[id].ch[0] = l;
                tr.nodes[id].ch[1] = r;
            }
            return id;
        }
    };

    int build(const vector<int> &v) {
        int n = v.size();
        if (n <= 24) {
            Small solver(*this, v);
            unsigned s = (1u << n) - 1;
            solver.solve(s);
            return solver.emit(s);
        }
        int id = makeNode(n, v.front(), v.back());
        array<int, T> sum{};
        for (int z : v)
            for (int q = 0; q < nt; ++q) sum[q] += bits[z][q];
        vector<pair<int,int>> opt;
        for (int q = 0; q < nt; ++q)
            if (sum[q] && sum[q] < n)
                opt.emplace_back(abs(2 * sum[q] - n), q);
        sort(opt.begin(), opt.end());
        tuple<int,int,long long> best{1000, INT_MAX, LLONG_MAX};
        int chosen = -1;
        for (int i = 0; i < min(4, (int)opt.size()); ++i) {
            int q = opt[i].second;
            array<int, T> both{};
            for (int z : v) if (bits[z][q])
                for (int j = 0; j < nt; ++j) both[j] += bits[z][j];
            int h = 0, c = n;
            long long sq = 0;
            for (int b = 0; b <= 1; ++b) {
                int s = b ? sum[q] : n - sum[q];
                if (s == 1) continue;
                int d = s;
                for (int j = 0; j < nt; ++j)
                    d = min(d, abs(s - 2 * (b ? both[j] : sum[j] - both[j])));
                int u = (s + d) / 2, w = s - u;
                h = max(h, 2 + heightLB(u));
                c += s + costLB(u) + costLB(w);
                sq += 1LL * u * u + 1LL * w * w;
            }
            auto quality = make_tuple(h, c, sq);
            if (quality < best) best = quality, chosen = q;
        }
        vector<int> child[2];
        for (int z : v) child[bits[z][chosen]].push_back(z);
        nodes[id].test = chosen;
        for (int b = 0; b < 2; ++b) {
            int j = build(child[b]);
            nodes[id].ch[b] = j;
        }
        return id;
    }

    void collect(int id, int d, int mode) {
        const Node &v = nodes[id];
        if (v.test < 0 || (mode && v.gain)) {
            int g = v.gain, count = pw[m - g];
            auto &s = st[mode];
            s.deepest[g] = max(s.deepest[g], d);
            s.mass[g] += count;
            s.cost += 1LL * count * d;
        } else {
            collect(v.ch[0], d + 1, mode);
            collect(v.ch[1], d + 1, mode);
        }
    }
    void init(int len) {
        m = len;
        for (int i = 0; i < m + 2; ++i)
            for (int j = i + 1; j < m + 2; ++j)
                if (i || j != 1) x[nt] = i, y[nt++] = j;
        orders.reserve(pw[m]);
        bits.reserve(pw[m]);
        nodes.reserve(2 * pw[m]);
        Order a{};
        a[1] = 1;
        generate(0, a);
        vector<int> v(pw[m]);
        iota(v.begin(), v.end(), 0);
        root = build(v);
        decltype(bits)().swap(bits);
        collect(root, 0, 0);
        collect(root, 0, 1);
    }
};

bool lessAt(int i, int j) {
    cout << "? " << i << ' ' << j << '\n' << flush;
    string s;
    if (!(cin >> s) || (s != "<" && s != ">")) exit(0);
    return s == "<";
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    int n;
    if (!(cin >> n) || n < 1) return 0;
    if (n == 1) {
        cout << "! 1\n" << flush;
        return 0;
    }
    int hi = n, lo = n - 1;
    if (lessAt(hi, lo)) swap(hi, lo);

    int limit = min(M, n - 2);
    pw[0] = 1;
    vector<Tree> trees(limit + 1);
    for (int m = 1; m <= limit; ++m) {
        pw[m] = pw[m - 1] * 3;
        trees[m].init(m);
    }

    vector<int> worst(n), block(n), mode(n);
    vector<double> mean(n);
    for (int r = 1; r <= n - 2; ++r) {
        worst[r] = INT_MAX;
        for (int m = 1; m <= min(limit, r); ++m)
            for (int b = 0; b <= 1; ++b) {
                const Stats &s = trees[m].st[b];
                int w = 0;
                double e = s.cost;
                for (int g = 1; g <= m; ++g) if (s.mass[g]) {
                    w = max(w, s.deepest[g] + worst[r - g]);
                    e += s.mass[g] * mean[r - g];
                }
                e /= pw[m];
                if (w < worst[r] || (w == worst[r] && e < mean[r])) {
                    worst[r] = w;
                    mean[r] = e;
                    block[r] = m;
                    mode[r] = b;
                }
            }
    }

    vector<int> ans(n + 1);
    for (int r = n - 2; r > 0; ) {
        const Tree &t = trees[block[r]];
        array<int, K> pos{};
        pos[0] = hi;
        pos[1] = lo;
        for (int i = 0; i < t.m; ++i) pos[2 + i] = r - i;
        int id = t.root;
        while (t.nodes[id].test >= 0 && !(mode[r] && t.nodes[id].gain)) {
            int q = t.nodes[id].test;
            int b = lessAt(pos[t.x[q]], pos[t.y[q]]);
            id = t.nodes[id].ch[b];
        }
        const Node &v = t.nodes[id];
        int g = v.gain, k = 0;
        for (int i = 0; i < t.m + 2; ++i) {
            int j = t.orders[v.rep][i];
            if (j >= g + 2) continue;
            if (k < g) ans[pos[j]] = r + 2 - k;
            else if (k == g) hi = pos[j];
            else lo = pos[j];
            ++k;
        }
        r -= g;
    }
    ans[hi] = 2;
    ans[lo] = 1;
    cout << '!';
    for (int i = 1; i <= n; ++i) cout << ' ' << ans[i];
    cout << '\n' << flush;
}
