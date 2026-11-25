#include <bits/stdc++.h>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    cin >> n;

    // Find a nonzero pivot p.
    int p = 1;
    int a[4] = {1, 2, 3, 4};
    for (int i = 0; i < 4; ++i) {
        int x = a[i], y = a[(i + 1) % 4], z = a[(i + 2) % 4];
        cout << "? " << x << " " << y << " " << z << endl;
        int r;
        cin >> r;
        if (r == 0) {
            p = x;
            break;
        }
    }

    // Use p as a pivot and find the index giving the largest range.
    int bestIdx = p;
    int bestVal = -1;
    for (int i = 1; i <= n; ++i) {
        if (i == p) continue;
        int j = (i % n) + 1;
        if (j == p) j = (j % n) + 1;
        cout << "? " << p << " " << i << " " << j << endl;
        int r;
        cin >> r;
        if (r > bestVal) {
            bestVal = r;
            bestIdx = i;
        }
    }

    cout << "! " << bestIdx << " " << p << endl;
    return 0;
}
