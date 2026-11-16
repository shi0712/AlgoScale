#include <bits/stdc++.h>
using namespace std;

static constexpr int MOD = 1000000007;

struct Solver {
    int n, points, stride, treeSize;
    vector<long long> l, r, coord;
    vector<int> type, inv, invFact;
    vector<int> tree;
    long long total = 1;
    int answer = 0;

    static int norm(long long x) {
        x %= MOD;
        if (x < 0) x += MOD;
        return static_cast<int>(x);
    }

    int status(int i, long long x) const {
        if (x <= l[i]) return 0;
        if (x > r[i]) return 2;
        return 1;
    }

    void setLeaf(int node, int i, int kind) {
        int* out = tree.data() + (static_cast<size_t>(node) * stride) * 9;
        const int len = norm(r[i] - l[i] + 1);
        for (int x = 0; x < points; ++x, out += 9) {
            fill(out, out + 9, 0);
            if (kind == 0) {
                out[1] = len;
            } else if (kind == 2) {
                out[0] = len;
                out[5] = len;
                out[6] = len;
            } else {
                const int low = norm(x - l[i]);
                const int high = norm(r[i] + 1 - x);
                out[0] = low;
                out[1] = high;
                out[5] = low;
                out[6] = low;
            }
        }
    }

    void multiplyNode(int node, int degree) {
        const int left = node * 2, right = left + 1;
        const int* a = tree.data() + (static_cast<size_t>(left) * stride) * 9;
        const int* b = tree.data() + (static_cast<size_t>(right) * stride) * 9;
        int* out = tree.data() + (static_cast<size_t>(node) * stride) * 9;
        for (int x = 0; x <= degree; ++x, a += 9, b += 9, out += 9) {
            for (int i = 0; i < 3; ++i) {
                const long long a0 = a[3 * i];
                const long long a1 = a[3 * i + 1];
                const long long a2 = a[3 * i + 2];
                for (int j = 0; j < 3; ++j) {
                    out[3 * i + j] = (a0 * b[j] + a1 * b[3 + j] + a2 * b[6 + j]) % MOD;
                }
            }
        }
    }

    void updatePosition(int i, int kind) {
        int node = treeSize + i;
        setLeaf(node, i, kind);
        while ((node >>= 1) != 0) {
            multiplyNode(node, points - 1);
        }
    }

    int interpolate(const vector<int>& values, long long argument) const {
        const int m = static_cast<int>(values.size()) - 1;
        if (argument <= m) return values[argument];
        vector<int> prefix(m + 2, 1), suffix(m + 2, 1);
        for (int i = 0; i <= m; ++i) {
            prefix[i + 1] = static_cast<long long>(prefix[i]) * norm(argument - i) % MOD;
        }
        for (int i = m; i >= 0; --i) {
            suffix[i] = static_cast<long long>(suffix[i + 1]) * norm(argument - i) % MOD;
        }
        int result = 0;
        for (int i = 0; i <= m; ++i) {
            long long term = static_cast<long long>(values[i]) * prefix[i] % MOD * suffix[i + 1] % MOD;
            term = term * invFact[i] % MOD * invFact[m - i] % MOD;
            if ((m - i) & 1) result = (result + MOD - term % MOD) % MOD;
            else result = (result + term) % MOD;
        }
        return result;
    }

    int rangeSum(const vector<int>& values, long long L, long long R) const {
        vector<int> antiderivative(values.size() + 1, 0);
        for (int i = 0; i < static_cast<int>(values.size()); ++i) {
            antiderivative[i + 1] = antiderivative[i] + values[i];
            if (antiderivative[i + 1] >= MOD) antiderivative[i + 1] -= MOD;
        }
        int result = interpolate(antiderivative, R + 1) - interpolate(antiderivative, L);
        if (result < 0) result += MOD;
        return result;
    }

    void processSegment(int s) {
        const long long L = coord[s], R = coord[s + 1] - 1;
        const int degree = points - 1;
        const int* root = tree.data() + stride * 9;
        vector<int> values(degree + 1);
        for (int x = 0; x <= degree; ++x, root += 9) {
            values[x] = root[0] + root[1];
            if (values[x] >= MOD) values[x] -= MOD;
            values[x] += root[2];
            if (values[x] >= MOD) values[x] -= MOD;
        }
        int contribution = static_cast<long long>(norm(R - L + 1)) * total % MOD;
        contribution -= rangeSum(values, L, R);
        if (contribution < 0) contribution += MOD;
        answer += contribution;
        if (answer >= MOD) answer -= MOD;
    }

    int run() {
        cin >> n;
        l.resize(n);
        r.resize(n);
        for (long long& x : l) cin >> x;
        for (long long& x : r) cin >> x;

        long long maxR = 0;
        coord = {1};
        for (int i = 0; i < n; ++i) {
            total = total * norm(r[i] - l[i] + 1) % MOD;
            maxR = max(maxR, r[i]);
            coord.push_back(l[i] + 1);
            coord.push_back(r[i] + 1);
        }
        coord.push_back(maxR + 1);
        sort(coord.begin(), coord.end());
        coord.erase(unique(coord.begin(), coord.end()), coord.end());

        inv.resize(n + 4);
        invFact.resize(n + 4);
        inv[1] = 1;
        invFact[0] = invFact[1] = 1;
        for (int i = 2; i < static_cast<int>(inv.size()); ++i) {
            inv[i] = MOD - static_cast<long long>(MOD / i) * inv[MOD % i] % MOD;
        }
        for (int i = 2; i < static_cast<int>(invFact.size()); ++i) {
            invFact[i] = static_cast<long long>(invFact[i - 1]) * inv[i] % MOD;
        }

        treeSize = 1;
        while (treeSize < n) treeSize *= 2;
        points = stride = n + 1;
        tree.assign(static_cast<size_t>(2 * treeSize) * stride * 9, 0);
        type.resize(n);

        for (int i = 0; i < n; ++i) {
            type[i] = status(i, coord[0]);
            setLeaf(treeSize + i, i, type[i]);
        }
        for (int i = n; i < treeSize; ++i) {
            int* out = tree.data() + (static_cast<size_t>(treeSize + i) * points) * 9;
            for (int x = 0; x < points; ++x, out += 9) {
                out[0] = out[4] = out[8] = 1;
            }
        }
        for (int node = treeSize - 1; node >= 1; --node) {
            multiplyNode(node, n);
        }

        for (int s = 0; s + 1 < static_cast<int>(coord.size()); ++s) {
            processSegment(s);
            if (s + 2 == static_cast<int>(coord.size())) break;
            const long long nextX = coord[s + 1];
            vector<int> nextTypes(n);
            for (int i = 0; i < n; ++i) {
                nextTypes[i] = status(i, nextX);
            }
            for (int i = 0; i < n; ++i) {
                if (nextTypes[i] != type[i]) {
                    updatePosition(i, nextTypes[i]);
                    type[i] = nextTypes[i];
                }
            }
        }
        return answer;
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int T;
    cin >> T;
    while (T--) {
        Solver solver;
        cout << solver.run() << '\n';
    }
    return 0;
}
