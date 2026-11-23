#include <bits/stdc++.h>
using namespace std;

int n;

static int ask(int i, int j) {
    printf("? %d %d\n", i, j);
    fflush(stdout);
    int x;
    if (scanf("%d", &x) != 1) exit(0);
    if (x < 0) exit(0);
    return x;
}

int main() {
    if (scanf("%d", &n) != 1) return 0;
    vector<int> q(n);
    for (int j = 0; j < n; j++) q[j] = ask(0, j);
    vector<int> cand;
    vector<vector<uint16_t>> vec;
    vector<int> b(n), p(n), seen(n, -1);
    for (int a = 0; a < n; a++) {
        bool ok = true;
        for (int j = 0; j < n && ok; j++) {
            int v = a ^ q[j];
            if (v >= n || seen[v] == a) { ok = false; break; }
            seen[v] = a;
            b[j] = v;
        }
        if (!ok) continue;
        for (int j = 0; j < n; j++) p[b[j]] = j;
        if (p[0] != a) continue;
        vector<uint16_t> v(n);
        for (int i = 0; i < n; i++) v[i] = p[i] ^ b[0];
        cand.push_back(a);
        vec.push_back(move(v));
    }
    vector<int> alive(cand.size());
    iota(alive.begin(), alive.end(), 0);
    mt19937 rng(712367);
    vector<int> cnt(8192 * 2, 0);
    while (alive.size() > 1 && n > 1) {
        int bestI = -1;
        size_t bestMax = alive.size();
        for (int t = 0; t < 25; t++) {
            int i = 1 + rng() % (n - 1);
            size_t mx = 0;
            for (int c : alive) { int v = vec[c][i]; cnt[v]++; mx = max(mx, (size_t)cnt[v]); }
            for (int c : alive) cnt[vec[c][i]] = 0;
            if (mx < bestMax) { bestMax = mx; bestI = i; }
        }
        if (bestI < 0) {
            // fallback: find any index where some candidate differs from first
            int c0 = alive[0];
            for (size_t k = 1; k < alive.size() && bestI < 0; k++) {
                int c = alive[k];
                for (int i = 1; i < n; i++) if (vec[c][i] != vec[c0][i]) { bestI = i; break; }
            }
            if (bestI < 0) break;
        }
        int ans = ask(bestI, 0);
        vector<int> na;
        for (int c : alive) if (vec[c][bestI] == ans) na.push_back(c);
        alive = na;
        if (alive.empty()) return 0;
    }
    int a = cand[alive[0]];
    for (int j = 0; j < n; j++) b[j] = a ^ q[j];
    for (int j = 0; j < n; j++) p[b[j]] = j;
    printf("!\n%d\n", (int)alive.size());
    for (int i = 0; i < n; i++) printf("%d%c", p[i], i + 1 < n ? ' ' : '\n');
    fflush(stdout);
    return 0;
}
