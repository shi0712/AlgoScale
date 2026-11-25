#include <bits/stdc++.h>
using namespace std;

long long n, m;
unordered_map<long long, long long> cache;

long long mod(long long x) {
    x %= n;
    if (x < 0) x += n;
    return x;
}

long long ask(long long x) {
    x = mod(x);
    auto it = cache.find(x);
    if (it != cache.end()) return it->second;
    cout << "? " << x + 1 << "\n";
    cout.flush();
    long long d;
    if (!(cin >> d) || d == -1) exit(0);
    cache[x] = d;
    return d;
}

void solve_arc(long long start, long long dir, long long L) {
    long long lo = 0, hi = L - 1;
    while (lo < hi) {
        long long mid = (lo + hi) / 2;
        long long f1 = ask(start + dir * mid);
        long long f2 = ask(start + dir * (mid + 1));
        if (f1 <= f2)
            hi = mid;
        else
            lo = mid + 1;
    }
    long long i = lo;
    long long left = mod(start + dir * i);
    long long s = ask(left);
    long long right = mod(left + dir * s);
    cout << "! " << left + 1 << " " << right + 1 << "\n";
    cout.flush();
    exit(0);
}

void solve_min_plateau(long long x, long long s) {
    long long a;
    if (ask(x + 1) > s) {
        a = x;
    } else {
        long long lo = 2, hi = m + 1;
        while (lo < hi) {
            long long mid = (lo + hi) / 2;
            if (ask(x + mid) > s)
                hi = mid;
            else
                lo = mid + 1;
        }
        long long L = lo - 1;
        a = mod(x + L);
    }
    long long b = mod(a - s);
    cout << "! " << a + 1 << " " << b + 1 << "\n";
    cout.flush();
    exit(0);
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(0);

    if (!(cin >> n)) return 0;
    m = (n - 1) / 2;

    long long x = 0;
    while (true) {
        long long v = ask(x);
        long long vp = ask(x + 1);
        long long vm = ask(x - 1);

        if (vp < v) {
            solve_arc(x, 1, m + 1);
        }
        if (vm < v) {
            solve_arc(x, -1, m + 1);
        }
        if (vp > v && vm > v) {
            solve_min_plateau(x, v);
        }

        long long far = ask(x + m + 1);
        if (far > v) {
            solve_min_plateau(x, v);
        } else {
            long long step = 1;
            while (true) {
                long long y = x + step;
                long long vy = ask(y);
                if (vy != v) {
                    x = mod(y);
                    break;
                }
                step *= 2;
                if (step > n) step = n;
            }
        }
    }
    return 0;
}
