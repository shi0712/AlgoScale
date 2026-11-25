#include <bits/stdc++.h>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    const long long MAXM = 100000000000000LL;
    long long L = 0, U = MAXM, B = 1;

    while (L < U) {
        long long x;

        if (B <= L) {
            // Build capital using a known successful operation.
            x = (L == 0 ? 1 : L);
        } else {
            long long n = U - L;
            long long mid = L + (n + 1) / 2; // upper middle

            if (B >= 2 * U) {
                // Enough capital to test the upper bound safely.
                x = U;
            } else {
                x = min(B, mid);
            }
        }

        cout << "? " << x << "\n" << flush;

        string response;
        if (!(cin >> response)) return 0;

        if (response == "Fired!") return 0;

        if (response == "Lucky!") {
            if (x > L) L = x;
            B += x;
        } else if (response == "Fraudster!") {
            if (x - 1 < U) U = x - 1;
            B -= x;
            if (B < 0) B = 0;
        } else {
            return 0;
        }

        if (L == U) {
            cout << "! " << L << "\n" << flush;
            return 0;
        }
    }

    cout << "! " << L << "\n" << flush;
    return 0;
}
