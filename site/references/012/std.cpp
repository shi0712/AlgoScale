// 树海归航 / standard solution. Expected Treap-union algorithm.
// No answer tables. C++17; see admin/editorial.md for proof and arithmetic model.
#include <bits/stdc++.h>
using namespace std;
using Real = long double;

// 一段函数：y(x) = m*x + b；起点收益函数的导数为 p。
struct Line {
    Real m = 0, b = 0, p = 0;
};

// x' = a*x + b*y + c
// y' = d*x + e*y + f
// m_new = (d + e*m) / (a + b*m)
// p_new = (g + h*m + i*p) / (a + b*m)
struct Tag {
    Real a = 1, b = 0, c = 0;
    Real d = 0, e = 1, f = 0;
    Real g = 0, h = 0, i = 1;
};

struct Node {
    int l = 0, r = 0;
    uint64_t priority = 0;
    Real x = 0, y = 0, m = 0, p = 0;
    Tag tag;
    bool dirty = false;
};

struct Event {
    Real x, p;
};

class CurveTreap {
    vector<Node> tr;
    mt19937_64 &rng;

    static Tag compose(const Tag &u, const Tag &v) {
        // 返回复合变换 u(v(.))
        Tag w;
        w.a = u.a*v.a + u.b*v.d;
        w.b = u.a*v.b + u.b*v.e;
        w.c = u.a*v.c + u.b*v.f + u.c;
        w.d = u.d*v.a + u.e*v.d;
        w.e = u.d*v.b + u.e*v.e;
        w.f = u.d*v.c + u.e*v.f + u.f;
        w.g = u.g*v.a + u.h*v.d + u.i*v.g;
        w.h = u.g*v.b + u.h*v.e + u.i*v.h;
        w.i = u.i*v.i;
        return w;
    }

    void apply(int u, const Tag &t) {
        if (!u) return;
        Node &v = tr[u];
        Real x = v.x, y = v.y, m = v.m, p = v.p;
        Real den = t.a + t.b*m;
        v.x = t.a*x + t.b*y + t.c;
        v.y = t.d*x + t.e*y + t.f;
        v.m = (t.d + t.e*m) / den;
        v.p = (t.g + t.h*m + t.i*p) / den;
        v.tag = v.dirty ? compose(t, v.tag) : t;
        v.dirty = true;
    }

    void push(int u) {
        if (!u || !tr[u].dirty) return;
        Tag t = tr[u].tag;
        apply(tr[u].l, t);
        apply(tr[u].r, t);
        tr[u].tag = Tag{};
        tr[u].dirty = false;
    }

    Line rightLine(int u) const {
        const Node &v = tr[u];
        return {v.m, v.y - v.m*v.x, v.p};
    }

    // 整棵 Treap：y += s.m*x + s.b，p += s.p。
    void addLine(int u, const Line &s) {
        if (!u || (s.m == 0 && s.b == 0 && s.p == 0)) return;
        Node &v = tr[u];
        v.y += s.m*v.x + s.b;
        v.m += s.m;
        v.p += s.p;
        Tag &t = v.tag;
        t.d += s.m*t.a;
        t.e += s.m*t.b;
        t.f += s.m*t.c + s.b;
        t.g += s.p*t.a;
        t.h += s.p*t.b;
        v.dirty = true;
    }

    // 按横坐标 x 分裂，同时返回 x 右侧的线性段。
    // 恰好位于 x 的断点会与另一棵 Treap 的根合并。
    void split(int u, Real x, int &l, int &r, Line &mid) {
        if (!u) {
            l = r = 0;
            return;
        }
        push(u);
        if (tr[u].x == x) {
            l = tr[u].l;
            r = tr[u].r;
            mid = rightLine(u);
        } else if (tr[u].x < x) {
            mid = rightLine(u);
            int a, b;
            split(tr[u].r, x, a, b, mid);
            tr[u].r = a;
            l = u;
            r = b;
        } else {
            int a, b;
            split(tr[u].l, x, a, b, mid);
            tr[u].l = b;
            l = a;
            r = u;
        }
    }

    int join(int a, int b) {
        if (!a || !b) return a ? a : b;
        if (tr[a].priority > tr[b].priority) {
            push(a);
            tr[a].r = join(tr[a].r, b);
            return a;
        }
        push(b);
        tr[b].l = join(a, tr[b].l);
        return b;
    }

    void collect(int u, vector<Event> &out) {
        if (!u) return;
        push(u);
        collect(tr[u].l, out);
        out.push_back({tr[u].x, tr[u].p});
        collect(tr[u].r, out);
    }

public:
    CurveTreap(int n, mt19937_64 &generator) : rng(generator) {
        tr.reserve(n + 1);
        tr.emplace_back();
    }

    // 两个分段线性函数逐点相加。
    // la、lb 分别是各自第一个断点之前的线性段。
    int meld(int a, int b, Line la, Line lb) {
        if (!a) {
            addLine(b, la);
            return b;
        }
        if (!b) {
            addLine(a, lb);
            return a;
        }
        if (tr[a].priority < tr[b].priority) {
            swap(a, b);
            swap(la, lb);
        }
        push(a);
        Line ar = rightLine(a);
        Line br = lb;
        int bl, rr;
        split(b, tr[a].x, bl, rr, br);

        tr[a].l = meld(tr[a].l, bl, la, lb);
        tr[a].r = meld(tr[a].r, rr, ar, br);
        tr[a].y += br.m*tr[a].x + br.b;
        tr[a].m += br.m;
        tr[a].p += br.p;
        return a;
    }

    // 将孩子函数之和 S(t) 转成当前顶点的函数：
    // X = t + S(t) + degree，Y = S(t) + degree。
    int extend(int root, int degree, int children, bool isStart) {
        if (root) {
            Node &v = tr[root];
            Tag &t = v.tag;
            if (isStart) {
                v.p = 1;
                t.g = t.a;
                t.h = t.b;
                t.i = 0;
            }
            Real den = 1 + v.m;
            v.x += v.y + degree;
            v.y += degree;
            v.m /= den;
            v.p /= den;
            t.a += t.d;
            t.b += t.e;
            t.c += t.f + degree;
            t.f += degree;
            v.dirty = true;
        }
        Node v;
        v.priority = rng();
        v.x = v.y = degree;
        v.m = Real(children) / (children + 1);
        v.p = isStart ? Real(1) / (children + 1) : 0;
        tr.push_back(v);
        int u = int(tr.size()) - 1;
        return join(u, root);
    }

    vector<Event> events(int root) {
        vector<Event> out;
        collect(root, out);
        // 数学上横坐标和 p 都不下降；修正浮点舍入造成的微小偏差。
        Real lastX = 0, lastP = 0;
        for (auto &e : out) {
#ifdef LOCAL_CHECK
            assert(isfinite(e.x) && isfinite(e.p));
            assert(e.x >= lastX - 1e-10L * max(1.0L, lastX));
            assert(e.p >= lastP - 1e-10L && e.p <= 1 + 1e-10L);
#endif
            e.x = max(e.x, lastX);
            e.p = max(lastP, min(Real(1), max(Real(0), e.p)));
            lastX = e.x;
            lastP = e.p;
        }
        if (!out.empty()) out.back().p = 1;
        return out;
    }
};

struct HeapItem {
    Real x;
    int tree, pos;
    bool operator>(const HeapItem &o) const { return x > o.x; }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    int k;
    if (!(cin >> k)) return 0;
    // Only Treap priorities depend on this seed; no Monte Carlo estimation.
    uint64_t seed = chrono::steady_clock::now().time_since_epoch().count();
    if (const char *e = getenv("TREAP_SEED")) seed = strtoull(e, nullptr, 10);
    mt19937_64 rng(seed);
    vector<vector<Event>> all(k);
    for (int i = 0; i < k; ++i) {
        int n, s;
        cin >> n >> s;
        vector<vector<int>> adj(n + 1);
        for (int j = 1; j < n; ++j) {
            int u, v;
            cin >> u >> v;
            adj[u].push_back(v);
            adj[v].push_back(u);
        }
        CurveTreap curves(n, rng);
        // Iterative postorder: safe even when the input tree is a long path.
        vector<int> parent(n + 1), order{1}, rt(n + 1);
        for (size_t j = 0; j < order.size(); ++j) {
            int u = order[j];
            for (int v : adj[u]) if (v != parent[u]) {
                parent[v] = u;
                order.push_back(v);
            }
        }
        for (int j = n - 1; j >= 0; --j) {
            int u = order[j], root = 0, children = 0;
            for (int v : adj[u]) if (parent[v] == u) {
                root = curves.meld(root, rt[v],
                                   {Real(children), 0, 0}, {1, 0, 0});
                ++children;
            }
            rt[u] = (u == 1) ? root : curves.extend(
                root, int(adj[u].size()), children, u == s);
        }
        all[i] = curves.events(rt[1]);
    }

    priority_queue<HeapItem, vector<HeapItem>, greater<HeapItem>> heap;
    for (int i = 0; i < k; ++i) {
        heap.push({all[i][0].x, i, 0});
    }
    vector<Real> q(k, 1);
    Real product = 1, previous = 0, answer = 0;
    while (!heap.empty()) {
        auto [x, i, pos] = heap.top();
        heap.pop();
        answer += (x - previous)*product;
        previous = x;
        Real nextQ = 1 - all[i][pos].p;
        if (nextQ <= 0) break;
        product = product / q[i] * nextQ;
        q[i] = nextQ;
        if (product == 0) break;
        if (pos + 1 < int(all[i].size())) {
            heap.push({all[i][pos + 1].x, i, pos + 1});
        }
    }
#ifdef LOCAL_CHECK
    assert(isfinite(answer) && answer >= 1 - 1e-9L && answer <= 998001.01L);
#endif
    cout << fixed << setprecision(15) << answer << '\n';
    return 0;
}
