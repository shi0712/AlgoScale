#include <bits/stdc++.h>
using namespace std;
using ll = long long;

// 用 AVL 维护带权断点 (位置, 权值)。
// 允许不同断点具有相同位置。
struct Profile {
    struct Node {
        int l = 0, r = 0, h = 1, sz = 1;
        int key = 0, lo = 0, hi = 0;
        int add[2] = {0, 0}, keep[2] = {1, 1};
        ll val = 0, sum[2] = {0, 0};
    };
    vector<Node> t;
    int used = 0;

    explicit Profile(int n) : t(n + 5) {
        t[0].h = t[0].sz = 0;
    }
    int height(int x) const { return t[x].h; }
    ll mass(int x, int parity) const { return t[x].sum[parity]; }
    int make(int key, ll val) {
        int x = ++used;
        assert(x < (int)t.size());
        t[x] = Node();
        t[x].key = t[x].lo = t[x].hi = key;
        t[x].val = t[x].sum[key & 1] = val;
        return x;
    }
    void pull(int x) {
        auto &u = t[x];
        u.h = max(height(u.l), height(u.r)) + 1;
        u.sz = t[u.l].sz + t[u.r].sz + 1;
        for (int b = 0; b < 2; ++b)
            u.sum[b] = mass(u.l, b) + mass(u.r, b)
                     + ((u.key & 1) == b ? u.val : 0);
        u.lo = u.l ? t[u.l].lo : u.key;
        u.hi = u.r ? t[u.r].hi : u.key;
    }
    // 原位置奇偶性为 b 的点：位置加 d[b]，权值乘 k[b]。
    void apply(int x, int d0, int d1, int k0, int k1) {
        if (!x) return;
        auto &u = t[x];
        int d[2] = {d0, d1}, k[2] = {k0, k1};
        ll ns[2] = {0, 0};
        for (int b = 0; b < 2; ++b)
            ns[(b + d[b]) & 1] += u.sum[b] * k[b];
        int b = u.key & 1;
        u.val *= k[b];
        u.key += d[b];
        u.lo += d[u.lo & 1];
        u.hi += d[u.hi & 1];
        for (int c = 0; c < 2; ++c) {
            int now = (c + u.add[c]) & 1;
            u.add[c] += d[now];
            u.keep[c] *= k[now];
            u.sum[c] = ns[c];
        }
    }
    void push(int x) {
        auto &u = t[x];
        if (u.add[0] || u.add[1] || !u.keep[0] || !u.keep[1]) {
            apply(u.l, u.add[0], u.add[1], u.keep[0], u.keep[1]);
            apply(u.r, u.add[0], u.add[1], u.keep[0], u.keep[1]);
            u.add[0] = u.add[1] = 0;
            u.keep[0] = u.keep[1] = 1;
        }
    }
    void clearParity(int x, int b) {
        apply(x, 0, 0, b != 0, b != 1);
    }
    void roundTo(int x, int b) {
        // 将位置向上取整到奇偶性 b。
        apply(x, b == 1, b == 0, 1, 1);
    }
    int rotateLeft(int x) {
        push(x);
        int y = t[x].r;
        push(y);
        t[x].r = t[y].l;
        t[y].l = x;
        pull(x); pull(y);
        return y;
    }
    int rotateRight(int x) {
        push(x);
        int y = t[x].l;
        push(y);
        t[x].l = t[y].r;
        t[y].r = x;
        pull(x); pull(y);
        return y;
    }
    int balance(int x) {
        pull(x);
        if (height(t[x].l) > height(t[x].r) + 1) {
            push(t[x].l);
            int y = t[x].l;
            if (height(t[y].l) < height(t[y].r))
                t[x].l = rotateLeft(y);
            return rotateRight(x);
        }
        if (height(t[x].r) > height(t[x].l) + 1) {
            push(t[x].r);
            int y = t[x].r;
            if (height(t[y].r) < height(t[y].l))
                t[x].r = rotateRight(y);
            return rotateLeft(x);
        }
        return x;
    }
    // 按顺序连接 L、单个断点 x、R；x 的标记已经下传。
    int join(int L, int x, int R) {
        if (height(L) > height(R) + 1) {
            push(L);
            t[L].r = join(t[L].r, x, R);
            return balance(L);
        }
        if (height(R) > height(L) + 1) {
            push(R);
            t[R].l = join(L, x, t[R].l);
            return balance(R);
        }
        t[x].l = L; t[x].r = R;
        pull(x);
        return x;
    }
    int popMax(int x, int &point) {
        push(x);
        if (!t[x].r) {
            point = x;
            int L = t[x].l;
            t[x].l = 0;
            pull(x);
            return L;
        }
        t[x].r = popMax(t[x].r, point);
        return balance(x);
    }
    int concat(int L, int R) {
        if (!L || !R) return L | R;
        int x;
        L = popMax(L, x);
        return join(L, x, R);
    }
    pair<int,int> splitKey(int x, int key) {
        if (!x) return {0, 0};
        if (t[x].hi <= key) return {x, 0};
        if (t[x].lo > key) return {0, x};
        push(x);
        int L = t[x].l, R = t[x].r;
        if (t[x].key <= key) {
            auto [a, b] = splitKey(R, key);
            return {join(L, x, a), b};
        }
        auto [a, b] = splitKey(L, key);
        return {a, join(b, x, R)};
    }
    // 破坏式合并：合并两棵 AVL 所表示的断点集合。
    int unite(int a, int b) {
        if (!a || !b) return a | b;
        if (t[a].hi <= t[b].lo) return concat(a, b);
        if (t[b].hi <= t[a].lo) return concat(b, a);
        if (t[a].sz > t[b].sz) swap(a, b);
        push(a);
        int L = t[a].l, R = t[a].r;
        auto [x, y] = splitKey(b, t[a].key);
        return join(unite(L, x), a, unite(R, y));
    }
    // 从右向左切分：R 恰好包含 need 单位、奇偶性为 b 的权值。
    // 至多将一个断点拆成两个。
    pair<int,int> splitMass(int x, int b, ll need) {
        if (!x || need == 0) return {x, 0};
        if (need >= mass(x, b)) return {0, x};
        push(x);
        int L = t[x].l, R = t[x].r;
        ll rightMass = mass(R, b);
        ll own = ((t[x].key & 1) == b ? t[x].val : 0);
        if (need < rightMass) {
            auto [a, c] = splitMass(R, b, need);
            return {join(L, x, a), c};
        }
        if (need > rightMass + own) {
            auto [a, c] = splitMass(L, b, need - rightMass - own);
            return {a, join(c, x, R)};
        }
        if (need == rightMass) return {join(L, x, 0), R};
        if (need == rightMass + own) return {L, join(0, x, R)};
        ll take = need - rightMass;
        int y = make(t[x].key, take);
        t[x].val -= take;
        return {join(L, x, 0), join(0, y, R)};
    }
    // 将奇偶性 b 的最后若干单位权值移动到位置 +1。
    int transfer(int x, int b, ll need) {
        need = min(need, mass(x, b));
        if (!need) return x;
        auto [L, R] = splitMass(x, b, need);
        roundTo(R, b ^ 1);
        return concat(L, R);
    }
    // 删除奇偶性 b 的最后 need 单位权值；保留权值为零的节点。
    void trim(int x, int b, ll need) {
        if (!x || !need) return;
        if (need >= mass(x, b)) {
            clearParity(x, b);
            return;
        }
        push(x);
        ll z = mass(t[x].r, b);
        if (need <= z) {
            trim(t[x].r, b, need);
        } else {
            clearParity(t[x].r, b);
            need -= z;
            if ((t[x].key & 1) == b) {
                ll take = min(need, t[x].val);
                t[x].val -= take;
                need -= take;
            }
            if (need) trim(t[x].l, b, need);
        }
        pull(x);
    }
    // 找最大的 k，使奇偶性 b 对应的第 k 项边际收益大于 threshold。
    int lastPositive(int x, int b, ll threshold) {
        if (threshold < 0) return INT_MAX;
        if (mass(x, b) <= threshold) return 0;
        while (x) {
            push(x);
            ll z = mass(t[x].r, b);
            if (z > threshold) { x = t[x].r; continue; }
            threshold -= z;
            if ((t[x].key & 1) == b) {
                if (t[x].val > threshold) return (t[x].key + 1) / 2;
                threshold -= t[x].val;
            }
            x = t[x].l;
        }
        return 0;
    }
    int addVertex(int x, ll w, ll p) {
        ll A = p - w, B = p + w;
        if (lastPositive(x, 1, A) <= lastPositive(x, 0, B)) {
            x = transfer(x, 1, A);
            trim(x, 0, 2 * p);
        } else {
            ll take = min(B, mass(x, 0));
            x = transfer(x, 0, B);
            if (B > take) x = concat(make(1, B - take), x);
            trim(x, 1, 2 * p);
        }
        return x;
    }
};

struct Info {
    ll x = 0, y = 0, px = 0, py = 0, best = 0;
};
Info combine(const Info &a, const Info &b) {
    return {
        a.x + b.x, a.y + b.y,
        max(a.px, a.x + b.px),
        max(a.py, a.y + b.py),
        max({a.best, a.x + a.py + b.px, a.x + a.y + b.best})
    };
}
Info leaf(ll x, ll y) {
    return {x, y, max(0LL, x), max(0LL, y), max({0LL, x, x + y})};
}
struct SegTree {
    int n;
    vector<Info> tr;
    SegTree(const vector<ll> &x, const vector<ll> &y)
        : n((int)x.size()), tr(4 * n) { build(1, 0, n - 1, x, y); }
    void build(int u, int l, int r, const vector<ll> &x, const vector<ll> &y) {
        if (l == r) { tr[u] = leaf(x[l], y[l]); return; }
        int mid = (l + r) / 2;
        build(u * 2, l, mid, x, y);
        build(u * 2 + 1, mid + 1, r, x, y);
        tr[u] = combine(tr[u * 2], tr[u * 2 + 1]);
    }
    void add(int u, int l, int r, int pos, ll dx, ll dy) {
        if (l == r) { tr[u] = leaf(tr[u].x + dx, tr[u].y + dy); return; }
        int mid = (l + r) / 2;
        if (pos <= mid) add(u * 2, l, mid, pos, dx, dy);
        else add(u * 2 + 1, mid + 1, r, pos, dx, dy);
        tr[u] = combine(tr[u * 2], tr[u * 2 + 1]);
    }
    void add(int pos, ll dx, ll dy) { add(1, 0, n - 1, pos, dx, dy); }
    const Info &all() const { return tr[1]; }
};
struct Event { int id, parity; ll val; };

ll solve(int n, int s, const vector<ll> &w, const vector<ll> &p,
         const vector<int> &a) {
    vector<vector<int>> child(n);
    for (int i = 0; i < n; ++i) child[a[i]].push_back(i);
    vector<int> seen(n), path;
    int cur = s;
    while (!seen[cur]) {
        seen[cur] = 1;
        path.push_back(cur);
        cur = a[cur];
    }
    bool onCycle = (cur == s);
    vector<int> cycle;
    vector<char> inCycle(n, false);
    if (onCycle) {
        cycle = path;
        reverse(cycle.begin() + 1, cycle.end());
        for (int v : cycle) inCycle[v] = true;
    }
    vector<int> order = onCycle ? cycle : vector<int>{s};
    for (int j = 0; j < (int)order.size(); ++j)
        for (int u : child[order[j]])
            if (!inCycle[u]) order.push_back(u);

    vector<ll> f1(n), f2(n), t1(n), t2(n);
    for (int j = (int)order.size() - 1; j >= 0; --j) {
        int v = order[j];
        for (int u : child[v]) if (!inCycle[u]) {
            t1[v] += f1[u]; t2[v] += f2[u];
        }
        f1[v] = max(0LL, w[v] - p[v] + t1[v]);
        f2[v] = max(f1[v], -2 * p[v] + t2[v]);
    }
    if (!onCycle) return max(w[s] + t1[s], -p[s] + t2[s]);
    int m = (int)cycle.size();
    if (m == 1) return w[s] + t1[s]; // 题目不允许自环，保留此分支使实现完整。
    if (m == 2) {
        int v = cycle[1];
        return max({w[s] + t1[s], -p[s] + t2[s],
                    w[s] + w[v] - p[v] + t1[s] + t1[v]});
    }

    Profile prof(n);
    vector<int> root(n);
    for (int j = (int)order.size() - 1; j >= 0; --j) {
        int v = order[j], rt = 0;
        for (int u : child[v]) if (!inCycle[u])
            rt = prof.unite(rt, root[u]);
        if (!inCycle[v]) rt = prof.addVertex(rt, w[v], p[v]);
        root[v] = rt;
    }
    vector<vector<Event>> events(n + 3);
    vector<ll> sa(m), sb(m), firstA(m);
    int K = 0;
    auto collect = [&](auto &&self, int x, int id) -> void {
        if (!x) return;
        prof.push(x);
        self(self, prof.t[x].l, id);
        auto &u = prof.t[x];
        if (u.val) {
            int k = (u.key + 1) / 2, b = u.key & 1;
            events[k].push_back({id, b, u.val});
            K = max(K, k);
            if (b) { sa[id] += u.val; if (k == 1) firstA[id] += u.val; }
            else sb[id] += u.val;
        }
        self(self, prof.t[x].r, id);
    };
    for (int i = 0; i < m; ++i) collect(collect, root[cycle[i]], i);

    vector<ll> xe(m), ye(m), xo(m), yo(m);
    for (int i = 0; i < m; ++i) {
        int v = cycle[i];
        xe[i] = w[v] - p[v] + sa[i];
        ye[i] = -w[v] - p[v] + sb[i];
        xo[i] = ye[i];
        yo[i] = xe[i] - firstA[i];
    }
    const ll NEG = numeric_limits<ll>::lowest() / 4;
    ll nonempty = NEG, px = 0, py = 0, bestPy = 0;
    for (int i = 0; i < m; ++i) {
        px += xe[i]; py += ye[i]; bestPy = max(bestPy, py);
        nonempty = max(nonempty, px + bestPy);
    }
    SegTree even(xe, ye), odd(xo, yo);
    ll answer = NEG, base = 0;
    for (int k = 0; k <= K; ++k) {
        Info e = even.all(), o = odd.all();
        answer = max(answer, p[s] + base + (k ? e.best : nonempty));
        answer = max(answer, p[s] + base + e.x + o.best);
        if (k == K) break;
        base += e.x + e.y;
        for (const Event &ev : events[k + 1]) {
            if (ev.parity) even.add(ev.id, -ev.val, 0);
            else {
                even.add(ev.id, 0, -ev.val);
                odd.add(ev.id, -ev.val, 0);
            }
        }
        for (const Event &ev : events[k + 2])
            if (ev.parity) odd.add(ev.id, 0, -ev.val);
    }
    return answer;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    int n, s;
    if (!(cin >> n >> s)) return 0;
    --s;
    vector<ll> w(n), p(n);
    vector<int> a(n);
    for (ll &x : w) cin >> x;
    for (ll &x : p) cin >> x;
    for (int &x : a) { cin >> x; --x; }
    cout << solve(n, s, w, p, a) << '\n';
    return 0;
}
