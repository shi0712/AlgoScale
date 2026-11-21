#include <bits/stdc++.h>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    long long n;
    if (!(cin >> n)) return 0;

    long long max_val = 0;
    for (long long i = 1; i <= n; ++i) {
        cout << "? " << i << " " << i << "\n" << flush;
        long long resp;
        if (!(cin >> resp)) return 0;
        if (resp == -1) return 0;
        max_val = max(max_val, resp);
    }

    cout << "! " << max_val << "\n" << flush;
    return 0;
}
