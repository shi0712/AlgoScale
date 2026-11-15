#include <bits/stdc++.h>
using namespace std;
typedef unsigned long long ull;
typedef long long ll;
typedef unsigned int u32;
static const u32 MOD = 998244353;

u32 pw(u32 b, ull e) {
    ull r = 1, x = b;
    while (e) { if (e & 1) r = r * x % MOD; x = x * x % MOD; e >>= 1; }
    return (u32)r;
}

void ntt(vector<u32>& a, bool inv) {
    int n = a.size();
    for (int i = 1, j = 0; i < n; i++) {
        int bit = n >> 1;
        for (; j & bit; bit >>= 1) j ^= bit;
        j ^= bit;
        if (i < j) swap(a[i], a[j]);
    }
    for (int len = 2; len <= n; len <<= 1) {
        u32 w = pw(3, (MOD - 1) / len);
        if (inv) w = pw(w, MOD - 2);
        int half = len >> 1;
        vector<u32> ws(half);
        ws[0] = 1;
        for (int i = 1; i < half; i++) ws[i] = (ull)ws[i - 1] * w % MOD;
        for (int i = 0; i < n; i += len) {
            for (int j = 0; j < half; j++) {
                u32 u = a[i + j];
                u32 v = (ull)a[i + j + half] * ws[j] % MOD;
                u32 x = u + v; if (x >= MOD) x -= MOD;
                u32 y = u + MOD - v; if (y >= MOD) y -= MOD;
                a[i + j] = x; a[i + j + half] = y;
            }
        }
    }
    if (inv) {
        u32 ni = pw(n, MOD - 2);
        for (auto& x : a) x = (ull)x * ni % MOD;
    }
}

// result truncated to 'lim' terms (0..lim-1)
vector<u32> mul(const vector<u32>& A, const vector<u32>& B, size_t lim) {
    size_t la = A.size(), lb = B.size();
    size_t rl = min(lim, la + lb - 1);
    vector<u32> res(rl, 0);
    if (min(la, lb) <= 32) {
        for (size_t i = 0; i < la; i++) {
            if (!A[i]) continue;
            for (size_t j = 0; j < lb && i + j < rl; j++)
                res[i + j] = (res[i + j] + (ull)A[i] * B[j]) % MOD;
        }
        return res;
    }
    size_t N = 1;
    while (N < la + lb - 1) N <<= 1;
    vector<u32> fa(A.begin(), A.end()), fb(B.begin(), B.end());
    fa.resize(N); fb.resize(N);
    ntt(fa, false); ntt(fb, false);
    for (size_t i = 0; i < N; i++) fa[i] = (ull)fa[i] * fb[i] % MOD;
    ntt(fa, true);
    for (size_t i = 0; i < rl; i++) res[i] = fa[i];
    return res;
}

vector<u32> fact, ifact;

struct State {
    vector<u32> poly;
    int form = 0; // 0 = G, 1 = H
    int kind = 1; // 0 generic, 1 no edges (poly=[1]), 2 complete
    int a = 0, b = 0;
};

void materialize(State& s) {
    if (s.kind == 0) return;
    int m = min(s.a, s.b);
    if (s.kind == 1) {
        s.poly.assign(1, 1);
    } else {
        s.poly.assign(m + 1, 0);
        for (int x = 0; x <= m; x++) {
            s.poly[x] = (ull)fact[s.a] * fact[s.b] % MOD * ifact[s.a - x] % MOD
                        * ifact[s.b - x] % MOD * ifact[x] % MOD;
        }
    }
    s.kind = 0;
}

void convertTo(State& s, int f) {
    if (s.form == f) return;
    s.form = f;
    int m = min(s.a, s.b);
    if (m == 0) { s.kind = 1; s.poly.assign(1, 1); return; }
    if (s.kind == 1) { s.kind = 2; s.poly.clear(); return; }
    if (s.kind == 2) { s.kind = 1; s.poly.clear(); return; }
    int L = s.poly.size();
    vector<u32> h(L);
    for (int j = 0; j < L; j++) {
        u32 v = (ull)s.poly[j] * fact[s.a - j] % MOD * fact[s.b - j] % MOD;
        if (j & 1) v = v ? MOD - v : 0;
        h[j] = v;
    }
    vector<u32> ker(ifact.begin(), ifact.begin() + m + 1);
    vector<u32> r = mul(h, ker, m + 1);
    r.resize(m + 1, 0);
    for (int x = 0; x <= m; x++)
        r[x] = (ull)r[x] * ifact[s.a - x] % MOD * ifact[s.b - x] % MOD;
    s.poly = move(r);
}

State combine(State s1, State s2, int target) {
    if (s2.a == 0 && s2.b == 0) return s1;
    if (s1.a == 0 && s1.b == 0) return s2;
    convertTo(s1, target);
    convertTo(s2, target);
    State res;
    res.form = target;
    res.a = s1.a + s2.a;
    res.b = s1.b + s2.b;
    if (s1.kind == 1 && s2.kind == 1) {
        res.kind = 1;
        return res;
    }
    materialize(s1);
    materialize(s2);
    int m = min(res.a, res.b);
    res.poly = mul(s1.poly, s2.poly, m + 1);
    res.kind = 0;
    return res;
}

vector<ull> A, B;
ull K;

State solve(int p, int al, int ar, int bl, int br) {
    int na = ar - al, nb = br - bl;
    State st;
    st.a = na; st.b = nb;
    if (na == 0 || nb == 0) { st.kind = 1; st.form = 0; return st; }
    if (p < 0) { st.kind = 2; st.form = 0; return st; }
    int am = partition_point(A.begin() + al, A.begin() + ar,
                             [&](ull x) { return ((x >> p) & 1) == 0; }) - A.begin();
    int bm = partition_point(B.begin() + bl, B.begin() + br,
                             [&](ull x) { return ((x >> p) & 1) == 0; }) - B.begin();
    int kb = (K >> p) & 1;
    if (kb == 0) {
        State s1 = solve(p - 1, al, am, bl, bm);
        State s2 = solve(p - 1, am, ar, bm, br);
        return combine(move(s1), move(s2), 1);
    } else {
        State s1 = solve(p - 1, al, am, bm, br);
        State s2 = solve(p - 1, am, ar, bl, bm);
        return combine(move(s1), move(s2), 0);
    }
}

int main() {
    int n;
    scanf("%d %llu", &n, &K);
    A.resize(n); B.resize(n);
    for (auto& x : A) scanf("%llu", &x);
    for (auto& x : B) scanf("%llu", &x);
    sort(A.begin(), A.end());
    sort(B.begin(), B.end());
    int F = n + 5;
    fact.assign(F, 1); ifact.assign(F, 1);
    for (int i = 1; i < F; i++) fact[i] = (ull)fact[i - 1] * i % MOD;
    ifact[F - 1] = pw(fact[F - 1], MOD - 2);
    for (int i = F - 1; i > 0; i--) ifact[i - 1] = (ull)ifact[i] * i % MOD;

    State root = solve(59, 0, n, 0, n);
    convertTo(root, 0);
    materialize(root);
    string out;
    for (int x = 1; x <= n; x++) {
        u32 v = x < (int)root.poly.size() ? root.poly[x] : 0;
        out += to_string(v);
        out += (x == n ? '\n' : ' ');
    }
    fputs(out.c_str(), stdout);
    return 0;
}
