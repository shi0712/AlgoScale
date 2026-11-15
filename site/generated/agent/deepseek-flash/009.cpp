#include <bits/stdc++.h>
using namespace std;
typedef long long ll;
static const ll LIM = 1000000000LL;

struct Fenwick {
    int n;
    vector<ll> t;
    void init(int n_) { n = n_; t.assign(n + 1, 0); }
    void clear() { fill(t.begin(), t.end(), 0); }
    inline void add(int i, ll v) { for (; i <= n; i += i & -i) t[i] += v; }
    inline ll sum(int i) const { ll s = 0; for (; i > 0; i -= i & -i) s += t[i]; return s; }
    // smallest index with prefix sum >= target (requires total >= target, target > 0)
    int lower_bound(ll target) const {
        int idx = 0; ll cur = 0;
        int bit = 1;
        while ((bit << 1) <= n) bit <<= 1;
        for (; bit; bit >>= 1) {
            int nxt = idx + bit;
            if (nxt <= n && cur + t[nxt] < target) { idx = nxt; cur += t[nxt]; }
        }
        return idx + 1;
    }
};

struct Item {
    ll w;
    int idx;
    bool isV;
    ll val; // for rigid
};

int main() {
    int T;
    if (scanf("%d", &T) != 1) return 0;
    while (T--) {
        int n; ll W;
        scanf("%d %lld", &n, &W);
        vector<ll> w(n + 1), r(n + 1);
        for (int i = 1; i <= n; i++) scanf("%lld", &w[i]);
        for (int i = 1; i <= n; i++) scanf("%lld", &r[i]);

        vector<Item> solid;
        vector<pair<ll,int>> Wfree; // (r, idx)
        for (int i = 1; i <= n; i++) {
            if (w[i] != 0) {
                if (r[i] != 0) solid.push_back({w[i], i, false, r[i]});
                else solid.push_back({w[i], i, true, 0});
            } else {
                Wfree.push_back({r[i], i});
            }
        }
        sort(solid.begin(), solid.end(), [](const Item&a, const Item&b) {
            if (a.w != b.w) return a.w < b.w;
            return a.idx < b.idx;
        });
        int p = (int)solid.size();
        int mw = (int)Wfree.size();
        vector<ll> P(p + 1, 0);
        for (int i = 1; i <= p; i++) P[i] = P[i-1] + solid[i-1].w;

        vector<ll> ow(n + 1), orv(n + 1);
        for (int i = 1; i <= n; i++) { ow[i] = w[i]; orv[i] = r[i]; }

        // composite key: higher = earlier in value order (value desc, index asc)
        auto comp = [&](ll val, int idx) -> ll {
            return val * (ll)(n + 2) + (ll)(n + 1 - idx);
        };

        int kmax = 0;
        for (int k = 0; k <= p; k++) {
            if (P[k] <= W) kmax = k; else break;
        }

        bool done = false;

        auto emit = [&]() {
            printf("Yes\n");
            for (int i = 1; i <= n; i++) { printf("%lld", ow[i]); if (i < n) printf(" "); }
            printf("\n");
            for (int i = 1; i <= n; i++) { printf("%lld", orv[i]); if (i < n) printf(" "); }
            printf("\n");
        };

        // ---------------- candidate k = p (all solid items selected) -------------
        if (kmax == p) {
            bool kp_feasible = false;
            bool include_top = false;
            if (W < LIM) {
                kp_feasible = true;
            } else {
                // W == 1e9
                if (mw == 0) {
                    kp_feasible = true;
                } else {
                    ll budget = W - P[p];
                    int minWidx = INT_MAX;
                    for (auto &e : Wfree) minWidx = min(minWidx, e.second);
                    bool tieOK = true;
                    if (p >= 1 && solid[p-1].w == W) {
                        if (minWidx < solid[p-1].idx) tieOK = false;
                    }
                    if (!tieOK) {
                        kp_feasible = false;
                    } else if (budget >= 1) {
                        kp_feasible = true;
                        include_top = true;
                    } else {
                        // budget == 0: need a solid item with rank above all W items
                        ll maxSolidRank = LLONG_MIN;
                        for (int i = 0; i < p; i++) {
                            ll keyS = solid[i].isV ? comp(LIM, solid[i].idx) : comp(solid[i].val, solid[i].idx);
                            maxSolidRank = max(maxSolidRank, keyS);
                        }
                        ll topWkey = LLONG_MIN;
                        for (auto &e : Wfree) topWkey = max(topWkey, comp(e.first, e.second));
                        kp_feasible = (maxSolidRank > topWkey);
                    }
                }
            }
            if (kp_feasible) {
                // assign values of V items (all solid are selected)
                for (int i = 0; i < p; i++) if (solid[i].isV) orv[solid[i].idx] = LIM;
                // weights of Wfree items
                if (W < LIM) {
                    for (auto &e : Wfree) ow[e.second] = W + 1;
                } else {
                    if (mw > 0) {
                        // find top W item
                        int topIdx = -1; ll topKey = LLONG_MIN;
                        for (auto &e : Wfree) {
                            ll kk = comp(e.first, e.second);
                            if (kk > topKey) { topKey = kk; topIdx = e.second; }
                        }
                        for (auto &e : Wfree) ow[e.second] = W;
                        if (include_top) ow[topIdx] = 1;
                    }
                }
                emit();
                done = true;
            }
        }

        if (!done && p >= 1) {
            // ------------- general search over k in [0, min(kmax, p-1)] -----------
            int Kgen = min(kmax, p - 1);
            // build keys
            vector<ll> keyS(p), keyT(p);
            for (int i = 0; i < p; i++) {
                if (solid[i].isV) {
                    keyS[i] = comp(LIM, solid[i].idx);
                    keyT[i] = comp(1, solid[i].idx);
                } else {
                    keyS[i] = comp(solid[i].val, solid[i].idx);
                    keyT[i] = keyS[i];
                }
            }
            vector<int> descOrder(p); // positions sorted by keyS descending
            for (int i = 0; i < p; i++) descOrder[i] = i;
            sort(descOrder.begin(), descOrder.end(), [&](int a, int b) { return keyS[a] > keyS[b]; });
            // ascending keys for rAbove
            vector<ll> keysAsc(keyS.begin(), keyS.end());
            sort(keysAsc.begin(), keysAsc.end());
            vector<int> rAbove(p);
            for (int i = 0; i < p; i++) {
                int cnt = p - (int)(upper_bound(keysAsc.begin(), keysAsc.end(), keyT[i]) - keysAsc.begin());
                rAbove[i] = cnt;
            }
            // W items sorted by keyW descending
            vector<pair<ll,int>> Wsorted; // (key, idx)
            Wsorted.reserve(mw);
            for (auto &e : Wfree) Wsorted.push_back({comp(e.first, e.second), e.second});
            sort(Wsorted.begin(), Wsorted.end(), [](const pair<ll,int>&a, const pair<ll,int>&b){ return a.first > b.first; });
            vector<ll> wKeysAsc(mw);
            for (int i = 0; i < mw; i++) wKeysAsc[i] = Wsorted[i].first;
            sort(wKeysAsc.begin(), wKeysAsc.end());
            vector<int> wAbove(p);
            for (int i = 0; i < p; i++) {
                int cnt = mw - (int)(upper_bound(wKeysAsc.begin(), wKeysAsc.end(), keyT[i]) - wKeysAsc.begin());
                wAbove[i] = cnt;
            }
            // prefix max of keyS over solid positions (for E(k))
            vector<ll> maxKeyPos(p + 1, LLONG_MIN);
            for (int i = 0; i < p; i++) maxKeyPos[i+1] = max(maxKeyPos[i], keyS[i]);
            // top W info
            ll topWkey = (mw > 0 ? Wsorted[0].first : LLONG_MIN);
            int topWidx = (mw > 0 ? Wsorted[0].second : -1);
            int minWidx = INT_MAX;
            for (auto &e : Wfree) minWidx = min(minWidx, e.second);

            // rank prefix temporary arrays
            vector<ll> rankPref(p + 1);
            vector<int> wcnt(mw + 1);

            auto check2 = [&](int k) -> bool {
                ll C = solid[k].w;
                int I = solid[k].idx;
                // rankPref
                rankPref[0] = 0;
                for (int r = 0; r < p; r++) {
                    int pos = descOrder[r];
                    ll add = (pos < k) ? solid[pos].w : 0; // prefix items are positions [0, k-1]
                    rankPref[r+1] = rankPref[r] + add;
                }
                // wcnt
                wcnt[0] = 0;
                for (int t = 0; t < mw; t++) {
                    wcnt[t+1] = wcnt[t] + (Wsorted[t].second > I ? 1 : 0);
                }
                for (int pos = k; pos < p; pos++) {
                    ll base = rankPref[rAbove[pos]];
                    ll cap = C * (ll)wAbove[pos] - (ll)wcnt[wAbove[pos]];
                    if (base + cap + solid[pos].w <= W) return false;
                }
                return true;
            };

            bool gen_ok = false;
            int bestk = -1;
            if (Kgen >= 0 && check2(Kgen)) {
                // binary search smallest k in [0,Kgen] with check2 true
                int lo = 0, hi = Kgen;
                while (lo < hi) {
                    int mid = (lo + hi) >> 1;
                    if (check2(mid)) hi = mid; else lo = mid + 1;
                }
                int L = lo;
                // K1 computation
                vector<int> K1(p, 0);
                Fenwick fw;
                fw.init(p);
                // rigid pass in increasing keyS
                for (int r = p - 1; r >= 0; r--) {
                    int pos = descOrder[r];
                    if (!solid[pos].isV) {
                        ll total = fw.sum(pos); // positions strictly before pos (1-based indices 1..pos)
                        if (total < solid[pos].w) K1[pos] = pos;
                        else {
                            int q = fw.lower_bound(solid[pos].w);
                            K1[pos] = q - 1;
                        }
                    }
                    fw.add(pos + 1, solid[pos].w);
                }
                // V pass: insert rigid items with value 1 and idx > idx_x
                fw.clear();
                vector<int> rigidV1; // positions of rigid items with value 1
                for (int i = 0; i < p; i++) if (!solid[i].isV && solid[i].val == 1) rigidV1.push_back(i);
                vector<int> Vpos;
                for (int i = 0; i < p; i++) if (solid[i].isV) Vpos.push_back(i);
                sort(rigidV1.begin(), rigidV1.end(), [&](int a, int b){ return solid[a].idx > solid[b].idx; });
                sort(Vpos.begin(), Vpos.end(), [&](int a, int b){ return solid[a].idx > solid[b].idx; });
                int ptr = 0;
                for (int pos : Vpos) {
                    while (ptr < (int)rigidV1.size() && solid[rigidV1[ptr]].idx > solid[pos].idx) {
                        int q = rigidV1[ptr];
                        fw.add(q + 1, solid[q].w);
                        ptr++;
                    }
                    ll total = fw.sum(pos);
                    if (total < solid[pos].w) K1[pos] = pos;
                    else {
                        int q = fw.lower_bound(solid[pos].w);
                        K1[pos] = q - 1;
                    }
                }
                // mark forbidden intervals
                vector<int> diff(Kgen + 2, 0);
                for (int pos = 0; pos < p; pos++) {
                    int a = K1[pos] + 1;
                    int b = pos; // j-1 in 1-based, pos in 0-based
                    if (a <= b) {
                        if (a > Kgen) continue;
                        if (b > Kgen) b = Kgen;
                        diff[a]++; diff[b+1]--;
                    }
                }
                vector<int> forb(Kgen + 1, 0);
                int cur = 0;
                for (int k = 0; k <= Kgen; k++) { cur += diff[k]; forb[k] = cur; }

                for (int k = L; k <= Kgen; k++) {
                    if (forb[k] != 0) continue;
                    bool Eok = true;
                    if (W == LIM && mw > 0) {
                        // tie condition
                        if (k >= 1 && solid[k-1].w == W) {
                            if (minWidx < solid[k-1].idx) Eok = false;
                        }
                        if (Eok) {
                            ll budget = W - P[k];
                            ll C = solid[k].w;
                            int I = solid[k].idx;
                            int capTop = (int)C - (topWidx > I ? 1 : 0);
                            bool topIncluded = (budget >= 1 && capTop >= 1);
                            if (!topIncluded) {
                                if (!(maxKeyPos[k] > topWkey)) Eok = false;
                            }
                        }
                    }
                    if (Eok) { gen_ok = true; bestk = k; break; }
                }
            }

            if (gen_ok) {
                int k = bestk;
                ll budget = W - P[k];
                ll C = solid[k].w;
                int I = solid[k].idx;
                // assign W items
                vector<ll> wAssign(mw, 0);
                for (int t = 0; t < mw; t++) {
                    ll cap = C - (Wsorted[t].second > I ? 1 : 0);
                    if (W == LIM) cap = min(cap, W - 1);
                    if (cap > 0 && budget > 0) {
                        ll take = min(cap, budget);
                        wAssign[t] = take;
                        budget -= take;
                    } else {
                        wAssign[t] = (W < LIM ? W + 1 : W);
                    }
                }
                for (int t = 0; t < mw; t++) ow[Wsorted[t].second] = wAssign[t];
                // solid values
                for (int i = 0; i < p; i++) {
                    if (solid[i].isV) {
                        orv[solid[i].idx] = (i < k) ? LIM : 1;
                    }
                }
                emit();
                done = true;
            }
        }

        if (!done) {
            printf("No\n");
        }
    }
    return 0;
}
