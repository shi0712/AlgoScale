// Infinite Strife
//
// a_i = number of paths (0,0)->(i,i) with steps (0,2),(2,0),(1,1)
//     = sum_m i! / ((i-2m)! (m!)^2)  = central trinomial coefficient T_i.
//
// Modulo a prime p, T_i = CT((1+x+1/x)^i) and the Frobenius identity gives
//      T_{p*k+r} == T_k * T_r   (mod p),  0 <= r < p .
// For p = 11 all T_0..T_10 are non zero, hence T_i is never 0 (mod 11).
//
// The c-fold prefix product has exponent of a_j equal to C(n-j+c-1, c-1),
// so with g=2 a primitive root of F_11^*
//      answer = 2^S,  S = sum_{j=1..n} L_j * C(n-j+c-1, c-1)  (mod 10),
// where L_j = discrete log of T_j.  Since T_j = prod_i T_{digit_i(j)} (base 11)
// we have L_j = sum_i Ld[digit_i(j)] with Ld[d] = dlog(T_d).
//
// Grouping j by the lowest base-11 digit block (hockey-stick) and telescoping
// over the digit values yields
//      S = sum_{i>=0} sum_{s=1}^{floor(n/11^i)} wt[s mod 11] * C(n+c-s*11^i, c)
// with wt[d] = Ld[d]-Ld[d-1] (cyclic).  Every inner sum is an arithmetic
// progression sum of binomial coefficients, which is evaluated mod 2 and mod 5
// by an LSB-first digit DP (Lucas): the state is the carry of the subtraction
// N = A - s*M together with the residue of s mod 11 and the borrow needed for
// the upper bound.  The carry is reduced modulo p^(L-j) (L = digits of c),
// which keeps the number of states at O(sqrt(p^L)) = O(sqrt(c)).
#include <bits/stdc++.h>
using namespace std;
using u64 = unsigned long long;
using i64 = long long;

static int wt10[11];      // wt10[w] = (Ld[w] - Ld[w-1]) cyclic, in [0,10)
static int C2[2][2];
static int C5[5][5];

static inline i64 fdiv(i64 a, i64 b){ // floor(a/b), b > 0
    i64 q = a / b, r = a % b;
    if (r < 0) --q;
    return q;
}

// C(n,k) mod p
static int Cmodp(u64 n, u64 k, int p){
    if (k > n) return 0;
    int res = 1;
    while (n || k){
        int a = (int)(n % (u64)p), b = (int)(k % (u64)p);
        if (b > a) return 0;
        res = res * (p == 2 ? C2[a][b] : C5[a][b]) % p;
        n /= (u64)p; k /= (u64)p;
    }
    return res;
}

// returns sum_{s=1}^{Smax} wt10[s mod 11] * C(A - s*M, c)  (mod p)
static int wsum(int p, u64 M, u64 A, u64 c, u64 Smax){
    if (Smax == 0) return 0;
    // small number of terms: enumerate
    if (Smax <= 20000){
        int res = 0;
        for (u64 s = 1; s <= Smax; s++){
            int f = Cmodp(A - s*M, c, p);
            res = (res + f * (wt10[s % 11] % p)) % p;
        }
        return res;
    }
    // number of base-p digits of c  ->  p^L > c
    int L = 0; u64 pw = 1;
    while (pw <= c){ pw *= (u64)p; ++L; }
    // digits of Smax
    int Ds = 0; { u64 t = Smax; while (t){ t /= (u64)p; ++Ds; } }
    int D = max(L, Ds);
    int j0 = (L + 1) / 2;

    static vector<int> cd, sd, pjm;
    cd.assign(D + 1, 0); sd.assign(D + 1, 0); pjm.assign(D + 2, 1);
    { u64 t = c;    for (int j = 0; j < D && t; j++){ cd[j] = (int)(t % (u64)p); t /= (u64)p; } }
    { u64 t = Smax; for (int j = 0; j < D; j++){ sd[j] = (int)(t % (u64)p); t /= (u64)p; } }
    for (int j = 1; j <= D + 1; j++) pjm[j] = (int)((i64)pjm[j-1] * p % 11);

    static vector<unsigned char> dp, ndp;

    // ---- phase 1: states indexed by the processed low digits s_low < p^j ----
    i64 curN = 1;
    dp.assign(22, 0);
    dp[0] = (unsigned char)(1 % p);   // s_low = 0, borrow = 0, weight = 0
    u64 pj = 1;
    for (int j = 0; j < j0; j++){
        int cj = cd[j];
        i64 newN = curN * p;
        ndp.assign((size_t)newN * 22, 0);
        int pm = pjm[j];
        for (i64 sl = 0; sl < curN; sl++){
            __int128 num = (__int128)A - (__int128)sl * (__int128)M;
            i64 q = (i64)(num / (__int128)pj);
            if (num < 0 && (num % (__int128)pj) != 0) --q;   // floor
            int base = (int)(sl * 22);
            for (int br = 0; br < 2; br++){
                int b2 = base + br * 11;
                for (int w = 0; w < 11; w++){
                    unsigned char v = dp[b2 + w];
                    if (!v) continue;
                    for (int d = 0; d < p; d++){
                        i64 x = q - (i64)M * d;
                        int dig = (int)(x % p); if (dig < 0) dig += p;
                        int f = (p == 2) ? C2[dig][cj] : C5[dig][cj];
                        if (!f) continue;
                        int nbr = (sd[j] - d - br < 0) ? 1 : 0;
                        int nw = (w + d * pm) % 11;
                        size_t idx = (size_t)(sl + curN * d) * 22 + nbr * 11 + nw;
                        int nv = ndp[idx] + v * f;
                        ndp[idx] = (unsigned char)(nv % p);
                    }
                }
            }
        }
        dp.swap(ndp);
        curN = newN;
        pj *= (u64)p;
    }

    // ---- switch to residues modulo p^(L-j) ----
    u64 curQ;
    {
        int lj = max(0, L - j0);
        curQ = 1; for (int t = 0; t < lj; t++) curQ *= (u64)p;
        ndp.assign((size_t)curQ * 22, 0);
        for (i64 sl = 0; sl < curN; sl++){
            __int128 num = (__int128)A - (__int128)sl * (__int128)M;
            i64 q = (i64)(num / (__int128)pj);
            if (num < 0 && (num % (__int128)pj) != 0) --q;
            i64 qi = q % (i64)curQ; if (qi < 0) qi += (i64)curQ;
            int src = (int)(sl * 22), dst = (int)(qi * 22);
            for (int t = 0; t < 22; t++){
                unsigned char v = dp[src + t];
                if (!v) continue;
                int nv = ndp[dst + t] + v;
                ndp[dst + t] = (unsigned char)(nv >= p ? nv - p : nv);
            }
        }
        dp.swap(ndp);
    }

    // ---- phase 2: dense residues modulo p^(L-j) ----
    for (int j = j0; j < D; j++){
        bool forced = (j >= Ds);           // then all remaining digits of s are 0
        int cj = (j < L) ? cd[j] : 0;
        u64 newQ = curQ / (u64)p; if (newQ < 1) newQ = 1;
        ndp.assign((size_t)newQ * 22, 0);
        int pm = pjm[j];
        int dmax = forced ? 0 : (p - 1);
        for (u64 qi = 0; qi < curQ; qi++){
            for (int br = 0; br < 2; br++){
                if (forced && br) continue;      // dead: cannot return to borrow 0
                int b2 = (int)(qi * 22) + br * 11;
                for (int w = 0; w < 11; w++){
                    unsigned char v = dp[b2 + w];
                    if (!v) continue;
                    for (int d = 0; d <= dmax; d++){
                        i64 x = (i64)qi - (i64)M * d;
                        int dig = (int)(x % p); if (dig < 0) dig += p;
                        int f = (p == 2) ? C2[dig][cj] : C5[dig][cj];
                        if (!f) continue;
                        int nbr = forced ? 0 : ((sd[j] - d - br < 0) ? 1 : 0);
                        i64 nq = fdiv(x, p);
                        i64 nqi = nq % (i64)newQ; if (nqi < 0) nqi += (i64)newQ;
                        int nw = (w + d * pm) % 11;
                        size_t idx = (size_t)nqi * 22 + nbr * 11 + nw;
                        int nv = ndp[idx] + v * f;
                        ndp[idx] = (unsigned char)(nv % p);
                    }
                }
            }
        }
        dp.swap(ndp);
        curQ = newQ;
    }

    // ---- finalize: sum states with borrow 0, remove the artificial s = 0 ----
    int res = 0;
    for (u64 qi = 0; qi < curQ; qi++){
        int b2 = (int)(qi * 22);
        for (int w = 0; w < 11; w++){
            unsigned char v = dp[b2 + w];
            if (v) res = (res + v * (wt10[w] % p)) % p;
        }
    }
    res = (res - Cmodp(A, c, p) * (wt10[0] % p)) % p;
    if (res < 0) res += p;
    return res;
}

int main(){
    // central trinomial coefficients T_0..T_10 and their discrete logs (base 2)
    {
        long long fact[11]; fact[0] = 1;
        for (int i = 1; i <= 10; i++) fact[i] = fact[i-1] * i;
        int Ld[11];
        int log2map[11]; int pw = 1;
        for (int e = 0; e < 10; e++){ log2map[pw] = e; pw = pw * 2 % 11; }
        for (int n = 0; n <= 10; n++){
            long long s = 0;
            for (int m = 0; 2*m <= n; m++) s += fact[n] / (fact[n-2*m] * fact[m] * fact[m]);
            Ld[n] = log2map[s % 11];
        }
        int r[12]; r[0] = 0;
        for (int e = 1; e <= 10; e++) r[e] = ((Ld[e] - Ld[e-1]) % 10 + 10) % 10;
        r[11] = ((-Ld[10]) % 10 + 10) % 10;
        for (int w = 0; w < 11; w++) wt10[w] = r[((w - 1) % 11 + 11) % 11 + 1];
    }
    for (int a = 0; a < 2; a++) for (int b = 0; b < 2; b++) C2[a][b] = (a >= b) ? 1 : 0;
    for (int a = 0; a < 5; a++) for (int b = 0; b < 5; b++){
        int v = 0;
        if (b <= a){ long long c = 1; for (int i = 0; i < b; i++) c = c * (a - i) / (i + 1); v = (int)(c % 5); }
        C5[a][b] = v;
    }

    // fast input
    static char buf[1 << 16];
    size_t blen = fread(buf, 1, sizeof(buf), stdin), bpos = 0;
    auto readU64 = [&]() -> u64 {
        while (bpos < blen && (buf[bpos] < '0' || buf[bpos] > '9')){
            if (++bpos == blen){ blen = fread(buf, 1, sizeof(buf), stdin); bpos = 0; }
        }
        u64 x = 0;
        while (bpos < blen && buf[bpos] >= '0' && buf[bpos] <= '9'){
            x = x * 10 + (u64)(buf[bpos] - '0');
            if (++bpos == blen){ blen = fread(buf, 1, sizeof(buf), stdin); bpos = 0; }
        }
        return x;
    };

    int T = (int)readU64();
    string out;
    out.reserve((size_t)T * 4);
    while (T--){
        u64 n = readU64(), c = readU64();
        u64 A = n + c;
        int s2 = 0, s5 = 0;
        for (u64 M = 1; ; M *= 11){
            u64 Smax = n / M;
            s2 = (s2 + wsum(2, M, A, c, Smax)) % 2;
            s5 = (s5 + wsum(5, M, A, c, Smax)) % 5;
            if (M > n / 11) break;
        }
        int S = 0;
        while ((S % 2) != s2 || (S % 5) != s5) ++S;   // CRT, 0 <= S < 10
        int ans = 1;
        for (int i = 0; i < S; i++) ans = ans * 2 % 11;
        out += to_string(ans); out += '\n';
    }
    fwrite(out.data(), 1, out.size(), stdout);
    return 0;
}
