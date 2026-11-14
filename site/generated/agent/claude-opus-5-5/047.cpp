#include <bits/stdc++.h>
using namespace std;
typedef long long ll;

// Rotated view: a = cost to (1,1) = x+y-2, K-a = cost to (L,L),
//               b = cost to (1,L) = x-y+L-1, K-b = cost to (L,1), K = 2L-2.
// Optimal solutions: A/A' split by a threshold alpha in a-order; B/B' by a
// threshold beta in b-order. For fixed alpha, all beta are handled by a sweep
// with four two-bin knapsacks (one per quadrant cell). alpha is searched using
// quasiconvexity of Phi (min A s.t. others<=D) and Psi (min A' s.t. others<=D).

static int L, N, T, D, K;
static vector<int> A_, B_, ur, ov;
static const int INF32 = 1000000000;
static vector<int> pool;     // stored prefix arrays
static chrono::steady_clock::time_point t0;
static bool forceScan = false, searchOnly = false;

static double elapsed() { return chrono::duration<double>(chrono::steady_clock::now() - t0).count(); }

// f: min U given V budget x (x in [0,D]); item: U cost cu or V cost cv. out-of-place.
static inline void addH(const int* __restrict f, int* __restrict nf, int cu, int cv) {
    int lim = min(cv, D + 1);
    for (int x = 0; x < lim; x++) nf[x] = f[x] + cu;
    for (int x = lim; x <= D; x++) { int v1 = f[x] + cu, v2 = f[x - cv]; nf[x] = v1 < v2 ? v1 : v2; }
}
// g: min V given U budget a (capped at CAP); item: U cost cu or V cost cv.
static inline void addG(const int* __restrict f, int* __restrict nf, int cu, int cv, int CAP) {
    int lim = min(cu, D + 1);
    for (int x = 0; x < lim; x++) { int v = f[x] + cv; nf[x] = v < CAP ? v : CAP; }
    for (int x = lim; x <= D; x++) { int v1 = f[x] + cv; v1 = v1 < CAP ? v1 : CAP; int v2 = f[x - cu]; nf[x] = v1 < v2 ? v1 : v2; }
}

struct Res { ll phi, psi; };
static const ll LINF = (ll)4e18;

// returns phi, psi; phi<=D means feasible
static Res evalAlpha(int al) {
    int W = D + 1;
    vector<int> idxLL(N + 1), idxHL(N + 1);
    // pool slot 0: zeros (initial LL), slot 1: zeros (initial HL)
    int* base = pool.data();
    fill(base, base + 2 * (size_t)W, 0);
    int nxt = 2;
    idxLL[0] = 0; idxHL[0] = 1;
    for (int k = 0; k < N; k++) {
        int i = ov[k];
        idxLL[k + 1] = idxLL[k]; idxHL[k + 1] = idxHL[k];
        if (ur[i] < al) {
            addH(base + (size_t)idxLL[k] * W, base + (size_t)nxt * W, A_[i], B_[i]);
            idxLL[k + 1] = nxt++;
        } else {
            addH(base + (size_t)idxHL[k] * W, base + (size_t)nxt * W, K - A_[i], B_[i]);
            idxHL[k + 1] = nxt++;
        }
    }
    int CAP = D + 1;
    vector<int> gLH(W, 0), hLH(W, 0), gHH(W, 0), hHH(W, 0), tmp(W);
    ll phi = LINF, psi = LINF;
    for (int be = N; be >= 0; be--) {
        const int* hLL = base + (size_t)idxLL[be] * W;
        const int* hHL = base + (size_t)idxHL[be] * W;
        // Phi
        for (int x = 0; x <= D; x++) {
            int aHL = hHL[D - x];
            if (aHL > D) continue;
            int bh = gHH[D - aHL];
            if (bh > D) continue;
            ll c = (ll)hLL[x] + hLH[D - bh];
            if (c < phi) phi = c;
        }
        if (phi <= D) return {phi, psi};
        for (int x = 0; x <= D; x++) {
            int aLL = hLL[x];
            if (aLL > D) continue;
            int bl = gLH[D - aLL];
            if (bl > D) continue;
            ll c = (ll)hHL[D - x] + hHH[D - bl];
            if (c < psi) psi = c;
        }
        if (be == 0) break;
        int i = ov[be - 1];
        if (ur[i] < al) {
            addG(gLH.data(), tmp.data(), A_[i], K - B_[i], CAP); swap(gLH, tmp);
            addH(hLH.data(), tmp.data(), A_[i], K - B_[i]); swap(hLH, tmp);
        } else {
            addG(gHH.data(), tmp.data(), K - A_[i], K - B_[i], CAP); swap(gHH, tmp);
            addH(hHH.data(), tmp.data(), K - A_[i], K - B_[i]); swap(hHH, tmp);
        }
    }
    return {phi, psi};
}

int main(int argc, char** argv) {
    t0 = chrono::steady_clock::now();
    if (argc > 1 && string(argv[1]) == "scan") forceScan = true;
    if (argc > 1 && string(argv[1]) == "search") searchOnly = true;
    if (scanf("%d %d %d", &L, &N, &T) != 3) return 0;
    vector<int> X(N), Y(N);
    for (int i = 0; i < N; i++) scanf("%d %d", &X[i], &Y[i]);
    D = T / 2; K = 2 * L - 2;
    A_.resize(N); B_.resize(N);
    ll sumMin = 0;
    for (int i = 0; i < N; i++) {
        A_[i] = X[i] + Y[i] - 2; B_[i] = X[i] - Y[i] + L - 1;
        int m = min(min(A_[i], K - A_[i]), min(B_[i], K - B_[i]));
        if (m > D) { puts("No"); return 0; }
        sumMin += m;
    }
    if (sumMin > 4LL * D) { puts("No"); return 0; }
    vector<int> ou(N); ov.resize(N); ur.resize(N);
    iota(ou.begin(), ou.end(), 0); iota(ov.begin(), ov.end(), 0);
    sort(ou.begin(), ou.end(), [&](int i, int j) { return A_[i] != A_[j] ? A_[i] < A_[j] : i < j; });
    sort(ov.begin(), ov.end(), [&](int i, int j) { return B_[i] != B_[j] ? B_[i] < B_[j] : i < j; });
    for (int k = 0; k < N; k++) ur[ou[k]] = k;
    pool.assign((size_t)(N + 2) * (D + 1), 0);

    int lo0 = 0, hi0 = 0;
    for (int i = 0; i < N; i++) { if (A_[i] < K - D) lo0++; if (A_[i] <= D) hi0++; }
    if (lo0 > hi0) lo0 = hi0;

    map<int, Res> cache;
    auto E = [&](int al) -> Res {
        auto it = cache.find(al);
        if (it != cache.end()) return it->second;
        Res r = evalAlpha(al);
        if (r.phi <= D) { puts("Yes"); exit(0); }
        cache[al] = r;
        return r;
    };

    if (argc > 1 && string(argv[1]) == "dump") {
        for (int al = 0; al <= N; al++) { Res r = evalAlpha(al); if (r.phi>=LINF) printf("inf/"); else printf("%lld/", r.phi - D); if (r.psi>=LINF) printf("inf "); else printf("%lld ", r.psi - D);} puts(""); return 0; }
    if (!forceScan) {
        int lo = lo0, hi = hi0;
        while (lo <= hi) {
            int m = (lo + hi) / 2;
            Res r = E(m);
            if (r.phi >= LINF) { lo = m + 1; continue; }
            if (r.psi >= LINF) { hi = m - 1; continue; }
            int prevEq = m, dir = -1;
            ll off = 1;
            while (true) {
                int j = (int)min<ll>((ll)m + off, hi);
                if (j <= prevEq) break;
                Res q = E(j);
                if (q.phi != r.phi || q.psi != r.psi) {
                    bool lower = (q.psi != r.psi) ? (q.psi < r.psi) : (q.phi < r.phi);
                    dir = lower ? 1 : -1;
                    break;
                }
                prevEq = j;
                off = off < 4 ? off + 1 : off * 2;
            }
            if (dir == 1) lo = prevEq + 1; else hi = m - 1;
        }
    }
    // fallback: scan remaining candidates while time allows
    int center = (lo0 + hi0) / 2;
    {
        ll best = LINF;
        for (auto& [al, r] : cache) {
            if (r.phi < LINF && r.psi < LINF && r.phi + r.psi < best) { best = r.phi + r.psi; center = al; }
        }
    }
    vector<int> order;
    for (int al = lo0; al <= hi0; al++) if (!cache.count(al)) order.push_back(al);
    sort(order.begin(), order.end(), [&](int x, int y) { return abs(x - center) < abs(y - center); });
    double lastCost = 0;
    if (searchOnly) order.clear();
    for (int al : order) {
        double st = elapsed();
        if (!forceScan && st + lastCost * 1.5 > 6.0) break;
        E(al);
        lastCost = elapsed() - st;
    }
    if (getenv("DBG")) fprintf(stderr, "evals=%zu range=[%d,%d] t=%.2f\n", cache.size(), lo0, hi0, elapsed());
    puts("No");
    return 0;
}
