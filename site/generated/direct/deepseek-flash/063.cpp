#include <bits/stdc++.h>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, c;
    cin >> n >> c;

    vector<long long> a(n + 1);
    for (int i = 1; i <= n; ++i) cin >> a[i];

    sort(a.begin() + 1, a.end());

    // prefC[i] = a[i] + a[i-c] + a[i-2c] + ...
    vector<long long> prefC(n + 1, 0);
    for (int i = 1; i <= n; ++i) {
        prefC[i] = a[i];
        if (i > c) prefC[i] += prefC[i - c];
    }

    // C[h] = cost for one helper block that moves h full groups of c people.
    vector<long long> prefSum(c + 2, 0);
    for (int i = 1; i <= c; ++i) {
        prefSum[i] = prefSum[i - 1] + a[i];
    }

    vector<long long> C(c + 2, 0);
    for (int h = 1; h <= c - 1; ++h) {
        C[h] = prefSum[h + 1] + a[h + 1];
    }

    const long long INF = (1LL << 62);
    vector<long long> dp(n + 1, INF);

    deque<int> dq;

    for (int i = 1; i <= n; ++i) {
        if (i > 1) {
            while (!dq.empty() && dp[dq.back()] >= dp[i - 1])
                dq.pop_back();
            dq.push_back(i - 1);

            int low = i - c + 1;
            while (!dq.empty() && dq.front() < low)
                dq.pop_front();
        }

        if (i <= c) {
            dp[i] = a[i];
            continue;
        }

        // Move up to c-1 slow people together with the fastest person.
        long long bestPrev = dp[dq.front()];
        dp[i] = a[i] + a[1] + bestPrev;

        // Move h full groups of c people using h+1 fastest helpers.
        int H = min(c - 1, (i - 1) / (c + 1));

        for (int h = 1; h <= H; ++h) {
            int rem = i - h * c;
            long long slowCost = prefC[i] - (rem > 0 ? prefC[rem] : 0);
            long long val = slowCost + C[h] + dp[rem];
            if (val < dp[i]) dp[i] = val;
        }
    }

    cout << dp[n] << '\n';
    return 0;
}
