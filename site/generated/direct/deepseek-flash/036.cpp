#include <bits/stdc++.h>
using namespace std;

static const long long INF64 = (long long)4e18;

struct LiChaoRange {
    int n, mode;
    vector<int> line;
    vector<int> *x, *p;

    LiChaoRange() {}
    LiChaoRange(int _n, int _mode, vector<int>* _x, vector<int>* _p)
        : n(_n), mode(_mode), x(_x), p(_p) {
        line.assign(4 * n + 10, -1);
    }

    void reset() {
        fill(line.begin(), line.end(), -1);
    }

    double evalD(int id, int pos) {
        int xi = (*x)[id], xq = (*x)[pos], pi = (*p)[id];
        if (mode == 0) {
            double d = (double)xi - xq;
            if (d < 0 && d > -1e-9) d = 0;
            return 2.0 * xi + 2.0 * pi + 2.0 * sqrt(2.0 * pi * d);
        } else {
            double d = (double)xq - xi;
            if (d < 0 && d > -1e-9) d = 0;
            return -2.0 * xi + 2.0 * pi + 2.0 * sqrt(2.0 * pi * d);
        }
    }

    long double evalL(int id, int pos) {
        int xi = (*x)[id], xq = (*x)[pos], pi = (*p)[id];
        if (mode == 0) {
            long double d = (long double)xi - xq;
            if (d < 0) d = 0;
            return 2.0L * xi + 2.0L * pi + 2.0L * sqrtl(2.0L * pi * d);
        } else {
            long double d = (long double)xq - xi;
            if (d < 0) d = 0;
            return -2.0L * xi + 2.0L * pi + 2.0L * sqrtl(2.0L * pi * d);
        }
    }

    void addLine(int node, int l, int r, int id) {
        int cur = line[node];
        if (cur == -1) {
            line[node] = id;
            return;
        }

        int mid = (l + r) >> 1;
        bool betterMid = evalD(id, mid) < evalD(cur, mid);
        if (betterMid) {
            swap(line[node], id);
        }

        if (l == r) return;

        if (evalD(id, l) < evalD(line[node], l)) {
            addLine(node << 1, l, mid, id);
        } else if (evalD(id, r) < evalD(line[node], r)) {
            addLine(node << 1 | 1, mid + 1, r, id);
        }
    }

    void insertRange(int node, int l, int r, int ql, int qr, int id) {
        if (ql <= l && r <= qr) {
            addLine(node, l, r, id);
            return;
        }
        int mid = (l + r) >> 1;
        if (ql <= mid) insertRange(node << 1, l, mid, ql, qr, id);
        if (qr > mid) insertRange(node << 1 | 1, mid + 1, r, ql, qr, id);
    }

    void insert(int ql, int qr, int id) {
        if (ql > qr) return;
        insertRange(1, 1, n, ql, qr, id);
    }

    long double query(int pos) {
        long double ans = 1e100L;
        int node = 1, l = 1, r = n;

        while (true) {
            if (line[node] != -1) {
                long double v = evalL(line[node], pos);
                if (v < ans) ans = v;
            }
            if (l == r) break;

            int mid = (l + r) >> 1;
            if (pos <= mid) {
                node <<= 1;
                r = mid;
            } else {
                node = node << 1 | 1;
                l = mid + 1;
            }
        }
        return ans;
    }
};

struct BlockLiChao {
    int Xmax;
    int beta;
    vector<int> line;
    vector<int> *x;
    vector<long long> *dp;

    BlockLiChao() {}
    BlockLiChao(int _Xmax, int _beta, vector<int>* _x, vector<long long>* _dp)
        : Xmax(_Xmax), beta(_beta), x(_x), dp(_dp) {
        line.assign(4 * (Xmax + 5), -1);
    }

    void reset() {
        fill(line.begin(), line.end(), -1);
    }

    long long eval(int id, int xq) {
        long long diff = (long long)xq - (*x)[id];
        __int128 val = (__int128)(*dp)[id] + (__int128)beta * diff * diff;
        if (val > INF64) return INF64;
        return (long long)val;
    }

    void addLine(int node, int l, int r, int id) {
        int cur = line[node];
        if (cur == -1) {
            line[node] = id;
            return;
        }

        int mid = (l + r) >> 1;
        if (eval(id, mid) < eval(cur, mid)) {
            swap(line[node], id);
        }

        if (l == r) return;

        if (eval(id, l) < eval(line[node], l)) {
            addLine(node << 1, l, mid, id);
        } else if (eval(id, r) < eval(line[node], r)) {
            addLine(node << 1 | 1, mid + 1, r, id);
        }
    }

    void addLine(int id) {
        addLine(1, 0, Xmax, id);
    }

    long long query(int xq) {
        long long ans = INF64;
        int node = 1, l = 0, r = Xmax;

        while (true) {
            if (line[node] != -1) {
                ans = min(ans, eval(line[node], xq));
            }
            if (l == r) break;

            int mid = (l + r) >> 1;
            if (xq <= mid) {
                node <<= 1;
                r = mid;
            } else {
                node = node << 1 | 1;
                l = mid + 1;
            }
        }
        return ans;
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, h, alpha, beta;
    cin >> n >> h >> alpha >> beta;

    vector<int> x(n + 5), y(n + 5), p(n + 5);
    for (int i = 1; i <= n; ++i) {
        cin >> x[i] >> y[i];
        p[i] = h - y[i];
    }

    vector<int> R(n + 5), L(n + 5);

    // Compute R[i]: the largest index j such that x_j <= B_i.
    {
        LiChaoRange lc(n, 0, &x, &p);

        for (int i = n; i >= 1; --i) {
            // Function of point i is valid for query indices 1..i.
            lc.insert(1, i, i);

            long double minv = lc.query(i);
            long double Bval = -(long double)x[i] + minv;

            long long bmax = (long long)floorl(Bval + 1e-10L);
            if (bmax < x[i]) bmax = x[i];
            if (bmax > 1000000000LL) bmax = 1000000000LL;

            int bm = (int)bmax;
            R[i] = int(upper_bound(x.begin() + 1, x.begin() + n + 1, bm) - x.begin()) - 1;
            if (R[i] < i) R[i] = i;
        }
    }

    // Compute L[j]: the smallest index i such that x_i >= P_j.
    {
        LiChaoRange lc(n, 1, &x, &p);

        for (int j = 1; j <= n; ++j) {
            // Function of point j is valid for query indices j..n.
            lc.insert(j, n, j);

            long double minv = lc.query(j);
            long double Pval = -(long double)x[j] - minv;

            long long pceil = (long long)ceill(Pval - 1e-10L);
            if (pceil < -1000000000LL) pceil = -1000000000LL;
            if (pceil > 1000000000LL) pceil = 1000000000LL;

            int pc = (int)pceil;
            L[j] = int(lower_bound(x.begin() + 1, x.begin() + n + 1, pc) - x.begin());
            if (L[j] < 1) L[j] = 1;
            if (L[j] > n + 1) L[j] = n + 1;
        }
    }

    const int S = 700;
    int Xmax = x[n];

    vector<long long> dp(n + 5, INF64), best(n + 5, INF64);
    dp[1] = (long long)alpha * p[1];

    BlockLiChao blc(Xmax, beta, &x, &dp);

    for (int j = 1; j <= n; ++j) {
        if (j > 1) {
            long long mincand = best[j];

            int lj = L[j];
            if (lj < 1) lj = 1;

            int cb = (j - 1) / S;
            int lb = (lj - 1) / S;

            auto relax = [&](int i) {
                if (dp[i] >= INF64 / 2) return;
                if (j <= R[i]) {
                    long long diff = (long long)x[j] - x[i];
                    long long val = dp[i] + (long long)beta * diff * diff;
                    if (val < mincand) mincand = val;
                }
            };

            if (lb == cb) {
                for (int i = lj; i < j; ++i) relax(i);
            } else {
                int endLb = min(n, (lb + 1) * S);
                for (int i = lj; i <= endLb && i < j; ++i) relax(i);

                int startCb = cb * S + 1;
                for (int i = startCb; i < j; ++i) relax(i);
            }

            if (mincand >= INF64 / 2) {
                dp[j] = INF64;
            } else {
                dp[j] = mincand + (long long)alpha * p[j];
            }
        }

        // When a block is completed, add its contribution to all future queries.
        if (j % S == 0 && j < n) {
            int start = j - S + 1;
            int end = j;

            vector<int> ids;
            ids.reserve(S);
            for (int i = start; i <= end; ++i) {
                if (dp[i] < INF64 / 2) ids.push_back(i);
            }

            sort(ids.begin(), ids.end(), [&](int a, int b) {
                return R[a] > R[b];
            });

            blc.reset();

            int ptr = 0;
            for (int jj = n; jj > end; --jj) {
                while (ptr < (int)ids.size() && R[ids[ptr]] >= jj) {
                    blc.addLine(ids[ptr]);
                    ++ptr;
                }

                if (L[jj] <= start) {
                    long long val = blc.query(x[jj]);
                    if (val < best[jj]) best[jj] = val;
                }
            }
        }
    }

    if (dp[n] >= INF64 / 2) {
        cout << "impossible\n";
    } else {
        cout << dp[n] << '\n';
    }

    return 0;
}
