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

        auto normalize = [&](int64 x) {
            x %= n;
            if (x <= 0) x += n;
            return x;
        };

        auto ask = [&](int64 x) {
            cout << "? " << normalize(x) << '\n' << flush;
            int64 response;
            if (!(cin >> response)) exit(0);
            return response;
        };

        const int64 q = ask(1);
        const int64 plateauLength = (q <= m ? q : n - q);

        int64 lo = 0, hi = m + 1;
        int64 responseAtLo = q, responseAtHi = -1, responseAtOne = -1;

        auto good = [&](int64 k, int64 response) {
            return response == q + 2 * k ||
                   response == q - 2 * k ||
                   (response == q && k < plateauLength);
        };

        while (hi - lo > 1) {
            int64 mid = (lo + hi) / 2;
            int64 response = ask(1 + mid);
            if (mid == 1) responseAtOne = response;
            if (good(mid, response)) {
                lo = mid;
                responseAtLo = response;
            } else {
                hi = mid;
                responseAtHi = response;
            }
        }

        int64 a, b;
        if (lo > 0 && responseAtLo == q + 2 * lo) {
            int64 separation = n - 1 - q - 2 * lo;
            a = 1 + lo - m;
            b = a + separation;
        } else if (lo > 0 && responseAtLo == q - 2 * lo) {
            int64 separation = q - 2 * lo;
            a = 1 + lo;
            b = a + separation;
        } else if (lo > 0) {
            if (q <= m) {
                int64 separation = q;
                b = 1 + lo + (hi == q && responseAtHi == q);
                a = b - separation;
            } else {
                int64 separation = n - q;
                a = 1 + lo - (m + separation);
                b = a + separation;
            }
        } else if (responseAtOne == q + 1) {
            int64 separation = n - 1 - q;
            a = 1 - m;
            b = a + separation;
        } else if (responseAtOne == q && q == 1) {
            a = 1;
            b = 2;
        } else {
            int64 separation = n - q;
            a = 1 - (m + separation);
            b = a + separation;
        }

        cout << "! " << normalize(a) << ' ' << normalize(b) << '\n' << flush;
    }

    return 0;
}
