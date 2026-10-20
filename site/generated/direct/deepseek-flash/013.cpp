#include <bits/stdc++.h>
using namespace std;

static const int MOD = 998244353;
static const int G = 3;

int mod_pow(long long a, long long e) {
    long long r = 1;
    while (e) {
        if (e & 1) r = r * a % MOD;
        a = a * a % MOD;
        e >>= 1;
    }
    return (int)r;
}

void ntt(vector<int>& a, bool inv) {
    int n = (int)a.size();
    for (int i = 1, j = 0; i < n; i++) {
        int bit = n >> 1;
        while (j & bit) {
            j ^= bit;
            bit >>= 1;
        }
        j ^= bit;
        if (i < j) swap(a[i], a[j]);
    }

    for (int len = 2; len <= n; len <<= 1) {
        int wlen = mod_pow(G, (MOD - 1) / len);
        if (inv) wlen = mod_pow(wlen, MOD - 2);

        for (int i = 0; i < n; i += len) {
            long long w = 1;
            for (int j = 0; j < len / 2; j++) {
                int u = a[i + j];
                int v = (int)(w * a[i + j + len / 2] % MOD);
                int x = u + v;
                if (x >= MOD) x -= MOD;
                int y = u - v;
                if (y < 0) y += MOD;
                a[i + j] = x;
                a[i + j + len / 2] = y;
                w = w * wlen % MOD;
            }
        }
    }

    if (inv) {
        int inv_n = mod_pow(n, MOD - 2);
        for (int& x : a) x = (long long)x * inv_n % MOD;
    }
}

vector<int> multiply(vector<int> a, vector<int> b) {
    if (a.empty() || b.empty()) return {};
    if (min(a.size(), b.size()) <= 32) {
        vector<int> c(a.size() + b.size() - 1, 0);
        for (int i = 0; i < (int)a.size(); i++) {
            if (!a[i]) continue;
            for (int j = 0; j < (int)b.size(); j++) {
                c[i + j] = (c[i + j] + (long long)a[i] * b[j]) % MOD;
            }
        }
        while (c.size() > 1 && c.back() == 0) c.pop_back();
        return c;
    }

    int n = 1;
    int need = (int)a.size() + (int)b.size() - 1;
    while (n < need) n <<= 1;

    a.resize(n);
    b.resize(n);
    ntt(a, false);
    ntt(b, false);
    for (int i = 0; i < n; i++) a[i] = (long long)a[i] * b[i] % MOD;
    ntt(a, true);
    a.resize(need);

    while (a.size() > 1 && a.back() == 0) a.pop_back();
    return a;
}

vector<int> poly_inv_series(const vector<int>& a, int need) {
    vector<int> res(1, mod_pow(a[0], MOD - 2));

    while ((int)res.size() < need) {
        int nxt = min((int)res.size() * 2, need);

        vector<int> f(a.begin(), a.begin() + min((int)a.size(), nxt));
        vector<int> fr = multiply(f, res);
        fr.resize(nxt);

        for (int i = 0; i < nxt; i++) {
            if (fr[i]) fr[i] = MOD - fr[i];
        }
        fr[0] += 2;
        if (fr[0] >= MOD) fr[0] -= MOD;
        if (fr[0] >= MOD) fr[0] -= MOD;

        res = multiply(res, fr);
        res.resize(nxt);
    }

    return res;
}

vector<int> poly_mod(vector<int> A, const vector<int>& M) {
    int n = (int)A.size() - 1;
    int d = (int)M.size() - 1;
    if (n < d) return A;

    int qlen = n - d + 1;

    vector<int> Arev(qlen);
    for (int i = 0; i < qlen; i++) Arev[i] = A[n - i];

    vector<int> Mrev(d + 1);
    for (int i = 0; i <= d; i++) Mrev[i] = M[d - i];

    vector<int> invM = poly_inv_series(Mrev, qlen);
    vector<int> qrev = multiply(Arev, invM);
    qrev.resize(qlen);

    vector<int> q(qlen);
    for (int i = 0; i < qlen; i++) q[i] = qrev[qlen - 1 - i];

    vector<int> prod = multiply(q, M);

    vector<int> rem(d);
    for (int i = 0; i < d; i++) {
        int ai = i < (int)A.size() ? A[i] : 0;
        int pi = i < (int)prod.size() ? prod[i] : 0;
        int x = ai - pi;
        if (x < 0) x += MOD;
        rem[i] = x;
    }

    while (rem.size() > 1 && rem.back() == 0) rem.pop_back();
    return rem;
}

struct Factor {
    int d;
    int base;
    int invbase;
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int T;
    cin >> T;

    const int BLOCK = 2000;

    while (T--) {
        int n, m;
        cin >> n >> m;

        vector<int> a(m + 1);
        for (int i = 1; i <= m; i++) cin >> a[i];

        int L = n + 1;

        vector<int> last(2 * n + 1, 0), nxt(m + 2, 0);
        for (int i = m; i >= 1; i--) {
            int c = a[i];
            nxt[i] = last[c];
            last[c] = i;
        }

        vector<long long> W(m + 2, 0);
        for (int i = m; i >= 1; i--) {
            long long val = 0;
            if (i < m) val = W[i + 1] + 1;
            if (nxt[i]) {
                val = max(val, W[nxt[i]] + L);
            }
            W[i] = val;
        }

        long long w1 = W[1];

        vector<int> seen(2 * n + 1, 0);
        vector<Factor> factors;
        int rank = 0;

        for (int i = 1; i <= m; i++) {
            int c = a[i];
            if (seen[c]) continue;
            seen[c] = 1;
            rank++;

            long long dll = w1 - W[i];
            if (dll >= L) break;

            int d = (int)dll;
            int base = 2 * n + 1 - rank;
            int invbase = mod_pow(base, MOD - 2);
            factors.push_back({d, base, invbase});
        }

        int h = (int)factors.size();

        vector<int> Q(1, 1);
        int idx = 0;
        int sumG = 0;

        for (int l = 0; l < L; l += BLOCK) {
            int r = min(L - 1, l + BLOCK - 1);
            int len = r - l + 1;

            vector<int> M(1, 1);
            for (int y = l; y <= r; y++) {
                vector<int> nM(M.size() + 1, 0);
                int negy = (y == 0 ? 0 : MOD - y);
                for (int k = 0; k < (int)M.size(); k++) {
                    nM[k] = (nM[k] + (long long)M[k] * negy) % MOD;
                    int v = nM[k + 1] + M[k];
                    if (v >= MOD) v -= MOD;
                    nM[k + 1] = v;
                }
                M.swap(nM);
            }

            vector<int> R;
            if ((int)Q.size() - 1 < len) R = Q;
            else R = poly_mod(Q, M);

            int startIdx = idx;
            int degR = (int)R.size() - 1;

            for (int y = l; y <= r; y++) {
                long long cur = 0;
                for (int k = degR; k >= 0; k--) {
                    cur = (cur * y + R[k]) % MOD;
                }
                int qy = (int)cur;

                while (idx < h && factors[idx].d <= y) idx++;

                long long active = 1;
                for (int j = startIdx; j < idx; j++) {
                    int val = factors[j].base + factors[j].d - y;
                    if (val >= MOD) val %= MOD;
                    int f = (long long)val * factors[j].invbase % MOD;
                    active = active * f % MOD;
                }

                int F = (long long)qy * active % MOD;
                sumG += F;
                if (sumG >= MOD) sumG -= MOD;
            }

            vector<int> Rnew(1, 1);
            for (int j = startIdx; j < idx; j++) {
                int base = factors[j].base;
                int d = factors[j].d;
                int invbase = factors[j].invbase;

                int cst = (long long)(base + d) % MOD * invbase % MOD;
                int cln = (invbase == 0 ? 0 : MOD - invbase);

                vector<int> nw(Rnew.size() + 1, 0);
                for (int k = 0; k < (int)Rnew.size(); k++) {
                    nw[k] = (nw[k] + (long long)Rnew[k] * cst) % MOD;
                    int v = nw[k + 1] + (long long)Rnew[k] * cln % MOD;
                    if (v >= MOD) v %= MOD;
                    nw[k + 1] = v;
                }
                Rnew.swap(nw);
            }

            if (!(Rnew.size() == 1 && Rnew[0] == 1)) {
                Q = multiply(Q, Rnew);
            }
        }

        long long ans = (w1 % MOD + L + 1 - sumG) % MOD;
        if (ans < 0) ans += MOD;

        cout << ans << '\n';
    }

    return 0;
}
