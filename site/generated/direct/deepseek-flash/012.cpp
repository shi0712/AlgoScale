#include <bits/stdc++.h>
using namespace std;

using ll = long long;
const ll INF = (ll)4e18;

struct SegTree {
    int n;
    vector<ll> mn, lazy;

    SegTree() {}
    SegTree(const vector<ll>& a) { init(a); }

    void init(const vector<ll>& a) {
        int sz = (int)a.size();
        n = 1;
        while (n < sz) n <<= 1;
        mn.assign(2 * n, INF);
        lazy.assign(2 * n, 0);
        for (int i = 0; i < sz; ++i) mn[n + i] = a[i];
        for (int i = n - 1; i >= 1; --i)
            mn[i] = min(mn[i << 1], mn[i << 1 | 1]);
    }

    void apply(int v, ll x) {
        mn[v] += x;
        lazy[v] += x;
    }

    void push(int v) {
        if (lazy[v] != 0) {
            apply(v << 1, lazy[v]);
            apply(v << 1 | 1, lazy[v]);
            lazy[v] = 0;
        }
    }

    void range_add(int v, int l, int r, int ql, int qr, ll x) {
        if (ql <= l && r <= qr) {
            apply(v, x);
            return;
        }
        push(v);
        int m = (l + r) >> 1;
        if (ql <= m) range_add(v << 1, l, m, ql, qr, x);
        if (qr > m) range_add(v << 1 | 1, m + 1, r, ql, qr, x);
        mn[v] = min(mn[v << 1], mn[v << 1 | 1]);
    }

    void range_add(int l, int r, ll x) {
        if (l > r) return;
        range_add(1, 0, n - 1, l, r, x);
    }

    void point_set(int v, int l, int r, int pos, ll x) {
        if (l == r) {
            mn[v] = x;
            lazy[v] = 0;
            return;
        }
        push(v);
        int m = (l + r) >> 1;
        if (pos <= m) point_set(v << 1, l, m, pos, x);
        else point_set(v << 1 | 1, m + 1, r, pos, x);
        mn[v] = min(mn[v << 1], mn[v << 1 | 1]);
    }

    void point_set(int pos, ll x) {
        point_set(1, 0, n - 1, pos, x);
    }

    ll get_min() const {
        return mn[1];
    }
};

struct Solver {
    int n;
    ll W;
    vector<ll> w0, r0;
    vector<int> known;          // items with w_i > 0, sorted by (w, id)
    vector<int> rankW;          // rank in known, -1 if w_i == 0
    vector<ll> pref;            // pref[k] = sum of first k known items
    vector<int> zeroKnown;      // known items with r_i == 0, sorted by id
    vector<int> fixedSorted;    // items with r_i > 0, sorted by (r desc, id asc)
    vector<int> optionalSorted;// items with w_i == 0, sorted by (r desc, id asc)

    vector<ll> outW, outR;

    int m;

    bool verify() {
        vector<int> ordA(n), ordB(n);
        iota(ordA.begin(), ordA.end(), 0);
        iota(ordB.begin(), ordB.end(), 0);

        sort(ordA.begin(), ordA.end(), [&](int a, int b) {
            if (outW[a] != outW[b]) return outW[a] < outW[b];
            return a < b;
        });
        sort(ordB.begin(), ordB.end(), [&](int a, int b) {
            if (outR[a] != outR[b]) return outR[a] > outR[b];
            return a < b;
        });

        vector<char> inA(n, 0), inB(n, 0);

        ll sum = 0;
        for (int id : ordA) {
            if (sum + outW[id] <= W) {
                inA[id] = 1;
                sum += outW[id];
            } else {
                break;
            }
        }

        ll rem = W;
        for (int id : ordB) {
            if (outW[id] <= rem) {
                inB[id] = 1;
                rem -= outW[id];
            }
        }

        for (int i = 0; i < n; ++i)
            if (inA[i] != inB[i]) return false;
        return true;
    }

    bool tryKnownC(int k) {
        outW = w0;
        outR = r0;

        int c = known[k];
        ll wc = w0[c];
        int idc = c;

        for (int idx : known) {
            int rk = rankW[idx];
            if (r0[idx] == 0) {
                if (rk < k) outR[idx] = 1000000000LL;
                else outR[idx] = 1LL;
            }
        }

        ll cap = W - pref[k];

        for (int idx : optionalSorted) {
            ll ub;
            if (wc <= W) {
                ub = wc - (idx > idc ? 1 : 0);
                if (ub < 0) ub = 0;
            } else {
                ub = W;
            }
            if (ub > 1000000000LL) ub = 1000000000LL;

            ll take = min(cap, ub);
            if (take >= 1) {
                outW[idx] = take;
                cap -= take;
            } else {
                outW[idx] = 1000000000LL;
            }
        }

        return verify();
    }

    bool checkB(int k) {
        int c = known[k];
        ll wc = w0[c];
        int idc = c;

        vector<int> pz, cz;
        pz.reserve(zeroKnown.size());
        cz.reserve(zeroKnown.size());

        for (int idx : zeroKnown) {
            if (rankW[idx] < k) pz.push_back(idx);
            else cz.push_back(idx);
        }

        int ip = 0, ic = 0, iff = 0;
        ll sum = 0;

        while (ip < (int)pz.size() || ic < (int)cz.size() || iff < (int)fixedSorted.size()) {
            ll bestVal = -1;
            int bestId = INT_MAX;
            int choice = -1;

            if (ip < (int)pz.size()) {
                ll val = 1000000000LL;
                int id = pz[ip];
                if (val > bestVal || (val == bestVal && id < bestId)) {
                    bestVal = val;
                    bestId = id;
                    choice = 0;
                }
            }
            if (ic < (int)cz.size()) {
                ll val = 1LL;
                int id = cz[ic];
                if (val > bestVal || (val == bestVal && id < bestId)) {
                    bestVal = val;
                    bestId = id;
                    choice = 1;
                }
            }
            if (iff < (int)fixedSorted.size()) {
                int idx = fixedSorted[iff];
                ll val = r0[idx];
                int id = idx;
                if (val > bestVal || (val == bestVal && id < bestId)) {
                    bestVal = val;
                    bestId = id;
                    choice = 2;
                }
            }

            if (choice == 0) {
                int idx = pz[ip++];
                sum += w0[idx];
            } else if (choice == 1) {
                int idx = cz[ic++];
                ll wj = w0[idx];
                ll req = (wj <= W ? W - wj + 1 : 0);
                if (req > 0 && sum < req) return false;
            } else {
                int idx = fixedSorted[iff++];

                if (w0[idx] > 0) {
                    if (rankW[idx] < k) {
                        sum += w0[idx];
                    } else {
                        ll wj = w0[idx];
                        ll req = (wj <= W ? W - wj + 1 : 0);
                        if (req > 0 && sum < req) return false;
                    }
                } else {
                    ll ub;
                    if (wc <= W) {
                        ub = wc - (idx > idc ? 1 : 0);
                        if (ub < 0) ub = 0;
                    } else {
                        ub = W;
                    }
                    sum += ub;
                }
            }
        }

        return true;
    }

    bool tryNoKnownC() {
        if (pref[m] > W) return false;

        outW = w0;
        outR = r0;

        for (int idx : known) {
            if (r0[idx] == 0) outR[idx] = 1000000000LL;
        }

        if (W < 1000000000LL) {
            for (int idx = 0; idx < n; ++idx)
                if (w0[idx] == 0)
                    outW[idx] = 1000000000LL;
            return verify();
        } else {
            // W == 1e9
            int p = -1;
            for (int idx = 0; idx < n; ++idx) {
                if (w0[idx] == 0) {
                    if (p == -1) p = idx;
                    else {
                        if (r0[idx] > r0[p] || (r0[idx] == r0[p] && idx < p))
                            p = idx;
                    }
                }
            }

            if (p != -1 && pref[m] < W) {
                for (int idx = 0; idx < n; ++idx)
                    if (w0[idx] == 0)
                        outW[idx] = 1000000000LL;
                outW[p] = 1;
            } else {
                for (int idx = 0; idx < n; ++idx)
                    if (w0[idx] == 0)
                        outW[idx] = 1000000000LL;
            }

            return verify();
        }
    }

    void solveOne() {
        cin >> n >> W;
        w0.resize(n);
        r0.resize(n);
        for (int i = 0; i < n; ++i) cin >> w0[i];
        for (int i = 0; i < n; ++i) cin >> r0[i];

        rankW.assign(n, -1);

        for (int i = 0; i < n; ++i)
            if (w0[i] > 0)
                known.push_back(i);

        if (known.empty()) {
            outW = w0;
            outR = r0;

            if (W < 1000000000LL) {
                for (int i = 0; i < n; ++i) outW[i] = 1000000000LL;
            } else {
                int p = 0;
                for (int i = 1; i < n; ++i) {
                    if (r0[i] > r0[p] || (r0[i] == r0[p] && i < p))
                        p = i;
                }
                for (int i = 0; i < n; ++i) outW[i] = 1000000000LL;
                outW[p] = 1;
            }

            cout << "Yes\n";
            for (int i = 0; i < n; ++i) cout << outW[i] << " \n"[i + 1 == n];
            for (int i = 0; i < n; ++i) cout << outR[i] << " \n"[i + 1 == n];
            return;
        }

        sort(known.begin(), known.end(), [&](int a, int b) {
            if (w0[a] != w0[b]) return w0[a] < w0[b];
            return a < b;
        });

        m = (int)known.size();
        for (int i = 0; i < m; ++i) rankW[known[i]] = i;

        pref.assign(m + 1, 0);
        for (int i = 0; i < m; ++i)
            pref[i + 1] = pref[i] + w0[known[i]];

        for (int i = 0; i < n; ++i)
            if (w0[i] > 0 && r0[i] == 0)
                zeroKnown.push_back(i);

        for (int i = 0; i < n; ++i)
            if (r0[i] > 0)
                fixedSorted.push_back(i);

        sort(fixedSorted.begin(), fixedSorted.end(), [&](int a, int b) {
            if (r0[a] != r0[b]) return r0[a] > r0[b];
            return a < b;
        });

        for (int i = 0; i < n; ++i)
            if (w0[i] == 0)
                optionalSorted.push_back(i);

        sort(optionalSorted.begin(), optionalSorted.end(), [&](int a, int b) {
            if (r0[a] != r0[b]) return r0[a] > r0[b];
            return a < b;
        });

        if (tryNoKnownC()) {
            cout << "Yes\n";
            for (int i = 0; i < n; ++i) cout << outW[i] << " \n"[i + 1 == n];
            for (int i = 0; i < n; ++i) cout << outR[i] << " \n"[i + 1 == n];
            return;
        }

        // Candidate: the first non-selected known item is known[k].
        vector<int> cOrder(m);
        iota(cOrder.begin(), cOrder.end(), 0);

        sort(cOrder.begin(), cOrder.end(), [&](int a, int b) {
            int ia = known[a], ib = known[b];
            ll va = (r0[ia] == 0 ? 1LL : r0[ia]);
            ll vb = (r0[ib] == 0 ? 1LL : r0[ib]);
            if (va != vb) return va > vb;
            return ia < ib;
        });

        vector<int> posC(m);
        vector<ll> cval(m), cid(m);
        for (int i = 0; i < m; ++i) {
            int rk = cOrder[i];
            int idx = known[rk];
            posC[rk] = i;
            cval[i] = (r0[idx] == 0 ? 1LL : r0[idx]);
            cid[i] = idx;
        }

        vector<ll> init(m, INF);
        for (int i = 0; i < m; ++i) {
            int idx = known[i];
            ll req = (w0[idx] <= W ? W - w0[idx] + 1 : 0);
            if (req > 0) init[posC[i]] = -req;
        }

        SegTree seg(init);

        vector<char> goodA(m, 0);

        for (int k = 0; k < m; ++k) {
            ll before = pref[k];
            if (before <= W) {
                ll mn = seg.get_min();
                goodA[k] = (mn >= before - W);
            }

            int p = known[k];
            ll wp = w0[p];
            ll pv = (r0[p] == 0 ? 1000000000LL : r0[p]);
            int idp = p;

            int lo = 0, hi = m;
            while (lo < hi) {
                int mid = (lo + hi) >> 1;
                ll v = cval[mid];
                int id = (int)cid[mid];

                bool after = (v < pv) || (v == pv && id > idp);
                if (after) hi = mid;
                else lo = mid + 1;
            }
            int start = lo;

            if (start < m) seg.range_add(start, m - 1, wp);
            seg.point_set(posC[k], INF);
        }

        int kmax = -1;
        for (int k = 0; k < m; ++k)
            if (pref[k] <= W) kmax = k;

        if (kmax >= 0) {
            int lo = 0, hi = kmax, ansB = -1;

            while (lo <= hi) {
                int mid = (lo + hi) >> 1;
                if (checkB(mid)) {
                    ansB = mid;
                    hi = mid - 1;
                } else {
                    lo = mid + 1;
                }
            }

            if (ansB != -1) {
                for (int k = ansB; k <= kmax; ++k) {
                    if (!goodA[k]) continue;
                    if (tryKnownC(k)) {
                        cout << "Yes\n";
                        for (int i = 0; i < n; ++i) cout << outW[i] << " \n"[i + 1 == n];
                        for (int i = 0; i < n; ++i) cout << outR[i] << " \n"[i + 1 == n];
                        return;
                    }
                }
            }
        }

        cout << "No\n";
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int T;
    cin >> T;
    while (T--) {
        Solver solver;
        solver.solveOne();
    }
    return 0;
}
