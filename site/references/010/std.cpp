#include <bits/stdc++.h>
using namespace std;

using int64 = long long;
// VALUE_LIMIT is overridden only by the reduced-domain verification harness.
// The UOJ standard program is compiled without this override: M = 10^9.
#ifndef VALUE_LIMIT
#define VALUE_LIMIT 1000000000LL
#endif
const int64 M = VALUE_LIMIT;
const int64 INF = (1LL << 60);

struct SegmentTree {
    int n;
    vector<int64> mn, lazy;

    explicit SegmentTree(const vector<int64>& a)
        : n((int)a.size()), mn(4 * n), lazy(4 * n, 0) {
        build(1, 0, n - 1, a);
    }

    void build(int p, int l, int r, const vector<int64>& a) {
        if (l == r) {
            mn[p] = a[l];
            return;
        }
        int mid = (l + r) / 2;
        build(p * 2, l, mid, a);
        build(p * 2 + 1, mid + 1, r, a);
        pull(p);
    }

    void pull(int p) {
        mn[p] = min(mn[p * 2], mn[p * 2 + 1]);
    }

    void apply(int p, int64 delta) {
        mn[p] += delta;
        lazy[p] += delta;
    }

    void push(int p) {
        if (lazy[p] != 0) {
            apply(p * 2, lazy[p]);
            apply(p * 2 + 1, lazy[p]);
            lazy[p] = 0;
        }
    }

    // 给前 len 个位置加上 delta。
    void addPrefix(int len, int64 delta) {
        if (len > 0) addPrefix(1, 0, n - 1, len - 1, delta);
    }

    void addPrefix(int p, int l, int r, int last, int64 delta) {
        if (r <= last) {
            apply(p, delta);
            return;
        }
        push(p);
        int mid = (l + r) / 2;
        addPrefix(p * 2, l, mid, last, delta);
        if (last > mid) {
            addPrefix(p * 2 + 1, mid + 1, r, last, delta);
        }
        pull(p);
    }

    // 已选中的固定重量物品，不再需要检查它能否被跳过。
    void erase(int pos) {
        erase(1, 0, n - 1, pos);
    }

    void erase(int p, int l, int r, int pos) {
        if (l == r) {
            mn[p] = INF;
            lazy[p] = 0;
            return;
        }
        push(p);
        int mid = (l + r) / 2;
        if (pos <= mid) erase(p * 2, l, mid, pos);
        else erase(p * 2 + 1, mid + 1, r, pos);
        pull(p);
    }

    int64 minimum() const {
        return mn[1];
    }
};

bool solveCase(int n, int64 W, vector<int64>& w, vector<int64>& r) {
    // Alice 选中重量为 M 的物品时，唯一可能是：
    // W = M，所有重量均为 M，且选中的只有 1 号物品。
    if (W == M) {
        bool allCanBeM = true;
        int64 firstValue = (r[0] == 0 ? M : r[0]);
        bool firstCanLead = true;
        for (int i = 0; i < n; ++i) {
            if (w[i] != 0 && w[i] != M) allCanBeM = false;
            if (r[i] > firstValue) firstCanLead = false;
        }
        if (allCanBeM && firstCanLead) {
            for (int i = 0; i < n; ++i) {
                if (w[i] == 0) w[i] = M;
                if (r[i] == 0) r[i] = (i == 0 ? M : 1);
            }
            return true;
        }
    }

    vector<int> fixedItems, unknownItems;
    for (int i = 0; i < n; ++i) {
        if (w[i] != 0) fixedItems.push_back(i);
        else unknownItems.push_back(i);
    }
    sort(fixedItems.begin(), fixedItems.end(), [&](int a, int b) {
        if (w[a] != w[b]) return w[a] < w[b];
        return a < b;
    });
    sort(unknownItems.begin(), unknownItems.end(), [&](int a, int b) {
        if (r[a] != r[b]) return r[a] > r[b];
        return a < b;
    });

    // 优先级为 (-价值, 原下标)，越小越靠前。
    // 未知价值：未选中时设为 1，选中时设为 M。
    vector<pair<int64, int>> keys;
    for (int i : fixedItems) {
        keys.emplace_back(-(r[i] == 0 ? 1 : r[i]), i);
    }
    // 虚拟检查点：Bob 顺序中第一个未知重量物品。
    if (!unknownItems.empty()) {
        int u = unknownItems[0];
        keys.emplace_back(-r[u], u);
    }
    sort(keys.begin(), keys.end());

    vector<int> pos(n, -1);
    vector<int64> initial(keys.size());
    for (int t = 0; t < (int)keys.size(); ++t) {
        int i = keys[t].second;
        pos[i] = t;
        initial[t] = (w[i] == 0 ? M : w[i]);
    }
    SegmentTree seg(initial);

    int best = 0;
    int64 sum = 0, bestSum = 0;
    int m = (int)fixedItems.size();

    for (int k = 0; k < m; ++k) {
        int i = fixedItems[k];
        if (w[i] == M || sum + w[i] > W) break;
        sum += w[i];

        seg.erase(pos[i]);
        int64 selectedValue = (r[i] == 0 ? M : r[i]);
        pair<int64, int> selectedKey = {-selectedValue, i};
        int len = (int)(lower_bound(keys.begin(), keys.end(), selectedKey)
                        - keys.begin());
        // 排在它前面的检查点，其“后方已选总重”增加 w[i]。
        seg.addPrefix(len, -w[i]);

        if (seg.minimum() > 0) {
            best = k + 1;
            bestSum = sum;
        }
    }

    vector<char> chosen(n, false);
    for (int k = 0; k < best; ++k) chosen[fixedItems[k]] = true;

    // 按 Bob 的顺序，贪心分配未知重量物品的重量预算。
    int64 budget = W - bestSum;
    for (int i : unknownItems) {
        int64 cap = M - 1;
        if (best < m) {
            int j = fixedItems[best];  // 第一个未选中的固定重量物品。
            cap = min(cap, w[j] - (i > j ? 1LL : 0LL));
        }
        int64 take = min(budget, cap);
        if (take > 0) {
            w[i] = take;
            budget -= take;
            chosen[i] = true;
        } else {
            w[i] = M;
        }
    }
    for (int i = 0; i < n; ++i) {
        if (r[i] == 0) r[i] = (chosen[i] ? M : 1);
    }

    // Bob 若恰好选中 chosen，Alice 也一定恰好选中 chosen。
    vector<int> order(n);
    iota(order.begin(), order.end(), 0);
    sort(order.begin(), order.end(), [&](int a, int b) {
        if (r[a] != r[b]) return r[a] > r[b];
        return a < b;
    });
    int64 remaining = W;
    for (int i : order) {
        bool take = (w[i] <= remaining);
        if (take != (bool)chosen[i]) return false;
        if (take) remaining -= w[i];
    }
    return true;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int T;
    cin >> T;
    while (T--) {
        int n;
        int64 W;
        cin >> n >> W;
        vector<int64> w(n), r(n);
        for (auto& x : w) cin >> x;
        for (auto& x : r) cin >> x;

        if (!solveCase(n, W, w, r)) {
            cout << "No\n";
            continue;
        }
        cout << "Yes\n";
        for (int i = 0; i < n; ++i) {
            cout << w[i] << (i + 1 == n ? '\n' : ' ');
        }
        for (int i = 0; i < n; ++i) {
            cout << r[i] << (i + 1 == n ? '\n' : ' ');
        }
    }
    return 0;
}
