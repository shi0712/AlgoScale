#include <bits/stdc++.h>
using namespace std;

using int64 = long long;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n)) return 0;

    if (n == 4) {
        cout << "! 1 3 4\n" << flush;
        return 0;
    }

    auto ask = [&](int l, int r) -> int64 {
        cout << "? " << l << ' ' << r << '\n' << flush;
        int64 response;
        if (!(cin >> response) || response == -1) exit(0);
        return response;
    };

    const int64 total = ask(1, n);

    int low = 1;
    int high = n - 2;
    while (high - low > 1) {
        int mid = low + (high - low) / 2;
        if (ask(1, mid) > 0)
            high = mid;
        else
            low = mid;
    }

    const int64 i = high - 1;
    int64 firstLength, secondLength;

    if (total == 2) {
        firstLength = secondLength = 2;
    } else {
        const int64 remaining = ask(static_cast<int>(i + 1), n);
        firstLength = total - remaining + 1;
        const int64 otherInversions =
            total - firstLength * (firstLength - 1) / 2;

        const int64 discriminant = 1 + 8 * otherInversions;
        int64 root = sqrt(static_cast<long double>(discriminant));
        while ((__int128)(root + 1) * (root + 1) <= discriminant) ++root;
        while ((__int128)root * root > discriminant) --root;
        secondLength = (root + 1) / 2;
    }

    const int64 j = i + firstLength;
    const int64 k = j + secondLength - 1;
    cout << "! " << i << ' ' << j << ' ' << k << '\n' << flush;
    return 0;
}
