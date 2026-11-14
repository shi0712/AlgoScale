#include <bits/stdc++.h>
using namespace std;

using u32 = uint32_t;
using u64 = uint64_t;
using i64 = long long;

static constexpr u32 MOD = 1'000'000'007;

static inline u32 mulmod(u32 a, u32 b) {
    return static_cast<u64>(a) * b % MOD;
}

static inline u32 dot3(u32 a, u32 b, u32 c,
                       u32 x, u32 y, u32 z) {
    return (static_cast<u64>(a) * x
          + static_cast<u64>(b) * y
          + static_cast<u64>(c) * z) % MOD;
}

static u32 modpow(u32 a, u32 e) {
    u32 result = 1;
    while (e) {
        if (e & 1) result = mulmod(result, a);
        a = mulmod(a, a);
        e >>= 1;
    }
    return result;
}

/*
For each interpolation point t = 0..n, maintain a product of 3x3 matrices.

A rank-one matrix is stored as u * v^T:
    data[0..2] = u
    data[3..5] = v

Otherwise, the matrix is stored in row-major order.
*/
class MatrixSegmentTree {
    struct Node {
        int left = -1, right = -1, parent = -1;
        int position = -1;
        int state = -1; // Leaf: 0 = always high, 1 = active, 2 = always low.
        bool rankOne = false;
        bool dirty = false;
        vector<u32> data;
    };

    int n, points;
    const vector<int>& lo;
    const vector<int>& hi;
    vector<Node> tree;
    vector<int> leaf;

    void setLeaf(int id, int threshold) {
        Node& node = tree[id];
        int p = node.position;
        u32 length = hi[p] - lo[p] + 1;

        if (threshold <= lo[p]) {
            node.state = 0;
            node.rankOne = true;

            for (int t = 0; t < points; ++t) {
                u32* a = node.data.data() + 9LL * t;
                a[0] = 0;
                a[1] = length;
                a[2] = 0;
                a[3] = 1;
                a[4] = 0;
                a[5] = 0;
            }
        } else if (threshold > hi[p]) {
            node.state = 2;
            node.rankOne = false;

            for (int t = 0; t < points; ++t) {
                u32* a = node.data.data() + 9LL * t;
                a[0] = length;
                a[1] = 0;
                a[2] = length;
                a[3] = 0;
                a[4] = 0;
                a[5] = 0;
                a[6] = 0;
                a[7] = length;
                a[8] = 0;
            }
        } else {
            node.state = 1;
            node.rankOne = false;

            // Extrapolate the active formulas to every interpolation point t:
            // low(t) = t - l, high(t) = r + 1 - t.
            u32 b = MOD - static_cast<u32>(lo[p]);
            u32 h = static_cast<u32>(hi[p] + 1);

            for (int t = 0; t < points; ++t) {
                u32* a = node.data.data() + 9LL * t;
                a[0] = b;
                a[1] = 0;
                a[2] = b;
                a[3] = h;
                a[4] = 0;
                a[5] = 0;
                a[6] = 0;
                a[7] = b;
                a[8] = 0;

                if (++b == MOD) b = 0;
                h = (h == 0 ? MOD - 1 : h - 1);
            }
        }
    }

    void pull(int id) {
        Node& node = tree[id];
        const Node& left = tree[node.left];
        const Node& right = tree[node.right];

        // The matrix product is right * left.
        bool twoLeaves = left.position != -1 && right.position != -1;

        // Two consecutive always-low positions give a rank-one product.
        if (twoLeaves && left.state == 2 && right.state == 2) {
            node.rankOne = true;
            u32 coefficient = mulmod(left.data[0], right.data[0]);

            for (int t = 0; t < points; ++t) {
                u32* c = node.data.data() + 9LL * t;
                c[0] = coefficient;
                c[1] = 0;
                c[2] = 0;
                c[3] = 1;
                c[4] = 1;
                c[5] = 1;
            }
            return;
        }

        node.rankOne = left.rankOne || right.rankOne;

        // A specialized product for two ordinary leaf matrices.
        if (twoLeaves && !node.rankOne) {
            for (int t = 0; t < points; ++t) {
                const u32* l = left.data.data() + 9LL * t;
                const u32* r = right.data.data() + 9LL * t;
                u32* c = node.data.data() + 9LL * t;

                u32 p = mulmod(l[0], r[0]);
                u32 q = mulmod(l[0], r[3]);
                u32 s = mulmod(l[3], r[0]);

                c[0] = p;
                c[1] = p;
                c[2] = p;
                c[3] = q;
                c[4] = 0;
                c[5] = q;
                c[6] = s;
                c[7] = 0;
                c[8] = 0;
            }
            return;
        }

        if (left.rankOne && right.rankOne) {
            for (int t = 0; t < points; ++t) {
                const u32* l = left.data.data() + 9LL * t;
                const u32* r = right.data.data() + 9LL * t;
                u32* c = node.data.data() + 9LL * t;

                u32 scale = dot3(r[3], r[4], r[5], l[0], l[1], l[2]);

                c[0] = mulmod(r[0], scale);
                c[1] = mulmod(r[1], scale);
                c[2] = mulmod(r[2], scale);
                c[3] = l[3];
                c[4] = l[4];
                c[5] = l[5];
            }
        } else if (right.rankOne) {
            for (int t = 0; t < points; ++t) {
                const u32* l = left.data.data() + 9LL * t;
                const u32* r = right.data.data() + 9LL * t;
                u32* c = node.data.data() + 9LL * t;

                c[0] = r[0];
                c[1] = r[1];
                c[2] = r[2];

                c[3] = dot3(r[3], r[4], r[5], l[0], l[3], l[6]);
                c[4] = dot3(r[3], r[4], r[5], l[1], l[4], l[7]);
                c[5] = dot3(r[3], r[4], r[5], l[2], l[5], l[8]);
            }
        } else if (left.rankOne) {
            for (int t = 0; t < points; ++t) {
                const u32* l = left.data.data() + 9LL * t;
                const u32* r = right.data.data() + 9LL * t;
                u32* c = node.data.data() + 9LL * t;

                c[0] = dot3(r[0], r[1], r[2], l[0], l[1], l[2]);
                c[1] = dot3(r[3], r[4], r[5], l[0], l[1], l[2]);
                c[2] = dot3(r[6], r[7], r[8], l[0], l[1], l[2]);

                c[3] = l[3];
                c[4] = l[4];
                c[5] = l[5];
            }
        } else {
            for (int t = 0; t < points; ++t) {
                const u32* l = left.data.data() + 9LL * t;
                const u32* r = right.data.data() + 9LL * t;
                u32* c = node.data.data() + 9LL * t;

                for (int row = 0; row < 3; ++row) {
                    int k = 3 * row;
                    c[k] = dot3(r[k], r[k + 1], r[k + 2],
                                l[0], l[3], l[6]);
                    c[k + 1] = dot3(r[k], r[k + 1], r[k + 2],
                                    l[1], l[4], l[7]);
                    c[k + 2] = dot3(r[k], r[k + 1], r[k + 2],
                                    l[2], l[5], l[8]);
                }
            }
        }
    }

    int build(int l, int r, int parent, int threshold) {
        int id = static_cast<int>(tree.size());
        tree.emplace_back();
        tree[id].parent = parent;
        tree[id].data.resize(9LL * points);

        if (l == r) {
            tree[id].position = l;
            leaf[l] = id;
            setLeaf(id, threshold);
        } else {
            int mid = (l + r) / 2;
            int lc = build(l, mid, id, threshold);
            int rc = build(mid + 1, r, id, threshold);
            tree[id].left = lc;
            tree[id].right = rc;
            pull(id);
        }
        return id;
    }

public:
    MatrixSegmentTree(const vector<int>& l, const vector<int>& r,
                      int initialThreshold)
        : n(static_cast<int>(l.size())),
          points(n + 1),
          lo(l),
          hi(r),
          leaf(n) {
        tree.reserve(2 * n);
        build(0, n - 1, -1, initialThreshold);
    }

    void updateBatch(const vector<int>& positions, int threshold) {
        vector<int> dirtyNodes;
        dirtyNodes.reserve(positions.size() * 12);

        for (int p : positions) {
            int id = leaf[p];
            setLeaf(id, threshold);

            for (int v = tree[id].parent; v != -1; v = tree[v].parent) {
                if (tree[v].dirty) break;
                tree[v].dirty = true;
                dirtyNodes.push_back(v);
            }
        }

        // Nodes are numbered in preorder, so children have larger IDs.
        sort(dirtyNodes.begin(), dirtyNodes.end(), greater<int>());
        for (int id : dirtyNodes) {
            pull(id);
            tree[id].dirty = false;
        }
    }

    // Returns Q(k) = sum_{t=0}^{k-1} P(t), for k = 0..n+1.
    void prefixPolynomialValues(vector<u32>& q) const {
        q.resize(n + 2);
        q[0] = 0;

        const Node& root = tree[0];
        for (int t = 0; t <= n; ++t) {
            const u32* a = root.data.data() + 9LL * t;
            u32 value;

            if (root.rankOne) {
                value = ((static_cast<u64>(a[0]) + a[1] + a[2]) * a[3]) % MOD;
            } else {
                value = (static_cast<u64>(a[0]) + a[3] + a[6]) % MOD;
            }

            u32 sum = q[t] + value;
            if (sum >= MOD) sum -= MOD;
            q[t + 1] = sum;
        }
    }
};

class Interpolator {
    int degree;
    vector<u32> inverseFactorial;
    vector<u32> prefix, suffix;

public:
    explicit Interpolator(int d)
        : degree(d),
          inverseFactorial(d + 1),
          prefix(d + 2),
          suffix(d + 2) {
        u32 factorial = 1;
        for (int i = 1; i <= d; ++i) factorial = mulmod(factorial, i);

        inverseFactorial[d] = modpow(factorial, MOD - 2);
        for (int i = d; i >= 1; --i) {
            inverseFactorial[i - 1] = mulmod(inverseFactorial[i], i);
        }
    }

    u32 evaluate(const vector<u32>& values, int x) {
        if (x <= degree) return values[x];

        prefix[0] = 1;
        for (int i = 0; i <= degree; ++i) {
            u32 difference = static_cast<u32>(x - i);
            prefix[i + 1] = mulmod(prefix[i], difference);
        }

        suffix[degree + 1] = 1;
        for (int i = degree; i >= 0; --i) {
            u32 difference = static_cast<u32>(x - i);
            suffix[i] = mulmod(suffix[i + 1], difference);
        }

        u32 answer = 0;
        for (int i = 0; i <= degree; ++i) {
            u32 term = mulmod(values[i], inverseFactorial[i]);
            term = mulmod(term, inverseFactorial[degree - i]);
            term = mulmod(term, prefix[i]);
            term = mulmod(term, suffix[i + 1]);

            if ((degree - i) & 1) {
                answer = (answer >= term ? answer - term : answer + MOD - term);
            } else {
                answer += term;
                if (answer >= MOD) answer -= MOD;
            }
        }
        return answer;
    }
};

static int sequenceValue(const vector<int>& a) {
    int result = 0;
    int n = static_cast<int>(a.size());

    for (int i = 0; i < n; ++i) {
        if (i + 1 < n) result = max(result, min(a[i], a[i + 1]));
        if (i + 2 < n) result = max(result, min(a[i], a[i + 2]));
    }
    return result;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int T;
    cin >> T;

    while (T--) {
        int n;
        cin >> n;

        vector<int> l(n), r(n);
        for (int& x : l) cin >> x;
        for (int& x : r) cin >> x;

        u32 total = 1;
        for (int i = 0; i < n; ++i) {
            total = mulmod(total, r[i] - l[i] + 1);
        }

        int lower = sequenceValue(l);
        int upper = sequenceValue(r);

        if (lower == upper) {
            cout << mulmod(total, lower) << '\n';
            continue;
        }

        int current = lower + 1;

        vector<pair<int, int>> events;
        events.reserve(2 * n);
        for (int i = 0; i < n; ++i) {
            int first = l[i] + 1;
            int second = r[i] + 1;

            if (current < first && first <= upper) {
                events.emplace_back(first, i);
            }
            if (current < second && second <= upper) {
                events.emplace_back(second, i);
            }
        }
        sort(events.begin(), events.end());
        events.erase(unique(events.begin(), events.end()), events.end());

        MatrixSegmentTree segmentTree(l, r, current);
        Interpolator interpolator(n + 1);
        vector<u32> q;
        vector<int> changed;

        u32 badSum = 0;
        size_t eventIndex = 0;

        while (current <= upper) {
            int next = (eventIndex < events.size()
                        ? events[eventIndex].first
                        : upper + 1);

            segmentTree.prefixPolynomialValues(q);

            u32 rightSum = interpolator.evaluate(q, next);
            u32 leftSum = interpolator.evaluate(q, current);
            u32 contribution = (rightSum >= leftSum
                                ? rightSum - leftSum
                                : rightSum + MOD - leftSum);

            badSum += contribution;
            if (badSum >= MOD) badSum -= MOD;

            if (next > upper) break;

            changed.clear();
            while (eventIndex < events.size()
                   && events[eventIndex].first == next) {
                changed.push_back(events[eventIndex].second);
                ++eventIndex;
            }

            segmentTree.updateBatch(changed, next);
            current = next;
        }

        u32 answer = mulmod(total, upper);
        answer = (answer >= badSum ? answer - badSum : answer + MOD - badSum);
        cout << answer << '\n';
    }

    return 0;
}
