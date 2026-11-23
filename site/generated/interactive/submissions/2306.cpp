#include <bits/stdc++.h>
using namespace std;

int ask(int i, int j) {
    cout << "? " << i << " " << j << "\n" << flush;
    int g;
    if (!(cin >> g)) exit(0);
    if (g == -1) exit(0);
    return g;
}

int main() {
    int n;
    if (!(cin >> n)) return 0;
    int x = 1, y = 2;
    for (int z = 3; z <= n; z++) {
        int g1 = ask(x, z);
        int g2 = ask(y, z);
        if (g1 == g2) continue;
        if (g1 > g2) y = z;
        else x = z;
    }
    cout << "! " << x << " " << y << "\n" << flush;
    int r;
    cin >> r;
    return 0;
}
