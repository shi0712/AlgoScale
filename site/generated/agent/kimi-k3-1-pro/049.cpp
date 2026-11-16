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
    return static_cast<int>(1LL * a * b % MOD);
}

int powmod(int a, int e) {
    int r = 1;
    while (e > 0) {
        if (e & 1) r = mulmod(r, a);
        a = mulmod(a, a);
        e >>= 1;
    }
    return r;
}

int solve(const vector<int>& p) {
    int n = static_cast<int>(p.size()) - 1;
    if (n == 1) return 0;

    bool sorted = true;
    for (int i = 1; i <= n; ++i) {
        if (p[i] != i) {
            sorted = false;
            break;
        }
    }
    if (sorted) return 0;

    int x = powmod(n - 1, MOD - 2);
    int a = addmod(1, x);
    int b = addmod(1, mulmod(2, x));

    vector<vector<int>> U(n, vector<int>(n));
    vector<vector<int>> V(n, vector<int>(n));
    U[0][0] = 1;
    for (int u = 0; u < n; ++u) {
        for (int v = 0; v < n; ++v) {
            if (u == 0 && v == 0) continue;
            int value = 0;
            if (u > 0) value = addmod(value, mulmod(a, U[u - 1][v]));
            if (v > 0) value = addmod(value, mulmod(a, U[u][v - 1]));
            if (u > 0 && v > 0) {
                value = submod(value, mulmod(b, U[u - 1][v - 1]));
            }
            U[u][v] = value;
        }
    }
    for (int u = 0; u < n; ++u) {
        for (int v = 0; v < n; ++v) {
            V[u][v] = U[u][v];
            if (u > 0) {
                V[u][v] = addmod(V[u][v],
                    submod(mulmod(a, V[u - 1][v]), U[u - 1][v]));
            }
        }
    }

    vector<int> powerA(n);
    powerA[0] = 1;
    for (int i = 1; i < n; ++i) powerA[i] = mulmod(powerA[i - 1], a);

    vector<int> suffixMin(n + 2, n + 1);
    for (int i = n; i >= 1; --i) suffixMin[i] = min(suffixMin[i + 1], p[i]);

    vector<char> inPrefix(n + 1, false);
    int maxPrefix = 0;
    int sumH = 0;

    for (int cut = 1; cut < n; ++cut) {
        inPrefix[p[cut]] = true;
        maxPrefix = max(maxPrefix, p[cut]);
        int low = suffixMin[cut + 1];
        int high = maxPrefix;
        if (low > high) continue;

        int totalA = 0;
        for (int value = low + 1; value < high; ++value) {
            totalA += inPrefix[value];
        }

        int H = powerA[totalA];
        int seenA = 0;
        int lowerB = 0;
        for (int value = low + 1; value < high; ++value) {
            if (inPrefix[value]) {
                ++seenA;
            } else {
                int upperA = totalA - seenA;
                H = addmod(H, mulmod(x, V[upperA][lowerB]));
                ++lowerB;
            }
        }
        sumH = addmod(sumH, H);
    }

    return addmod(1, mulmod(x, sumH));
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int T;
    cin >> T;
    while (T--) {
        int n;
        cin >> n;
        vector<int> p(n + 1);
        for (int i = 1; i <= n; ++i) cin >> p[i];
        cout << solve(p) << '\n';
    }
    return 0;
}
