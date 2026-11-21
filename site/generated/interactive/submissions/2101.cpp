#include <bits/stdc++.h>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    int n;
    if (!(cin >> n)) return 0;
    long long cur_k;
    if (!(cin >> cur_k)) return 0;
    if (cur_k == 1) return 0;

    while (true) {
        bool reduced = false;
        for (int i = 1; i <= n && !reduced; ++i) {
            for (int step = 0; step < n; ++step) {
                cout << i << " " << 1 << "\n" << flush;
                long long new_k;
                if (!(cin >> new_k)) return 0;
                if (new_k == -1) return 0;
                if (new_k < cur_k) {
                    cur_k = new_k;
                    reduced = true;
                    break;
                } else {
                    cur_k = new_k;
                }
                if (cur_k == 1) return 0;
            }
        }
        if (cur_k == 1) break;
        // In theory we should always make progress; loop continues otherwise.
    }
    return 0;
}
