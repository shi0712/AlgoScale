#include <bits/stdc++.h>
using namespace std;

static constexpr int MOD = 1000000007;

int modpow(int a, long long e) {
    long long r = 1, x = a;
    while (e) {
        if (e & 1) r = r * x % MOD;
        x = x * x % MOD;
        e >>= 1;
    }
    return int(r);
}

struct Mat {
    // Row-vector transfer matrix.
    uint32_t a[9];
};

Mat identityMat() {
    Mat m{};
    m.a[0] = m.a[4] = m.a[8] = 1;
    return m;
}

Mat multiply(const Mat& A, const Mat& B) {
    Mat C;
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            unsigned long long s =
                (unsigned long long)A.a[3 * i] * B.a[j] +
                (unsigned long long)A.a[3 * i + 1] * B.a[3 + j] +
                (unsigned long long)A.a[3 * i + 2] * B.a[6 + j];
            C.a[3 * i + j] = uint32_t(s % MOD);
        }
    }
    return C;
}

Mat leafMat(int L, int H) {
    // States:
    // 0: no high element among the last two positions
    // 1: the last position is high
    // 2: the position before the last one is high
    Mat m{};
    m.a[0] = L; m.a[1] = H; m.a[2] = 0;
    m.a[3] = 0; m.a[4] = 0; m.a[5] = L;
    m.a[6] = L; m.a[7] = 0; m.a[8] = 0;
    return m;
}

struct SegTree {
    int n, sz;
    vector<Mat> tr;

    explicit SegTree(int n_) : n(n_) {
        sz = 1;
        while (sz < n) sz <<= 1;
        tr.resize(2 * sz);
    }

    void build(const vector<Mat>& leaves) {
        fill(tr.begin(), tr.end(), identityMat());
        for (int i = 0; i < n; ++i) tr[sz + i] = leaves[i];
        for (int i = sz - 1; i; --i)
            tr[i] = multiply(tr[i << 1], tr[i << 1 | 1]);
    }

    void update(int pos, const Mat& m) {
        int p = sz + pos;
        tr[p] = m;
        for (p >>= 1; p; p >>= 1)
            tr[p] = multiply(tr[p << 1], tr[p << 1 | 1]);
    }

    int value() const {
        // Initial vector is (1,0,0), and all final states are accepted.
        return int(((unsigned long long)tr[1].a[0] +
                    tr[1].a[1] + tr[1].a[2]) % MOD);
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int T;
    cin >> T;

    while (T--) {
        int n;
        cin >> n;

        vector<long long> l(n), r(n);
        for (auto& x : l) cin >> x;
        for (auto& x : r) cin >> x;

        vector<int> len(n);
        long long maxR = 0;
        int total = 1;

        for (int i = 0; i < n; ++i) {
            len[i] = int(r[i] - l[i] + 1);
            total = int((long long)total * len[i] % MOD);
            maxR = max(maxR, r[i]);
        }

        // Threshold segments.
        vector<long long> xs = {1};
        for (int i = 0; i < n; ++i) {
            xs.push_back(l[i]);
            xs.push_back(r[i] + 1);
        }
        sort(xs.begin(), xs.end());
        xs.erase(unique(xs.begin(), xs.end()), xs.end());

        int C = int(xs.size());
        int m = C - 1; // Segment s is [xs[s], xs[s+1]-1].

        vector<vector<int>> events(C);
        vector<int> delta(C, 0);

        for (int i = 0; i < n; ++i) {
            int p = int(lower_bound(xs.begin(), xs.end(), l[i]) - xs.begin());
            int q = int(lower_bound(xs.begin(), xs.end(), r[i] + 1) - xs.begin());
            events[p].push_back(i);
            events[q].push_back(i);
            ++delta[p];
            --delta[q];
        }

        // Number of affine positions on each segment = polynomial degree bound.
        vector<int> deg(m);
        deg[0] = delta[0];
        for (int s = 1; s < m; ++s)
            deg[s] = deg[s - 1] + delta[s];

        int maxDeg = *max_element(deg.begin(), deg.end());

        auto normalize = [](long long x) {
            x %= MOD;
            if (x < 0) x += MOD;
            return int(x);
        };

        // Weights at interpolation point t, using the regime at segment start A.
        auto getWeights = [&](int i, int t, long long A) -> pair<int, int> {
            if (A < l[i]) {
                return {0, len[i]};
            }
            if (A <= r[i]) {
                // Affine formulas, intentionally also used outside the segment.
                return {
                    normalize((long long)t - l[i]),
                    normalize(r[i] + 1 - (long long)t)
                };
            }
            return {len[i], 0};
        };

        auto directEval = [&](int s, int t) {
            long long A = xs[s];
            int s0 = 1, s1 = 0, s2 = 0;

            for (int i = 0; i < n; ++i) {
                auto [L, H] = getWeights(i, t, A);
                int n0 = int((long long)L * ((s0 + s2) % MOD) % MOD);
                int n1 = int((long long)H * s0 % MOD);
                int n2 = int((long long)L * s1 % MOD);
                s0 = n0;
                s1 = n1;
                s2 = n2;
            }
            return int(((long long)s0 + s1 + s2) % MOD);
        };

        // values[s*(n+1)+t] = P_s(t), for 0 <= t <= deg[s].
        vector<int> values((size_t)m * (n + 1), 0);

        vector<int> first(maxDeg + 1, m);
        vector<int> last(maxDeg + 1, -1);
        vector<int> needed(maxDeg + 1, 0);

        for (int s = 0; s < m; ++s) {
            for (int t = 0; t <= deg[s]; ++t) {
                if (first[t] == m) first[t] = s;
                last[t] = s;
                ++needed[t];
            }
        }

        SegTree segTree(n);
        vector<Mat> leaves(n);

        for (int t = 0; t <= maxDeg; ++t) {
            if (needed[t] == 0) continue;

            // This avoids segment-tree overhead when only a few evaluations
            // at the current interpolation point are required.
            if (needed[t] <= 20) {
                for (int s = 0; s < m; ++s) {
                    if (deg[s] >= t) {
                        values[(size_t)s * (n + 1) + t] = directEval(s, t);
                    }
                }
                continue;
            }

            int f = first[t], b = last[t];

            for (int i = 0; i < n; ++i) {
                auto [L, H] = getWeights(i, t, xs[f]);
                leaves[i] = leafMat(L, H);
            }
            segTree.build(leaves);

            for (int s = f; s <= b; ++s) {
                if (deg[s] >= t) {
                    values[(size_t)s * (n + 1) + t] = segTree.value();
                }

                if (s == b) break;

                for (int pos : events[s + 1]) {
                    auto [L, H] = getWeights(pos, t, xs[s + 1]);
                    segTree.update(pos, leafMat(L, H));
                }
            }
        }

        // Factorials for Lagrange interpolation.
        vector<int> fact(n + 3), invfact(n + 3);
        fact[0] = 1;
        for (int i = 1; i <= n + 2; ++i)
            fact[i] = int((long long)fact[i - 1] * i % MOD);

        invfact[n + 2] = modpow(fact[n + 2], MOD - 2);
        for (int i = n + 2; i; --i)
            invfact[i - 1] = int((long long)invfact[i] * i % MOD);

        vector<int> Q(n + 2), pref(n + 3), suff(n + 3);

        // Q is the prefix-sum polynomial of the current P_s.
        // Its values at nodes 0..N are interpolated, N=deg[s]+1.
        auto evaluateQ = [&](int N, long long X) -> int {
            if (X <= N) return Q[int(X)];

            pref[0] = 1;
            for (int i = 0; i <= N; ++i)
                pref[i + 1] = int((long long)pref[i] * (X - i) % MOD);

            suff[N + 1] = 1;
            for (int i = N; i >= 0; --i)
                suff[i] = int((long long)suff[i + 1] * (X - i) % MOD);

            int ans = 0;
            for (int i = 0; i <= N; ++i) {
                long long term = (long long)Q[i] * pref[i] % MOD;
                term = term * suff[i + 1] % MOD;
                term = term * invfact[i] % MOD;
                term = term * invfact[N - i] % MOD;

                if ((N - i) & 1)
                    ans = int((ans - term + MOD) % MOD);
                else
                    ans = int((ans + term) % MOD);
            }
            return ans;
        };

        int sumBad = 0;

        for (int s = 0; s < m; ++s) {
            int d = deg[s];

            Q[0] = 0;
            for (int t = 0; t <= d; ++t) {
                Q[t + 1] = Q[t] + values[(size_t)s * (n + 1) + t];
                if (Q[t + 1] >= MOD) Q[t + 1] -= MOD;
            }

            int N = d + 1;
            int left = evaluateQ(N, xs[s]);
            int right = evaluateQ(N, xs[s + 1]);

            sumBad = int(((long long)sumBad + right - left + MOD) % MOD);
        }

        long long answer = (long long)total * (maxR % MOD) % MOD;
        answer = (answer - sumBad + MOD) % MOD;
        cout << answer << '\n';
    }

    return 0;
}
