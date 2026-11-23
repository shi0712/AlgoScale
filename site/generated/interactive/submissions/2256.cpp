#include <bits/stdc++.h>
using namespace std;

using int64 = long long;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    const int64 LIMIT = 100000000000000LL;

    int64 low = 0, high = LIMIT, balance = 1;

    vector<int64> capacity = {1, 2};
    while (capacity.back() < LIMIT + 1) {
        capacity.push_back(capacity[capacity.size() - 1] +
                           capacity[capacity.size() - 2]);
    }

    auto ask = [](int64 x) -> int {
        cout << "? " << x << '\n' << flush;
        string response;
        if (!(cin >> response)) exit(0);
        if (response == "Fired!" || response == "-1") exit(0);
        if (response == "Lucky!") return 1;
        if (response == "Fraudster!") return 0;
        exit(0);
    };

    auto chooseDelta = [&](int64 n) -> int64 {
        if (n <= 2) return 1;

        int k = int(lower_bound(capacity.begin(), capacity.end(), n) -
                    capacity.begin());
        if (k <= 1) return 1;

        int64 minimum = n - capacity[k - 1];
        int64 maximum = capacity[k - 2];
        int64 preferred = int64((long double)n *
                                0.3819660112501051L + 0.5L);

        int64 d = max(minimum, min(maximum, preferred));
        d = max<int64>(1, d);
        if (d >= n) d = n - 1;
        return d;
    };

    // Grow the guaranteed lower bound and the available balance. Once the
    // weighted probe is affordable, switch to searching the finite interval.
    while (low < high) {
        int64 n = high - low + 1;
        int64 desired = low + chooseDelta(n);

        if (desired <= balance) break;
        if (balance <= low || balance > high) break;

        int64 x = balance;
        int result = ask(x);
        if (result == 1) {
            balance += x;
            low = x;
        } else {
            balance -= x;
            high = x - 1;
            break;
        }
    }

    while (low < high) {
        int64 n = high - low + 1;
        int64 x = low + chooseDelta(n);

        // Transfers of low euros are guaranteed to succeed.
        while (balance < x) {
            int result = ask(low);
            if (result != 1) return 0;
            balance += low;
        }

        int result = ask(x);
        if (result == 1) {
            balance += x;
            low = x;
        } else {
            balance -= x;
            high = x - 1;
        }
    }

    cout << "! " << low << '\n' << flush;
    return 0;
}
