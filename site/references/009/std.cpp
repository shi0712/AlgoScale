// Source: user-provided final OptimizedSolver; executable algorithm preserved.
// 2e6 release: 1 <= n <= 2,000,000; 0 <= P,Q,h[i] <= 2,000,000.
// Audit: <=5 effective square roots/item; <=21 halvings/item; total sum <=4e12.
// Counts in int are <=10,000,000; aggregate costs and threshold squares use int64.
// No precomputed answers. See admin/range_audit.md for the expansion audit.
#include <bits/stdc++.h>
using namespace std;
using ll = long long;

struct Result {
    ll val, qmin;
};

// 一个固定高度分布的除法收益后缀表。
struct HalfTable {
    ll sum = 0;
    vector<ll> cnt, gain;

    void build(const vector<int>& freq) {
        int h = static_cast<int>(freq.size()) - 1;
        vector<ll> c(freq.begin(), freq.end());
        cnt.assign(h + 2, 0);
        gain.assign(h + 2, 0);
        sum = 0;
        for (int x = h; x >= 1; --x) {
            sum += 1LL * x * freq[x];
            cnt[(x + 1) / 2] += c[x];
            c[x / 2] += c[x];
        }
        for (int x = h; x >= 0; --x) {
            gain[x] = gain[x + 1] + 1LL * x * cnt[x];
            cnt[x] += cnt[x + 1];
        }
    }

    Result query(int w) const {
        int pos = min(w + 1, static_cast<int>(cnt.size()) - 1);
        return {sum - gain[pos] + 1LL * w * cnt[pos], cnt[pos]};
    }
};

struct Item {
    int d, k, count;
};

struct OptimizedSolver {
    int H, P, total, border = 0, threshold;
    vector<int> rt, lg, cnt, pref, chosen;
    HalfTable original, baseline;

    OptimizedSolver(vector<int> freq, int p) : P(p) {
        H = static_cast<int>(freq.size()) - 1;
        rt.assign(H + 1, 0);
        lg.assign(H + 1, 0);
        for (int x = 1; x <= H; ++x) {
            rt[x] = rt[x - 1];
            if (1LL * (rt[x] + 1) * (rt[x] + 1) <= x) ++rt[x];
            lg[x] = lg[x / 2] + 1;
        }
        threshold = rt[H] + (1LL * rt[H] * rt[H] < H);
        original.build(freq);
        cnt = freq;
        for (int x = H; x >= 2; --x) cnt[rt[x]] += cnt[x];
        pref.assign(H + 1, 0);
        for (int x = 2; x <= H; ++x) pref[x] = pref[x - 1] + cnt[x];
        total = H >= 2 ? pref[H] : 0;
        P = min(P, total);
        if (!P) return;

        // 按步骤起始高度选最大的 P 份，并真正更新基准高度分布。
        chosen.assign(H + 1, 0);
        int need = P;
        for (int x = H; x >= 2; --x) {
            int take = min(need, cnt[x]);
            chosen[x] = take;
            if (take) border = x;
            freq[x] -= take;
            freq[rt[x]] += take;
            need -= take;
        }
        baseline.build(freq);
    }

    // lg[t] 是 t 的二进制位数，lg[0]=0。
    int divisions(int x, int w) const {
        return lg[x / (2 * w + 1)];
    }

    int value(int x, int w, int b) const {
        return (x >> b) + w * b;
    }

    // 按 (d,k) 降序选 need 份，返回总收益和节省的除法次数。
    pair<ll, ll> select(const vector<Item>& items, int need) const {
        if (!need) return {0, 0};
        int low = H, high = 0;
        for (const auto& t : items) {
            low = min(low, t.d);
            high = max(high, t.d);
        }
        vector<int> amount(high - low + 1, 0);
        vector<ll> saved(high - low + 1, 0);
        for (const auto& t : items) {
            amount[t.d - low] += t.count;
            saved[t.d - low] += 1LL * t.count * t.k;
        }
        ll gain_sum = 0, saved_sum = 0;
        for (int d = high; d >= low; --d) {
            if (need > amount[d - low]) {
                gain_sum += 1LL * d * amount[d - low];
                saved_sum += saved[d - low];
                need -= amount[d - low];
                continue;
            }
            gain_sum += 1LL * d * need;
            vector<int> by_k(lg[H] + 1, 0);
            for (const auto& t : items)
                if (t.d == d) by_k[t.k] += t.count;
            for (int k = lg[H]; k >= 0; --k) {
                int take = min(need, by_k[k]);
                saved_sum += 1LL * take * k;
                need -= take;
            }
            return {gain_sum, saved_sum};
        }
        return {gain_sum, saved_sum};
    }

    Result calc(int w) const {
        if (!P) return original.query(w);
        if (P == total) return baseline.query(w);
        vector<Item> items;
        Result res;
        int need;
        if (w < threshold) {
            res = original.query(w);
            need = P;
            for (int x = 2; x <= H;) {
                int s = rt[x];
                int l = divisions(x, w), m = divisions(s, w);
                ll a = 2LL * w + 1;
                ll next_s = (1LL * (s >> m) + 1) << m;
                ll next_m = a << m;
                ll r = min({
                    H + 1LL,
                    (1LL * (x >> l) + 1) << l,
                    a << l,
                    next_s * next_s,
                    next_m * next_m
                });
                int end = static_cast<int>(r);
                int count = pref[end - 1] - pref[x - 1];
                if (count) {
                    int d = value(x, w, l) - value(s, w, m);
                    items.push_back({d, l - m, count});
                }
                x = end;
            }
        } else {
            res = baseline.query(w);
            need = 0;
            int radius = static_cast<int>(4LL * H / w + 1);
            int left = max(2, border - radius);
            int right = min(H, border + radius);
            for (int x = left; x <= right; ++x) {
                if (!cnt[x]) continue;
                int k = divisions(x, w);
                int d = value(x, w, k) - rt[x];
                items.push_back({d, k, cnt[x]});
                // 撤回窗口内基准方案的选择，再统一重新选取。
                res.val += 1LL * chosen[x] * d;
                res.qmin += 1LL * chosen[x] * k;
                need += chosen[x];
            }
        }
        auto [gain_sum, saved_sum] = select(items, need);
        res.val -= gain_sum;
        res.qmin -= saved_sum;
        return res;
    }

    ll solve(int q) const {
        Result first = calc(1);
        if (q >= first.val) return 0;
        int left = 1, right = max(1, H);
        while (left < right) {
            int mid = (left + right) / 2;
            if (calc(mid).qmin <= q) right = mid;
            else left = mid + 1;
        }
        return calc(left).val - 1LL * left * q;
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    int n, p, q, H = 0;
    cin >> n >> p >> q;
    vector<int> freq(1, 0);
    for (int i = 0, x; i < n; ++i) {
        cin >> x;
        if (x >= static_cast<int>(freq.size()))
            freq.resize(max(x + 1, 2 * static_cast<int>(freq.size())), 0);
        ++freq[x];
        H = max(H, x);
    }
    freq.resize(H + 1);
    cout << OptimizedSolver(std::move(freq), p).solve(q) << '\n';
    return 0;
}
