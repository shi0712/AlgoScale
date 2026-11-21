#include <bits/stdc++.h>
using namespace std;

// ---------------- geometry ----------------
static int N, M;
static int covm[64];
static int ivA[64], ivB[64];
static int idxof[11][11];

static void build(int n) {
    N = n; M = 0;
    for (int i = 0; i < N; i++) for (int j = i; j < N; j++) {
        ivA[M] = i; ivB[M] = j;
        int cm = 0; for (int c = i; c <= j; c++) cm |= 1 << c;
        covm[M] = cm; idxof[i][j] = M; M++;
    }
}

// ---------------- rng ----------------
static uint64_t rngs = 0x243F6A8885A308D3ULL;
static inline uint64_t xr() { rngs ^= rngs << 13; rngs ^= rngs >> 7; rngs ^= rngs << 17; return rngs; }
static inline double rnd01() { return (double)(xr() >> 11) / 9007199254740992.0; }
static inline int rndi(int k) { return (int)(xr() % (uint64_t)k); }

// ---------------- particles ----------------
struct PW { uint64_t w; uint8_t h[11]; uint16_t vis, one; };

static inline int delta_from(const PW& p, int b) {
    if ((p.w >> b) & 1ULL) return __builtin_popcount((unsigned)(covm[b] & p.one));
    else return -__builtin_popcount((unsigned)(covm[b] & p.vis));
}
static inline void refresh(PW& p) {
    p.vis = 0; p.one = 0;
    for (int c = 0; c < N; c++) {
        if (p.h[c] == 0) p.vis |= (uint16_t)(1u << c);
        else if (p.h[c] == 1) p.one |= (uint16_t)(1u << c);
    }
}
static inline void toggle_p(PW& p, int b) {
    bool act = (p.w >> b) & 1ULL; int cb = covm[b];
    if (act) { for (int c = 0; c < N; c++) if ((cb >> c) & 1) p.h[c]--; }
    else { for (int c = 0; c < N; c++) if ((cb >> c) & 1) p.h[c]++; }
    p.w ^= 1ULL << b;
    refresh(p);
}
static inline int pcnt(const PW& p) { return __builtin_popcount((unsigned)p.vis); }

static PW make_particle(uint64_t w) {
    PW p; p.w = w;
    for (int c = 0; c < N; c++) p.h[c] = 0;
    uint64_t t = w;
    while (t) { int b = __builtin_ctzll(t); t &= t - 1; int cb = covm[b]; for (int c = 0; c < N; c++) if ((cb >> c) & 1) p.h[c]++; }
    refresh(p);
    return p;
}

static uint64_t sample_world() {
    double r = rnd01();
    int kind;
    if (r < 0.45) kind = 0;
    else if (r < 0.60) kind = 1;
    else if (r < 0.80) kind = 2;
    else kind = 3;
    uint64_t w = 0;
    if (kind == 0) {
        for (int b = 0; b < M; b++) if (rnd01() < 0.5) w |= 1ULL << b;
    } else if (kind == 1) {
        int t = rndi(12);
        if (t == 0) w = (M >= 64 ? ~0ULL : ((1ULL << M) - 1));
        else if (t == 1) { for (int c = 0; c < N; c++) w |= 1ULL << idxof[c][c]; }
        else if (t == 2) { for (int j = 0; j < N; j++) w |= 1ULL << idxof[0][j]; }
        else if (t == 3) { for (int i = 0; i < N; i++) w |= 1ULL << idxof[i][N-1]; }
        else if (t == 4) { int L = 1 + rndi(N); for (int i = 0; i + L - 1 < N; i++) w |= 1ULL << idxof[i][i+L-1]; }
        else if (t == 5) { int l = rndi(N), r = l + rndi(N - l); for (int i = l; i <= r; i++) for (int j = i; j <= r; j++) w |= 1ULL << idxof[i][j]; }
        else if (t == 6) { int i = rndi(N), j = i + rndi(N - i); w |= 1ULL << idxof[i][j]; }
        else if (t == 7) { int l = rndi(N), r = l + rndi(N - l); for (int i = l; i <= r; i++) for (int j = i; j <= r; j++) if (j - i + 1 >= 2) w |= 1ULL << idxof[i][j]; }
        else if (t == 8) { int l = rndi(N), r = l + rndi(N - l); for (int i = l; i <= r; i++) for (int j = i; j <= r; j++) if (rnd01() < 0.5) w |= 1ULL << idxof[i][j]; }
        else if (t == 9) { int k = 1 + rndi(M); for (int tt = 0; tt < k; tt++) { int b; do { b = rndi(M); } while ((w >> b) & 1ULL); w |= 1ULL << b; } }
        else if (t == 10) { int l = rndi(N), r = l + rndi(N - l), L = 1 + rndi(3); for (int i = l; i <= r; i++) for (int j = i; j <= r; j++) if (j - i + 1 >= L) w |= 1ULL << idxof[i][j]; }
        else { int i = rndi(N); for (int j = i; j < N; j++) if (rnd01() < 0.7) w |= 1ULL << idxof[i][j]; }
    } else if (kind == 2) {
        static const int ks[10] = {1,1,2,2,3,3,4,5,6,8};
        int k = ks[rndi(10)]; if (k > M) k = M;
        for (int t = 0; t < k; t++) { int b; do { b = rndi(M); } while ((w >> b) & 1ULL); w |= 1ULL << b; }
    } else {
        static const double ps[7] = {0.03, 0.07, 0.15, 0.3, 0.6, 0.8, 0.92};
        double p = ps[rndi(7)];
        for (int b = 0; b < M; b++) if (rnd01() < p) w |= 1ULL << b;
    }
    return w;
}

// ---------------- policy ----------------
static vector<PW> pool;
static int KPOOL = 200000;
static int MAXSEL = 20000;
static int prevAction = -1;
static bool poolReset = false;

static int choose_action() {
    int P = (int)pool.size();
    int step = (P > MAXSEL) ? (P / MAXSEL) : 1;
    int bestb = 0; int bestpos = -1, bestact = -1;
    static int posc[64], actc[64];
    for (int b = 0; b < M; b++) { posc[b] = 0; actc[b] = 0; }
    for (int i = 0; i < P; i += step) {
        const PW& p = pool[i];
        for (int b = 0; b < M; b++) if (delta_from(p, b) > 0) posc[b]++;
    }
    int step2 = step * 2;
    for (int i = 0; i < P; i += step2) {
        uint64_t w = pool[i].w;
        for (int b = 0; b < M; b++) if ((w >> b) & 1ULL) actc[b]++;
    }
    for (int b = 0; b < M; b++) {
        if (poolReset && b == prevAction) continue;
        if (posc[b] > bestpos || (posc[b] == bestpos && actc[b] > bestact)) {
            bestpos = posc[b]; bestact = actc[b]; bestb = b;
        }
    }
    if (bestpos < 0) bestb = rndi(M);
    return bestb;
}

static void seed_pool(int target) {
    pool.clear(); pool.reserve(target);
    for (int i = 0; i < target; i++) pool.push_back(make_particle(sample_world()));
}

static void init_pool(int cnt) {
    if (M <= 10) KPOOL = 400000;
    else if (M <= 21) KPOOL = 300000;
    else if (M <= 36) KPOOL = 200000;
    else KPOOL = 250000;
    MAXSEL = (M > 36) ? 12000 : 20000;
    seed_pool(KPOOL);
    vector<PW> np; np.reserve(KPOOL);
    for (size_t i = 0; i < pool.size(); i++) if (pcnt(pool[i]) == cnt) np.push_back(pool[i]);
    if ((int)np.size() >= KPOOL / 10) pool.swap(np);
}

// filter pool after observing that flipping b gives new count nc
static void observe(int b, int nc) {
    vector<PW> np; np.reserve(pool.size());
    for (size_t i = 0; i < pool.size(); i++) {
        const PW& p = pool[i];
        int d = delta_from(p, b);
        if (pcnt(p) + d == nc) { PW q = p; toggle_p(q, b); np.push_back(q); }
    }
    if ((int)np.size() >= KPOOL / 4) { pool.swap(np); return; }
    if (!np.empty()) {
        vector<PW> rp; rp.reserve(KPOOL);
        int base = (int)np.size();
        for (int i = 0; i < KPOOL; i++) rp.push_back(np[rndi(base)]);
        pool.swap(rp);
        return;
    }
    seed_pool(KPOOL);
}

#ifndef LOCAL_TEST
int main() {
    if (!(cin >> N)) return 0;
    build(N);
    int cnt;
    if (!(cin >> cnt)) return 0;
    if (cnt == N) return 0;
    init_pool(cnt);
    for (long long flips = 0; flips < 200000; flips++) {
        int b = choose_action();
        cout << (ivA[b] + 1) << " " << (ivB[b] + 1) << "\n" << flush;
        int nc;
        if (!(cin >> nc)) return 0;
        if (nc == N) return 0;
        prevAction = b;
        observe(b, nc);
    }
    return 0;
}
#endif
