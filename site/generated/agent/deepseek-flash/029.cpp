#include <bits/stdc++.h>
using namespace std;

static const int MOD = 998244353;
static const int G = 3;

static inline long long modpow(long long a, long long e) {
    long long r = 1;
    a %= MOD;
    if (a < 0) a += MOD;
    while (e > 0) {
        if (e & 1) r = r * a % MOD;
        a = a * a % MOD;
        e >>= 1;
    }
    return r;
}

// iterative NTT, roots cached across calls
static void ntt(vector<int>& a, bool invert) {
    int n = (int)a.size();
    if (n == 1) return;
    // bit reversal
    for (int i = 1, j = 0; i < n; i++) {
        int bit = n >> 1;
        while (j & bit) { j ^= bit; bit >>= 1; }
        j ^= bit;
        if (i < j) swap(a[i], a[j]);
    }
    static vector<int> roots{0, 1};
    if ((int)roots.size() < n) {
        int k = __builtin_ctz((int)roots.size());
        roots.resize(n);
        while ((1 << k) < n) {
            int e = (int)modpow(G, (MOD - 1) >> (k + 1));
            for (int i = 1 << (k - 1); i < (1 << k); i++) {
                roots[2 * i] = roots[i];
                roots[2 * i + 1] = (int)((long long)roots[i] * e % MOD);
            }
            k++;
        }
    }
    for (int len = 1; len < n; len <<= 1) {
        for (int i = 0; i < n; i += 2 * len) {
            for (int j = 0; j < len; j++) {
                int u = a[i + j];
                int v = (int)((long long)a[i + j + len] * roots[len + j] % MOD);
                int s = u + v; if (s >= MOD) s -= MOD;
                int d = u - v; if (d < 0) d += MOD;
                a[i + j] = s;
                a[i + j + len] = d;
            }
        }
    }
    if (invert) {
        reverse(a.begin() + 1, a.end());
        int ninv = (int)modpow(n, MOD - 2);
        for (int& x : a) x = (int)((long long)x * ninv % MOD);
    }
}

// multiply a*b, truncated to length cap
static vector<int> multiply_trunc(const vector<int>& a, const vector<int>& b, int cap) {
    if (a.empty() || b.empty() || cap <= 0) return vector<int>();
    long long full = (long long)a.size() + (long long)b.size() - 1;
    int need = (int)min<long long>(full, cap);
    if (min(a.size(), b.size()) <= 40) {
        vector<int> c(need, 0);
        for (size_t i = 0; i < a.size() && (int)i < need; i++) {
            if (!a[i]) continue;
            int lim = (int)min<long long>((long long)b.size(), (long long)need - (long long)i);
            for (int j = 0; j < lim; j++) {
                c[i + j] = (int)((c[i + j] + (long long)a[i] * b[j]) % MOD);
            }
        }
        return c;
    }
    int sz = 1;
    while (sz < (int)full) sz <<= 1;
    vector<int> fa(a), fb(b);
    fa.resize(sz); fb.resize(sz);
    ntt(fa, false);
    ntt(fb, false);
    for (int i = 0; i < sz; i++) fa[i] = (int)((long long)fa[i] * fb[i] % MOD);
    ntt(fa, true);
    fa.resize(need);
    return fa;
}

struct FastReader {
    static const int BUF = 1 << 16;
    char buf[BUF];
    int pos = 0, len = 0;
    inline int get() {
        if (pos >= len) {
            len = (int)fread(buf, 1, BUF, stdin);
            pos = 0;
            if (len <= 0) return -1;
        }
        return (unsigned char)buf[pos++];
    }
    long long readInt() {
        int c = get();
        while (c != -1 && (c < '0' || c > '9') && c != '-') c = get();
        bool neg = false;
        if (c == '-') { neg = true; c = get(); }
        long long v = 0;
        while (c >= '0' && c <= '9') { v = v * 10 + (c - '0'); c = get(); }
        return neg ? -v : v;
    }
};

int main() {
    FastReader fr;
    long long nn = fr.readInt();
    if (nn <= 0) return 0;
    int n = (int)nn;
    vector<int> a(n);
    long long m = 0;
    int maxa = 0;
    for (int i = 0; i < n; i++) {
        long long x = fr.readInt();
        if (x < 0) x = 0;
        a[i] = (int)x;
        m += x;
        if ((int)x > maxa) maxa = (int)x;
    }
    vector<int> cnt(maxa + 2, 0);
    for (int i = 0; i < n; i++) if (a[i] > 0) cnt[a[i]]++;

    int L = (int)(m / 2);

    // modular inverses up to max(L,1)
    int invN = max(L + 1, 2);
    vector<int> inv(invN + 1, 0);
    inv[1] = 1;
    for (int i = 2; i <= invN; i++) {
        inv[i] = (int)((MOD - (long long)(MOD / i) * inv[MOD % i] % MOD) % MOD);
    }

    // For each distinct value v with multiplicity c: Q_v = (1+x+...+x^v)^c  (truncated to deg L)
    // recurrence (from (1-x)(1-x^{v+1}) Q' = c(1-(v+1)x^v+v x^{v+1}) Q):
    // k*q_k = (k-1+c) q_{k-1} + ((k-v-1)-c(v+1)) q_{k-v-1} + (c*v-(k-v-2)) q_{k-v-2}
    vector<vector<int>> polys;
    polys.reserve(2048);
    for (int v = 1; v <= maxa; v++) {
        if (cnt[v] == 0) continue;
        long long c = cnt[v];
        long long maxdeg = (long long)v * c;
        int N = (int)min<long long>((long long)L, maxdeg);
        vector<int> q(N + 1);
        q[0] = 1;
        long long cvp1 = c * (v + 1);   // <= c*(v+1) <= m + c
        long long cv = c * v;           // <= m
        for (int k = 1; k <= N; k++) {
            long long s = 0;
            s += ((long long)(k - 1) + c) % MOD * q[k - 1] % MOD;
            int i1 = k - v - 1;
            if (i1 >= 0) {
                long long coef = ((long long)(k - v - 1) - cvp1) % MOD;
                if (coef < 0) coef += MOD;
                s += coef * q[i1] % MOD;
            }
            int i2 = k - v - 2;
            if (i2 >= 0) {
                long long coef = ((cv - (long long)(k - v - 2)) % MOD + MOD) % MOD;
                s += coef * q[i2] % MOD;
            }
            q[k] = (int)(s % MOD * inv[k] % MOD);
        }
        polys.push_back(move(q));
    }

    // Huffman-style product tree, all truncated at length L+1
    int cap = L + 1;
    priority_queue<pair<int,int>, vector<pair<int,int>>, greater<pair<int,int>>> pq;
    for (int i = 0; i < (int)polys.size(); i++) pq.push({(int)polys[i].size(), i});
    while (pq.size() > 1) {
        auto [s1, i1] = pq.top(); pq.pop();
        auto [s2, i2] = pq.top(); pq.pop();
        vector<int> r = multiply_trunc(polys[i1], polys[i2], cap);
        polys[i1].clear(); polys[i1].shrink_to_fit();
        polys[i2].clear(); polys[i2].shrink_to_fit();
        int ni = (int)polys.size();
        polys.push_back(move(r));
        pq.push({(int)polys[ni].size(), ni});
    }

    vector<int> M(cap, 0);
    if (!pq.empty()) {
        const vector<int>& P = polys[pq.top().second];
        for (int k = 0; k < (int)P.size() && k < cap; k++) M[k] = P[k];
    } else {
        M[0] = 1; // no cards with positive count (cannot happen: m>=1)
    }

    // answer = sum_{k=0}^{L} (C(m,k)-C(m,k-1)) * (M(k)-M(k-1))
    vector<int> comb(L + 1);
    comb[0] = 1;
    for (int k = 1; k <= L; k++) {
        comb[k] = (int)((long long)comb[k - 1] * ((m - k + 1) % MOD) % MOD * inv[k] % MOD);
    }
    long long ans = 0;
    int prevM = 0;
    for (int k = 0; k <= L; k++) {
        long long f = comb[k] - (k >= 1 ? (long long)comb[k - 1] : 0LL);
        f %= MOD; if (f < 0) f += MOD;
        long long d = (long long)M[k] - prevM;
        d %= MOD; if (d < 0) d += MOD;
        ans = (ans + f * d) % MOD;
        prevM = M[k];
    }
    printf("%lld\n", ans % MOD);
    return 0;
}
