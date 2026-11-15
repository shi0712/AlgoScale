#include <bits/stdc++.h>
using namespace std;

static const int MOD = 998244353;
static const int G = 3;

long long modpow(long long a, long long e) {
    long long r = 1 % MOD;
    a %= MOD;
    while (e > 0) {
        if (e & 1) r = r * a % MOD;
        a = a * a % MOD;
        e >>= 1;
    }
    return r;
}

// ---------- NTT ----------
void ntt(vector<int>& a, bool invert) {
    int n = (int)a.size();
    for (int i = 1, j = 0; i < n; ++i) {
        int bit = n >> 1;
        for (; j & bit; bit >>= 1) j ^= bit;
        j ^= bit;
        if (i < j) swap(a[i], a[j]);
    }
    for (int len = 2; len <= n; len <<= 1) {
        int wlen = (int)modpow(G, (MOD - 1) / len);
        if (invert) wlen = (int)modpow(wlen, MOD - 2);
        int half = len >> 1;
        for (int i = 0; i < n; i += len) {
            long long w = 1;
            for (int j = 0; j < half; ++j) {
                int u = a[i + j];
                int v = (int)(w * a[i + j + half] % MOD);
                int x = u + v;
                if (x >= MOD) x -= MOD;
                int y = u - v;
                if (y < 0) y += MOD;
                a[i + j] = x;
                a[i + j + half] = y;
                w = w * wlen % MOD;
            }
        }
    }
    if (invert) {
        int invn = (int)modpow(n, MOD - 2);
        for (int &x : a) x = (int)((long long)x * invn % MOD);
    }
}

vector<int> multiply(const vector<int>& a, const vector<int>& b, int limit = -1) {
    if (a.empty() || b.empty()) return {};
    int n = (int)a.size(), m = (int)b.size();
    int need = n + m - 1;
    if (limit >= 0 && need > limit) need = limit;
    if (need <= 0) return {};
    // naive for small products
    if (min(n, m) <= 48 || (long long)n * m <= 4096) {
        vector<int> c(need, 0);
        for (int i = 0; i < n && i < need; ++i) {
            if (a[i] == 0) continue;
            int jmax = min(m, need - i);
            long long ai = a[i];
            for (int j = 0; j < jmax; ++j) {
                c[i + j] = (int)((c[i + j] + ai * b[j]) % MOD);
            }
        }
        return c;
    }
    int sz = 1;
    while (sz < n + m - 1) sz <<= 1;
    vector<int> fa(sz, 0), fb(sz, 0);
    copy(a.begin(), a.end(), fa.begin());
    copy(b.begin(), b.end(), fb.begin());
    ntt(fa, false);
    ntt(fb, false);
    for (int i = 0; i < sz; ++i) fa[i] = (int)((long long)fa[i] * fb[i] % MOD);
    ntt(fa, true);
    fa.resize(need);
    return fa;
}

// ---------- FPS ----------
vector<int> poly_inv(const vector<int>& a, int n) {
    // a[0] != 0, returns inverse modulo x^n
    vector<int> b(1, (int)modpow(a[0], MOD - 2));
    int m = 1;
    while (m < n) {
        int len = min(2 * m, n);
        vector<int> f(a.begin(), a.begin() + min((int)a.size(), len));
        f.resize(len, 0);
        vector<int> fb = multiply(f, b, len);
        for (int &x : fb) {
            if (x) x = MOD - x;
        }
        fb[0] += 2;
        if (fb[0] >= MOD) fb[0] -= MOD;
        b = multiply(b, fb, len);
        b.resize(len, 0);
        m = len;
    }
    b.resize(n, 0);
    return b;
}

vector<int> poly_log(const vector<int>& a, int n) {
    // requires a[0] == 1
    if (n <= 1) return vector<int>(n, 0);
    int len = min((int)a.size(), n);
    vector<int> der(max(0, len - 1));
    for (int i = 1; i < len; ++i) der[i - 1] = (int)((long long)a[i] * i % MOD);
    vector<int> inva = poly_inv(a, n);
    vector<int> prod = multiply(der, inva, n - 1);
    vector<int> res(n, 0);
    for (int i = 0; i < (int)prod.size() && i + 1 < n; ++i) {
        res[i + 1] = (int)((long long)prod[i] * modpow(i + 1, MOD - 2) % MOD);
    }
    return res;
}

vector<int> poly_exp(const vector<int>& a, int n) {
    // requires a[0] == 0
    vector<int> b(1, 1);
    int m = 1;
    while (m < n) {
        int len = min(2 * m, n);
        vector<int> logb = poly_log(b, len);
        vector<int> diff(len, 0);
        for (int i = 0; i < len; ++i) {
            int ai = (i < (int)a.size() ? a[i] : 0);
            int v = ai - logb[i];
            if (v < 0) v += MOD;
            diff[i] = v;
        }
        diff[0] += 1;
        if (diff[0] >= MOD) diff[0] -= MOD;
        b = multiply(b, diff, len);
        b.resize(len, 0);
        m = len;
    }
    b.resize(n, 0);
    return b;
}

// ---------- Data for a run length ----------
struct RunData {
    vector<int> H, S, T;
};

static const int SQRT2 = 116195171; // square root of 2 modulo MOD

RunData compute_run_data(int N) {
    RunData rd;
    if (N == 0) {
        rd.H = {1};
        rd.S = {0};
        rd.T = {0};
        return rd;
    }
    if (N == 1) {
        rd.H = {0};
        rd.S = {1};
        rd.T = {1};
        return rd;
    }
    int len = N + 1; // coefficients 0..N

    // Solve w^2 = 1 - (1-z)e^z for z = psi(w), via Newton.
    // Let z = w * U(w).  The equation w^2 = 1-(1-z)e^z becomes
    // (A(wU)-w^2)/w^2 = 0, whose unknown U has non-zero derivative at w=0.
    vector<int> U(len, 0);
    U[0] = SQRT2;
    int prec = 1;
    while (prec < len) {
        int newp = min(2 * prec, len);
        int flen = newp + 2; // need F through degree newp+1
        vector<int> zt(flen, 0);
        for (int i = 1; i < flen; ++i) {
            int idx = i - 1;
            zt[i] = (idx < len ? U[idx] : 0);
        }
        vector<int> e = poly_exp(zt, flen);

        vector<int> om(flen, 0);
        om[0] = 1;
        for (int i = 1; i < flen; ++i) om[i] = (zt[i] == 0 ? 0 : MOD - zt[i]);
        vector<int> prod = multiply(om, e, flen);

        vector<int> F(flen, 0);
        F[0] = 1 - prod[0];
        if (F[0] < 0) F[0] += MOD;
        for (int i = 1; i < flen; ++i) F[i] = (prod[i] == 0 ? 0 : MOD - prod[i]);
        F[2] -= 1;
        if (F[2] < 0) F[2] += MOD;

        vector<int> Up(flen, 0);
        for (int i = 0; i < flen && i < len; ++i) Up[i] = U[i];
        vector<int> Fd = multiply(Up, e, flen); // U e^z
        vector<int> Fs2(newp);
        for (int i = 0; i < newp; ++i) Fs2[i] = F[i + 2];
        vector<int> invFd = poly_inv(Fd, newp);
        vector<int> q = multiply(Fs2, invFd, newp);
        q.resize(newp, 0);
        for (int i = 0; i < newp; ++i) {
            U[i] -= q[i];
            if (U[i] < 0) U[i] += MOD;
        }
        prec = newp;
    }

    vector<int> z(len, 0);
    for (int i = 1; i < len; ++i) z[i] = U[i - 1];

    // E(w)=exp(psi(w))
    vector<int> E = poly_exp(z, len);

    // Phi(w)=w/psi(w)
    vector<int> Uphi(len, 0);
    for (int i = 1; i < len; ++i) Uphi[i - 1] = z[i];
    vector<int> Phi = poly_inv(Uphi, len);

    // P = Phi^N
    vector<int> P(1, 1);
    {
        vector<int> base = Phi;
        int expn = N;
        int lim = len;
        while (expn > 0) {
            if (expn & 1) P = multiply(P, base, lim);
            expn >>= 1;
            if (expn) base = multiply(base, base, lim);
        }
        P.resize(lim, 0);
    }

    // derivatives / products
    vector<int> Ep(len, 0);
    for (int i = 0; i + 1 < len; ++i) Ep[i] = (int)((long long)E[i + 1] * (i + 1) % MOD);

    vector<int> psiE = multiply(z, E, len);
    vector<int> psiEp(len, 0);
    for (int i = 0; i + 1 < len; ++i) psiEp[i] = (int)((long long)psiE[i + 1] * (i + 1) % MOD);

    vector<int> Em1 = E;
    Em1[0] -= 1;
    if (Em1[0] < 0) Em1[0] += MOD;

    vector<int> Qs = multiply(Ep, P, len);
    vector<int> Rs = multiply(Em1, P, len);
    vector<int> Us = multiply(psiEp, P, len);
    vector<int> Vs = multiply(psiE, P, len);

    int invN = (int)modpow(N, MOD - 2);
    int invN2 = (int)(2LL * invN % MOD);

    rd.H.assign(len, 0);
    rd.S.assign(len, 0);
    rd.T.assign(len, 0);

    // H_N(x)
    for (int k = 0;; ++k) {
        int idx = N - 2 * k - 2;
        if (idx < 0) break;
        long long val = (long long)invN2 * (k + 1) % MOD * P[idx] % MOD;
        rd.H[k + 1] = (int)val;
    }
    // S_N(x)
    for (int m = 0;; ++m) {
        int idx = N - 1 - 2 * m;
        if (idx < 0) break;
        rd.S[m] = (int)((long long)invN * Qs[idx] % MOD);
    }
    for (int k = 0;; ++k) {
        int idx = N - 2 - 2 * k;
        if (idx < 0) break;
        long long val = (long long)invN2 * (k + 1) % MOD * Rs[idx] % MOD;
        rd.S[k + 1] += (int)val;
        if (rd.S[k + 1] >= MOD) rd.S[k + 1] -= MOD;
    }
    // T_N(x)
    for (int m = 0;; ++m) {
        int idx = N - 1 - 2 * m;
        if (idx < 0) break;
        rd.T[m] = (int)((long long)invN * Us[idx] % MOD);
    }
    for (int k = 0;; ++k) {
        int idx = N - 2 - 2 * k;
        if (idx < 0) break;
        long long val = (long long)invN2 * (k + 1) % MOD * Vs[idx] % MOD;
        rd.T[k + 1] += (int)val;
        if (rd.T[k + 1] >= MOD) rd.T[k + 1] -= MOD;
    }

    // trim
    auto trim = [](vector<int>& v) {
        while (v.size() > 1 && v.back() == 0) v.pop_back();
    };
    trim(rd.H);
    trim(rd.S);
    trim(rd.T);
    return rd;
}

unordered_map<int, RunData> cache;

const RunData& get_run(int N) {
    auto it = cache.find(N);
    if (it != cache.end()) return it->second;
    RunData rd = compute_run_data(N);
    auto res = cache.emplace(N, std::move(rd));
    return res.first->second;
}

vector<int> add_poly(const vector<int>& a, const vector<int>& b) {
    int n = max(a.size(), b.size());
    vector<int> c(n, 0);
    for (int i = 0; i < (int)a.size(); ++i) c[i] = a[i];
    for (int i = 0; i < (int)b.size(); ++i) {
        c[i] += b[i];
        if (c[i] >= MOD) c[i] -= MOD;
    }
    return c;
}

vector<int> x_shift(const vector<int>& a) {
    vector<int> c(a.size() + 1, 0);
    for (int i = 0; i < (int)a.size(); ++i) c[i + 1] = a[i];
    return c;
}

vector<int> region_poly_mixed(int a, int b) {
    const RunData& A = get_run(a);
    const RunData& B = get_run(b);
    int L = a + b;
    vector<int> q = multiply(A.H, B.H, L + 2);

    vector<int> t1 = multiply(A.T, B.S, L + 2);
    vector<int> t2 = multiply(A.S, B.T, L + 2);
    vector<int> t3 = multiply(A.S, B.S, L + 2);
    int sz = max({t1.size(), t2.size(), t3.size()});
    vector<int> term(sz, 0);
    for (int i = 0; i < sz; ++i) {
        long long v = 0;
        if (i < (int)t1.size()) v += t1[i];
        if (i < (int)t2.size()) v += t2[i];
        if (i < (int)t3.size()) v -= t3[i];
        v %= MOD;
        if (v < 0) v += MOD;
        term[i] = (int)v;
    }
    vector<int> sh = x_shift(term);
    if (q.size() < sh.size()) q.resize(sh.size(), 0);
    for (int i = 0; i < (int)sh.size(); ++i) {
        q[i] += sh[i];
        if (q[i] >= MOD) q[i] -= MOD;
    }
    return q;
}

vector<int> region_poly_hom(int L) {
    return get_run(L).H;
}

vector<int> region_poly_open_hom(int L) {
    const RunData& rd = get_run(L);
    vector<int> q = rd.H;
    vector<int> sh = x_shift(rd.S);
    if (q.size() < sh.size()) q.resize(sh.size(), 0);
    for (int i = 0; i < (int)sh.size(); ++i) {
        q[i] += sh[i];
        if (q[i] >= MOD) q[i] -= MOD;
    }
    return q;
}

vector<int> multiply_all(vector<vector<int>> polys) {
    if (polys.empty()) return {1};
    // remove zero polynomials? They are not expected, but guard.
    vector<vector<int>> tmp;
    for (auto& p : polys) {
        if (!p.empty()) tmp.push_back(std::move(p));
    }
    polys.swap(tmp);
    if (polys.empty()) return {1};
    while (polys.size() > 1) {
        vector<vector<int>> nxt;
        nxt.reserve((polys.size() + 1) / 2);
        for (size_t i = 0; i + 1 < polys.size(); i += 2) {
            nxt.push_back(multiply(polys[i], polys[i + 1]));
        }
        if (polys.size() & 1) nxt.push_back(std::move(polys.back()));
        polys.swap(nxt);
    }
    return polys[0];
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    string s;
    cin >> n >> s;

    // Split at LR boundaries. Each region is R^a L^b.
    vector<pair<int, int>> regions;
    int start = 0;
    for (int i = 0; i + 1 < n; ++i) {
        if (s[i] == 'L' && s[i + 1] == 'R') {
            regions.push_back({start, i});
            start = i + 1;
        }
    }
    regions.push_back({start, n - 1});

    vector<vector<int>> polys;
    polys.reserve(regions.size());

    for (int idx = 0; idx < (int)regions.size(); ++idx) {
        int l = regions[idx].first;
        int r = regions[idx].second;
        int L = r - l + 1;
        int a = 0;
        while (l + a <= r && s[l + a] == 'R') ++a;
        int b = L - a;
        vector<int> q;
        if (idx == 0 && a == 0) {
            q = region_poly_open_hom(L);
        } else if (idx == (int)regions.size() - 1 && b == 0) {
            q = region_poly_open_hom(L);
        } else if (a == 0 || b == 0) {
            q = region_poly_hom(L);
        } else {
            q = region_poly_mixed(a, b);
        }
        polys.push_back(std::move(q));
    }

    vector<int> Q = multiply_all(std::move(polys));

    long long fact = 1;
    for (int i = 2; i <= n; ++i) fact = fact * i % MOD;

    for (int k = 1; k <= n; ++k) {
        long long ans = (k < (int)Q.size() ? (long long)Q[k] * fact % MOD : 0);
        if (k > 1) cout << ' ';
        cout << ans;
    }
    cout << '\n';
    return 0;
}
