#include <bits/stdc++.h>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, P, Q;
    cin >> n >> P >> Q;

    const int MAXH = 2000000;
    vector<int> freq(MAXH + 1, 0);

    int M = 0;
    for (int i = 0; i < n; ++i) {
        int h;
        cin >> h;
        ++freq[h];
        if (h > M) M = h;
    }

    if (M == 0) {
        cout << 0 << '\n';
        return 0;
    }

    vector<int> isqrt(M + 1);
    int r = 0;
    for (int x = 0; x <= M; ++x) {
        while (1LL * (r + 1) * (r + 1) <= x) ++r;
        isqrt[x] = r;
    }

    vector<int> W(M + 1, 0);

    for (int x = 2; x <= M; ++x) {
        int c = freq[x];
        if (c == 0) continue;

        int cur = x;
        while (cur > 1) {
            W[cur] += c;
            cur = isqrt[cur];
        }
    }

    vector<int> g(M + 1);
    vector<int> cnt(M + 1, 0);
    vector<int> mark(M + 1, 0);
    vector<long long> cache(M + 1, -1);

    int stamp = 0;

    auto computeL = [&](int lambda) -> long long {
        if (cache[lambda] != -1) return cache[lambda];

        ++stamp;
        g[0] = 0;

        long long base = 0;
        int maxB = 0;

        for (int x = 1; x <= M; ++x) {
            int half = x >> 1;
            int val = lambda + g[half];
            int gx = (x < val ? x : val);
            g[x] = gx;

            if (freq[x]) {
                base += 1LL * freq[x] * gx;
            }

            int w = W[x];
            if (w > 0) {
                int b = gx - g[isqrt[x]];
                if (b > 0) {
                    if (mark[b] != stamp) {
                        mark[b] = stamp;
                        cnt[b] = w;
                    } else {
                        cnt[b] += w;
                    }
                    if (b > maxB) maxB = b;
                }
            }
        }

        long long reduction = 0;
        long long need = P;

        for (int b = maxB; b >= 1 && need > 0; --b) {
            if (mark[b] == stamp) {
                int c = cnt[b];
                long long take = min<long long>(c, need);
                reduction += take * b;
                need -= take;
            }
        }

        long long res = base - reduction;
        cache[lambda] = res;
        return res;
    };

    auto getF = [&](int lambda) -> long long {
        return computeL(lambda) - 1LL * lambda * Q;
    };

    int lo = 0, hi = M;

    while (lo < hi) {
        int mid = (lo + hi) >> 1;
        long long f1 = getF(mid);
        long long f2 = getF(mid + 1);

        if (f1 < f2) lo = mid + 1;
        else hi = mid;
    }

    cout << getF(lo) << '\n';
    return 0;
}
