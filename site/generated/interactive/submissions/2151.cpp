#include <bits/stdc++.h>
using namespace std;

long long n, m;
unordered_map<long long, long long> cache;

long long norm(long long x) {
    x %= n;
    if (x < 0) x += n;
    return x;
}

long long query(long long x) {
    x = norm(x);
    auto it = cache.find(x);
    if (it != cache.end()) return it->second;

    cout << "? " << x + 1 << '\n';
    cout.flush();

    long long d;
    if (!(cin >> d)) exit(0);
    if (d == -1) exit(0);

    cache[x] = d;
    return d;
}

void answer(long long a, long long b) {
    a = norm(a);
    b = norm(b);
    cout << "! " << a + 1 << " " << b + 1 << '\n';
    cout.flush();
    exit(0);
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    if (!(cin >> n)) return 0;
    m = (n - 1) / 2;
    cache.reserve(128);

    long long s0 = query(0);
    long long sm = query(m);

    // c = number of transmitters in the counter-clockwise half (m+1 .. n-1)
    long long c = s0 + sm - 2 * m;

    if (c == 0) {
        // Both are in [0, m] (clockwise half, including 0).
        long long t = s0 / 2;
        long long u;
        if (t == 0) u = s0;
        else u = query(t);

        long long r1 = (s0 - u) / 2;
        long long r2 = (s0 + u) / 2;
        answer(r1, r2);
    } else if (c == 2) {
        // Both are in the counter-clockwise half.
        long long t = s0 / 2;
        long long u;
        if (t == 0) u = s0;
        else u = query(-t);

        long long q1 = (s0 - u) / 2;
        long long q2 = (s0 + u) / 2;
        answer(-q1, -q2);
    } else if (c == 1) {
        // One on each side of 0.
        long long k, p, lo, hi;

        if (s0 <= m) {
            // 0 is on the minimum plateau, minimum value is s0.
            k = s0;
            p = 0;
            lo = 0;
            hi = k - 1; // the counter-clockwise transmitter is not at 0
        } else {
            // m is on the minimum plateau, minimum value is n - s0.
            k = n - s0;
            p = m;
            lo = 1;     // the clockwise endpoint from m is not m itself
            hi = k;
        }

        if (hi < lo) hi = lo;

        // Find the clockwise endpoint of the minimum plateau.
        // P(d) = (f(p + d) == k) is true for d <= true_d.
        while (lo < hi) {
            long long mid = (lo + hi + 1) / 2;
            long long val = query(p + mid);

            if (val == k) {
                lo = mid;
            } else {
                long long maxVal = n - k;

                // If we are in the increasing linear part, the value gives d directly:
                // val = 2*mid + k - 2*d  =>  d = mid + (k - val) / 2.
                if (val < maxVal) {
                    long long diff = k - val;
                    if (diff % 2 == 0) {
                        long long d = mid + diff / 2;
                        if (d >= lo && d <= hi) {
                            lo = hi = d;
                            break;
                        }
                    }
                }

                // Otherwise it is on the maximum plateau; only a false predicate.
                hi = mid - 1;
            }
        }

        long long d = lo;
        long long e1 = p + d;
        long long e2 = p - (k - d);
        answer(e1, e2);
    } else {
        exit(0);
    }

    return 0;
}
