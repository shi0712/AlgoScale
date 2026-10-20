#include <bits/stdc++.h>
using namespace std;

static const int INF = 1000000000;

struct SegDP {
    int n, N, base;
    vector<int> a;
    vector<int> t00, t01, t10, t11;

    SegDP(int _n, const vector<int>& _a) : n(_n), a(_a) {
        N = max(0, n - 1);
        base = 1;
        while (base < max(1, N)) base <<= 1;

        int sz = 2 * base;
        t00.assign(sz, INF);
        t01.assign(sz, INF);
        t10.assign(sz, INF);
        t11.assign(sz, INF);

        // Extra leaves are identity matrices.
        for (int idx = N; idx < base; ++idx) {
            int node = base + idx;
            t00[node] = 0;
            t11[node] = 0;
            t01[node] = t10[node] = INF;
        }
    }

    inline int mn(int x, int y) {
        return x < y ? x : y;
    }

    inline void pull(int node) {
        int l = node << 1;
        int r = l | 1;

        t00[node] = mn(t00[l] + t00[r], t01[l] + t10[r]);
        t01[node] = mn(t00[l] + t01[r], t01[l] + t11[r]);
        t10[node] = mn(t10[l] + t00[r], t11[l] + t10[r]);
        t11[node] = mn(t10[l] + t01[r], t11[l] + t11[r]);
    }

    // Leaf idx represents transition from position idx to idx+1, 0-based.
    void setLeaf(int idx, bool allowed_low, int d) {
        int delta = a[idx] - a[idx + 1];

        int c01 = delta + d;
        if (c01 < 0) c01 = 0;

        int c11 = delta;
        if (c11 < 0) c11 = 0;

        int node = base + idx;
        t01[node] = c01;
        t11[node] = c11;

        if (allowed_low) {
            int c00 = delta;
            if (c00 < 0) c00 = 0;

            int c10 = delta - d;
            if (c10 < 0) c10 = 0;

            t00[node] = c00;
            t10[node] = c10;
        } else {
            t00[node] = INF;
            t10[node] = INF;
        }
    }

    void build(int d, int L) {
        for (int idx = 0; idx < N; ++idx) {
            setLeaf(idx, a[idx + 1] <= L, d);
        }
        for (int node = base - 1; node >= 1; --node) {
            pull(node);
        }
    }

    void updateAllowed(int idx, int d) {
        setLeaf(idx, true, d);

        int node = (base + idx) >> 1;
        while (node) {
            pull(node);
            node >>= 1;
        }
    }

    int query(int p, int d) {
        int dp0 = (a[0] <= p ? -a[0] : INF);
        int dp1 = d - a[0];

        int P00 = t00[1], P01 = t01[1];
        int P10 = t10[1], P11 = t11[1];

        int f0 = mn(dp0 + P00, dp1 + P10);
        int f1 = mn(dp0 + P01, dp1 + P11);

        int H = mn(f0, f1);
        return p + H;
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int T;
    cin >> T;

    while (T--) {
        int n, m;
        cin >> n >> m;

        vector<int> a(n);
        int M = 0, mnA = INF;

        for (int i = 0; i < n; ++i) {
            cin >> a[i];
            M = max(M, a[i]);
            mnA = min(mnA, a[i]);
        }

        vector<int> vals = a;
        sort(vals.begin(), vals.end());
        vals.erase(unique(vals.begin(), vals.end()), vals.end());

        if ((int)vals.size() <= m) {
            cout << 0 << '\n';
            continue;
        }

        // Cost if all final values are equal to M.
        int oneValueCost = M - a[0];
        for (int i = 1; i < n; ++i) {
            if (a[i - 1] > a[i]) {
                oneValueCost += a[i - 1] - a[i];
            }
        }

        if (m == 1) {
            cout << oneValueCost << '\n';
            continue;
        }

        int V = M - mnA;
        int ans = oneValueCost;

        vector<vector<int>> bucket(M + 1);

        // Leaf idx corresponds to current position idx+1.
        for (int idx = 0; idx < n - 1; ++idx) {
            bucket[a[idx + 1]].push_back(idx);
        }

        SegDP seg(n, a);

        for (int d = 1; d <= V; ++d) {
            int L = M - d; // minimum possible lower final value

            seg.build(d, L);
            ans = min(ans, seg.query(L, d));

            for (int v : vals) {
                if (v <= L) continue;

                for (int idx : bucket[v]) {
                    seg.updateAllowed(idx, d);
                }

                ans = min(ans, seg.query(v, d));
            }
        }

        cout << ans << '\n';
    }

    return 0;
}
