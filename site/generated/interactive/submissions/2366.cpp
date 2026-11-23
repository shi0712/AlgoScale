#include <bits/stdc++.h>
using namespace std;

int main() {
    long long cnt;
    if (!(cin >> cnt)) return 0;
    long long n = 0;
    cout << "- 1" << endl;
    long long p;
    if (!(cin >> p)) return 0;
    if (p == -1) return 0;
    long long t = p - cnt + 1;
    n += (1LL << t);
    long long remaining = cnt - 1;
    while (remaining > 0) {
        cout << "- " << (1LL << t) << endl;
        if (!(cin >> p)) return 0;
        if (p == -1) return 0;
        t = p - remaining + 1;
        n += (1LL << t);
        remaining--;
    }
    cout << "! " << n << endl;
    return 0;
}
