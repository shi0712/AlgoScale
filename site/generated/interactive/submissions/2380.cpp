#include <bits/stdc++.h>
using namespace std;

int main() {
    int n;
    if (!(cin >> n)) return 0;
    int trans[32][2];
    for (int m = 0; m < 32; m++) {
        for (int D = 0; D < 2; D++) {
            int nm = 0;
            for (int s = 0; s < 5; s++) {
                if (!(m >> s & 1)) continue;
                if (s == 4) {
                    nm |= 1 << (D * 2);
                } else {
                    int d = s / 2, k = s % 2 + 1;
                    if (d != D) nm |= 1 << (D * 2);
                    else if (k == 1) nm |= 1 << (D * 2 + 1);
                }
            }
            trans[m][D] = nm;
        }
    }
    auto f = [&](int m) -> long long {
        if (m == 0) return 0;
        return 4 + __builtin_popcount(m);
    };
    vector<int> mask(n + 2, 16);
    vector<long long> P[2];
    P[0].assign(n + 2, 0);
    P[1].assign(n + 2, 0);
    while (true) {
        vector<int> alive;
        for (int x = 1; x <= n; x++)
            if (mask[x]) alive.push_back(x);
        if (alive.size() <= 2) break;
        long long tot0[2] = {0, 0};
        for (int y = 0; y < 2; y++) {
            P[y][0] = 0;
            for (int x = 1; x <= n; x++) {
                long long v1 = f(trans[mask[x]][y ^ 1]);
                long long v0 = f(trans[mask[x]][y]);
                tot0[y] += v0;
                P[y][x] = P[y][x - 1] + (v1 - v0);
            }
        }
        int K = min<int>(alive.size(), 80);
        vector<int> b;
        b.push_back(1);
        b.push_back(n + 1);
        for (int i = 0; i < K; i++) b.push_back(alive[(long long)i * alive.size() / K]);
        sort(b.begin(), b.end());
        b.erase(unique(b.begin(), b.end()), b.end());
        long long bestMax = LLONG_MAX, bestSum = LLONG_MAX;
        int bl = 1, br = 1;
        for (size_t i = 0; i < b.size(); i++) {
            for (size_t j = i + 1; j < b.size(); j++) {
                int l = b[i], r = b[j] - 1;
                if (r < l) continue;
                long long p0 = tot0[0] + P[0][r] - P[0][l - 1];
                long long p1 = tot0[1] + P[1][r] - P[1][l - 1];
                long long mx = max(p0, p1), sm = p0 + p1;
                if (mx < bestMax || (mx == bestMax && sm < bestSum)) {
                    bestMax = mx;
                    bestSum = sm;
                    bl = l;
                    br = r;
                }
            }
        }
        printf("? %d %d\n", bl, br);
        fflush(stdout);
        int h;
        if (scanf("%d", &h) != 1) return 0;
        if (h < 0) return 0;
        int len = br - bl + 1;
        int y = (h == len - 1) ? 1 : 0;
        for (int x = 1; x <= n; x++) {
            if (!mask[x]) continue;
            int A = (x >= bl && x <= br) ? 1 : 0;
            mask[x] = trans[mask[x]][y ^ A];
        }
    }
    vector<int> alive;
    for (int x = 1; x <= n; x++)
        if (mask[x]) alive.push_back(x);
    if (alive.empty()) alive.push_back(1);
    for (size_t i = 0; i < alive.size(); i++) {
        printf("! %d\n", alive[i]);
        fflush(stdout);
        int r;
        if (scanf("%d", &r) != 1) return 0;
        if (r == 1 || r < 0) break;
    }
    printf("#\n");
    fflush(stdout);
    return 0;
}
