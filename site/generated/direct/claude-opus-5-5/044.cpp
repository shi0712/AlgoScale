#include <bits/stdc++.h>
using namespace std;
typedef unsigned long long u64;

int N, M, K;
u64 adjm[40], cl[40], sq[40];
mt19937_64 rng(712367);

static inline int pc(u64 x) { return __builtin_popcountll(x); }
static inline int ctz(u64 x) { return __builtin_ctzll(x); }

int capk(int k) {
    int d = 2 * k + 1;
    return d <= K - 1 ? d + 1 : 2 * K - 1 - d;
}

static inline void masks(int a, int y, u64 *cov, u64 &m1, u64 &m2) {
    u64 H = cl[a] & cl[y];
    m1 = H & ~cov[a] & ~(1ULL << a);
    if (y == a) {
        m2 = 0;
    } else {
        m2 = H & ~cov[y] & ~(1ULL << y);
        if ((adjm[a] >> y & 1) && !((cov[a] >> y) & 1)) m2 &= ~(1ULL << a);
    }
}

int attempt(int start, int wUnc, int noise, vector<int> &u, vector<vector<int>> &hosts) {
    u64 cov[40];
    memset(cov, 0, sizeof(cov));
    int R = M;
    u.assign(1, start);
    hosts.clear();
    int a = start;
    for (int k = 0; R > 0 && k <= K - 2; k++) {
        int cap = capk(k);
        long long bs = -1;
        int by = -1;
        for (u64 t = sq[a]; t; t &= t - 1) {
            int y = ctz(t);
            u64 m1, m2;
            masks(a, y, cov, m1, m2);
            int c2 = pc(m1 & m2), c1 = pc(m1 ^ m2);
            int t2 = min(c2, cap);
            int gain = 2 * t2 + min(c1, cap - t2);
            if (gain == 0) continue;
            long long sc = (long long)gain * 64 + (long long)wUnc * pc(adjm[y] & ~cov[y]) + (long long)(rng() % noise);
            if (sc > bs) { bs = sc; by = y; }
        }
        int y;
        vector<int> hl;
        if (by < 0) {
            int dist[40];
            for (int i = 0; i < N; i++) dist[i] = -1;
            int q[40], qh = 0, qt = 0;
            for (int i = 0; i < N; i++)
                if (adjm[i] & ~cov[i]) { dist[i] = 0; q[qt++] = i; }
            while (qh < qt) {
                int v = q[qh++];
                for (u64 t = sq[v]; t; t &= t - 1) {
                    int w = ctz(t);
                    if (dist[w] < 0) { dist[w] = dist[v] + 1; q[qt++] = w; }
                }
            }
            int bd = 1000;
            vector<int> opts;
            for (u64 t = sq[a]; t; t &= t - 1) {
                int w = ctz(t);
                if (dist[w] < 0) continue;
                if (dist[w] < bd) { bd = dist[w]; opts.clear(); }
                if (dist[w] == bd) opts.push_back(w);
            }
            if (opts.empty()) break;
            y = opts[rng() % opts.size()];
        } else {
            y = by;
            u64 m1, m2;
            masks(a, y, cov, m1, m2);
            int left = cap;
            for (u64 t = m1 & m2; t && left > 0; t &= t - 1, left--) hl.push_back(ctz(t));
            for (u64 t = m1 ^ m2; t && left > 0; t &= t - 1, left--) hl.push_back(ctz(t));
            for (int x : hl) {
                if (x != a && !((cov[a] >> x) & 1)) {
                    cov[a] |= 1ULL << x; cov[x] |= 1ULL << a; R--;
                }
                if (x != y && !((cov[y] >> x) & 1)) {
                    cov[y] |= 1ULL << x; cov[x] |= 1ULL << y; R--;
                }
            }
        }
        hosts.push_back(hl);
        u.push_back(y);
        a = y;
    }
    return R;
}

int main() {
    int T;
    scanf("%d", &T);
    while (T--) {
        scanf("%d %d", &N, &M);
        for (int i = 0; i < N; i++) adjm[i] = 0;
        for (int i = 0; i < M; i++) {
            int a, b;
            scanf("%d %d", &a, &b);
            a--; b--;
            adjm[a] |= 1ULL << b;
            adjm[b] |= 1ULL << a;
        }
        if (N == 1) { printf("1\n1\n"); continue; }
        K = (5 * N) / 4;
        for (int i = 0; i < N; i++) cl[i] = adjm[i] | (1ULL << i);
        for (int i = 0; i < N; i++) {
            sq[i] = 0;
            for (u64 t = cl[i]; t; t &= t - 1) sq[i] |= cl[ctz(t)];
        }
        auto st = chrono::steady_clock::now();
        vector<int> bu;
        vector<vector<int>> bh;
        int bestRem = INT_MAX;
        int it = 0;
        while (true) {
            int start;
            if (it == 0) {
                start = 0;
                for (int i = 0; i < N; i++) if (pc(adjm[i]) > pc(adjm[start])) start = i;
            } else start = rng() % N;
            int wUnc = (it == 0) ? 8 : (int)(rng() % 40);
            int noise = (it == 0) ? 1 : 1 + (int)(rng() % 80);
            vector<int> u;
            vector<vector<int>> h;
            int rem = attempt(start, wUnc, noise, u, h);
            if (rem < bestRem) { bestRem = rem; bu = u; bh = h; }
            it++;
            if (bestRem == 0) break;
            double el = chrono::duration<double>(chrono::steady_clock::now() - st).count();
            if (el > 0.004) break;
        }
        vector<int> u = bu;
        while ((int)u.size() < K) u.push_back(u.back());
        if ((int)u.size() > K) u.resize(K);
        vector<vector<int>> g(K, vector<int>(K, 0));
        for (int d = 0; d <= 2 * K - 2; d++) {
            int imin = max(0, d - (K - 1)), imax = min(d, K - 1);
            if (d % 2 == 0) {
                for (int i = imin; i <= imax; i++) g[i][d - i] = u[d / 2];
            } else {
                int k = d / 2;
                u64 H = cl[u[k]] & cl[u[k + 1]];
                int filler = (H >> u[k] & 1) ? u[k] : ctz(H);
                int idx = 0;
                for (int i = imin; i <= imax; i++, idx++) {
                    int c = filler;
                    if (k < (int)bh.size() && idx < (int)bh[k].size()) c = bh[k][idx];
                    g[i][d - i] = c;
                }
            }
        }
        printf("%d\n", K);
        for (int i = 0; i < K; i++) {
            for (int j = 0; j < K; j++) printf("%d%c", g[i][j] + 1, j + 1 == K ? '\n' : ' ');
        }
    }
    return 0;
}
