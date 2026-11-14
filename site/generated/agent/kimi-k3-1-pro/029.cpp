#include <bits/stdc++.h>
using namespace std;

static constexpr int MOD = 1000000007;

struct Solver {
    int n, size = 1, maxR = 0;
    vector<int> l, r, len, type;
    vector<int> fact, invFact;
    vector<int> values;
    vector<vector<int>> events;

    int &entry(int node, int point, int e) {
        return values[(node * (n + 2) + point) * 9 + e];
    }
    const int &entry(int node, int point, int e) const {
        return values[(node * (n + 2) + point) * 9 + e];
    }

    void multiplyNode(int node, int point) {
        int a = node << 1, b = a | 1;
        const int *A = &entry(a, point, 0);
        const int *B = &entry(b, point, 0);
        int *C = &entry(node, point, 0);
        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 3; ++j) {
                unsigned long long sum = 0;
                for (int k = 0; k < 3; ++k)
                    sum += (unsigned long long)A[i * 3 + k] * B[k * 3 + j];
                C[i * 3 + j] = sum % MOD;
            }
        }
    }

    void setLeaf(int i, int point) {
        int node = size + i;
        int L, H;
        if (type[i] == 0) {
            L = 0;
            H = len[i];
        } else if (type[i] == 2) {
            L = len[i];
            H = 0;
        } else {
            L = (point - l[i]) % MOD;
            if (L < 0) L += MOD;
            H = (r[i] - point + 1) % MOD;
            if (H < 0) H += MOD;
        }
        int *M = &entry(node, point, 0);
        M[0] = L; M[1] = H; M[2] = 0;
        M[3] = 0; M[4] = 0; M[5] = L;
        M[6] = L; M[7] = 0; M[8] = 0;
    }

    void setIdentity(int node, int point) {
        int *M = &entry(node, point, 0);
        M[0] = M[4] = M[8] = 1;
        M[1] = M[2] = M[3] = M[5] = M[6] = M[7] = 0;
    }

    void computePoint(int point) {
        for (int i = 0; i < n; ++i) setLeaf(i, point);
        for (int i = n; i < size; ++i) setIdentity(size + i, point);
        for (int node = size - 1; node >= 1; --node)
            multiplyNode(node, point);
    }

    void updatePosition(int i, int points) {
        int node = size + i;
        for (int p = 0; p < points; ++p) setLeaf(i, p);
        for (node >>= 1; node; node >>= 1)
            for (int p = 0; p < points; ++p) multiplyNode(node, p);
    }

    int prefixSum(int degree, long long x, const vector<int> &q) const {
        if (x < 0) return 0;
        int d = degree + 1;
        if (x <= d) return q[(int)x];

        int X = x % MOD;
        vector<int> pre(d + 1), suf(d + 1);
        pre[0] = 1;
        for (int i = 1; i <= d; ++i)
            pre[i] = (long long)pre[i - 1] * (X - (i - 1)) % MOD;
        suf[d] = 1;
        for (int i = d - 1; i >= 0; --i)
            suf[i] = (long long)suf[i + 1] * (X - (i + 1)) % MOD;

        int ans = 0;
        for (int i = 0; i <= d; ++i) {
            int term = (long long)q[i] * pre[i] % MOD * suf[i] % MOD;
            term = (long long)term * invFact[i] % MOD * invFact[d - i] % MOD;
            if ((d - i) & 1) ans = (ans - term + MOD) % MOD;
            else ans = (ans + term) % MOD;
        }
        return ans;
    }

    int rangeSum(int degree, int left, int right) const {
        vector<int> q(degree + 2);
        int sum = 0;
        for (int i = 0; i <= degree + 1; ++i) {
            const int *M = &entry(1, i, 0);
            sum = (sum + (long long)M[0] + M[1] + M[2]) % MOD;
            q[i] = sum;
        }
        int ans = prefixSum(degree, right, q) - prefixSum(degree, left - 1LL, q);
        if (ans < 0) ans += MOD;
        return ans;
    }

    int run() {
        cin >> n;
        l.resize(n); r.resize(n); len.resize(n); type.assign(n, 0);
        for (int &x : l) cin >> x;
        for (int &x : r) {
            cin >> x;
            maxR = max(maxR, x);
        }

        while (size < n) size <<= 1;
        values.assign((2 * size * (n + 2)) * 9, 0);

        fact.resize(n + 2);
        invFact.resize(n + 2);
        fact[0] = 1;
        for (int i = 1; i <= n + 1; ++i) fact[i] = (long long)fact[i - 1] * i % MOD;
        invFact[n + 1] = 1;
        long long base = fact[n + 1], e = MOD - 2;
        while (e) {
            if (e & 1) invFact[n + 1] = invFact[n + 1] * base % MOD;
            base = base * base % MOD;
            e >>= 1;
        }
        for (int i = n + 1; i; --i) invFact[i - 1] = (long long)invFact[i] * i % MOD;

        int total = 1;
        vector<int> coords;
        coords.reserve(2 * n);
        for (int i = 0; i < n; ++i) {
            len[i] = r[i] - l[i] + 1;
            total = (long long)total * len[i] % MOD;
            coords.push_back(l[i] + 1);
            coords.push_back(r[i] + 1);
        }
        sort(coords.begin(), coords.end());
        coords.erase(unique(coords.begin(), coords.end()), coords.end());
        events.resize(coords.size());
        for (int i = 0; i < n; ++i) {
            if (l[i] < r[i]) {
                int at = lower_bound(coords.begin(), coords.end(), l[i] + 1) - coords.begin();
                events[at].push_back(i);
            }
            int at = lower_bound(coords.begin(), coords.end(), r[i] + 1) - coords.begin();
            events[at].push_back(i);
        }

        int cur = 1, active = 0;
        computePoint(0);
        computePoint(1);
        long long badSum = 0;
        for (int eventIndex = 0; eventIndex < (int)coords.size(); ++eventIndex) {
            int next = coords[eventIndex];
            if (next > maxR + 1) break;
            if (next > cur) {
                int bad = rangeSum(active, cur, next - 1);
                badSum = (badSum + bad) % MOD;
                cur = next;
            }
            if (cur > maxR) break;

            int oldActive = active;
            vector<int> changed;
            for (int i : events[eventIndex]) {
                if (type[i] == 0) {
                    if (l[i] < r[i]) {
                        type[i] = 1;
                        ++active;
                    } else {
                        type[i] = 2;
                    }
                } else {
                    type[i] = 2;
                    --active;
                }
                changed.push_back(i);
            }
            if (active > oldActive) {
                for (int p = oldActive + 2; p <= active + 1; ++p)
                    computePoint(p);
            }
            int points = min(oldActive, active) + 2;
            for (int i : changed) updatePosition(i, points);
        }

        return ((long long)total * (maxR % MOD) - badSum) % MOD;
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int T;
    cin >> T;
    while (T--) {
        Solver solver;
        int ans = solver.run();
        if (ans < 0) ans += MOD;
        cout << ans << '\n';
    }
}
