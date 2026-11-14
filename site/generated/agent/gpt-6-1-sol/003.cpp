#include <bits/stdc++.h>
using namespace std;

/*
Let M be the actual maximum. Move everything after the last M to the front,
increasing those moved entries by one, to obtain H, which ends in M.
The original queue starts partway through a traversal of H. In later
traversals, a state is a suffix U followed by an already processed prefix V;
the entries of V have been decreased one extra time. The last current
maximum identifies the end of U, so this decomposition is unique.

An omitted entry must be smaller than the next retained entry. Thus a full
traversal is described by a language L of record-admissible subsequences,
with every M compulsory. Its first-maximum Cartesian tree is an unambiguous
grammar. At a node of value v, with child languages A and B, the language is
    A (epsilon | v B)   if v < M,    and    A v B   if v = M.

At level d, delete letters <= d from this language. A state corresponds to
splitting a word before its first letter d+1, or at any earlier position;
the prefix is decreased once more. Summing these split counts over all
levels telescopes. A word of minimum x, length len, and p letters before
its first minimum contributes (x-1)*len+p, with the additional |L|-1 term.
The final all-one queues contribute the number of M's in H. Only the first
two traversals have not yet had enough time to make every admissible
omission; these are replaced by direct linear-time counts below.

The grammar statistics are merged by minimum using destructive segment
trees. Each tree node is created/destroyed O(1) times, for O(n log M) time.
Evaluating larger Cartesian subtrees first bounds the live distributions.
*/

static constexpr int MOD = 998244353;
inline int addmod(int a, int b) { int s = a + b; return s >= MOD ? s - MOD : s; }
inline int mulmod(int a, int b) { return int((long long)a * b % MOD); }

/*
For a language of words we store, for each possible minimum x:
  f = number of words,
  s = sum of their lengths,
  p = sum of the number of letters before their FIRST minimum.
Empty words are handled separately.

Concatenation combines these distributions by a minimum convolution. A
segment-tree merge performs it in time proportional to the nodes destroyed.
The lazy operation is (f,s,p) -> (a*f, a*s+b*f, a*p+c*f).
*/
class Distributions {
    struct Node {
        int l = 0, r = 0;
        int f = 0, s = 0, p = 0;
        int a = 1, b = 0, c = 0;
    };
    vector<Node> t;
    int freeHead = 0;
    int newNode() {
        if (freeHead) {
            int u = freeHead;
            freeHead = t[u].l;
            t[u] = Node();
            return u;
        }
        t.emplace_back();
        return int(t.size()) - 1;
    }
    void release(int u) { t[u].l = freeHead; freeHead = u; }
    void apply(int u, int a, int b, int c) {
        if (!u) return;
        int oldF = t[u].f;
        t[u].f = mulmod(a, oldF);
        t[u].s = addmod(mulmod(a, t[u].s), mulmod(b, oldF));
        t[u].p = addmod(mulmod(a, t[u].p), mulmod(c, oldF));
        t[u].b = addmod(mulmod(a, t[u].b), mulmod(b, t[u].a));
        t[u].c = addmod(mulmod(a, t[u].c), mulmod(c, t[u].a));
        t[u].a = mulmod(a, t[u].a);
    }
    void push(int u) {
        if (t[u].a != 1 || t[u].b || t[u].c) {
            apply(t[u].l, t[u].a, t[u].b, t[u].c);
            apply(t[u].r, t[u].a, t[u].b, t[u].c);
            t[u].a = 1; t[u].b = t[u].c = 0;
        }
    }
    void pull(int u) {
        int l = t[u].l, r = t[u].r;
        t[u].f = addmod(t[l].f, t[r].f);
        t[u].s = addmod(t[l].s, t[r].s);
        t[u].p = addmod(t[l].p, t[r].p);
    }
    int point(int u, int lo, int hi, int x, int emptyRight) {
        if (!u) {
            if (!emptyRight) return 0;
            u = newNode();
        }
        if (lo == hi) {
            t[u].f = addmod(t[u].f, emptyRight);
            t[u].s = addmod(t[u].s, emptyRight);
            t[u].p = 0;
            return u;
        }
        push(u);
        int mid = (lo + hi) / 2;
        if (x <= mid) {
            int child = point(t[u].l, lo, mid, x, emptyRight);
            t[u].l = child;
        } else {
            int child = point(t[u].r, mid + 1, hi, x, emptyRight);
            t[u].r = child;
        }
        pull(u);
        return u;
    }
    // af/as and bf/bs are counts/length sums at strictly greater minima,
    // including the empty word (whose minimum is infinity).
    int merge(int a, int b, int lo, int hi, int af, int as, int bf, int bs) {
        if (!a) { apply(b, af, as, as); return b; }
        if (!b) { apply(a, bf, bs, 0); return a; }
        if (lo == hi) {
            int fa = t[a].f, sa = t[a].s, pa = t[a].p;
            int fb = t[b].f, sb = t[b].s, pb = t[b].p;
            int bg = addmod(bf, fb);
            t[a].f = addmod(mulmod(fa, bg), mulmod(af, fb));
            t[a].s = addmod(addmod(mulmod(sa, bg), mulmod(fa, addmod(sb, bs))),
                                addmod(mulmod(as, fb), mulmod(af, sb)));
            t[a].p = addmod(mulmod(pa, bg), addmod(mulmod(as, fb), mulmod(af, pb)));
            t[a].a = 1; t[a].b = t[a].c = 0;
            release(b);
            return a;
        }
        push(a); push(b);
        int ar = t[a].r, br = t[b].r;
        int nextAF = addmod(af, t[ar].f), nextAS = addmod(as, t[ar].s);
        int nextBF = addmod(bf, t[br].f), nextBS = addmod(bs, t[br].s);
        int mid = (lo + hi) / 2;
        t[a].r = merge(ar, br, mid + 1, hi, af, as, bf, bs);
        t[a].l = merge(t[a].l, t[b].l, lo, mid, nextAF, nextAS, nextBF, nextBS);
        pull(a);
        release(b);
        return a;
    }
    int statistic(int u, int lo, int hi) {
        if (!u) return 0;
        if (lo == hi) return addmod(mulmod(lo - 1, t[u].s), t[u].p);
        push(u);
        int mid = (lo + hi) / 2;
        return addmod(statistic(t[u].l, lo, mid), statistic(t[u].r, mid + 1, hi));
    }
public:
    explicit Distributions() { t.reserve(262144); t.emplace_back(); }
    int combine(int left, int right, int value, int maximum, int emptyRight) {
        // Prepend the root letter to the right language.
        apply(right, 1, 1, 1);
        right = point(right, 1, maximum, value, emptyRight);
        // A non-maximum root may instead be omitted, giving the empty word.
        return merge(left, right, 1, maximum, 1, 0, value < maximum, 0);
    }
    int count(int root) const { return t[root].f; }
    int statistic(int root, int maximum) { return statistic(root, 1, maximum); }
};

class Solver {
    int n, M, initialPrefix;
    vector<int> h, nextGreater, leftChild, rightChild, subtreeSize, distribution;
    vector<int> stack, f, k;

    void suffixCounts(int d) {
        f[n + 1] = 1;
        for (int i = n; i >= 1; --i) {
            f[i] = f[i + 1];
            if (h[i] > d && nextGreater[i] <= n)
                f[i] = addmod(f[i], f[nextGreater[i]]);
        }
    }
    // Count distinct prefix outputs followed by a suffix from the supplied
    // language. The next output is a strict prefix record above threshold.
    template<class Weight>
    int prefixCount(int threshold, int start, Weight weight) {
        int following = 0;
        for (int i = n; i >= start; --i) {
            if (i == n) k[i] = 0; // The last maximum must stay in the suffix.
            else if (h[i] <= threshold) k[i] = k[i + 1];
            else k[i] = addmod(following, nextGreater[i] <= n ? k[nextGreater[i]] : 0);
            following = addmod(weight(i), k[i]);
        }
        return following;
    }
    int hypotheticalStage(int d) {
        suffixCounts(d);
        return prefixCount(d + 1, 1, [&](int i) { return f[i]; });
    }
    int firstStage() {
        int ones = 0;
        return prefixCount(1, initialPrefix + 1, [&](int i) {
            ones = h[i] == 1 ? ones + 1 : 0;
            return ones + 1;
        });
    }
    int secondStage() {
        suffixCounts(1);
        // Number of suffix words whose first letter is greater than one.
        k[n + 1] = 0;
        for (int i = n; i > initialPrefix; --i) {
            if (h[i] <= 2) k[i] = k[i + 1];
            else k[i] = addmod(f[i + 1], nextGreater[i] <= n ? k[nextGreater[i]] : 0);
        }
        int suffixF = f[initialPrefix + 1];
        int nonOne = k[initialPrefix + 1];
        int ones = 0;
        return prefixCount(2, 1, [&](int i) {
            if (i > initialPrefix) return f[i];
            ones = h[i] == 2 ? ones + 1 : 0;
            if (i + ones - 1 == initialPrefix)
                return addmod(suffixF, mulmod(ones, nonOne));
            return mulmod(ones + 1, suffixF);
        });
    }
    int allHypotheticalStages(int maximumCount) {
        // First-maximum Cartesian tree. Evaluate the larger subtree first,
        // so only O(log n) completed distributions are held simultaneously.
        Distributions languages;
        distribution.assign(n + 1, 0);
        stack.clear();
        int root = 1;
        while (h[root] != M) ++root;
        stack.push_back(root);
        while (!stack.empty()) {
            int u = stack.back(); stack.pop_back();
            if (u > 0) {
                stack.push_back(-u);
                int a = leftChild[u], b = rightChild[u];
                if (subtreeSize[a] < subtreeSize[b]) swap(a, b);
                if (b) stack.push_back(b);
                if (a) stack.push_back(a);
            } else {
                u = -u;
                int l = leftChild[u], r = rightChild[u];
                int emptyRight = !r || h[r] < M;
                distribution[u] = languages.combine(distribution[l], distribution[r], h[u], M, emptyRight);
            }
        }
        int r = distribution[root];
        // Sum over all levels telescopes to:
        // |L| - 1 + sum_words ((min-1)*length + prefix_before_first_min).
        // At maximum one, only the lengths 1..maximumCount remain.
        int result = addmod(languages.count(r), languages.statistic(r, M));
        result = addmod(result, maximumCount - 1);
        return result;
    }
public:
    int run(vector<int> a) {
        n = int(a.size()); M = *max_element(a.begin(), a.end());
        if (M == 1) return n;
        int lastMaximum = n - 1;
        while (a[lastMaximum] != M) --lastMaximum;
        initialPrefix = n - 1 - lastMaximum;
        h.assign(n + 2, 0);
        int at = 1, maximumCount = 0;
        for (int i = lastMaximum + 1; i < n; ++i) h[at++] = a[i] + 1;
        for (int i = 0; i <= lastMaximum; ++i) h[at++] = a[i];
        vector<int>().swap(a);
        for (int i = 1; i <= n; ++i) maximumCount += h[i] == M;

        nextGreater.assign(n + 2, n + 1);
        leftChild.assign(n + 1, 0); rightChild.assign(n + 1, 0);
        subtreeSize.assign(n + 1, 0);
        stack.reserve(n + 1);
        for (int i = 1; i <= n; ++i) {
            int last = 0;
            while (!stack.empty() && h[stack.back()] < h[i]) {
                last = stack.back(); stack.pop_back();
                nextGreater[last] = i;
            }
            subtreeSize[i] = stack.empty() ? 0 : stack.back(); // previous >=
            if (!stack.empty()) rightChild[stack.back()] = i;
            leftChild[i] = last;
            stack.push_back(i);
        }
        for (int i = 1; i <= n; ++i)
            subtreeSize[i] = nextGreater[i] - subtreeSize[i] - 1;
        f.assign(n + 2, 0); k.assign(n + 2, 0);
        int actual0 = firstStage();
        if (M == 2) return addmod(actual0, maximumCount);
        int actual1 = secondStage();
        if (M == 3) return addmod(addmod(actual0, actual1), maximumCount);
        int hyp0 = hypotheticalStage(0), hyp1 = hypotheticalStage(1);
        int result = allHypotheticalStages(maximumCount);
        result = addmod(result, MOD - hyp0);
        result = addmod(result, MOD - hyp1);
        result = addmod(result, actual0);
        result = addmod(result, actual1);
        return result;
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    int c, T;
    if (!(cin >> c >> T)) return 0;
    while (T--) {
        int n, m; cin >> n >> m;
        vector<int> a(n);
        for (int &x : a) cin >> x;
        Solver solver;
        cout << solver.run(move(a)) << '\n';
    }
    return 0;
}
