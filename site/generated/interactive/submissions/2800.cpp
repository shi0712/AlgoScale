#include <bits/stdc++.h>
using namespace std;

int n;

int ask(int l, int r, int k) {
    cout << "? " << l << ' ' << r << ' ' << k << endl;

    int response;
    if (!(cin >> response) || response == -1) {
        exit(0);
    }
    return response;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    if (!(cin >> n)) {
        return 0;
    }

    const int globalDistinct = n / 2 + 1;
    int positionOfN = -1;

    if (n % 2 == 0) {
        int response = ask(2, n, n);

        if (response == 1) {
            positionOfN = 1;
        } else {
            int low = 2, high = n;

            while (low < high) {
                int mid = (low + high) / 2;
                int value = ask(1, mid, n);

                if (value == 2) {
                    high = mid;
                } else {
                    low = mid + 1;
                }
            }

            positionOfN = low;
        }
    }

    int low = 1, high = n;

    while (low < high) {
        int mid = (low + high) / 2;

        int leftDistinct = ask(1, mid, 2);
        int rightDistinct = ask(mid + 1, n, 2);

        int splitClasses = leftDistinct + rightDistinct - globalDistinct;
        int singletonClasses = 2 * leftDistinct - mid - splitClasses;

        if (n % 2 == 0 && positionOfN <= mid) {
            --singletonClasses;
        }

        if (singletonClasses > 0) {
            high = mid;
        } else {
            low = mid + 1;
        }
    }

    cout << "! " << low << endl;
    return 0;
}
