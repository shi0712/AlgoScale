#include <cmath>
#include <iostream>

using namespace std;
using int64 = long long;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int64 n;
    if (!(cin >> n)) return 0;

    if (n == 4) {
        cout << "! 1 3 4" << endl;
        return 0;
    }

    auto ask = [&](int64 l, int64 r) -> int64 {
        cout << "? " << l << ' ' << r << endl;
        int64 answer;
        if (!(cin >> answer) || answer < 0) exit(0);
        return answer;
    };

    int64 total = ask(1, n);

    int64 low = 1, high = n - 2;
    while (high - low > 1) {
        int64 mid = low + (high - low) / 2;
        if (ask(1, mid) == 0)
            low = mid;
        else
            high = mid;
    }

    int64 i = low;
    int64 firstLength, secondLength;
    if (total == 2) {
        firstLength = secondLength = 2;
    } else {
        int64 withoutFirst = ask(i + 1, n);
        firstLength = total - withoutFirst + 1;
        int64 remaining = total - firstLength * (firstLength - 1) / 2;
        secondLength = (1 + sqrtl(1 + 8.0L * remaining)) / 2;
        while (secondLength * (secondLength + 1) / 2 < remaining)
            ++secondLength;
        while ((secondLength - 1) * (secondLength - 2) / 2 >= remaining)
            --secondLength;
    }

    int64 j = i + firstLength;
    int64 k = j + secondLength - 1;
    cout << "! " << i << ' ' << j << ' ' << k << endl;
    return 0;
}
