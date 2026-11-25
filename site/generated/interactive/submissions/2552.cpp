#include <bits/stdc++.h>
using namespace std;

int n, t, B;
vector<int> blockZeros;
vector<vector<int>> pref;

int query(int l, int r) {
    cout << "? " << l << " " << r << endl;
    int res;
    cin >> res;
    if (res == -1) exit(0);
    return res;
}

int getPref(int b, int i) {
    if (i == 0) return 0;
    auto &vec = pref[b];
    if (vec.empty()) {
        int len = min(B, n - b * B);
        vec.assign(len + 1, -1);
        vec[0] = 0;
    }
    if (vec[i] == -1) {
        int L = b * B + 1;
        int R = L + i - 1;
        vec[i] = query(L, R);
    }
    return vec[i];
}

void flipZero(int b, int offset) {
    int len = min(B, n - b * B);
    auto &vec = pref[b];
    for (int i = offset + 1; i <= len; ++i) {
        if (vec[i] != -1) ++vec[i];
    }
    --blockZeros[b];
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(0);

    cin >> n >> t;

    int bestB = 1;
    double bestCost = 1e18;
    for (int b = 1; b <= n; ++b) {
        double cost = (n + b - 1) / b + t * log2(b);
        if (cost < bestCost) {
            bestCost = cost;
            bestB = b;
        }
    }
    B = bestB;
    int blockCount = (n + B - 1) / B;
    blockZeros.assign(blockCount, -1);
    pref.resize(blockCount);

    for (int round = 0; round < t; ++round) {
        int k;
        cin >> k;
        if (k == -1) break;

        int cumulative = 0;
        int b = 0;
        for (; b < blockCount; ++b) {
            if (blockZeros[b] == -1) {
                int L = b * B + 1;
                int R = min(n, (b + 1) * B);
                int len = R - L + 1;
                int ones = query(L, R);
                blockZeros[b] = len - ones;
                pref[b].assign(len + 1, -1);
                pref[b][0] = 0;
                pref[b][len] = ones;
            }
            cumulative += blockZeros[b];
            if (cumulative >= k) break;
        }

        int prev = cumulative - blockZeros[b];
        int k2 = k - prev;
        int len = min(B, n - b * B);
        int L = b * B + 1;

        int lo = 1, hi = len, ans = len;
        while (lo <= hi) {
            int mid = (lo + hi) / 2;
            int ones = getPref(b, mid);
            int zeros = mid - ones;
            if (zeros >= k2) {
                ans = mid;
                hi = mid - 1;
            } else {
                lo = mid + 1;
            }
        }

        int pos = L + ans - 1;
        cout << "! " << pos << endl;
        flipZero(b, ans - 1);
    }

    return 0;
}
