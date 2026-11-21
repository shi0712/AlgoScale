#include <bits/stdc++.h>
using namespace std;

using ll = long long;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    ll n;
    cin >> n;

    auto ask = [&](ll l, ll r) -> ll {
        cout << "xor " << l << " " << r << '\n' << flush;
        ll v;
        cin >> v;
        if (v == -1) exit(0);
        return v;
    };

    // With n = 3, all three books are stolen.
    if (n == 3) {
        cout << "ans 1 2 3\n" << flush;
        return 0;
    }

    // For small n, use an adaptive exhaustive strategy.
    if (n <= 30) {
        int N = (int)n;

        vector<array<int, 3>> cand;
        for (int a = 1; a <= N; ++a) {
            for (int b = a + 1; b <= N; ++b) {
                for (int c = b + 1; c <= N; ++c) {
                    cand.push_back({a, b, c});
                }
            }
        }

        ll totalXor = ask(1, n);

        vector<array<int, 3>> filtered;
        for (auto s : cand) {
            if ((ll)(s[0] ^ s[1] ^ s[2]) == totalXor) {
                filtered.push_back(s);
            }
        }
        cand.swap(filtered);

        while (cand.size() > 1) {
            int bestL = 1, bestR = 1;
            int bestMax = INT_MAX, bestDistinct = -1;

            for (int l = 1; l <= N; ++l) {
                for (int r = l; r <= N; ++r) {
                    unordered_map<ll, int> cnt;
                    cnt.reserve(cand.size() * 2 + 1);

                    int mx = 0;

                    for (auto s : cand) {
                        ll v = 0;
                        if (l <= s[0] && s[0] <= r) v ^= s[0];
                        if (l <= s[1] && s[1] <= r) v ^= s[1];
                        if (l <= s[2] && s[2] <= r) v ^= s[2];

                        int now = ++cnt[v];
                        mx = max(mx, now);
                    }

                    int distinct = (int)cnt.size();

                    if (mx < bestMax || (mx == bestMax && distinct > bestDistinct)) {
                        bestMax = mx;
                        bestDistinct = distinct;
                        bestL = l;
                        bestR = r;
                    }
                }
            }

            ll got = ask(bestL, bestR);

            vector<array<int, 3>> nxt;
            for (auto s : cand) {
                ll v = 0;
                if (bestL <= s[0] && s[0] <= bestR) v ^= s[0];
                if (bestL <= s[1] && s[1] <= bestR) v ^= s[1];
                if (bestL <= s[2] && s[2] <= bestR) v ^= s[2];

                if (v == got) nxt.push_back(s);
            }

            cand.swap(nxt);
        }

        auto s = cand[0];
        cout << "ans " << s[0] << ' ' << s[1] << ' ' << s[2] << '\n' << flush;
        return 0;
    }

    ll totalXor = ask(1, n);

    ll M = 1;
    while (M <= n) M <<= 1;

    vector<ll> ans;

    function<void(ll, ll, int, ll)> solve =
        [&](ll L, ll R, int cntElements, ll curXor) {
            if (cntElements <= 0) return;

            if (cntElements == 1) {
                ans.push_back(curXor);
                return;
            }

            ll sz = R - L + 1;
            int d = __builtin_ctzll((unsigned long long)sz);
            int bit = d - 1;

            ll mid = L + sz / 2;

            ll ql = max(mid, 1LL);
            ll qr = min(R, n);

            ll rightXor = 0;
            if (ql <= qr) {
                rightXor = ask(ql, qr);
            }

            int parityRight = (int)((rightXor >> bit) & 1LL);
            int rightCnt;

            if (parityRight == 0) {
                rightCnt = (rightXor == 0 ? 0 : 2);
            } else {
                if (cntElements == 3 && rightXor == curXor)
                    rightCnt = 3;
                else
                    rightCnt = 1;
            }

            int leftCnt = cntElements - rightCnt;
            ll leftXor = curXor ^ rightXor;

            if (leftCnt > 0) {
                solve(L, mid - 1, leftCnt, leftXor);
            }
            if (rightCnt > 0) {
                solve(mid, R, rightCnt, rightXor);
            }
        };

    solve(0, M - 1, 3, totalXor);

    cout << "ans " << ans[0] << ' ' << ans[1] << ' ' << ans[2] << '\n' << flush;

    return 0;
}
