#include <bits/stdc++.h>
using namespace std;

int n, k;

static bool rd(int &x) {
    if (!(cin >> x)) exit(0);
    return true;
}

int main() {
    mt19937 rng(712367);
    rd(n);
    rd(k);
    while (k > 1) {
        bool progress = false;
        for (int i = 1; i <= n && k > 1; i++) {
            int t = 0;
            while (true) {
                cout << i << " " << 1 << endl;
                int kk;
                rd(kk);
                t++;
                if (kk == 1) return 0;
                if (kk < k) {
                    k = kk;
                    progress = true;
                    break;
                }
                if (kk > k) {
                    cout << i << " " << -t << endl;
                    int k2;
                    rd(k2);
                    if (k2 == 1) return 0;
                    k = k2;
                    break;
                }
                if (t >= n) break;
            }
        }
        if (k > 1 && !progress) {
            int i = rng() % n + 1;
            int t = 0;
            while (true) {
                cout << i << " " << 1 << endl;
                int kk;
                rd(kk);
                t++;
                if (kk == 1) return 0;
                if (kk < k) {
                    k = kk;
                    break;
                }
                if (kk == k) break;
                if (t >= n) {
                    break;
                }
            }
        }
    }
    return 0;
}
