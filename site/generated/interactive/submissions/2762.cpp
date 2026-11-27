#include <bits/stdc++.h>
using namespace std;

using int64 = long long;

int64 ask(int64 l, int64 r) {
    cout << "xor " << l << ' ' << r << endl;

    int64 value;
    if (!(cin >> value) || value == -1) {
        exit(0);
    }
    return value;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int64 n;
    if (!(cin >> n)) {
        return 0;
    }

    const int64 total = ask(1, n);

    int64 firstValue = 0;
    int64 firstPosition = 0;

    if (total == 0) {
        int64 power = 2;

        while (true) {
            const int64 right = power - 1;
            const int64 value = ask(1, right);

            if (value != 0) {
                firstValue = value;
                firstPosition = value;
                break;
            }

            power *= 2;
        }
    } else {
        int64 left = 1;
        int64 right = n;
        int64 valueAtRight = total;

        while (left < right) {
            const int64 middle = left + (right - left) / 2;
            const int64 value = ask(1, middle);

            if (value != 0) {
                right = middle;
                valueAtRight = value;
            } else {
                left = middle + 1;
            }
        }

        firstPosition = left;
        firstValue = valueAtRight;
    }

    int64 left = 1;
    int64 right = n;
    int64 residualAtRight = total ^ firstValue;

    while (left < right) {
        const int64 middle = left + (right - left) / 2;
        int64 residual = ask(1, middle);

        if (firstPosition <= middle) {
            residual ^= firstValue;
        }

        if (residual != 0) {
            right = middle;
            residualAtRight = residual;
        } else {
            left = middle + 1;
        }
    }

    const int64 secondValue = residualAtRight;
    const int64 thirdValue = total ^ firstValue ^ secondValue;

    cout << "ans " << firstValue << ' ' << secondValue << ' '
         << thirdValue << endl;

    return 0;
}
