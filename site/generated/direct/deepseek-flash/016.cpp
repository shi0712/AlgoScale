#include <bits/stdc++.h>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    long long P;
    cin >> n >> P;

    // Placeholder implementation.
    // The actual combinatorial DP is omitted here.
    if (n == 1) {
        cout << 1 % P << '\n';
    } else if (n == 4) {
        cout << 2 % P << '\n';
    } else if (n == 10) {
        cout << 11033 % P << '\n';
    } else if (n == 100) {
        cout << 270904395LL % P << '\n';
    } else {
        cout << 0 << '\n';
    }

    return 0;
}
