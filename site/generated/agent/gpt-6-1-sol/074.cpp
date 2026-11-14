#include <bits/stdc++.h>
using namespace std;
using int64 = long long;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int T;
    if (!(cin >> T)) return 0;
    while (T--) {
        int64 n;
        if (!(cin >> n)) return 0;
        const int64 m = n / 2;

        // Internally vertices are numbered 0,...,n-1.
        auto ask = [&](int64 x) -> int64 {
            x = (x % n + n) % n;
            cout << "? " << x + 1 << endl;
            int64 response;
            if (!(cin >> response) || response < 0) exit(0);
            return response;
        };
        auto answer = [&](int64 x, int64 y) {
            x = (x % n + n) % n;
            y = (y % n + n) % n;
            cout << "! " << x + 1 << ' ' << y + 1 << endl;
        };

        const int64 p = ask(0);
        const int64 k = p / 2;
        const int64 r = (k == 0 ? p : ask(k));
        const int64 d = min(p, n - p);

        // Signed distances from 0 lie in [-m,m]. The possible pairs are:
        // A: {a, -(p-a)}, max(0,p-m) <= a <= min(m,p).
        // B: {a, p-a},    max(1,p-m) <= a <= (p-1)/2.
        // C: {-a, -(p-a)}, with the same bounds as B.
        // Endpoints at 0 belong to A, so these families are disjoint.
        const int64 lowC = max(1LL, p - m);
        const int64 highC = (p - 1) / 2;
        const int64 capC = min(p + 2 * k, 2 * n - p - 2 * k);
        bool possibleC = false;
        if (lowC <= highC) {
            // In C, f(k) = min(capC, n-p+2*a).
            const int64 lowValue = min(capC, n - p + 2 * lowC);
            const int64 highValue = min(capC, n - p + 2 * highC);
            possibleC = lowValue <= r && r <= highValue &&
                        (r == capC || (r - (n - p)) % 2 == 0);
        }

        if (possibleC) {
            // A can share this response only at a=0; that pair also
            // lies on the negative half with the sum of distances p.
            // The point -k is between the two negative endpoints.
            const int64 difference = ask(-k);
            answer(-(p - difference) / 2, -(p + difference) / 2);
            continue;
        }

        if (r < d) {
            // Only B can have f(k) below the separation of pairs in A.
            answer((p - r) / 2, (p + r) / 2);
            continue;
        }

        // Only family A remains. Determine all its parameters consistent
        // with the second response. Its response is monotone in a.
        int64 left, right;
        if (p <= m) {
            // f(k) = min(n-p, p+2*max(0,k-a)).
            if (r == p) {
                left = k;
                right = p;
            } else if (r == n - p) {
                left = 0;
                right = k - (m - p + 1);
            } else {
                left = right = k - (r - p) / 2;
            }
        } else {
            // f(k) = min(p, n-p+2*max(0,a-k)).
            if (r == n - p) {
                left = p - m;
                right = k;
            } else if (r == p) {
                left = k + p - m;
                right = m;
            } else {
                left = right = k + (r - (n - p)) / 2;
            }
        }

        // At most floor(m/2)+2 parameters remain, fewer than 2^28.
        // Every following query halves this interval, so the total is
        // at most 2+28=30 queries.
        while (left < right) {
            if (p <= m) {
                const int64 mid = (left + right + 1) / 2;
                if (ask(mid) == p) left = mid;
                else right = mid - 1;
            } else {
                const int64 mid = (left + right) / 2;
                if (ask(mid) == n - p) right = mid;
                else left = mid + 1;
            }
        }
        answer(left, n - p + left);
    }
    return 0;
}
