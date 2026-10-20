#pragma GCC optimize("O3")
#pragma GCC optimize("unroll-loops")

#include <bits/stdc++.h>
using namespace std;

static const long long NEG = -(1LL << 60);
static const long long NEG_ABILITY = -(1LL << 60);

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int T;
    cin >> T;

    while (T--) {
        int n, m;
        long long d;
        cin >> n >> m >> d;

        vector<int> L(n);
        for (int i = 0; i < n; ++i) cin >> L[i];
        sort(L.begin(), L.end());

        const int maxL = L.back();

        vector<pair<int, int>> cand;
        cand.reserve(m);

        for (int i = 0; i < m; ++i) {
            int e, t;
            cin >> e >> t;
            if (e <= maxL) {
                // senior = 0, junior = 1
                cand.push_back({e, t == 1 ? 1 : 0});
            }
        }

        sort(cand.begin(), cand.end());

        int outLen = 2 * n + 1;

        if ((int)cand.size() < 2 * n) {
            for (int i = 0; i < outLen; ++i) {
                if (i) cout << ' ';
                cout << -1;
            }
            cout << '\n';
            continue;
        }

        int S = 2 * n + 1;
        int states = (n + 1) * S;

        vector<long long> dp(states, NEG);
        vector<long long> active[2];
        active[0].assign(states, NEG);
        active[1].assign(states, NEG);
        vector<long long> temp(states, NEG);

        long long activeAbility[2] = {NEG_ABILITY, NEG_ABILITY};
        int activeMaxC[2] = {-1, -1};

        dp[0] = 0;

        int capStart = 0;
        int sz = (int)cand.size();

        for (int pos = 0; pos < sz; ++pos) {
            int e = cand[pos].first;
            int t = cand[pos].second;

            // Snapshot dp before processing this player as an upper endpoint.
            memcpy(temp.data(), dp.data(), (size_t)states * sizeof(long long));

            while (capStart < n && L[capStart] < e) ++capStart;

            // Complete pairs: current player is the higher-ability player.
            for (int lowerType = 0; lowerType < 2; ++lowerType) {
                if (activeMaxC[lowerType] < 0) continue;
                if ((long long)e - activeAbility[lowerType] > d) continue;

                int c0 = capStart;
                int c1 = min(activeMaxC[lowerType], n - 1);

                if (c0 > c1) continue;

                long long add = activeAbility[lowerType] + e;
                int q = t + lowerType;

                for (int c = c0; c <= c1; ++c) {
                    long long *src = active[lowerType].data() + (long long)c * S;
                    long long *dst = dp.data() + (long long)(c + 1) * S + q;

                    int maxj = 2 * c;
                    for (int j = 0; j <= maxj; ++j) {
                        long long v = src[j] + add;
                        if (v > dst[j]) dst[j] = v;
                    }
                }
            }

            // Current player becomes the latest possible lower endpoint of its type.
            swap(active[t], temp);
            activeAbility[t] = e;
            activeMaxC[t] = min(n - 1, pos / 2);
        }

        int base = n * S;
        for (int juniors = 0; juniors <= 2 * n; ++juniors) {
            long long ans = dp[base + juniors];
            if (juniors) cout << ' ';
            if (ans < 0) cout << -1;
            else cout << ans;
        }
        cout << '\n';
    }

    return 0;
}
