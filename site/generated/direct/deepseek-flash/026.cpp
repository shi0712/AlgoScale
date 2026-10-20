#include <bits/stdc++.h>
using namespace std;

static constexpr int MOD = 998244353;

long long mod_pow(long long a, long long e) {
    long long r = 1;
    while (e) {
        if (e & 1) r = r * a % MOD;
        a = a * a % MOD;
        e >>= 1;
    }
    return r;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, m, k;
    cin >> n >> m >> k;

    vector<int> a(n);
    for (int i = 0; i < n; ++i) cin >> a[i];

    vector<int> b(m);
    for (int i = 0; i < m; ++i) cin >> b[i];

    // X: normal cards first, then k bombs represented by -1
    // To distinguish identical values, we permute indices.
    int NX = n + k;
    vector<int> xval(NX);
    for (int i = 0; i < n; ++i) xval[i] = a[i];
    for (int i = 0; i < k; ++i) xval[n + i] = -1;

    vector<int> xidx(NX);
    iota(xidx.begin(), xidx.end(), 0);

    vector<int> yidx(m);
    iota(yidx.begin(), yidx.end(), 0);

    long long winX = 0, winY = 0, draw = 0;
    long long total = 0;

    // This brute-force version is intended for very small inputs.
    do {
        do {
            int px = 0, py = 0;
            // Simulate battle.
            // We need copies because queues are consumed.
            vector<int> qx(NX), qy(m);
            for (int i = 0; i < NX; ++i) qx[i] = xval[xidx[i]];
            for (int i = 0; i < m; ++i) qy[i] = b[yidx[i]];

            int ix = 0, iy = 0;
            while (ix < NX && iy < m) {
                int x = qx[ix];
                int y = qy[iy];

                if (x == -1) {
                    // bomb vs any Y card: both removed
                    ++ix;
                    ++iy;
                } else {
                    if (x > y) {
                        ++iy;
                    } else if (x < y) {
                        ++ix;
                    } else {
                        ++ix;
                        ++iy;
                    }
                }
            }

            if (ix == NX && iy == m) ++draw;
            else if (ix < NX) ++winX;
            else ++winY;

            ++total;
        } while (next_permutation(yidx.begin(), yidx.end()));
    } while (next_permutation(xidx.begin(), xidx.end()));

    total %= MOD;
    long long invTotal = mod_pow(total, MOD - 2);

    winX %= MOD;
    winY %= MOD;
    draw %= MOD;

    cout << winX * invTotal % MOD << '\n';
    cout << winY * invTotal % MOD << '\n';
    cout << draw * invTotal % MOD << '\n';

    return 0;
}
