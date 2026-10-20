#include <bits/stdc++.h>
using namespace std;

using ll = long long;
using i128 = __int128_t;

struct Solver {
    ll n, a, b;
    i128 total, maxVal;

    Solver(ll n_, ll a_, ll b_) : n(n_), a(a_), b(b_) {
        total = (i128)n * (n - 1) / 2;
        maxVal = (i128)(n - 1) * ((i128)a * (n + 1) + 2 * (i128)b);
    }

    // Maximum value in row d = max_i d * (a * (2i + d) + 2b)
    // with i + d <= n, i >= 1.
    inline i128 Nmax(i128 d) const {
        return d * ((i128)a * (2 * (i128)n - d) + 2 * (i128)b);
    }

    // Minimum value in row d = value at i = 1.
    inline i128 Nmin(i128 d) const {
        return d * ((i128)a * (d + 2) + 2 * (i128)b);
    }

    // Number of pairs with N(d, i) <= X.
    i128 count_le(i128 X) const {
        if (X <= 0) return 0;
        if (X >= maxVal) return total;

        // d such that the whole row is <= X.
        i128 lo = 0, hi = n;
        while (hi - lo > 1) {
            i128 mid = (lo + hi) / 2;
            if (Nmax(mid) <= X) lo = mid;
            else hi = mid;
        }
        i128 dfull = lo;

        // Sum_{d=1}^{dfull} (n-d).
        i128 ans = dfull * (2 * (i128)n - dfull - 1) / 2;

        // d such that at least i=1 is possible.
        lo = 0, hi = n;
        while (hi - lo > 1) {
            i128 mid = (lo + hi) / 2;
            if (Nmin(mid) <= X) lo = mid;
            else hi = mid;
        }
        i128 dmax = lo;

        ll start = (ll)dfull + 1;
        ll endd = (ll)dmax;
        for (ll d = start; d <= endd; ++d) {
            i128 q = X / (i128)d;
            i128 c = (q - (i128)a * d - 2 * (i128)b) / (2 * (i128)a);
            ans += c;
        }
        return ans;
    }

    // Number of pairs with N(d, i) >= X.
    i128 count_ge(i128 X) const {
        if (X <= 0) return total;
        if (X > maxVal) return 0;

        // First d with row maximum >= X.
        i128 lo = 1, hi = n - 1;
        while (lo < hi) {
            i128 mid = (lo + hi) / 2;
            if (Nmax(mid) >= X) hi = mid;
            else lo = mid + 1;
        }
        ll d0 = (ll)lo;

        // First d with row minimum >= X; rows from here are fully counted.
        i128 dAll;
        if (Nmin((i128)n - 1) < X) {
            dAll = n;
        } else {
            lo = 1, hi = n - 1;
            while (lo < hi) {
                i128 mid = (lo + hi) / 2;
                if (Nmin(mid) >= X) hi = mid;
                else lo = mid + 1;
            }
            dAll = lo;
        }

        i128 ans = 0;

        ll partialEnd = (ll)dAll - 1;
        for (ll d = d0; d <= partialEnd && d <= n - 1; ++d) {
            i128 qless = (X - 1) / (i128)d;
            i128 less = (qless - (i128)a * d - 2 * (i128)b) / (2 * (i128)a);
            if (less < 0) less = 0;

            i128 len = (i128)n - d;
            if (less > len) less = len;
            ans += len - less;
        }

        if (dAll <= n - 1) {
            i128 r = (i128)n - dAll; // sum 1 + 2 + ... + r
            ans += r * (r + 1) / 2;
        }

        return ans;
    }

    // Find K-th smallest value.
    i128 solve_small(i128 K) const {
        i128 lo = 0, clo = 0;
        i128 hi = 1;
        i128 chi = count_le(hi);

        while (chi < K) {
            lo = hi;
            clo = chi;

            if (hi >= maxVal) {
                hi = maxVal;
                chi = count_le(hi);
                break;
            }

            hi *= 2;
            if (hi > maxVal) hi = maxVal;
            chi = count_le(hi);
        }

        int iter = 0;
        while (hi - lo > 1) {
            i128 mid;
            bool force = (iter % 5 == 4);

            if (force || chi == clo || chi == K) {
                mid = (lo + hi) / 2;
            } else {
                long double ratio =
                    (long double)(K - clo) / (long double)(chi - clo);
                if (ratio < 0) ratio = 0;
                if (ratio > 1) ratio = 1;
                mid = lo + (i128)((long double)(hi - lo) * ratio);
            }

            if (mid <= lo) mid = lo + 1;
            if (mid >= hi) mid = hi - 1;

            i128 c = count_le(mid);
            if (c >= K) {
                hi = mid;
                chi = c;
            } else {
                lo = mid;
                clo = c;
            }
            ++iter;
        }

        return hi;
    }

    // Find K-th largest value.
    i128 solve_large(i128 K) const {
        if (K == 1) return maxVal;

        i128 hi = maxVal + 1; // count_ge(hi) = 0 < K
        i128 lo, clo, chi;

        i128 c = count_ge(maxVal);

        if (c >= K) {
            lo = maxVal;
            clo = c;
            chi = 0;
        } else {
            i128 high = maxVal;
            i128 chiHigh = c;
            i128 gap = 1;

            while (true) {
                i128 nxt;
                if (gap >= maxVal) nxt = 0;
                else nxt = maxVal - gap;

                i128 cn = count_ge(nxt);

                if (cn >= K) {
                    lo = nxt;
                    clo = cn;
                    hi = high;
                    chi = chiHigh;
                    break;
                }

                high = nxt;
                chiHigh = cn;
                gap *= 2;
            }
        }

        int iter = 0;
        while (hi - lo > 1) {
            i128 mid;
            bool force = (iter % 5 == 4);

            if (force || clo == K || chi >= clo) {
                mid = (lo + hi) / 2;
            } else {
                long double ratio =
                    (long double)(clo - K) / (long double)(clo - chi);
                if (ratio < 0) ratio = 0;
                if (ratio > 1) ratio = 1;
                mid = lo + (i128)((long double)(hi - lo) * ratio);
            }

            if (mid <= lo) mid = lo + 1;
            if (mid >= hi) mid = hi - 1;

            i128 cm = count_ge(mid);
            if (cm >= K) {
                lo = mid;
                clo = cm;
            } else {
                hi = mid;
                chi = cm;
            }
            ++iter;
        }

        return lo;
    }
};

void print_i128(i128 x) {
    if (x == 0) {
        cout << '0';
        return;
    }

    if (x < 0) {
        cout << '-';
        x = -x;
    }

    string s;
    while (x > 0) {
        s.push_back(char('0' + x % 10));
        x /= 10;
    }
    reverse(s.begin(), s.end());
    cout << s;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    ll n, k, a, b;
    cin >> n >> k >> a >> b;

    Solver sol(n, a, b);

    i128 K;
    bool take_small;

    i128 kk = (i128)k;

    if (kk <= sol.total / 2) {
        take_small = true;
        K = kk;
    } else {
        take_small = false;
        K = sol.total - kk + 1;
    }

    i128 ans = take_small ? sol.solve_small(K) : sol.solve_large(K);

    ll aa = a;
    ll mod = (ll)(ans % aa);
    ll g = std::gcd(mod, aa);

    i128 p = ans / g;
    ll q = aa / g;

    print_i128(p);
    cout << ' ' << q << '\n';

    return 0;
}
