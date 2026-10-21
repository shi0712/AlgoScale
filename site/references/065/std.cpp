#include <bits/stdc++.h>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int T;
    if (!(cin >> T)) return 0;
    while (T--) {
        string s;
        int K;
        cin >> s >> K;
        int n = (int)s.size();
        if (n < 3) {
            cout << 0 << '\n';
            continue;
        }

        // A SINGLE is the maximal d^a c d^b interval (a,b<=2,a+b>=2)
        // present before any operation.  All useful EXTEND intervals are
        // obtained by trimming overlapping SINGLE intervals.
        vector<pair<int, int>> segs;
        for (int i = 0; i + 3 <= n; ++i) {
            char a = s[i], b = s[i + 1], c = s[i + 2], mode = 0;
            if (a == b) mode = a;
            else if (a == c) mode = a;
            else if (b == c) mode = b;
            else continue;
            int j = i + 3;
            while (j < n && s[j] == mode) ++j;
            if (segs.empty() || j > segs.back().second)
                segs.push_back({i, j});
        }
        int M = (int)segs.size();
        vector<int> mask(n + 1, 0); // lengths 3,4,5 ending at r
        auto add = [&](int l, int r) {
            int len = r - l;
            if (3 <= len && len <= 5 && 0 <= l && r <= n)
                mask[r] |= 1 << (len - 3);
        };
        for (auto [l, r] : segs) add(l, r);

        vector<int> index_of_start(n, -1);
        for (int i = 0; i < M; ++i) index_of_start[segs[i].first] = i;
        for (int i = 0; i < M; ++i) {
            int a1 = segs[i].first, a2 = segs[i].second;
            for (int b1 = a1 + 1; b1 < a2; ++b1) {
                int j = index_of_start[b1];
                if (j < 0) continue;
                add(a1, b1);
                add(a2, segs[j].second);
            }
        }
        for (int i = 0; i < M; ++i) {
            int al = segs[i].first, ar = segs[i].second;
            int jlo = max(0, i - 5), khi = min(M - 1, i + 5);
            for (int j = jlo; j <= i; ++j) {
                int br = segs[j].second;
                if (br <= al) continue;
                for (int k = i; k <= khi; ++k) {
                    int cl = segs[k].first;
                    if (cl >= ar) continue;
                    if (cl - br < 3) continue;
                    add(br, cl);
                }
            }
        }

        long long answer = LLONG_MAX;
        for (int lambda = 0; lambda <= 5; ++lambda) {
            vector<long long> dp(n + 1, 0);
            for (int i = 1; i <= n; ++i) {
                long long best = dp[i - 1];
                int m = mask[i];
                while (m) {
                    int bit = __builtin_ctz((unsigned)m);
                    m &= m - 1;
                    int len = bit + 3;
                    best = max(best, dp[i - len] + len - lambda);
                }
                dp[i] = best;
            }
            answer = min(answer, dp[n] + 1LL * lambda * K);
        }
        cout << answer << '\n';
    }
    return 0;
}
