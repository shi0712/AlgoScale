#include <bits/stdc++.h>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n)) return 0;

    int visible;
    if (!(cin >> visible)) return 0;
    if (visible < 0 || visible == n) return 0;

    auto flip = [&](int l, int r) -> int {
        cout << l << ' ' << r << endl;

        int response;
        if (!(cin >> response)) exit(0);
        if (response < 0) exit(0);

        visible = response;
        if (visible == n) exit(0);
        return response;
    };

    for (int c = 1; c <= n; ++c) {
        const int k = n - c + 1;
        const int pairs = 1 << (k - 1);

        for (int t = 0; t < pairs; ++t) {
            const int before = visible;
            const int after = flip(c, c);
            const int delta = after - before;

            if (delta == -1) {
                // The whole current group was already inactive.
                // Undo the temporary singleton probe.
                flip(c, c);
                break;
            }

            if (delta == 1) {
                // The singleton was the only active barrier in this group.
                // The probe has already made the entire group inactive.
                break;
            }

            if (delta != 0) exit(0);

            if (t + 1 < pairs) {
                // Consecutive representatives of the pairs differ in one
                // non-singleton interval of the current group.
                const int bit = __builtin_ctz(
                    static_cast<unsigned>(t + 1)
                );
                flip(c, c + 1 + bit);
            }
        }
    }

    return 0;
}
