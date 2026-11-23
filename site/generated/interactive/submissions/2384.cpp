#include <bits/stdc++.h>
using namespace std;

int n;
long long ask(int i, int j, int k) {
    printf("? %d %d %d\n", i, j, k);
    fflush(stdout);
    long long r;
    if (scanf("%lld", &r) != 1) exit(0);
    if (r < 0) exit(0);
    return r;
}

int main() {
    if (scanf("%d", &n) != 1) return 0;
    vector<long long> c(n + 1, 0);
    static long long R[6][6][6];
    for (int i = 1; i <= 5; i++)
        for (int j = i + 1; j <= 5; j++)
            for (int k = j + 1; k <= 5; k++) {
                long long r = ask(i, j, k);
                int a[3] = {i, j, k};
                sort(a, a + 3);
                do {
                    R[a[0]][a[1]][a[2]] = r;
                } while (next_permutation(a, a + 3));
            }
    auto res = [&](int x, int y, int z) { return R[x + 1][y + 1][z + 1]; };
    int p[5] = {0, 1, 2, 3, 4};
    bool found = false;
    do {
        long long ac = res(p[0], p[1], p[2]);
        long long ae = res(p[0], p[1], p[4]);
        long long ce = res(p[2], p[3], p[4]);
        long long ad = res(p[0], p[1], p[3]);
        long long bd = res(p[1], p[2], p[3]);
        long long t = ac + ae - ce;
        if (t < 0 || (t & 1)) continue;
        long long a = t / 2;
        long long cc = ac - a;
        long long e = ae - a;
        long long d = ad - a;
        long long b = bd - d;
        long long v[5] = {a, b, cc, d, e};
        bool ok = true;
        for (int i = 0; i < 5; i++) {
            if (v[i] < 0) ok = false;
            if (i && v[i] < v[i - 1]) ok = false;
        }
        if (!ok) continue;
        long long val[5];
        for (int i = 0; i < 5; i++) val[p[i]] = v[i];
        for (int i = 0; i < 5 && ok; i++)
            for (int j = i + 1; j < 5 && ok; j++)
                for (int k = j + 1; k < 5 && ok; k++) {
                    long long mn = min({val[i], val[j], val[k]});
                    long long mx = max({val[i], val[j], val[k]});
                    if (mn + mx != res(i, j, k)) ok = false;
                }
        if (!ok) continue;
        for (int i = 0; i < 5; i++) c[i + 1] = val[i];
        found = true;
        break;
    } while (next_permutation(p, p + 5));
    (void)found;
    vector<pair<long long, int>> kn;
    for (int i = 1; i <= 5; i++) kn.push_back({c[i], i});
    sort(kn.begin(), kn.end());
    for (int z = 6; z <= n; z++) {
        int m = kn.size();
        int best = 0;
        long long bg = LLONG_MAX;
        for (int i = 0; i + 1 < m; i++) {
            long long g = kn[i + 1].first - kn[i].first;
            if (g < bg) {
                bg = g;
                best = i;
            }
        }
        long long vx = kn[best].first, vy = kn[best + 1].first;
        int x = kn[best].second, y = kn[best + 1].second;
        long long r = ask(x, y, z);
        long long val;
        if (r < vx + vy) val = r - vy;
        else if (r > vx + vy) val = r - vx;
        else if (vx == vy) val = vx;
        else {
            if (best + 2 < m) {
                int y2 = kn[best + 2].second;
                long long r2 = ask(z, y, y2);
                val = r2 - kn[best + 2].first;
            } else {
                int x2 = kn[best - 1].second;
                long long r2 = ask(z, x, x2);
                val = r2 - kn[best - 1].first;
            }
        }
        c[z] = val;
        pair<long long, int> np = {val, z};
        kn.insert(upper_bound(kn.begin(), kn.end(), np), np);
    }
    printf("!");
    for (int i = 1; i <= n; i++) printf(" %lld", c[i]);
    printf("\n");
    fflush(stdout);
    return 0;
}
