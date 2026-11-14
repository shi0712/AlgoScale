#include <bits/stdc++.h>
using namespace std;
using int64 = long long;

class FastInput {
    static constexpr int SZ = 1 << 16;
    char buf[SZ];
    int pos = 0, len = 0;
    char get() {
        if (pos == len) {
            len = (int)fread(buf, 1, SZ, stdin);
            pos = 0;
            if (!len) return 0;
        }
        return buf[pos++];
    }
public:
    template<class T> void read(T &x) {
        char c;
        do c = get(); while (c <= ' ');
        bool neg = c == '-';
        if (neg) c = get();
        x = 0;
        while (c >= '0' && c <= '9') {
            x = x * 10 + c - '0';
            c = get();
        }
        if (neg) x = -x;
    }
};

/*
For a tree initially colored zero, let G(k) be its optimum if the root
can change color at most k times. Write its marginal gains as
    A[j] = G(2j+1)-G(2j), B[j] = G(2j+2)-G(2j+1).
Both A and B are nonnegative, nonincreasing, and eventually zero.
A treap stores their drops: an event (j,z) contributes z to entries 0..j.
Trees are combined by adding events. Taking the largest-position events
of total mass x implements max(sequence-x,0), with the taken events
representing the negative part. This permits the DP transition without
materializing its possibly linear-length array.
*/
class Slopes {
    struct Node {
        int l = 0, r = 0, key = 0, lazy = 0, mx = 0;
        uint32_t pri = 0;
        int64 val = 0, sum = 0, area = 0;
    };
    vector<Node> tr;
    uint32_t rng = 712367821;
    uint32_t randomPriority() {
        rng ^= rng << 13;
        rng ^= rng >> 17;
        rng ^= rng << 5;
        return rng;
    }
    void pull(int x) {
        Node &v = tr[x];
        v.sum = tr[v.l].sum + v.val + tr[v.r].sum;
        v.area = tr[v.l].area + v.val * (v.key + 1LL) + tr[v.r].area;
        v.mx = v.r ? tr[v.r].mx : v.key;
    }
    void shift(int x, int delta) {
        if (!x) return;
        tr[x].key += delta;
        tr[x].mx += delta;
        tr[x].lazy += delta;
        tr[x].area += tr[x].sum * delta;
    }
    void push(int x) {
        int d = tr[x].lazy;
        if (!d) return;
        shift(tr[x].l, d);
        shift(tr[x].r, d);
        tr[x].lazy = 0;
    }
    int make(int key, int64 val) {
        Node v;
        v.key = v.mx = key;
        v.val = v.sum = val;
        v.area = val * (key + 1LL);
        v.pri = randomPriority();
        tr.push_back(v);
        return (int)tr.size() - 1;
    }
    int concatenate(int a, int b) {
        if (!a || !b) return a | b;
        if (tr[a].pri > tr[b].pri) {
            push(a);
            tr[a].r = concatenate(tr[a].r, b);
            pull(a);
            return a;
        }
        push(b);
        tr[b].l = concatenate(a, tr[b].l);
        pull(b);
        return b;
    }
    // Split by key, isolating an event whose key equals k.
    void splitKey(int x, int k, int &a, int &eq, int &b) {
        if (!x) { a = eq = b = 0; return; }
        push(x);
        if (tr[x].key < k) {
            int z;
            splitKey(tr[x].r, k, z, eq, b);
            tr[x].r = z;
            pull(x);
            a = x;
        } else if (tr[x].key > k) {
            int z;
            splitKey(tr[x].l, k, a, eq, z);
            tr[x].l = z;
            pull(x);
            b = x;
        } else {
            a = tr[x].l;
            b = tr[x].r;
            eq = x;
            tr[x].l = tr[x].r = 0;
            pull(x);
        }
    }
    // Keep exactly k mass at the smallest keys; an event may be split.
    void splitMass(int x, int64 k, int &a, int &b) {
        if (!x) { a = b = 0; return; }
        if (k <= 0) { a = 0; b = x; return; }
        if (k >= tr[x].sum) { a = x; b = 0; return; }
        int at = x;
        int64 remaining = k;
        while (true) {
            push(at);
            int64 left = tr[tr[at].l].sum;
            if (remaining < left) at = tr[at].l;
            else if (remaining > left + tr[at].val) {
                remaining -= left + tr[at].val;
                at = tr[at].r;
            } else {
                remaining -= left;
                break;
            }
        }
        int key = tr[at].key;
        int l, eq, r;
        splitKey(x, key, l, eq, r);
        int64 rest = tr[eq].val - remaining;
        if (!remaining) {
            a = l;
            b = concatenate(eq, r);
        } else if (!rest) {
            a = concatenate(l, eq);
            b = r;
        } else {
            tr[eq].val = remaining;
            pull(eq);
            a = concatenate(l, eq);
            int extra = make(key, rest);
            b = concatenate(extra, r);
        }
    }
    int discard(int x, int64 k) {
        if (!x || k >= tr[x].sum) return 0;
        if (k <= 0) return x;
        push(x);
        int64 right = tr[tr[x].r].sum;
        if (k < right) {
            tr[x].r = discard(tr[x].r, k);
        } else if (k < right + tr[x].val) {
            tr[x].r = 0;
            tr[x].val -= k - right;
        } else {
            return discard(tr[x].l, k - right - tr[x].val);
        }
        pull(x);
        return x;
    }
    // Extract up to k mass, returning the unmet amount in missing.
    int take(int &root, int64 k, int64 &missing) {
        int64 total = tr[root].sum;
        if (total <= k) {
            int result = root;
            root = 0;
            missing = k - total;
            return result;
        }
        missing = 0;
        int a, b;
        splitMass(root, total - k, a, b);
        root = a;
        return b;
    }
public:
    explicit Slopes(int n) {
        tr.reserve(3 * n + 10);
        tr.emplace_back();
    }
    int unite(int a, int b) {
        if (!a || !b) return a | b;
        if (tr[a].pri < tr[b].pri) swap(a, b);
        push(a);
        int l, eq, r;
        splitKey(b, tr[a].key, l, eq, r);
        if (eq) tr[a].val += tr[eq].val;
        tr[a].l = unite(tr[a].l, l);
        tr[a].r = unite(tr[a].r, r);
        pull(a);
        return a;
    }
    void transition(int &a, int &b, int64 w, int64 p) {
        int64 missing;
        if (w >= p) {
            int moved = take(b, p + w, missing);
            shift(moved, 1);
            a = unite(a, moved);
            if (missing) a = unite(a, make(0, missing));
            a = discard(a, 2 * p);
        } else if (w <= -p) {
            int moved = take(a, p - w, missing);
            b = unite(b, moved);
            b = discard(b, 2 * p);
        } else {
            int64 missingA, missingB;
            int movedA = take(a, p - w, missingA);
            int movedB = take(b, p + w, missingB);
            shift(movedB, 1);
            a = unite(a, movedB);
            if (missingB) a = unite(a, make(0, missingB));
            b = unite(b, movedA);
            a = discard(a, p + w);
            b = discard(b, p - w);
        }
    }
    // Returns the sums of the first k and first k-1 sequence entries.
    pair<int64,int64> adjacentAreas(int x, int k) {
        if (!x || k <= 0) return {0, 0};
        const int cut = k - 1;
        if (tr[x].mx < cut) return {tr[x].area, tr[x].area};
        int64 lowArea = 0, highMass = 0;
        while (x) {
            if (tr[x].mx < cut) {
                lowArea += tr[x].area;
                break;
            }
            push(x);
            if (tr[x].key < cut) {
                lowArea += tr[tr[x].l].area + tr[x].val * (tr[x].key + 1LL);
                x = tr[x].r;
            } else {
                highMass += tr[tr[x].r].sum + tr[x].val;
                x = tr[x].l;
            }
        }
        int64 small = lowArea + highMass * cut;
        return {small + highMass, small};
    }
};

int main() {
    FastInput in;
    int n, s;
    in.read(n); in.read(s); --s;
    vector<int64> w(n), p(n);
    vector<int> a(n), head(n, -1), nxt(n);
    for (auto &v : w) in.read(v);
    for (auto &v : p) in.read(v);
    for (int i = 0; i < n; ++i) {
        in.read(a[i]); --a[i];
        nxt[i] = head[a[i]];
        head[a[i]] = i;
    }

    vector<char> seen(n, false);
    vector<int> path;
    int v = s;
    while (!seen[v]) {
        seen[v] = true;
        path.push_back(v);
        v = a[v];
    }
    bool onCycle = v == s;
    vector<int> cycle;
    vector<char> cyclic(n, false);
    if (onCycle) {
        cycle.push_back(s);
        for (int j = (int)path.size() - 1; j >= 1; --j)
            cycle.push_back(path[j]);
        for (int x : cycle) cyclic[x] = true;
    }

    vector<int> order = onCycle ? cycle : vector<int>{s};
    order.reserve(n);
    for (int j = 0; j < (int)order.size(); ++j) {
        int x = order[j];
        for (int y = head[x]; y != -1; y = nxt[y])
            if (!cyclic[y]) order.push_back(y);
    }

    if (!onCycle || cycle.size() == 2) {
        vector<int64> g1(n, 0), g2(n, 0);
        int roots = onCycle ? 2 : 1;
        for (int j = (int)order.size() - 1; j >= roots; --j) {
            int x = order[j];
            int64 best1 = max(0LL, w[x] - p[x] + g1[x]);
            int64 best2 = max(best1, g2[x] - 2 * p[x]);
            g1[a[x]] += best1;
            g2[a[x]] += best2;
        }
        int64 ans = max(w[s] + g1[s], -p[s] + g2[s]);
        if (onCycle) {
            int y = cycle[1];
            ans = max(ans, w[s] + g1[s] + w[y] - p[y] + g1[y]);
        }
        printf("%lld\n", ans);
        return 0;
    }

    Slopes slopes(n);
    vector<int> odd(n, 0), even(n, 0);
    const int len = (int)cycle.size();
    for (int j = (int)order.size() - 1; j >= len; --j) {
        int x = order[j];
        slopes.transition(odd[x], even[x], w[x], p[x]);
        odd[a[x]] = slopes.unite(odd[a[x]], odd[x]);
        even[a[x]] = slopes.unite(even[a[x]], even[x]);
    }

    /*
Use label t = number of flips + initial color. Its parity is the final
color, and labels cannot increase along propagation edges, except the
edge entering s, which allows an increase of two. Thus, with s labeled
t, the cycle labels are t, then t-1, then t-2 (either block may be empty).
A two-state color boundary can circulate on cycles of length at least 3;
these label constraints are sufficient there.
    */
    const int64 NEG = -4000000000000000000LL;
    // Hanging-tree DPs have saturated by their number of vertices.
    const int maxT = (int)order.size() - len + 3;
    vector<int64> cache(maxT + 3, 0);
    vector<char> calculated(maxT + 3, false);
    auto evaluate = [&](int t) -> int64 {
        if (calculated[t]) return cache[t];
        int64 d0 = 0, d1 = NEG, d2 = NEG;
        for (int i = 0; i < len; ++i) {
            int x = cycle[i], m = t / 2;
            auto oa = slopes.adjacentAreas(odd[x], (t + 1) / 2);
            auto ea = slopes.adjacentAreas(even[x], m);
            int64 g0 = oa.first + ea.first;
            int64 g1, g2;
            if (t & 1) {
                g1 = oa.second + ea.first;
                g2 = oa.second + ea.second;
            } else {
                g1 = oa.first + ea.second;
                g2 = oa.second + ea.second;
            }
            int64 h0 = g0 + ((t & 1) ? w[x] : 0) - p[x] * t;
            if (!i) { d0 = h0; continue; }
            int64 h1 = g1 + ((t & 1) ? 0 : w[x]) - p[x] * (t - 1);
            int64 h2 = g2 + ((t & 1) ? w[x] : 0) - p[x] * (t - 2);
            int64 n2 = t >= 2 ? max({d0, d1, d2}) + h2 : NEG;
            int64 n1 = max(d0, d1) + h1;
            d0 += h0;
            d1 = n1;
            d2 = n2;
        }
        calculated[t] = true;
        return cache[t] = max({d0, d1, d2}) + p[s];
    };

    // For each parity, the optimum with fixed root label is concave.
    // Balancing two feasible labelings by min(x+2,y), max(x,y-2)
    // preserves feasibility and the sum of both scores.
    int64 answer = NEG;
    for (int first = 1; first <= 2; ++first) {
        int lo = 0, hi = (maxT - first) / 2;
        while (lo < hi) {
            int mid = (lo + hi) / 2;
            int t = first + 2 * mid;
            if (evaluate(t + 2) > evaluate(t)) lo = mid + 1;
            else hi = mid;
        }
        answer = max(answer, evaluate(first + 2 * lo));
    }
    printf("%lld\n", answer);
    return 0;
}
