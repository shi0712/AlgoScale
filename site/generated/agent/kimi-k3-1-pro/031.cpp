#include <bits/stdc++.h>
using namespace std;

static constexpr int MOD = 998244353;

int addmod(int a, int b) {
    int s = a + b;
    return s >= MOD ? s - MOD : s;
}

int submod(int a, int b) {
    int s = a - b;
    return s < 0 ? s + MOD : s;
}

int mulmod(int a, int b) {
    return int(1LL * a * b % MOD);
}

int powmod(int a, int e) {
    int r = 1;
    while (e) {
        if (e & 1) r = mulmod(r, a);
        a = mulmod(a, a);
        e >>= 1;
    }
    return r;
}

struct Mat {
    int a00 = 1, a01 = 0, a10 = 0, a11 = 1;
};

Mat operator*(const Mat &a, const Mat &b) {
    return {
        addmod(mulmod(a.a00, b.a00), mulmod(a.a01, b.a10)),
        addmod(mulmod(a.a00, b.a01), mulmod(a.a01, b.a11)),
        addmod(mulmod(a.a10, b.a00), mulmod(a.a11, b.a10)),
        addmod(mulmod(a.a10, b.a01), mulmod(a.a11, b.a11))
    };
}

struct LinkCutTree {
    vector<array<int, 2>> ch;
    vector<int> parent, degree;
    vector<int> productNonzero, weightedSum, zeroValue;
    vector<int> zeroCount;
    vector<unsigned char> reversed;
    vector<Mat> forward, backward;

    explicit LinkCutTree(int n)
        : ch(n + 1), parent(n + 1), degree(n + 1),
          productNonzero(n + 1, 1), weightedSum(n + 1), zeroValue(n + 1),
          zeroCount(n + 1), reversed(n + 1), forward(n + 1), backward(n + 1) {}

    bool isRoot(int x) const {
        int p = parent[x];
        return p == 0 || (ch[p][0] != x && ch[p][1] != x);
    }

    int virtualProduct(int x) const {
        return zeroCount[x] == 0 ? productNonzero[x] : 0;
    }

    int virtualWeightedSum(int x) const {
        if (zeroCount[x] == 0) return mulmod(productNonzero[x], weightedSum[x]);
        if (zeroCount[x] == 1) return mulmod(productNonzero[x], zeroValue[x]);
        return 0;
    }

    Mat selfMatrix(int x) const {
        int p = virtualProduct(x);
        int q = virtualWeightedSum(x);
        return {submod(mulmod(degree[x], p), q), submod(0, p), p, 0};
    }

    void update(int x) {
        Mat self = selfMatrix(x);
        forward[x] = forward[ch[x][0]] * self * forward[ch[x][1]];
        backward[x] = backward[ch[x][1]] * self * backward[ch[x][0]];
    }

    pair<int, int> value(int x) const {
        return {forward[x].a00, forward[x].a10};
    }

    void addVirtual(int x, pair<int, int> v) {
        auto [d, b] = v;
        if (d == 0) {
            ++zeroCount[x];
            zeroValue[x] = addmod(zeroValue[x], b);
        } else {
            int inverse = powmod(d, MOD - 2);
            productNonzero[x] = mulmod(productNonzero[x], d);
            weightedSum[x] = addmod(weightedSum[x], mulmod(b, inverse));
        }
    }

    void removeVirtual(int x, pair<int, int> v) {
        auto [d, b] = v;
        if (d == 0) {
            --zeroCount[x];
            zeroValue[x] = submod(zeroValue[x], b);
        } else {
            int inverse = powmod(d, MOD - 2);
            productNonzero[x] = mulmod(productNonzero[x], inverse);
            weightedSum[x] = submod(weightedSum[x], mulmod(b, inverse));
        }
    }

    void applyReverse(int x) {
        swap(ch[x][0], ch[x][1]);
        swap(forward[x], backward[x]);
        reversed[x] ^= 1;
    }

    void push(int x) {
        if (!reversed[x]) return;
        if (ch[x][0]) applyReverse(ch[x][0]);
        if (ch[x][1]) applyReverse(ch[x][1]);
        reversed[x] = 0;
    }

    void rotate(int x) {
        int p = parent[x], g = parent[p];
        int side = ch[p][1] == x;
        int y = ch[x][side ^ 1];
        if (!isRoot(p)) ch[g][ch[g][1] == p] = x;
        parent[x] = g;
        ch[x][side ^ 1] = p;
        parent[p] = x;
        ch[p][side] = y;
        if (y) parent[y] = p;
        update(p);
        update(x);
    }

    void splay(int x) {
        static vector<int> path;
        path.clear();
        for (int y = x; ; y = parent[y]) {
            path.push_back(y);
            if (isRoot(y)) break;
        }
        for (auto it = path.rbegin(); it != path.rend(); ++it) push(*it);
        while (!isRoot(x)) {
            int p = parent[x], g = parent[p];
            if (!isRoot(p)) {
                if ((ch[p][0] == x) == (ch[g][0] == p)) rotate(p);
                else rotate(x);
            }
            rotate(x);
        }
    }

    void access(int x) {
        int last = 0;
        for (int y = x; y; y = parent[y]) {
            splay(y);
            if (ch[y][1]) addVirtual(y, value(ch[y][1]));
            if (last) removeVirtual(y, value(last));
            ch[y][1] = last;
            if (last) parent[last] = y;
            update(y);
            last = y;
        }
        splay(x);
    }

    void makeRoot(int x) {
        access(x);
        applyReverse(x);
    }

    void initialize(int x, int d) {
        degree[x] = d;
        update(x);
    }

    pair<int, int> query(int x) {
        makeRoot(x);
        access(x);
        return value(x);
    }

    void linkRooted(int x, int root) {
        access(root);
        parent[x] = root;
        addVirtual(root, value(x));
        update(root);
    }

    int rootedDeterminant(int x) const {
        return selfMatrix(x).a00;
    }
};

struct ComponentProduct {
    int zeroCount = 0;
    int product = 1;

    void add(int x) {
        if (x == 0) ++zeroCount;
        else product = mulmod(product, x);
    }

    void remove(int x) {
        if (x == 0) --zeroCount;
        else product = mulmod(product, powmod(x, MOD - 2));
    }

    int get() const {
        return zeroCount == 0 ? product : 0;
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    cin >> n;
    vector<vector<int>> tree(n + 1);
    for (int i = 0; i < n - 1; ++i) {
        int u, v;
        cin >> u >> v;
        tree[u].push_back(v);
        tree[v].push_back(u);
    }

    vector<int> power2(n + 1, 1);
    for (int i = 1; i <= n; ++i) power2[i] = mulmod(power2[i - 1], 2);

    LinkCutTree lct(n);
    ComponentProduct components;
    vector<bool> active(n + 1, false);
    int scale = 1;
    const int inverse2 = (MOD + 1) / 2;

    for (int step = 0; step < n - 1; ++step) {
        int x;
        cin >> x;
        lct.initialize(x, int(tree[x].size()));
        active[x] = true;

        int activeNeighbors = 0;
        for (int u : tree[x]) {
            if (!active[u]) continue;
            ++activeNeighbors;
            auto [determinant, cofactor] = lct.query(u);
            components.remove(determinant);
            lct.linkRooted(u, x);
        }

        components.add(lct.rootedDeterminant(x));
        int inactiveNeighbors = int(tree[x].size()) - activeNeighbors;
        int factor = inactiveNeighbors == 0 ? inverse2 : power2[inactiveNeighbors - 1];
        scale = mulmod(scale, factor);
        cout << mulmod(scale, components.get()) << '\n';
    }
    return 0;
}
