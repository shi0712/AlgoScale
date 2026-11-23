#include <bits/stdc++.h>
using namespace std;

int n;

int ask(int l, int r, int k) {
    cout << "? " << l << ' ' << r << ' ' << k << endl;
    int response;
    if (!(cin >> response) || response == -1) exit(0);
    return response;
}

// Whether the value n is in positions [1, mid]. n is even here.
bool nInPrefix(int mid) {
    if (mid >= 2) return ask(1, mid, n) == 2;
    // A singleton containing n would also return 1, so query its complement.
    return ask(2, n, n) == 1;
}

int main() {
    if (!(cin >> n)) return 0;

    const int fullDistinct = n / 2 + 1;
    int l = 1, r = n;

    // Used only for even n: the position of n is in [nl, nr].
    int nl = 1, nr = n;

    while (l < r) {
        // Both special values 1 and n are known to occupy these two positions.
        if (n % 2 == 0 && r - l == 1 && l == nl && r == nr) {
            bool nAtL = nInPrefix(l);
            cout << "! " << (nAtL ? r : l) << endl;
            return 0;
        }

        int mid = (l + r) / 2;

        int left = (mid == 1 ? 1 : ask(1, mid, 2));
        int right = (mid + 1 == n ? 1 : ask(mid + 1, n, 2));

        // For k=2, values (2,3), (4,5), ... form pairs. Value 1 is unpaired,
        // and value n is also unpaired when n is even. This expression is the
        // number of unpaired values present in the prefix [1,mid].
        int unpaired = left - right + fullDistinct - mid;

        bool oneInPrefix;
        if (n % 2 == 1) {
            oneInPrefix = (unpaired == 1);
        } else {
            bool nPrefix;
            if (unpaired == 0) {
                nPrefix = false;
                oneInPrefix = false;
            } else if (unpaired == 2) {
                nPrefix = true;
                oneInPrefix = true;
            } else {
                if (nr <= mid) nPrefix = true;
                else if (nl > mid) nPrefix = false;
                else nPrefix = nInPrefix(mid);
                oneInPrefix = !nPrefix;
            }

            if (nPrefix) nr = min(nr, mid);
            else nl = max(nl, mid + 1);
        }

        if (oneInPrefix) r = mid;
        else l = mid + 1;
    }

    cout << "! " << l << endl;
    return 0;
}
