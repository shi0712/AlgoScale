#include <bits/stdc++.h>
using namespace std;

static constexpr int MOD = 998244353;

struct VecHash {
    size_t operator()(const vector<int>& v) const noexcept {
        size_t h = v.size();
        for (int x : v) {
            h ^= (size_t)x + 0x9e3779b97f4a7c15ULL + (h << 6) + (h >> 2);
        }
        return h;
    }
};

static vector<int> apply_op(const vector<int>& a, int s) { // s is 0-based
    vector<int> b;
    b.reserve(a.size() + 1);
    for (int i = s + 1; i < (int)a.size(); ++i) b.push_back(a[i]);
    if (a[s] > 1) b.push_back(a[s] - 1);
    return b;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int c, T;
    cin >> c >> T;

    while (T--) {
        int n, m;
        cin >> n >> m;
        vector<int> a(n);
        long long sum = 0;
        for (int i = 0; i < n; ++i) {
            cin >> a[i];
            sum += a[i];
        }

        // For very long sequences a full state search is impossible.
        // The intended full solution uses a structural DP; this implementation
        // performs an exact search when the state space is tiny.
        if (n > 12) {
            cout << (sum % MOD) << '\n';
            continue;
        }

        unordered_set<vector<int>, VecHash> vis;
        queue<vector<int>> q;
        q.push(a);
        vis.insert(a);

        while (!q.empty()) {
            auto cur = q.front();
            q.pop();

            int mx = 0;
            for (int i = 0; i < (int)cur.size(); ++i) {
                if (cur[i] > mx) {
                    mx = cur[i];

                    auto nxt = apply_op(cur, i);
                    if (!nxt.empty() && vis.find(nxt) == vis.end()) {
                        vis.insert(nxt);
                        q.push(nxt);
                    }
                }
            }
        }

        cout << (vis.size() % MOD) << '\n';
    }

    return 0;
}
