#include <bits/stdc++.h>
using namespace std;

int n;
long long cntv;

long long doFlip(int a, int b) {
    printf("%d %d\n", a, b);
    fflush(stdout);
    long long r;
    if (scanf("%lld", &r) != 1) exit(0);
    if (r < 0) exit(0);
    cntv = r;
    if (r == n) exit(0);
    return r;
}

int main() {
    if (scanf("%d", &n) != 1) return 0;
    if (scanf("%lld", &cntv) != 1) return 0;
    if (cntv == n) return 0;
    vector<bool> V(n + 2, false);
    for (int i = 1; i <= n; i++) {
        if (cntv == n) return 0;
        if (V[i]) continue;
        int L = n + 1;
        for (int j = i + 1; j <= n; j++) if (V[j]) { L = j; break; }
        int r = L - 1 - i; // bits 0..r, bit q -> barrier [i, i+q]
        vector<pair<int, long long>> rec;
        int p = 0;
        rec.push_back({0, cntv});
        bool found = false;
        int total = 1 << (r + 1);
        for (int t = 1; t < total && !found; t++) {
            int q = __builtin_ctz(t);
            long long prev = cntv;
            doFlip(i, i + q);
            p ^= (1 << q);
            rec.push_back({p, cntv});
            if (q == 0) {
                long long d = cntv - prev;
                if (d == 1) found = true;
                else if (d == -1) {
                    doFlip(i, i);
                    p ^= 1;
                    rec.push_back({p, cntv});
                    found = true;
                }
            }
        }
        // derive knowledge
        int x0 = p;
        map<int, long long> G;
        G[n] = 0;
        for (auto &pr : rec) {
            int x = x0 ^ pr.first;
            int m = i;
            for (int q = r; q >= 1; q--) if (x >> q & 1) { m = i + q; break; }
            int c = (x == 0) ? 1 : 0;
            long long g = pr.second - (i - 1) - c;
            G[m] = g;
        }
        vector<pair<int, long long>> pts(G.begin(), G.end());
        for (size_t k = 0; k + 1 < pts.size(); k++) {
            int a = pts[k].first, b = pts[k + 1].first;
            long long diff = pts[k].second - pts[k + 1].second;
            if (diff == (long long)(b - a)) {
                for (int c = a + 1; c <= b; c++) if (c >= 1 && c <= n) V[c] = true;
            }
        }
    }
    return 0;
}
