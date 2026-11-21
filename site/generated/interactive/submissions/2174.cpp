#include <bits/stdc++.h>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n)) return 0;

    auto ask = [&](int l, int r, int k) -> int {
        cout << "? " << l << ' ' << r << ' ' << k << '\n';
        cout.flush();
        int res;
        if (!(cin >> res)) exit(0);
        if (res < 0) exit(0);
        return res;
    };

    int C = n / 2 + 1;
    int lo = 1, hi = n;
    int zlo = 1, zhi = n; // for even n: possible position of value n

    while (lo < hi) {
        int mid = (lo + hi) / 2;
        int qA = ask(1, mid, 2);
        int qB = ask(mid + 1, n, 2);
        int a = qA - qB + C - mid;

        int s;
        if (n % 2 == 1) {
            s = (a == 1 ? 1 : 0);
        } else {
            if (a == 0) {
                s = 0;
                zlo = max(zlo, mid + 1);
            } else if (a == 2) {
                s = 1;
                zhi = min(zhi, mid);
            } else { // a == 1
                if (zhi <= mid) {
                    s = 0;
                } else if (zlo > mid) {
                    s = 1;
                } else {
                    bool z_le;
                    if (mid == 1) {
                        int res = ask(2, n, n);
                        z_le = (res == 1);
                    } else {
                        int res = ask(1, mid, n);
                        z_le = (res == 2);
                    }
                    if (z_le) {
                        zhi = min(zhi, mid);
                        s = 0;
                    } else {
                        zlo = max(zlo, mid + 1);
                        s = 1;
                    }
                }
            }
        }

        if (s == 1) hi = mid;
        else lo = mid + 1;
    }

    cout << "! " << lo << '\n';
    cout.flush();
    return 0;
}
