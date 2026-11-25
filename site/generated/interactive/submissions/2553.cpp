#include <bits/stdc++.h>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int sx, sy;
    if (!(cin >> sx >> sy)) return 0;
    int x = sx, y = sy;

    while (true) {
        int mx, my;
        // Simple adaptive marking strategy.
        if (x > y) {
            mx = x + 1;
            my = y;
        } else if (y > x) {
            mx = x;
            my = y + 1;
        } else {
            mx = x + 1;
            my = y + 1;
        }

        // Keep within allowed marking range.
        if (mx > 1000) mx = 1000;
        if (my > 1000) my = 1000;
        if (mx < 1) mx = 1;
        if (my < 1) my = 1;

        cout << mx << ' ' << my << '\n';
        cout.flush();

        int nx, ny;
        if (!(cin >> nx >> ny)) return 0;
        if (nx == 0 && ny == 0) return 0;
        x = nx;
        y = ny;
    }

    return 0;
}
