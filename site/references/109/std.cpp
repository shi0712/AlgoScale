#include <bits/stdc++.h>
using namespace std;

int main() {
    int n, t; if (!(cin >> n >> t)) return 0;
    int width = 1;
    long long best = n;
    for (int w = 2, depth = 1; w < 2*n; w *= 2, ++depth) {
        long long cost = (n + w - 1) / w - 1 + 1LL * t * depth;
        if (cost < best) { best = cost; width = w; }
    }
    int blocks = (n + width - 1) / width;
    vector<int> bit(blocks + 1);
    auto add = [&](int p, int v) {
        for (++p; p <= blocks; p += p & -p) bit[p] += v;
    };
    auto sum = [&](int p) {
        int total = 0;
        for (; p; p -= p & -p) total += bit[p];
        return total;
    };
    auto zeros = [&](int l, int r) {
        cout << "? " << l << ' ' << r << endl;
        int ones; if (!(cin >> ones) || ones < 0) exit(0);
        return r - l + 1 - ones;
    };
    for (int turn = 0; turn < t; ++turn) {
        int k; if (!(cin >> k) || k < 1) return 0;
        if (turn == 0)
            for (int b = 0; b + 1 < blocks; ++b)
                add(b, zeros(b * width + 1, min(n, (b + 1) * width)));
        int low = 0, high = blocks - 1;
        while (low < high) {
            int mid = (low + high) / 2;
            if (sum(mid + 1) >= k) high = mid;
            else low = mid + 1;
        }
        int b = low; k -= sum(b);
        int l = b * width + 1, r = min(n, (b + 1) * width);
        while (l < r) {
            int mid = (l + r) / 2;
            int count = zeros(l, mid);
            if (k <= count) r = mid;
            else { l = mid + 1; k -= count; }
        }
        cout << "! " << l << endl;
        add(b, -1);
    }
}

