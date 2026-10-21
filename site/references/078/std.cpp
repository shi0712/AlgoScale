#include <bits/stdc++.h>
using namespace std;
using int64 = long long;

struct Block {
    int l, r, z;
};

static int64 solve_case(vector<int> a, int m) {
    int n = (int)a.size();
    int mn = *min_element(a.begin(), a.end());
    for (int &x : a) x -= mn;
    int V = *max_element(a.begin(), a.end());

    int64 C = -a[0];
    for (int i = 1; i < n; ++i) C += max(0, a[i - 1] - a[i]);

    int64 ans = (int64)V + C;
    if (m == 1 || V == 0) return ans;

    vector<int> vals = a;
    sort(vals.begin(), vals.end());
    vals.erase(unique(vals.begin(), vals.end()), vals.end());
    if ((int)vals.size() <= 2) return 0;

    vector<int> rank(V + 1, -1);
    for (int i = 0; i < (int)vals.size(); ++i) rank[vals[i]] = i;

    for (int D = 1; D <= V; ++D) {
        int base = V - D;
        int cut = upper_bound(vals.begin(), vals.end(), base) - vals.begin();

        vector<int> lows;
        lows.reserve((int)vals.size() - cut + 1);
        lows.push_back(base);
        for (int j = cut; j < (int)vals.size(); ++j) lows.push_back(vals[j]);
        int K = (int)lows.size();

        auto index_of = [&](int x) -> int {
            if (x <= base) return 0;
            return rank[x] - cut + 1;
        };

        vector<int64> diff(K + 1, 0);
        auto add = [&](int l, int r, int w) {
            if (l > r || w == 0) return;
            diff[l] += w;
            diff[r + 1] -= w;
        };

        deque<Block> dq;
        dq.push_back({index_of(a[0]), K - 1, D});

        for (int i = 1; i < n; ++i) {
            if (a[i] > a[i - 1]) {
                int cap = max(0, D - (a[i] - a[i - 1]));
                int left = K;

                while (!dq.empty() && dq.back().z > cap) {
                    Block b = dq.back();
                    dq.pop_back();
                    add(b.l, b.r, b.z - cap);
                    left = b.l;
                }

                if (left < K && cap > 0) {
                    if (!dq.empty() && dq.back().z == cap) dq.back().r = K - 1;
                    else dq.push_back({left, K - 1, cap});
                }

                int first = index_of(a[i]);
                while (!dq.empty() && dq.front().r < first) dq.pop_front();
                if (!dq.empty()) dq.front().l = max(dq.front().l, first);
            } else if (a[i] < a[i - 1]) {
                int floorv = min(D, a[i - 1] - a[i]);
                while (!dq.empty() && dq.front().z <= floorv) dq.pop_front();

                int l = index_of(a[i]);
                int r = dq.empty() ? K - 1 : dq.front().l - 1;
                if (l <= r && floorv > 0) dq.push_front({l, r, floorv});
            }
        }

        for (const Block &b : dq) add(b.l, b.r, b.z);

        int64 saved = 0;
        for (int j = 0; j < K; ++j) {
            saved += diff[j];
            ans = min(ans, (int64)lows[j] + D + C - saved);
        }
        if (ans == 0) break;
    }
    return ans;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int T;
    if (!(cin >> T)) return 0;
    while (T--) {
        int n, m;
        cin >> n >> m;
        vector<int> a(n);
        for (int &x : a) cin >> x;
        cout << solve_case(a, m) << '\n';
    }
    return 0;
}
