#include <bits/stdc++.h>
using namespace std;
using i64 = long long;
using i128 = __int128_t;

struct Monster { i64 s, h, p; int pos; };
struct Event { int pos; i64 delta; };

// An immutable AVL rope of lower-hull edge vectors.
struct Rope {
    struct Node {
        i64 d = 0, e = 0, weight = 0;
        int left = 0, right = 0, count = 0, height = 0, last = 0;
    };
    // Stable integer indices, with no full-arena reallocation/copy peaks.
    struct NodeArena {
        static constexpr unsigned SHIFT = 15;
        static constexpr size_t BLOCK_SIZE = size_t(1) << SHIFT;
        static constexpr size_t MASK = BLOCK_SIZE - 1;
        vector<unique_ptr<Node[]>> blocks;
        size_t used = 0;
        size_t size() const { return used; }
        size_t allocatedBytes() const { return blocks.size() * BLOCK_SIZE * sizeof(Node); }
        Node& operator[](size_t i) { return blocks[i >> SHIFT][i & MASK]; }
        const Node& operator[](size_t i) const { return blocks[i >> SHIFT][i & MASK]; }
        void push_back(const Node& node) {
            if ((used & MASK) == 0) blocks.emplace_back(new Node[BLOCK_SIZE]);
            (*this)[used++] = node;
        }
    };
    struct Prefix { int count = 0; i64 d = 0, e = 0; };
    NodeArena t;
    Rope(int) { t.push_back({}); }
    int leaf(i64 d, i64 e) {
        int id = int(t.size());
        t.push_back({d, e, 0, 0, 0, 1, 1, id});
        return id;
    }
    int make(int a, int b) {
        if (!a) return b;
        if (!b) return a;
        Node z;
        z.d = t[a].d + t[b].d;
        z.e = t[a].e + t[b].e;
        z.left = a; z.right = b;
        z.count = t[a].count + t[b].count;
        z.height = max(t[a].height, t[b].height) + 1;
        z.last = t[b].last;
        t.push_back(z);
        return int(t.size()) - 1;
    }
    int balance(int a, int b) {
        if (t[a].height > t[b].height + 1) {
            int l = t[a].left, r = t[a].right;
            if (t[l].height >= t[r].height) {
                int nr = make(r, b);
                return make(l, nr);
            }
            int rl = t[r].left, rr = t[r].right;
            int nl = make(l, rl), nr = make(rr, b);
            return make(nl, nr);
        }
        if (t[b].height > t[a].height + 1) {
            int l = t[b].left, r = t[b].right;
            if (t[r].height >= t[l].height) {
                int nl = make(a, l);
                return make(nl, r);
            }
            int ll = t[l].left, lr = t[l].right;
            int nl = make(a, ll), nr = make(lr, r);
            return make(nl, nr);
        }
        return make(a, b);
    }
    int join(int a, int b) {
        if (!a) return b;
        if (!b) return a;
        if (t[a].height > t[b].height + 1) {
            int l = t[a].left, r = t[a].right;
            int nr = join(r, b);
            return balance(l, nr);
        }
        if (t[b].height > t[a].height + 1) {
            int l = t[b].left, r = t[b].right;
            int nl = join(a, l);
            return balance(nl, r);
        }
        return make(a, b);
    }
    pair<int, int> split(int v, int k) {
        if (!k) return {0, v};
        if (k == t[v].count) return {v, 0};
        int l = t[v].left, r = t[v].right;
        int c = t[l].count;
        if (k == c) return {l, r};
        if (k < c) {
            auto [a, b] = split(l, k);
            return {a, join(b, r)};
        }
        auto [a, b] = split(r, k - c);
        return {join(l, a), b};
    }
    // The predicate is true on a prefix of the edge sequence.
    template<class Predicate>
    Prefix takePrefix(int v, Predicate pred) const {
        Prefix ans;
        while (v) {
            if (!t[v].left) {
                if (pred(ans.d, ans.e, t[v].d, t[v].e)) {
                    ++ans.count; ans.d += t[v].d; ans.e += t[v].e;
                }
                break;
            }
            int l = t[v].left, last = t[l].last;
            if (pred(ans.d + t[l].d - t[last].d,
                     ans.e + t[l].e - t[last].e,
                     t[last].d, t[last].e)) {
                ans.count += t[l].count;
                ans.d += t[l].d; ans.e += t[l].e;
                v = t[v].right;
            } else v = l;
        }
        return ans;
    }
    Prefix support(int v, i64 num, i64 den) const {
        return takePrefix(v, [&](i64, i64, i64 d, i64 e) {
            return i128(e) * den <= i128(d) * num;
        });
    }
};

struct Hull {
    int root = 0;
    i64 firstA = 0, firstC = -1, mass = 0;
};

struct HullTree {
    int n;
    Rope rope;
    vector<Hull> tree;
    HullTree(int n) : n(n), rope(n), tree(4 * n) {}
    Hull merge(const Hull& a, const Hull& b) {
        if (a.firstC < 0) return b;
        if (b.firstC < 0) return a;
        i64 ba = a.mass + b.firstA;
        auto pa = rope.takePrefix(a.root, [&](i64 d0, i64 e0, i64 d, i64 e) {
            auto pb = rope.support(b.root, e, d);
            i64 aa = a.firstA + d0 + d, ac = a.firstC + e0 + e;
            i64 bb = ba + pb.d, bc = b.firstC + pb.e;
            return i128(aa - bb) * e >= i128(ac - bc) * d;
        });
        i64 aa = a.firstA + pa.d, ac = a.firstC + pa.e;
        auto pb = rope.takePrefix(b.root, [&](i64 d0, i64 e0, i64 d, i64 e) {
            i64 bb = ba + d0 + d, bc = b.firstC + e0 + e;
            return i128(aa - bb) * e >= i128(ac - bc) * d;
        });
        i64 bb = ba + pb.d, bc = b.firstC + pb.e;
        int left = rope.split(a.root, pa.count).first;
        int right = rope.split(b.root, pb.count).second;
        int bridge = rope.leaf(bb - aa, bc - ac);
        int root = rope.join(rope.join(left, bridge), right);
        return {root, a.firstA, a.firstC, a.mass + b.mass};
    }
    void update(int v, int l, int r, int pos, i64 h, i64 c) {
        if (l == r) { tree[v] = {0, h, c, h}; return; }
        int mid = (l + r) / 2;
        if (pos <= mid) update(v * 2, l, mid, pos, h, c);
        else update(v * 2 + 1, mid + 1, r, pos, h, c);
        tree[v] = merge(tree[v * 2], tree[v * 2 + 1]);
    }
    void update(int pos, i64 h, i64 c) { update(1, 0, n - 1, pos, h, c); }
};

int main() {
    ios::sync_with_stdio(false); cin.tie(nullptr);
    int n, L; i64 T;
    cin >> n >> L >> T;
    vector<Monster> a(n);
    for (auto& m : a) cin >> m.s >> m.h >> m.p;
    sort(a.begin(), a.end(), [](auto x, auto y) { return x.s > y.s; });
    for (int i = 0; i < n; ++i) a[i].pos = i + 1;
    sort(a.begin(), a.end(), [](auto x, auto y) { return x.p > y.p; });

    HullTree ht(n + 1);
    ht.update(0, 0, 0); // The permanent line zero.
    for (int i = 0; i < n; ++i) {
        ht.update(a[i].pos, a[i].h, T - a[i].s);
        i64 weight = a[i].p - (i + 1 < n ? a[i + 1].p : 0);
        ht.rope.t[ht.tree[1].root].weight += weight;
    }
    vector<Event> events;
    auto add = [&](i64 pos, i64 delta) {
        if (pos <= L && delta) events.push_back({int(pos), delta});
    };
    auto& nodes = ht.rope.t;
    for (int v = int(nodes.size()) - 1; v; --v) {
        auto z = nodes[v];
        if (!z.weight) continue;
        if (z.left) {
            nodes[z.left].weight += z.weight;
            nodes[z.right].weight += z.weight;
        } else {
            i64 q = z.e / z.d, r = z.e % z.d;
            add(q + 1, z.weight * (z.d - r));
            add(q + 2, z.weight * r);
        }
    }
#ifdef STATS
    cerr << "nodes=" << nodes.size()
         << " node_size=" << sizeof(Rope::Node)
         << " used_node_bytes=" << nodes.size() * sizeof(Rope::Node)
         << " allocated_node_bytes=" << nodes.allocatedBytes()
         << " events=" << events.size() << '\n';
#endif
    sort(events.begin(), events.end(), [](auto a, auto b) { return a.pos < b.pos; });
    size_t at = 0;
    int pos = 0, Q;
    i64 value = 0, slope = 0;
    cin >> Q;
    while (Q--) {
        i64 M; cin >> M;
        int ans = L;
        while (pos < L) {
            while (at < events.size() && events[at].pos == pos + 1)
                slope += events[at++].delta;
            int end = at < events.size() ? events[at].pos - 1 : L;
            i64 endValue = value + slope * (end - pos);
            if (endValue > M) {
                ans = pos + int((M - value) / slope);
                break;
            }
            pos = end; value = endValue;
        }
        cout << ans << '\n';
    }
    return 0;
}
