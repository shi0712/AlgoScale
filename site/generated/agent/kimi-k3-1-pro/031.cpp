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
    return int((long long)a * b % MOD);
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

struct Matrix {
    int a, b, c, d;
};

Matrix multiply(const Matrix &x, const Matrix &y) {
    return {
        addmod(mulmod(x.a, y.a), mulmod(x.b, y.c)),
        addmod(mulmod(x.a, y.b), mulmod(x.b, y.d)),
        addmod(mulmod(x.c, y.a), mulmod(x.d, y.c)),
        addmod(mulmod(x.c, y.b), mulmod(x.d, y.d))
    };
}

struct Chain {
    int offset = 0;
    int size = 0;
    int capacity = 0;
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    cin >> n;
    vector<vector<int>> tree(n);
    for (int i = 0; i < n - 1; ++i) {
        int u, v;
        cin >> u >> v;
        --u;
        --v;
        tree[u].push_back(v);
        tree[v].push_back(u);
    }

    vector<int> parent(n, -1), depth(n), heavy(n, -1), subtree(n);
    vector<int> order;
    order.reserve(n);
    order.push_back(0);
    for (int i = 0; i < n; ++i) {
        int v = order[i];
        for (int u : tree[v]) {
            if (u == parent[v]) continue;
            parent[u] = v;
            depth[u] = depth[v] + 1;
            order.push_back(u);
        }
    }
    for (int i = n - 1; i >= 0; --i) {
        int v = order[i];
        subtree[v] = 1;
        int largest = 0;
        for (int u : tree[v]) {
            if (parent[u] != v) continue;
            subtree[v] += subtree[u];
            if (subtree[u] > largest) {
                largest = subtree[u];
                heavy[v] = u;
            }
        }
    }

    vector<int> head(n), position(n), chainId(n);
    vector<Chain> chains;
    for (int i = 0; i < n; ++i) {
        int v = order[i];
        if (parent[v] == -1 || heavy[parent[v]] != v) {
            int id = (int)chains.size();
            chains.push_back({});
            int length = 0;
            for (int u = v; u != -1; u = heavy[u]) ++length;
            chains[id].size = length;
        }
    }
    int currentChain = -1;
    for (int i = 0; i < n; ++i) {
        int v = order[i];
        if (parent[v] == -1 || heavy[parent[v]] != v) {
            ++currentChain;
            int p = 0;
            for (int u = v; u != -1; u = heavy[u]) {
                head[u] = v;
                chainId[u] = currentChain;
                position[u] = p++;
            }
        }
    }

    int totalCapacity = 0;
    for (Chain &chain : chains) {
        chain.offset = totalCapacity;
        chain.capacity = 1;
        while (chain.capacity < chain.size) chain.capacity <<= 1;
        totalCapacity += 2 * chain.capacity;
    }
    vector<Matrix> segments(totalCapacity, {1, 0, 0, 0});

    vector<int> productNonzero(n, 1), sumRatio(n, 0), zeroCount(n, 0), zeroBSum(n, 0);
    vector<unsigned char> processed(n, 0);
    vector<int> dpA(n, 1), dpB(n, 0);

    auto lightProduct = [&](int v) {
        return zeroCount[v] == 0 ? productNonzero[v] : 0;
    };

    auto lightSum = [&](int v) {
        if (zeroCount[v] == 0) return mulmod(productNonzero[v], sumRatio[v]);
        if (zeroCount[v] == 1) return mulmod(productNonzero[v], zeroBSum[v]);
        return 0;
    };

    auto vertexMatrix = [&](int v) {
        int p = lightProduct(v);
        if (!processed[v]) return Matrix{p, 0, 0, 0};
        int dp = mulmod((int)tree[v].size(), p);
        return Matrix{submod(dp, lightSum(v)), p == 0 ? 0 : MOD - p, p, 0};
    };

    auto setMatrix = [&](int v, const Matrix &matrix) {
        int id = chainId[v];
        int p = chains[id].capacity + position[v];
        segments[chains[id].offset + p] = matrix;
        while (p > 1) {
            p >>= 1;
            segments[chains[id].offset + p] = multiply(
                segments[chains[id].offset + (p << 1)],
                segments[chains[id].offset + (p << 1 | 1)]
            );
        }
    };

    for (int v = 0; v < n; ++v) setMatrix(v, vertexMatrix(v));

    auto changeLightChild = [&](int v, int oldA, int oldB, int newA, int newB) {
        if (oldA == newA && oldB == newB) return;
        if (oldA == 0) {
            --zeroCount[v];
            zeroBSum[v] = submod(zeroBSum[v], oldB);
        } else {
            int inverse = oldA == 1 ? 1 : powmod(oldA, MOD - 2);
            productNonzero[v] = mulmod(productNonzero[v], inverse);
            sumRatio[v] = submod(sumRatio[v], mulmod(oldB, inverse));
        }
        if (newA == 0) {
            ++zeroCount[v];
            zeroBSum[v] = addmod(zeroBSum[v], newB);
        } else {
            int inverse = newA == 1 ? 1 : powmod(newA, MOD - 2);
            productNonzero[v] = mulmod(productNonzero[v], newA);
            sumRatio[v] = addmod(sumRatio[v], mulmod(newB, inverse));
        }
    };

    auto activate = [&](int v) {
        processed[v] = 1;
        while (true) {
            int h = head[v];
            int oldA = dpA[h], oldB = dpB[h];
            setMatrix(v, vertexMatrix(v));
            const Matrix &whole = segments[chains[chainId[h]].offset + 1];
            dpA[h] = whole.a;
            dpB[h] = whole.c;
            int p = parent[h];
            if (p == -1) break;
            changeLightChild(p, oldA, oldB, dpA[h], dpB[h]);
            v = p;
        }
    };

    vector<int> powersOfTwo(n + 1, 1);
    for (int i = 1; i <= n; ++i) powersOfTwo[i] = addmod(powersOfTwo[i - 1], powersOfTwo[i - 1]);

    int exponent = 0;
    for (int step = 0; step < n - 1; ++step) {
        int x;
        cin >> x;
        --x;
        int processedNeighbors = 0;
        for (int u : tree[x]) processedNeighbors += processed[u];
        exponent += (int)tree[x].size() - processedNeighbors - 1;
        activate(x);
        cout << mulmod(powersOfTwo[exponent], dpA[0]) << '\n';
    }

    return 0;
}
