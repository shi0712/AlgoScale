#include <bits/stdc++.h>
using namespace std;

class Solver {
    using ll = long long;
    static constexpr ll INF = (1LL << 62);

    class SegTree {
        struct Node {
            ll minB = 0, maxB = 0;
            ll minW = 0, minBW = 0;
            ll mn = 0, mx = 0;
            ll maxDiff = 0;
            ll best = 0;
            ll lazy = 0;
            int kind = 0; // 0:none, 1:A=c, 2:A=beta+c
        };

        int n;
        vector<Node> tr;

        void apply(int p, int kind, ll c) {
            Node &u = tr[p];
            u.kind = kind;
            u.lazy = c;
            if (kind == 1) {
                u.mn = u.mx = c;
                u.maxDiff = c - u.minB;
                u.best = c + u.minW;
            } else {
                u.mn = u.minB + c;
                u.mx = u.maxB + c;
                u.maxDiff = c;
                u.best = c + u.minBW;
            }
        }

        void push(int p) {
            if (!tr[p].kind) return;
            apply(p << 1, tr[p].kind, tr[p].lazy);
            apply(p << 1 | 1, tr[p].kind, tr[p].lazy);
            tr[p].kind = 0;
        }

        void pull(int p) {
            const Node &L = tr[p << 1], &R = tr[p << 1 | 1];
            Node &u = tr[p];
            u.mn = min(L.mn, R.mn);
            u.mx = max(L.mx, R.mx);
            u.maxDiff = max(L.maxDiff, R.maxDiff);
            u.best = min(L.best, R.best);
        }

        void build(int p, int l, int r,
                   const vector<ll> &beta,
                   const vector<ll> &w,
                   const vector<ll> &initial) {
            if (l == r) {
                Node &u = tr[p];
                u.minB = u.maxB = beta[l];
                u.minW = w[l];
                u.minBW = beta[l] + w[l];
                apply(p, 1, initial[l]);
                return;
            }
            int mid = (l + r) >> 1;
            build(p << 1, l, mid, beta, w, initial);
            build(p << 1 | 1, mid + 1, r, beta, w, initial);
            Node &u = tr[p];
            u.minB = min(tr[p << 1].minB, tr[p << 1 | 1].minB);
            u.maxB = max(tr[p << 1].maxB, tr[p << 1 | 1].maxB);
            u.minW = min(tr[p << 1].minW, tr[p << 1 | 1].minW);
            u.minBW = min(tr[p << 1].minBW, tr[p << 1 | 1].minBW);
            pull(p);
        }

        void assignRange(int p, int l, int r, int ql, int qr, int kind, ll c) {
            if (qr < l || r < ql) return;
            if (ql <= l && r <= qr) {
                apply(p, kind, c);
                return;
            }
            push(p);
            int mid = (l + r) >> 1;
            assignRange(p << 1, l, mid, ql, qr, kind, c);
            assignRange(p << 1 | 1, mid + 1, r, ql, qr, kind, c);
            pull(p);
        }

        ll queryBest(int p, int l, int r, int ql, int qr) {
            if (qr < l || r < ql) return INF;
            if (ql <= l && r <= qr) return tr[p].best;
            push(p);
            int mid = (l + r) >> 1;
            return min(queryBest(p << 1, l, mid, ql, qr),
                       queryBest(p << 1 | 1, mid + 1, r, ql, qr));
        }

        // type 0: first A-beta > v
        // type 1: last A > v
        // type 2: first A < v
        int findPos(int p, int l, int r, int ql, int qr, int type, ll v) {
            if (qr < l || r < ql) return -1;
            if (type == 0 && tr[p].maxDiff <= v) return -1;
            if (type == 1 && tr[p].mx <= v) return -1;
            if (type == 2 && tr[p].mn >= v) return -1;
            if (l == r) return l;
            push(p);
            int mid = (l + r) >> 1;
            if (type == 1) {
                int res = findPos(p << 1 | 1, mid + 1, r, ql, qr, type, v);
                if (res != -1) return res;
                return findPos(p << 1, l, mid, ql, qr, type, v);
            }
            int res = findPos(p << 1, l, mid, ql, qr, type, v);
            if (res != -1) return res;
            return findPos(p << 1 | 1, mid + 1, r, ql, qr, type, v);
        }

        ll getValue(int p, int l, int r, int idx) {
            if (l == r) return tr[p].mn;
            push(p);
            int mid = (l + r) >> 1;
            if (idx <= mid) return getValue(p << 1, l, mid, idx);
            return getValue(p << 1 | 1, mid + 1, r, idx);
        }

    public:
        SegTree(const vector<ll> &beta,
                const vector<ll> &w,
                const vector<ll> &initial)
            : n((int)beta.size() - 1), tr(4 * n + 16) {
            build(1, 1, n, beta, w, initial);
        }

        void assignRange(int l, int r, int kind, ll c) {
            if (l <= r) assignRange(1, 1, n, l, r, kind, c);
        }
        ll queryBest(int l, int r) {
            return l <= r ? queryBest(1, 1, n, l, r) : INF;
        }
        int findPos(int l, int r, int type, ll v) {
            return l <= r ? findPos(1, 1, n, l, r, type, v) : -1;
        }
        ll getValue(int idx) { return getValue(1, 1, n, idx); }
    };

public:
    long long solve(int n, int start, const vector<pair<int,int>> &requests) {
        auto noravelqui = tie(n, start, requests);
        (void)noravelqui;

        vector<pair<ll,ll>> points;
        points.reserve(requests.size());
        for (auto [arrival, floor] : requests)
            points.emplace_back((ll)floor, (ll)arrival);
        sort(points.begin(), points.end());

        // Same floor: only the latest arrival matters.
        vector<ll> x(1, 0), release(1, 0);
        for (auto [floor, arrival] : points) {
            if (x.size() == 1 || x.back() != floor) {
                x.push_back(floor);
                release.push_back(arrival);
            } else {
                release.back() = max(release.back(), arrival);
            }
        }

        int k = (int)x.size() - 1;
        if (k == 1)
            return max(llabs((ll)start - x[1]), release[1]);

        vector<ll> beta(k + 1), w(k + 1), initial(k + 1);
        beta[k] = release[k] + x[k];
        for (int j = k - 1; j >= 1; --j)
            beta[j] = max(beta[j + 1], release[j] + x[j]);

        ll alpha = release[1] - x[1];
        ll tail = max(alpha, llabs((ll)start - x[1]) - x[1]);
        ll rightOnly = llabs((ll)start - x[k]) + x[k];

        for (int j = 1; j <= k; ++j) {
            initial[j] = max(alpha, max(beta[j], rightOnly) - 2 * x[1]);
            w[j] = 2 * x[j - 1];
        }

        SegTree seg(beta, w, initial);
        ll answer = seg.getValue(2) + x[1];

        for (int i = 2; i <= k; ++i) {
            alpha = max(alpha, release[i] - x[i]);
            int lo = i + 1;

            if (lo <= k) {
                ll P = min(seg.queryBest(lo, k), tail + 2 * x[k]);
                ll shift = -2 * x[i];
                ll C = P + shift;

                int l = seg.findPos(lo, k, 0, shift);
                int r = seg.findPos(lo, k, 1, C);

                if (l != -1 && r != -1 && l <= r) {
                    // beta is non-increasing: last p with beta[p] >= P.
                    int left = lo, right = k, p = lo - 1;
                    while (left <= right) {
                        int mid = (left + right) >> 1;
                        if (beta[mid] >= P) {
                            p = mid;
                            left = mid + 1;
                        } else {
                            right = mid - 1;
                        }
                    }
                    seg.assignRange(l, min(r, p), 2, shift);
                    seg.assignRange(max(l, p + 1), r, 1, C);
                }

                int q = seg.findPos(lo, k, 2, alpha);
                if (q != -1) seg.assignRange(q, k, 1, alpha);
            }

            tail = max(tail, alpha);
            ll cur = (lo <= k ? seg.getValue(lo) : tail) + x[i];
            answer = min(answer, cur);
        }

        return answer;
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, start, m;
    if (!(cin >> n >> start >> m)) return 0;
    vector<pair<int,int>> requests(m);
    for (auto &[arrival, floor] : requests) cin >> arrival >> floor;

    Solver solver;
    cout << solver.solve(n, start, requests) << '\n';
    return 0;
}
