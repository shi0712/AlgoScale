#include <bits/stdc++.h>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    
    int n;
    if (!(cin >> n)) return 0;
    vector<pair<int,int>> pts(n);
    int min_x = INT_MAX, max_x = INT_MIN;
    for (int i = 0; i < n; ++i) {
        int x, y;
        cin >> x >> y;
        pts[i] = {x, y};
        min_x = min(min_x, x);
        max_x = max(max_x, x);
    }
    
    if (min_x == max_x) {
        cout << "! 0 1\n" << flush;
        return 0;
    }
    
    long double total = 0.0L;
    for (int i = min_x; i < max_x; ++i) {
        long long p = 2LL * i + 1; // x = i + 0.5
        long long q = 2;
        cout << "? " << p << " " << q << "\n" << flush;
        long long r, s;
        if (!(cin >> r >> s)) return 0;
        if (r == -1) return 0; // failure response
        long double len = (long double)r / (long double)s;
        total += 2.0L * len; // accumulate 2 * length
    }
    
    long long area2 = llround(total); // 2 * area, should be integer
    if (area2 < 0) area2 = 0;
    
    long long u, v;
    if (area2 % 2 == 0) {
        u = area2 / 2;
        v = 1;
    } else {
        u = area2;
        v = 2;
    }
    
    cout << "! " << u << " " << v << "\n" << flush;
    return 0;
}
