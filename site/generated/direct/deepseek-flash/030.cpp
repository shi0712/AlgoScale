#include <bits/stdc++.h>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    long long p;
    cin >> n >> p;

    int M = n * (n + 1) / 2;

    // This is a placeholder code.
    // The actual solution is nontrivial and was not completed.
    for (int i = 1; i <= n; ++i) {
        for (int j = 1; j <= M; ++j) {
            if (j > 1) cout << ' ';
            cout << 0;
        }
        cout << '\n';
    }

    return 0;
}
