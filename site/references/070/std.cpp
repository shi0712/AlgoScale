#include <bits/stdc++.h>
using namespace std;

struct BigInt {
    static const int B = 1000000000;
    vector<int> a;
    int s = 0;

    BigInt(long long x = 0) { *this = x; }

    BigInt& operator=(long long x) {
        a.clear();
        s = 0;
        if (!x) return *this;
        if (x < 0) s = -1, x = -x;
        else s = 1;
        while (x) {
            a.push_back(x % B);
            x /= B;
        }
        return *this;
    }

    void trim() {
        while (!a.empty() && a.back() == 0) a.pop_back();
        if (a.empty()) s = 0;
    }

    static int cmp(const BigInt& x, const BigInt& y) {
        if (x.a.size() != y.a.size())
            return x.a.size() < y.a.size() ? -1 : 1;
        for (int i = (int)x.a.size() - 1; i >= 0; --i)
            if (x.a[i] != y.a[i])
                return x.a[i] < y.a[i] ? -1 : 1;
        return 0;
    }

    void addAbs(const BigInt& v) {
        long long c = 0;
        int n = max(a.size(), v.a.size());
        a.resize(n);
        for (int i = 0; i < n || c; ++i) {
            if (i == (int)a.size()) a.push_back(0);
            long long x = c + a[i];
            if (i < (int)v.a.size()) x += v.a[i];
            a[i] = x % B;
            c = x / B;
        }
    }

    void subAbs(const BigInt& v) {
        long long c = 0;
        for (int i = 0; i < (int)a.size(); ++i) {
            long long x = (long long)a[i]
                        - (i < (int)v.a.size() ? v.a[i] : 0) - c;
            if (x < 0) x += B, c = 1;
            else c = 0;
            a[i] = x;
        }
        trim();
    }

    BigInt& operator+=(const BigInt& v) {
        if (!v.s) return *this;
        if (!s) return *this = v;

        if (s == v.s) {
            addAbs(v);
        } else {
            int c = cmp(*this, v);
            if (!c) return *this = 0;
            if (c > 0) {
                subAbs(v);
            } else {
                BigInt t = v;
                t.subAbs(*this);
                *this = t;
            }
        }
        return *this;
    }

    friend BigInt operator+(BigInt x, const BigInt& y) {
        x += y;
        return x;
    }

    BigInt& operator-=(const BigInt& v) {
        BigInt t = v;
        t.s = -t.s;
        return *this += t;
    }

    void mul(long long v) {
        if (!s || !v) {
            *this = 0;
            return;
        }
        if (v < 0) s = -s, v = -v;

        __int128 c = 0;
        for (int& x : a) {
            __int128 y = (__int128)x * v + c;
            x = y % B;
            c = y / B;
        }
        while (c) {
            a.push_back(c % B);
            c /= B;
        }
    }

    void div(long long v) {
        __int128 r = 0;
        for (int i = (int)a.size() - 1; i >= 0; --i) {
            __int128 x = r * B + a[i];
            a[i] = x / v;
            r = x % v;
        }
        trim();
    }

    friend BigInt operator*(const BigInt& x, const BigInt& y) {
        if (!x.s || !y.s) return 0;

        vector<__int128> t(x.a.size() + y.a.size());

        for (int i = 0; i < (int)x.a.size(); ++i)
            for (int j = 0; j < (int)y.a.size(); ++j)
                t[i + j] += (__int128)x.a[i] * y.a[j];

        BigInt r;
        r.s = x.s * y.s;
        r.a.resize(t.size());

        for (int i = 0; i < (int)t.size(); ++i) {
            if (i + 1 < (int)t.size())
                t[i + 1] += t[i] / B;
            r.a[i] = t[i] % B;
        }

        r.trim();
        return r;
    }

    long double value() const {
        long double x = 0;
        for (int i = (int)a.size() - 1; i >= 0; --i)
            x = x * B + a[i];
        return s < 0 ? -x : x;
    }
};

BigInt Cbig(long long n, int k) {
    if (n < k) return 0;

    BigInt r = 1;
    for (int i = 1; i <= k; ++i) {
        r.mul(n - k + i);
        r.div(i);
    }
    return r;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, m;
    cin >> n >> m;

    // n <= 50，所以这些组合数可以直接放进 long long。
    vector<vector<BigInt>> C(n + 1, vector<BigInt>(n + 1));
    C[0][0] = 1;

    for (int i = 1; i <= n; ++i) {
        C[i][0] = C[i][i] = 1;
        for (int j = 1; j < i; ++j)
            C[i][j] = C[i - 1][j - 1] + C[i - 1][j];
    }

    int d = n + 1;

    // 插值只会用到 C(x,n)，x < n(n+2)。
    int lim = n * (n + 2);
    vector<BigInt> choose(lim + 1);

    choose[n] = 1;

    for (int x = n + 1; x <= lim; ++x) {
        choose[x] = choose[x - 1];
        choose[x].mul(x);
        choose[x].div(x - n);
    }

    // F[s] = C(n,s) * sum_q C(m-sq,n)。
    vector<BigInt> F(n + 1);

    for (int s = 1; s <= n; ++s) {
        int Q = m / s;
        int r = m % s;

        // P(j) = sum_{t=0}^j C(r+st,n)。
        vector<BigInt> p(d + 1);
        BigInt cur = 0;

        for (int j = 0; j <= d; ++j) {
            cur += choose[r + s * j];
            p[j] = cur;
        }

        BigInt S = 0;

        if (Q <= d) {
            S = p[Q];
        } else {
            // |L_0| = C(Q-1,d)。
            BigInt L = Cbig(Q - 1, d);

            for (int j = 0; j <= d; ++j) {
                BigInt term = p[j] * L;

                // 符号为 (-1)^(d-j)。
                if ((d - j) & 1) S -= term;
                else S += term;

                if (j < d) {
                    // |L_{j+1}| =
                    // |L_j| * (Q-j)/(j+1) * (d-j)/(Q-j-1)。
                    L.mul(Q - j);
                    L.div(j + 1);

                    L.mul(d - j);
                    L.div(Q - j - 1);
                }
            }
        }

        F[s] = S;
        F[s] = F[s] * C[n][s];
    }

    BigInt den = Cbig(m, n);
    long double D = den.value();

    cout << fixed << setprecision(12);

    for (int k = 1; k <= n; ++k) {
        int a = n - k;
        BigInt num = 0;

        if (a == 0) {
            num = F[1];
        } else {
            for (int s = a + 1; s <= n; ++s) {
                BigInt term = F[s];
                term = term * C[s - 2][a - 1];

                if ((s - a - 1) & 1) num -= term;
                else num += term;
            }
        }

        cout << num.value() / D << '\n';
    }

    return 0;
}
