#include <bits/stdc++.h>
using namespace std;
typedef unsigned long long ull;
typedef long long ll;
static const ull P = 998244353ULL;
static const ull NN = 998244352ULL;
static const int MAXV = 200005;

ull pw(ull a, ull e) {
    ull r = 1; a %= P;
    while (e) { if (e & 1) r = r * a % P; a = a * a % P; e >>= 1; }
    return r;
}

vector<ull> LG;
vector<ull> fct, ifct;

void ntt(vector<unsigned>& a, bool inv) {
    int n = a.size();
    for (int i = 1, j = 0; i < n; i++) {
        int bit = n >> 1;
        for (; j & bit; bit >>= 1) j ^= bit;
        j ^= bit;
        if (i < j) swap(a[i], a[j]);
    }
    vector<unsigned> w;
    for (int len = 2; len <= n; len <<= 1) {
        ull wl = pw(3, (P - 1) / len);
        if (inv) wl = pw(wl, P - 2);
        int h = len / 2;
        w.assign(h, 1);
        for (int i = 1; i < h; i++) w[i] = (ull)w[i - 1] * wl % P;
        for (int i = 0; i < n; i += len) {
            for (int j = 0; j < h; j++) {
                unsigned u = a[i + j];
                unsigned v = (ull)a[i + j + h] * w[j] % P;
                unsigned x = u + v; if (x >= P) x -= P;
                unsigned y = u + P - v; if (y >= P) y -= P;
                a[i + j] = x; a[i + j + h] = y;
            }
        }
    }
    if (inv) {
        ull ni = pw(n, P - 2);
        for (auto& x : a) x = (ull)x * ni % P;
    }
}

int n_g, U_g;
vector<int> zv;
vector<ull> S;

void crossBlock(int l, int mid, int r) {
    int zl = zv[l];
    int V = zv[mid - 1] - zl;
    int lenW = r - mid + V;
    int N = 1;
    while (N < lenW) N <<= 1;
    vector<unsigned> cr(N, 0), w0(N, 0), w1(N, 0), w2(N, 0);
    for (int d = l; d < mid; d++) cr[V - (zv[d] - zl)]++;
    int B0 = U_g - mid + zl;
    int lo = B0 - (r - mid - 1);
    for (int t = 0; t < lenW; t++) {
        ull v = LG[lo + t];
        w0[t] = v & 1023; w1[t] = (v >> 10) & 1023; w2[t] = v >> 20;
    }
    ntt(cr, false); ntt(w0, false); ntt(w1, false); ntt(w2, false);
    for (int i = 0; i < N; i++) {
        w0[i] = (ull)w0[i] * cr[i] % P;
        w1[i] = (ull)w1[i] * cr[i] % P;
        w2[i] = (ull)w2[i] * cr[i] % P;
    }
    ntt(w0, true); ntt(w1, true); ntt(w2, true);
    for (int k = 0; k < r - mid; k++) {
        int j = (r - mid - 1) + V - k;
        ull val = (ull)w0[j] + ((ull)w1[j] << 10) + ((ull)w2[j] << 20);
        S[mid + k] = (S[mid + k] + val) % NN;
    }
}

void solve(int l, int r) {
    if (r - l <= 64) {
        for (int K = l; K < r; K++) {
            ull s = 0;
            for (int d = l; d <= K; d++) s += LG[U_g - K + zv[d]];
            S[K] = (S[K] + s) % NN;
        }
        return;
    }
    int mid = (l + r) / 2;
    solve(l, mid);
    solve(mid, r);
    crossBlock(l, mid, r);
}

ull dlogPH(ull h, const vector<ull>& gi, const vector<ull>& tab2, ull inv2) {
    ull h1 = pw(h, 119);
    ull cur = h1;
    ull e2 = 0;
    for (int k = 0; k < 23; k++) {
        ull t = cur;
        for (int s = 0; s < 22 - k; s++) t = t * t % P;
        if (t != 1) { e2 |= (1ULL << k); cur = cur * gi[k] % P; }
    }
    ull h2 = pw(h, 1ULL << 23);
    int e119 = 0;
    for (int i = 0; i < 119; i++) if (tab2[i] == h2) { e119 = i; break; }
    ull t = ((ull)(e119 + 119 - (int)(e2 % 119)) % 119) * inv2 % 119;
    return e2 + (1ULL << 23) * t;
}

int main() {
    // precompute discrete logs
    {
        ull g1 = pw(3, 119);
        ull g1inv = pw(g1, P - 2);
        vector<ull> gi(23);
        gi[0] = g1inv;
        for (int k = 1; k < 23; k++) gi[k] = gi[k - 1] * gi[k - 1] % P;
        ull g2 = pw(3, 1ULL << 23);
        vector<ull> tab2(119);
        tab2[0] = 1;
        for (int i = 1; i < 119; i++) tab2[i] = tab2[i - 1] * g2 % P;
        ull pw23 = (1ULL << 23) % 119;
        ull inv2 = 1;
        for (ull x = 1; x < 119; x++) if (x * pw23 % 119 == 1) { inv2 = x; break; }
        vector<int> spf(MAXV, 0);
        for (int i = 2; i < MAXV; i++)
            if (!spf[i]) for (int j = i; j < MAXV; j += i) if (!spf[j]) spf[j] = i;
        LG.assign(MAXV, 0);
        for (int i = 2; i < MAXV; i++) {
            if (spf[i] == i) LG[i] = dlogPH(i, gi, tab2, inv2);
            else LG[i] = (LG[spf[i]] + LG[i / spf[i]]) % NN;
        }
        fct.assign(MAXV, 1); ifct.assign(MAXV, 1);
        for (int i = 1; i < MAXV; i++) fct[i] = fct[i - 1] * i % P;
        ifct[MAXV - 1] = pw(fct[MAXV - 1], P - 2);
        for (int i = MAXV - 1; i > 0; i--) ifct[i - 1] = ifct[i] * i % P;
    }
    int T;
    scanf("%d", &T);
    while (T--) {
        int n, m;
        scanf("%d %d", &n, &m);
        vector<int> a(m + 2);
        for (int i = 1; i <= m; i++) scanf("%d", &a[i]);
        vector<int> last(2 * n + 2, 0), nx(m + 2, 0);
        for (int i = m; i >= 1; i--) { nx[i] = last[a[i]]; last[a[i]] = i; }
        vector<ll> L(m + 2, 0);
        for (int i = m - 1; i >= 1; i--) {
            L[i] = L[i + 1] + 1;
            if (nx[i]) L[i] = max(L[i], L[nx[i]] + n + 1);
        }
        vector<char> isD(n, 0);
        for (int c = 1; c <= 2 * n; c++) {
            if (last[c]) {
                ll d = L[1] - L[last[c]];
                if (d < n) isD[d] = 1;
            }
        }
        zv.assign(n, 0);
        int cz = 0;
        for (int d = 0; d < n; d++) { if (!isD[d]) cz++; zv[d] = cz; }
        n_g = n; U_g = 2 * n - 1;
        S.assign(n, 0);
        solve(0, n);
        ull ans = (ull)((1 + L[1]) % (ll)P);
        ull ifac2n = ifct[2 * n];
        for (int K = 0; K < n; K++) {
            ull F = fct[2 * n - 1 - K] * ifac2n % P * pw(3, S[K]) % P;
            ans = (ans + 1 + P - F) % P;
        }
        printf("%llu\n", ans);
    }
    return 0;
}
