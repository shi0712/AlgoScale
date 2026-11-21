#include <bits/stdc++.h>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    const long long MAXM = 100000000000000LL;

    long long L = 0;          // known lower bound: M >= L
    long long R = MAXM;       // known upper bound: M <= R
    long long B = 1;          // current balance

    while (true) {
        if (L == R) {
            cout << "! " << L << '\n' << flush;
            return 0;
        }

        long long x;

        if (B == 0) {
            // Should not happen for an unresolved interval, but keep a safe fallback.
            if (L > 0) {
                x = 1; // guaranteed success because M >= L >= 1
            } else {
                cout << "! 0\n" << flush;
                return 0;
            }
        } else if (L == 0) {
            x = 1;
        } else if (B <= L) {
            // Guaranteed success: build balance by doubling.
            x = B;
        } else {
            // If the interval is relatively small and balance is not yet very large,
            // spend a few guaranteed queries to make binary search cheap.
            bool boost = (B >= R + 1 && 2 * L >= R && B < 4 * R && R - L > 64);

            if (boost) {
                x = L; // guaranteed success
            } else {
                // Leave a small reserve so that after a failure we can rebuild.
                long long reserve = L / 32;
                if (reserve < 1) reserve = 1;

                x = B - reserve;
                if (x <= L) x = L + 1;

                long long mid = L + (R - L + 1) / 2; // upper middle, always > L
                if (x >= mid) x = mid;
            }
        }

        // Safety clamps (the logic above should already guarantee these).
        if (x < 1) x = 1;
        if (x > B) {
            if (B >= L + 1) x = L + 1;
            else x = B;
        }
        if (x > R) x = R;
        if (x < 1) x = 1;

        cout << "? " << x << '\n' << flush;

        string resp;
        if (!(cin >> resp)) return 0;

        if (resp == "Fired!") {
            return 0;
        }

        if (resp == "Lucky!") {
            B += x;
            if (x > L) L = x;
        } else if (resp == "Fraudster!") {
            B -= x;
            if (x > L) R = x - 1;
            else R = L; // should not happen
        } else {
            return 0;
        }
    }
}
