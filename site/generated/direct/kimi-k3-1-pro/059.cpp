#include <bits/stdc++.h>
using namespace std;

using int64 = long long;
using uint64 = unsigned long long;

struct BigInt {
    static constexpr uint32_t BASE = 1000000000U;
    vector<uint32_t> a;

    BigInt(uint64_t x = 0) {
        while (x) {
            a.push_back(x % BASE);
            x /= BASE;
        }
    }

    bool isZero() const { return a.empty(); }

    void trim() {
        while (!a.empty() && a.back() == 0) a.pop_back();
    }

    void add(const BigInt &o) {
        if (o.a.empty()) return;
        if (a.size() < o.a.size()) a.resize(o.a.size(), 0);
        uint64_t carry = 0;
        for (size_t i = 0; i < o.a.size() || carry; ++i) {
            if (i == a.size()) a.push_back(0);
            uint64_t cur = a[i] + carry;
            if (i < o.a.size()) cur += o.a[i];
            if (cur >= BASE) {
                cur -= BASE;
                carry = 1;
            } else {
                carry = 0;
            }
            a[i] = (uint32_t)cur;
        }
    }

    // *this >= o
    void sub(const BigInt &o) {
        int64_t borrow = 0;
        for (size_t i = 0; i < a.size(); ++i) {
            int64_t cur = (int64_t)a[i] - borrow;
            if (i < o.a.size()) cur -= o.a[i];
            if (cur < 0) {
                cur += BASE;
                borrow = 1;
            } else {
                borrow = 0;
            }
            a[i] = (uint32_t)cur;
        }
        trim();
    }

    void mulSmall(uint32_t x) {
        if (x == 0 || isZero()) {
            if (x == 0) a.clear();
            return;
        }
        uint64_t carry = 0;
        for (size_t i = 0; i < a.size(); ++i) {
            uint64_t cur = (uint64_t)a[i] * x + carry;
            a[i] = cur % BASE;
            carry = cur / BASE;
        }
        while (carry) {
            a.push_back(carry % BASE);
            carry /= BASE;
        }
    }

    void divSmall(uint32_t x) {
        uint64_t rem = 0;
        for (int i = (int)a.size() - 1; i >= 0; --i) {
            uint64_t cur = rem * BASE + a[i];
            a[i] = cur / x;
            rem = cur % x;
        }
        trim();
    }
};

static long double logBig(const BigInt &b) {
    if (b.isZero()) return -numeric_limits<long double>::infinity();
    int len = b.a.size();
    long double top = b.a.back();
    if (len >= 2) top += (long double)b.a[len - 2] / BigInt::BASE;
    if (len >= 3) {
        top += (long double)b.a[len - 3] /
               ((long double)BigInt::BASE * BigInt::BASE);
    }
    return (len - 1) * logl((long double)BigInt::BASE) + logl(top);
}

static vector<long double> binomialRow(int n) {
    vector<long double> c(n + 1);
    c[0] = 1;
    for (int k = 1; k <= n; ++k)
        c[k] = c[k - 1] * (n - k + 1) / k;
    return c;
}

// Exact computation, used when m is moderate.
static vector<long double> solveExact(int n, int m) {
    vector<BigInt> bc(m + 1);
    bc[n] = BigInt(1);
    for (int x = n + 1; x <= m; ++x) {
        bc[x] = bc[x - 1];
        bc[x].mulSmall(x);
        bc[x].divSmall(x - n); // C(x,n) = C(x-1,n)*x/(x-n)
    }

    // A[q] = sum_{t>=1} C(m-q*t,n).
    vector<BigInt> A(n + 1);
    for (int q = 1; q <= n; ++q) {
        for (int x = m - q; x >= n; x -= q)
            A[q].add(bc[x]);
    }

    const long double logD = logBig(bc[m]);
    vector<long double> binom = binomialRow(n);
    vector<long double> V(n, 0);

    if (!A[n].isZero())
        V[0] = expl(logBig(A[n]) - logD);

    // Finite differences in q.  Row p contains D^p A(h).
    vector<BigInt> prev(n + 1);
    for (int h = 1; h <= n; ++h) prev[h] = A[h];

    for (int p = 1; p <= n - 1; ++p) {
        int len = n - p;
        vector<BigInt> cur(len + 1);
        for (int h = 1; h <= len; ++h) {
            cur[h] = prev[h];
            cur[h].sub(prev[h + 1]);
        }
        const BigInt &diag = cur[len];
        if (!diag.isZero()) {
            V[p] = expl(logl(binom[p]) + logBig(diag) - logD);
        }
        prev.swap(cur);
    }
    return V;
}

static long double smallBinom(int n, int k) {
    if (k < 0 || k > n) return 0;
    k = min(k, n - k);
    long double r = 1;
    for (int i = 1; i <= k; ++i)
        r = r * (n - k + i) / i;
    return r;
}

// Euler-Maclaurin evaluation, used for large m.
static vector<long double> solveLarge(int n, int m) {
    const int T = m - n;

    // F(y) = C(m-y,n)/C(m,n), extended as a polynomial.
    // J = integral_0^T F(y)dy.  160-point Gauss-Legendre is
    // exact for every polynomial of degree <= 319.
    const int N = 160;
    vector<long double> gx(N), gw(N);
    const long double PI = acosl(-1.0L);

    auto legendre = [&](long double z) {
        long double p0 = 1, p1 = z;
        for (int k = 2; k <= N; ++k) {
            long double p2 = ((2 * k - 1) * z * p1 - (k - 1) * p0) / k;
            p0 = p1;
            p1 = p2;
        }
        return pair<long double, long double>(p1, p0);
    };

    for (int i = 0; i < (N + 1) / 2; ++i) {
        long double z = cosl(PI * (i + 0.75L) / (N + 0.5L));
        for (int it = 0; it < 100; ++it) {
            auto [pn, pnm1] = legendre(z);
            long double dp = N * (z * pn - pnm1) / (z * z - 1);
            long double dz = pn / dp;
            z -= dz;
            if (fabsl(dz) < 1e-19L) break;
        }
        auto [pn, pnm1] = legendre(z);
        long double dp = N * (z * pn - pnm1) / (z * z - 1);
        long double w = 2 / ((1 - z * z) * dp * dp);
        gx[i] = -z;
        gx[N - 1 - i] = z;
        gw[i] = gw[N - 1 - i] = w;
    }

    long double J = 0;
    long double mid = (long double)T / 2;
    long double half = (long double)T / 2;
    for (int k = 0; k < N; ++k) {
        long double y = mid + half * gx[k];
        long double f = 1;
        for (int r = 0; r < n; ++r)
            f *= 1 - y / (long double)(m - r);
        J += half * gw[k] * f;
    }

    const int S = min(41, n);

    // Elementary symmetric polynomials give F^(s)(0).
    vector<long double> e(S + 1, 0), fact(S + 2, 1), der(S + 1, 0);
    e[0] = 1;
    for (int r = 0; r < n; ++r) {
        long double ar = 1.0L / (m - r);
        for (int s = min(S, r + 1); s >= 1; --s)
            e[s] += e[s - 1] * ar;
    }
    for (int s = 1; s <= S + 1; ++s) fact[s] = fact[s - 1] * s;
    for (int s = 0; s <= S; ++s) der[s] = fact[s] * e[s];

    // Bernoulli numbers B_0,...,B_{S+1}.
    vector<long double> bern(S + 2, 0);
    bern[0] = 1;
    bern[1] = -0.5L;
    for (int Nn = 2; Nn <= S + 1; ++Nn) {
        if (Nn & 1) {
            bern[Nn] = 0;
            continue;
        }
        long double sum = 0;
        for (int k = 0; k < Nn; ++k)
            sum += smallBinom(Nn + 1, k) * bern[k];
        bern[Nn] = -sum / (Nn + 1);
    }

    // U(q) approximately J/q - 1/2 + sum E[s] q^s (odd s).
    vector<long double> E(S + 1, 0);
    for (int s = 1; s <= S; s += 2)
        E[s] = bern[s + 1] / fact[s + 1] * der[s];

    // Stirling numbers and falling factorials for finite differences
    // of q^s.
    vector<vector<long double>> stir(S + 1, vector<long double>(S + 1, 0));
    vector<vector<long double>> rfal(S + 1, vector<long double>(S + 1, 0));
    stir[0][0] = 1;
    for (int s = 1; s <= S; ++s) {
        for (int r = 1; r <= s; ++r)
            stir[s][r] = stir[s - 1][r - 1] + r * stir[s - 1][r];
    }
    for (int r = 0; r <= S; ++r) {
        rfal[r][0] = 1;
        for (int p = 1; p <= r; ++p)
            rfal[r][p] = rfal[r][p - 1] * (r - p + 1);
    }

    auto deltaPower = [&](int h, int p, int s) {
        // Ordinary forward difference Delta^p h^s.
        long double res = 0;
        for (int r = p; r <= s; ++r) {
            int k = r - p;
            long double hf = 1;
            for (int z = 0; z < k; ++z) hf *= (h - z);
            res += stir[s][r] * rfal[r][p] * hf;
        }
        return res;
    };

    vector<long double> binom = binomialRow(n);
    vector<long double> V(n, 0);

    V[0] = J / n - 0.5L;
    long double qp = n;
    for (int s = 1; s <= S; s += 2) {
        V[0] += E[s] * qp;
        qp *= (long double)n * n;
    }

    for (int p = 1; p <= n - 1; ++p) {
        int h = n - p;
        V[p] = J / h;
        int first = (p & 1) ? p : p + 1;
        for (int s = first; s <= S; s += 2) {
            long double d = deltaPower(h, p, s);
            if (p & 1) d = -d; // D^p = (-1)^p Delta^p
            V[p] += binom[p] * E[s] * d;
        }
    }
    return V;
}

static vector<long double> solveTwo(int m) {
    long double mm = m;
    long double T = m - 2;
    long long Li = (m - 2) / 2;
    long double L = Li;

    long double S1 = L * (L + 1) / 2;
    long double S2 = L * (L + 1) * (2 * L + 1) / 6;

    // sum_{t=1}^L C(m-2t,2)
    long double A2 = 0.5L *
        (L * mm * (mm - 1) + (2 - 4 * mm) * S1 + 4 * S2);
    long double D = mm * (mm - 1) / 2;

    vector<long double> V(2);
    V[0] = A2 / D;
    V[1] = 2 * (T / 3 - V[0]);
    return V;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, m;
    if (!(cin >> n >> m)) return 0;

    if (n == 1) {
        cout << fixed << setprecision(12)
             << (long double)(m + 1) / 2 << '\n';
        return 0;
    }

    vector<long double> V;
    if (n == 2) {
        V = solveTwo(m);
    } else if (m <= 100000) {
        V = solveExact(n, m);
    } else {
        V = solveLarge(n, m);
    }

    vector<long double> ans(n);
    long double sumV = 0, S = 0;
    for (int i = 1; i <= n; ++i) {
        sumV += V[i - 1];
        S += sumV;
        ans[i - 1] = i + S;
    }

    // The last entry is E[a_n], which has a particularly simple form.
    ans[n - 1] = (long double)n * ((long double)m + 1) / (n + 1);

    cout << fixed << setprecision(12);
    for (long double x : ans) cout << x << '\n';
    return 0;
}
