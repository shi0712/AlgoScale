#pragma GCC optimize("O3,unroll-loops")
#include <bits/stdc++.h>
using namespace std;
typedef unsigned long long u64;
typedef unsigned int u32;
static u64 M;
u64 pw(u64 b, u64 e) {
    u64 r = 1; b %= M;
    while (e) { if (e & 1) r = r * b % M; b = b * b % M; e >>= 1; }
    return r;
}
int main() {
    int n;
    scanf("%d %llu", &n, &M);
    int N = n + 1;
    vector<u64> fact(N + 1), ifact(N + 1);
    fact[0] = 1;
    for (int i = 1; i <= n; i++) fact[i] = fact[i - 1] * i % M;
    ifact[n] = pw(fact[n], M - 2);
    for (int i = n; i > 0; i--) ifact[i - 1] = ifact[i] * i % M;
    vector<u32> Cb((size_t)N * N, 0), Eul((size_t)N * N, 0), St((size_t)N * N, 0);
    for (int i = 0; i <= n; i++) {
        Cb[(size_t)i * N] = 1;
        for (int j = 1; j <= i; j++)
            Cb[(size_t)i * N + j] = ((u64)Cb[(size_t)(i - 1) * N + j - 1] + Cb[(size_t)(i - 1) * N + j]) % M;
    }
    Eul[0] = 1;
    if (n >= 1) Eul[(size_t)1 * N] = 1;
    for (int e = 2; e <= n; e++)
        for (int k = 0; k < e; k++) {
            u64 v = (u64)(k + 1) * Eul[(size_t)(e - 1) * N + k] % M;
            if (k > 0) v += (u64)(e - k) * Eul[(size_t)(e - 1) * N + k - 1] % M;
            Eul[(size_t)e * N + k] = v % M;
        }
    St[0] = 1;
    for (int r = 0; r < n; r++)
        for (int e = 0; e <= r + 1; e++) {
            u64 t1 = e > 0 ? St[(size_t)r * N + e - 1] : 0;
            u64 t2 = (u64)r * St[(size_t)r * N + e] % M;
            St[(size_t)(r + 1) * N + e] = (t1 + M - t2) % M;
        }
    vector<u32> XT((size_t)n * N, 0), Y((size_t)N * n, 0);
    const int LIM = 18;
    vector<u64> phi(N + 1), acc(N + 2), P(N + 1), acc2(N + 2);
    for (int e = 0; e <= n; e++) {
        int L = n - e;
        for (int i = 0; i <= L; i++)
            phi[i] = (u64)Cb[(size_t)n * N + e + i] * St[(size_t)(e + i) * N + e] % M * ifact[e + i] % M;
        for (int m = 0; m <= L; m++) acc[m] = 0;
        int cnt = 0;
        for (int i = 0; i <= L; i++) {
            u64 f = phi[i];
            if (!f) continue;
            const u32* cr = &Cb[(size_t)i * N];
            for (int m = 0; m <= i; m++) acc[m] += f * cr[m];
            if (++cnt == LIM) { for (int m = 0; m <= L; m++) acc[m] %= M; cnt = 0; }
        }
        for (int m = 0; m <= L; m++) {
            u64 v = acc[m] % M;
            P[m] = (m & 1) ? (v ? M - v : 0) : v;
        }
        int lenA = (e == 0 ? 1 : e);
        for (int i = 0; i < lenA + L + 1; i++) acc2[i] = 0;
        cnt = 0;
        for (int k = 0; k < lenA; k++) {
            u64 c = Eul[(size_t)e * N + k];
            if (!c) continue;
            for (int m = 0; m <= L; m++) acc2[k + m] += c * P[m];
            if (++cnt == LIM) { for (int i = 0; i < lenA + L; i++) acc2[i] %= M; cnt = 0; }
        }
        for (int m = 0; m < n && m < lenA + L; m++) XT[(size_t)m * N + e] = acc2[m] % M;
        // Y_e
        vector<u64> B(L + 1);
        for (int i = 0; i <= L; i++) {
            u64 c = Cb[(size_t)L * N + i];
            B[i] = (i & 1) ? (c ? M - c : 0) : c;
        }
        vector<u64> ya(n, 0);
        if (e == 0) {
            for (int d = 0; d < n && d <= L; d++) ya[d] = B[d];
        } else {
            cnt = 0;
            for (int k = 0; k < e; k++) {
                u64 c = Eul[(size_t)e * N + k];
                if (!c) continue;
                int s = k + 1;
                for (int i = 0; i <= L && s + i < n; i++) ya[s + i] += c * B[i];
                if (++cnt == LIM) { for (int d = 0; d < n; d++) ya[d] %= M; cnt = 0; }
            }
        }
        for (int d = 0; d < n; d++) Y[(size_t)e * n + d] = ya[d] % M;
    }
    string out;
    out.reserve((size_t)n * n * 11 + 16);
    vector<u64> ac(n);
    char buf[24];
    for (int m = 0; m < n; m++) {
        fill(ac.begin(), ac.end(), 0ULL);
        int cnt = 0;
        for (int e = 0; e <= n; e++) {
            u64 x = XT[(size_t)m * N + e];
            if (!x) continue;
            const u32* yr = &Y[(size_t)e * n];
            for (int d = 0; d < n; d++) ac[d] += x * yr[d];
            if (++cnt == LIM) { for (int d = 0; d < n; d++) ac[d] %= M; cnt = 0; }
        }
        for (int d = 0; d < n; d++) {
            u64 v = ac[d] % M;
            int len = 0;
            if (v == 0) buf[len++] = '0';
            while (v) { buf[len++] = '0' + v % 10; v /= 10; }
            while (len) out.push_back(buf[--len]);
            out.push_back(d + 1 < n ? ' ' : '\n');
        }
    }
    fputs(out.c_str(), stdout);
    return 0;
}
