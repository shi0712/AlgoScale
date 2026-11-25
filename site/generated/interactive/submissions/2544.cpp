#include <bits/stdc++.h>
using namespace std;

long long ask(long long l, long long r) {
    cout << "? " << l << " " << r << endl;
    long long res;
    if (!(cin >> res)) exit(0);
    if (res == -1) exit(0);
    return res;
}

long long tri(long long m) {
    return m * (m - 1) / 2;
}

long long invTri(long long t) {
    if (t == 0) return 1;
    long double d = sqrt((long double)(8 * t + 1));
    long long m = (long long)((1 + d) / 2);
    while (tri(m) < t) ++m;
    while (m > 0 && tri(m) > t) --m;
    return m;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    long long n;
    if (!(cin >> n)) return 0;

    long long S = ask(1, n);

    // Find A = i. P(x) = 0 for x <= A, P(A+1) = 1.
    long long lo = 1, hi = n;
    while (lo < hi) {
        long long mid = (lo + hi) / 2;
        long long v = ask(1, mid);
        if (v > 0) hi = mid;
        else lo = mid + 1;
    }
    long long A = lo - 1;

    // Find B = j - 1. For x <= B, P(x) = tri(x - A + 1).
    long long maxL = invTri(S - 1); // since M >= 2, tri(M) >= 1
    long long loB = A + 1;
    long long hiB = min(n - 2, A + maxL - 1);
    if (hiB < loB) hiB = loB;
    while (loB < hiB) {
        long long mid = (loB + hiB + 1) / 2;
        long long v = ask(1, mid);
        long long t = tri(mid - A + 1);
        if (v == t) loB = mid;
        else hiB = mid - 1;
    }
    long long B = loB;

    long long L = B - A + 1;
    long long rem = S - tri(L);
    long long M = invTri(rem);

    long long i = A;
    long long j = B + 1;
    long long k = B + M;

    cout << "! " << i << " " << j << " " << k << endl;
    return 0;
}
