#include <bits/stdc++.h>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, t;
    if (!(cin >> n >> t)) return 0;          // no input → exit

    for (int round = 0; round < t; ++round) {
        int k;
        if (!(cin >> k)) return 0;           // EOF or error

        int lo = 1, hi = n;
        while (lo < hi) {
            int mid = lo + (hi - lo) / 2;
            cout << "? " << 1 << ' ' << mid << endl;   // flush with endl
            int sumOnes;
            if (!(cin >> sumOnes)) return 0;           // judge closed
            if (sumOnes == -1) return 0;               // invalid query

            int zeros = mid - sumOnes;                 // Lemma 1
            if (zeros >= k)
                hi = mid;
            else
                lo = mid + 1;
        }

        cout << "! " << lo << endl;                    // answer, flush
    }
    return 0;
}
