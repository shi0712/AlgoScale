#include <bits/stdc++.h>
using namespace std;

int n;

int ask(int l, int r, int k) {
    cout << "? " << l << " " << r << " " << k << '\n';
    cout.flush();
    int x;
    cin >> x;
    if (x == -1) exit(0);
    return x;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    cin >> n;

    if (n % 2 == 1) {
        int lo = 1, hi = n;

        while (lo < hi) {
            int mid = (lo + hi) / 2;

            int d1 = ask(1, mid, 2);
            int d2 = ask(mid + 1, n, 2);

            long long u1 = 2LL * d1 - mid;
            long long u2 = 2LL * d2 - (n - mid);

            if (u1 > u2) hi = mid;
            else lo = mid + 1;
        }

        cout << "! " << lo << '\n';
        cout.flush();
        return 0;
    }

    int L = 1, R = n;
    bool nInS = true;
    bool nLeft = false;
    int ans = -1;

    while (L < R) {
        if (nInS && R - L + 1 == 2) {
            if (L > 1) {
                int d = ask(L - 1, L, n);
                if (d == 2) ans = R;
                else ans = L;
            } else {
                int d = ask(R, R + 1, n);
                if (d == 2) ans = L;
                else ans = R;
            }
            break;
        }

        int mid = (L + R) / 2;

        int d1 = ask(1, mid, 2);
        int d2 = ask(mid + 1, n, 2);

        long long u1 = 2LL * d1 - mid;
        long long u2 = 2LL * d2 - (n - mid);

        int cnt = int((u1 - u2 + 2) / 2);

        if (nInS) {
            if (cnt == 0) {
                L = mid + 1;
            } else if (cnt == 2) {
                R = mid;
            } else {
                int lenA = mid - L + 1;
                int lenB = R - mid;
                bool nInA;

                if (lenA >= 2) {
                    int d = ask(L, mid, n);
                    nInA = (d == 2);
                } else {
                    int d = ask(mid + 1, R, n);
                    nInA = (d != 2);
                }

                if (nInA) {
                    L = mid + 1;
                    nInS = false;
                    nLeft = true;
                } else {
                    R = mid;
                    nInS = false;
                    nLeft = false;
                }
            }
        } else {
            int oneInA = cnt - (nLeft ? 1 : 0);

            if (oneInA == 1) {
                R = mid;
            } else {
                L = mid + 1;
            }
        }
    }

    if (ans == -1) ans = L;

    cout << "! " << ans << '\n';
    cout.flush();

    return 0;
}
