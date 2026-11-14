#include <bits/stdc++.h>
using namespace std;

using int64 = long long;
static constexpr int64 NEG = -(1LL << 60);

/*
For a tree vertex initially colored 0, let f(k) be the best contribution
of its subtree when its parent provides k alternating color changes,
starting with 0 -> 1.

The differences of f form two nonnegative, nonincreasing sequences:
    O[j] = f(2j-1) - f(2j-2)
    E[j] = f(2j)   - f(2j-1).

A nonincreasing sequence is stored by its drops:
a drop of weight d at position x contributes d to positions 1..x.
*/
class DropTreap {
    struct Node {
        int left = 0, right = 0;
        int key = 0, lazy = 0;
        uint32_t priority = 0;
        int64 value = 0, sum = 0;
    };

    vector<Node> tr;
    uint32_t randomState = 712367821;

    uint32_t randomPriority() {
        randomState ^= randomState << 13;
        randomState ^= randomState >> 17;
        randomState ^= randomState << 5;
        return randomState;
    }

    int newNode(int key, int64 value) {
        Node x;
        x.key = key;
        x.value = x.sum = value;
        x.priority = randomPriority();
        tr.push_back(x);
        return int(tr.size()) - 1;
    }

    void pull(int x) {
        tr[x].sum = sum(tr[x].left) + tr[x].value + sum(tr[x].right);
    }

    void push(int x) {
        if (!x || !tr[x].lazy) return;
        int d = tr[x].lazy;
        shift(tr[x].left, d);
        shift(tr[x].right, d);
        tr[x].lazy = 0;
    }

    int mergeOrdered(int x, int y) {
        if (!x || !y) return x ? x : y;
        if (tr[x].priority > tr[y].priority) {
            push(x);
            tr[x].right = mergeOrdered(tr[x].right, y);
            pull(x);
            return x;
        } else {
            push(y);
            tr[y].left = mergeOrdered(x, tr[y].left);
            pull(y);
            return y;
        }
    }

public:
    explicit DropTreap(int n) {
        tr.reserve(2 * n + 10);
        tr.push_back(Node{});
    }

    int64 sum(int x) const {
        return x ? tr[x].sum : 0;
    }

    void shift(int x, int d) {
        if (!x) return;
        tr[x].key += d;
        tr[x].lazy += d;
    }

    // Split into keys < key and keys >= key.
    void split(int x, int key, int &left, int &right) {
        if (!x) {
            left = right = 0;
            return;
        }
        push(x);
        if (tr[x].key < key) {
            left = x;
            split(tr[x].right, key, tr[x].right, right);
            pull(left);
        } else {
            right = x;
            split(tr[x].left, key, left, tr[x].left);
            pull(right);
        }
    }

    // Pointwise addition of the represented sequences.
    int unite(int x, int y) {
        if (!x || !y) return x ? x : y;
        if (tr[x].priority < tr[y].priority) swap(x, y);
        push(x);
        push(y);

        int yl, ym, yr, tmp;
        split(y, tr[x].key, yl, tmp);
        split(tmp, tr[x].key + 1, ym, yr);

        tr[x].value += sum(ym);
        tr[x].left = unite(tr[x].left, yl);
        tr[x].right = unite(tr[x].right, yr);
        pull(x);
        return x;
    }

    int addAt(int root, int key, int64 delta) {
        if (!delta) return root;

        int left, middle, right, tmp;
        split(root, key, left, tmp);
        split(tmp, key + 1, middle, right);

        int64 value = sum(middle) + delta;
        if (value == 0) {
            middle = 0;
        } else if (middle) {
            tr[middle].value = value;
            tr[middle].sum = value;
        } else {
            middle = newNode(key, value);
        }

        return mergeOrdered(mergeOrdered(left, middle), right);
    }

    // Replace sequence h by max(h - amount, 0).
    int trimRight(int root, int64 amount) {
        if (!root || amount == 0) return root;
        if (amount >= sum(root)) return 0;

        push(root);
        int64 rightSum = sum(tr[root].right);

        if (amount < rightSum) {
            tr[root].right = trimRight(tr[root].right, amount);
            pull(root);
            return root;
        }

        amount -= rightSum;
        tr[root].right = 0;

        if (amount < tr[root].value) {
            tr[root].value -= amount;
            pull(root);
            return root;
        }

        amount -= tr[root].value;
        return trimRight(tr[root].left, amount);
    }

    // First j for which h(j) + constant < 0.
    int firstNegative(int root, int64 constant) {
        if (constant >= 0) return INT_MAX / 4;

        int64 target = -constant;
        if (sum(root) < target) return 1;

        int x = root;
        int64 accumulated = 0;
        while (x) {
            push(x);
            int64 rightSum = sum(tr[x].right);
            if (accumulated + rightSum >= target) {
                x = tr[x].right;
            } else if (accumulated + rightSum + tr[x].value >= target) {
                return tr[x].key + 1;
            } else {
                accumulated += rightSum + tr[x].value;
                x = tr[x].left;
            }
        }
        return 1;
    }

    int64 at(int root, int position) {
        int64 result = 0;
        while (root) {
            push(root);
            if (tr[root].key >= position) {
                result += tr[root].value + sum(tr[root].right);
                root = tr[root].left;
            } else {
                root = tr[root].right;
            }
        }
        return result;
    }

    template<class F>
    void enumerate(int root, F &callback) {
        if (!root) return;
        push(root);
        enumerate(tr[root].left, callback);
        callback(tr[root].key, tr[root].value);
        enumerate(tr[root].right, callback);
    }

    /*
    Given H = sum of children's f, compute

      f(k) = max_{0 <= t <= k}
             (H(t) - p*t + (t odd ? w : 0)).
    */
    void transform(int &odd, int &even, int64 w, int64 p) {
        int64 A = w - p;
        int64 B = -w - p;

        int jo = firstNegative(odd, A);
        int je = firstNegative(even, B);

        if (jo == INT_MAX / 4 && je == INT_MAX / 4) return;

        if (2LL * jo - 1 < 2LL * je) {
            // First negative difference is odd, at position jo.
            int lo, ro, le, re;
            split(odd, jo, lo, ro);
            split(even, jo, le, re);

            int64 so = sum(ro), se = sum(re);
            int tail = unite(ro, re);
            tail = trimRight(tail, 2 * p);
            int64 st = sum(tail);

            if (jo > 1) {
                lo = addAt(lo, jo - 1, so + A);
                le = addAt(le, jo - 1, se + B - st);
            }

            odd = lo;
            even = unite(le, tail);
        } else {
            // First negative difference is even, at position je.
            int lo, ro, le, re;
            split(odd, je + 1, lo, ro);
            split(even, je, le, re);

            int64 so = sum(ro), se = sum(re);
            shift(re, 1);
            int tail = unite(ro, re);
            tail = trimRight(tail, 2 * p);
            int64 st = sum(tail);

            if (je > 1) le = addAt(le, je - 1, se + B);
            lo = addAt(lo, je, so + A - st);

            odd = unite(lo, tail);
            even = le;
        }
    }
};

// Upper triangular 3x3 max-plus matrix:
// entries 00, 01, 02, 11, 12, 22.
struct Matrix {
    int64 x[6];
};

static inline Matrix multiply(const Matrix &a, const Matrix &b) {
    Matrix c;
    c.x[0] = a.x[0] + b.x[0];
    c.x[1] = max(a.x[0] + b.x[1], a.x[1] + b.x[3]);
    c.x[2] = max({a.x[0] + b.x[2],
                  a.x[1] + b.x[4],
                  a.x[2] + b.x[5]});
    c.x[3] = a.x[3] + b.x[3];
    c.x[4] = max(a.x[3] + b.x[4], a.x[4] + b.x[5]);
    c.x[5] = a.x[5] + b.x[5];
    return c;
}

struct SegmentNode {
    Matrix first, second;
};

class SegmentTree {
    int size = 1;
    vector<SegmentNode> tree;

    static Matrix identity() {
        return Matrix{{0, NEG, NEG, 0, NEG, 0}};
    }

    static SegmentNode makeLeaf(int64 A, int64 C, int64 D) {
        SegmentNode result;
        // C* A* 0*
        result.first = Matrix{{C, A, 0, A, 0, 0}};
        // A* 0* D*
        result.second = Matrix{{A, 0, D, 0, D, D}};
        return result;
    }

    void pull(int x) {
        tree[x].first =
            multiply(tree[x << 1].first, tree[x << 1 | 1].first);
        tree[x].second =
            multiply(tree[x << 1].second, tree[x << 1 | 1].second);
    }

public:
    explicit SegmentTree(int n) {
        while (size < n) size <<= 1;
        tree.resize(size << 1);
    }

    int logarithm() const {
        return __lg(size) + 1;
    }

    void build(const vector<int64> &A,
               const vector<int64> &C,
               const vector<int64> &D) {
        int n = int(A.size());
        for (int i = 0; i < size; ++i) {
            if (i < n) tree[size + i] = makeLeaf(A[i], C[i], D[i]);
            else tree[size + i] = {identity(), identity()};
        }
        for (int i = size - 1; i; --i) pull(i);
    }

    void update(int position, int64 A, int64 C, int64 D) {
        int x = size + position;
        tree[x] = makeLeaf(A, C, D);
        for (x >>= 1; x; x >>= 1) pull(x);
    }

    int64 best() const {
        const auto &a = tree[1].first.x;
        const auto &b = tree[1].second.x;
        return max({a[0], a[1], a[2], b[0], b[1], b[2]});
    }
};

struct Event {
    int time, vertex, type;
    int64 amount;

    bool operator<(const Event &other) const {
        return time < other.time;
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, s;
    cin >> n >> s;
    --s;

    vector<int64> w(n), p(n);
    vector<int> a(n), head(n, -1), nextEdge(n), indegree(n, 0);

    for (auto &x : w) cin >> x;
    for (auto &x : p) cin >> x;
    for (int i = 0; i < n; ++i) {
        cin >> a[i];
        --a[i];
        nextEdge[i] = head[a[i]];
        head[a[i]] = i;
        ++indegree[a[i]];
    }

    vector<int> queue;
    queue.reserve(n);
    for (int i = 0; i < n; ++i)
        if (indegree[i] == 0) queue.push_back(i);

    for (int q = 0; q < int(queue.size()); ++q) {
        int u = queue[q];
        if (--indegree[a[u]] == 0) queue.push_back(a[u]);
    }

    // If s is not on a cycle, only its reverse subtree can become 1.
    if (indegree[s] == 0) {
        vector<int> order;
        order.reserve(n);
        order.push_back(s);

        for (int q = 0; q < int(order.size()); ++q) {
            int u = order[q];
            for (int v = head[u]; v != -1; v = nextEdge[v])
                order.push_back(v);
        }

        vector<int64> stayOne(n), endZero(n);

        for (int q = int(order.size()) - 1; q >= 0; --q) {
            int u = order[q];
            int64 x = w[u] - p[u];
            int64 y = -2 * p[u];

            for (int v = head[u]; v != -1; v = nextEdge[v]) {
                x += max<int64>(0, stayOne[v]);
                y += max({0LL, stayOne[v], endZero[v]});
            }

            stayOne[u] = x;
            endZero[u] = y;
        }

        // The initial activation of s is free.
        cout << max(stayOne[s], endZero[s]) + p[s] << '\n';
        return 0;
    }

    vector<int> cycleNext(n, -1);
    for (int i = 0; i < n; ++i)
        if (indegree[i] > 0) cycleNext[a[i]] = i;

    vector<int> cycle;
    int u = s;
    do {
        cycle.push_back(u);
        u = cycleNext[u];
    } while (u != s);

    int length = int(cycle.size());

    // Process all trees attached to this cycle.
    vector<int> order = cycle;
    order.reserve(n);
    for (int q = 0; q < int(order.size()); ++q) {
        int x = order[q];
        for (int v = head[x]; v != -1; v = nextEdge[v]) {
            if (indegree[v] == 0) order.push_back(v);
        }
    }

    DropTreap treap(n);
    vector<int> oddRoot(n, 0), evenRoot(n, 0);

    for (int q = int(order.size()) - 1; q >= 0; --q) {
        int x = order[q];
        if (indegree[x] > 0) continue;

        treap.transform(oddRoot[x], evenRoot[x], w[x], p[x]);

        int parent = a[x];
        oddRoot[parent] = treap.unite(oddRoot[parent], oddRoot[x]);
        evenRoot[parent] = treap.unite(evenRoot[parent], evenRoot[x]);
    }

    // A two-cycle cannot move its mixed-color block:
    // it can only stay unchanged, become all 0, or become all 1.
    if (length == 2) {
        int x = cycle[0], y = cycle[1];

        int64 hx1 = treap.sum(oddRoot[x]);
        int64 hx2 = hx1 + treap.sum(evenRoot[x]);
        int64 hy1 = treap.sum(oddRoot[y]);

        int64 unchanged = w[x] + hx1;
        int64 allZero = -p[x] + hx2;
        int64 allOne = unchanged + w[y] - p[y] + hy1;

        cout << max({unchanged, allZero, allOne}) << '\n';
        return 0;
    }

    vector<int64> oddNext(length), evenNext(length), evenCurrent(length);
    vector<int64> A(length), C(length), D(length);
    vector<Event> events;
    events.reserve(3 * n);

    int64 totalP = 0;
    int64 baseAtOne = 0;

    for (int i = 0; i < length; ++i) {
        int x = cycle[i];

        oddNext[i] = treap.sum(oddRoot[x]);
        evenNext[i] = treap.sum(evenRoot[x]);
        evenCurrent[i] = evenNext[i];

        totalP += p[x];
        baseAtOne += oddNext[i] + evenNext[i] - 2 * p[x];

        auto oddCallback = [&](int position, int64 value) {
            events.push_back({position, i, 0, value});
        };
        auto evenCallback = [&](int position, int64 value) {
            events.push_back({position, i, 1, value});
            events.push_back({position + 1, i, 2, value});
        };

        treap.enumerate(oddRoot[x], oddCallback);
        treap.enumerate(evenRoot[x], evenCallback);

        A[i] = oddNext[i] + w[x] - p[x];
        C[i] = oddNext[i] + evenNext[i] - 2 * p[x];
    }

    /*
    t = 0:
    choose a sequence of states C* A* 0*, but the first vertex
    must be in state C or A because s was initially activated.
    */
    int64 dp[3] = {C[0], A[0], NEG};
    for (int i = 1; i < length; ++i) {
        int64 ndp[3];
        ndp[0] = dp[0] + C[i];
        ndp[1] = max(dp[0], dp[1]) + A[i];
        ndp[2] = max({dp[0], dp[1], dp[2]});
        copy(begin(ndp), end(ndp), begin(dp));
    }
    int64 answer = max({dp[0], dp[1], dp[2]});

    sort(events.begin(), events.end());

    size_t eventIndex = 0;

    // Move the marginal values from t=0 to t=1.
    while (eventIndex < events.size() && events[eventIndex].time == 1) {
        const Event &e = events[eventIndex++];
        if (e.type == 0) oddNext[e.vertex] -= e.amount;
        else if (e.type == 1) evenNext[e.vertex] -= e.amount;
        else evenCurrent[e.vertex] -= e.amount;
    }

    int64 slope = 0;
    for (int i = 0; i < length; ++i) {
        int x = cycle[i];
        A[i] = oddNext[i] + w[x] - p[x];
        C[i] = oddNext[i] + evenNext[i] - 2 * p[x];
        D[i] = -evenCurrent[i] + w[x] + p[x];
        slope += C[i];
    }

    SegmentTree segmentTree(length);
    segmentTree.build(A, C, D);

    int currentTime = 1;
    int64 base = baseAtOne;

    vector<int> touched;
    vector<int> stamp(length, 0);
    int currentStamp = 0;

    while (true) {
        int64 offset = segmentTree.best();
        answer = max(answer, base + offset);

        if (eventIndex == events.size()) {
            // There are no remaining marginal gains.
            // Hence the remaining slope is -2 * totalP <= 0.
            break;
        }

        int nextTime = events[eventIndex].time;
        int gap = nextTime - currentTime;

        // Between events, offset is constant and base is linear.
        if (gap > 1 && slope > 0) {
            answer = max(answer, base + slope * (gap - 1) + offset);
        }

        base += slope * gap;
        currentTime = nextTime;

        ++currentStamp;
        touched.clear();

        while (eventIndex < events.size() &&
               events[eventIndex].time == currentTime) {
            const Event &e = events[eventIndex++];
            int i = e.vertex;

            if (stamp[i] != currentStamp) {
                stamp[i] = currentStamp;
                touched.push_back(i);
            }

            if (e.type == 0) {
                oddNext[i] -= e.amount;
                slope -= e.amount;
            } else if (e.type == 1) {
                evenNext[i] -= e.amount;
                slope -= e.amount;
            } else {
                evenCurrent[i] -= e.amount;
            }
        }

        for (int i : touched) {
            int x = cycle[i];
            A[i] = oddNext[i] + w[x] - p[x];
            C[i] = oddNext[i] + evenNext[i] - 2 * p[x];
            D[i] = -evenCurrent[i] + w[x] + p[x];
        }

        // Rebuilding is faster when many leaves change simultaneously.
        if (int64(touched.size()) * segmentTree.logarithm() > length) {
            segmentTree.build(A, C, D);
        } else {
            for (int i : touched)
                segmentTree.update(i, A[i], C[i], D[i]);
        }
    }

    // The virtual initial activation of s was charged p[s]; refund it.
    cout << answer + p[s] << '\n';
    return 0;
}
