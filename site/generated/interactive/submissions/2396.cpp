#include <bits/stdc++.h>
using namespace std;
typedef long long ll;

int main() {
    ll lo = 0, hi = 100000000000000LL, B = 1;
    // first query 1 (B=1, lo=0): general loop handles it
    while (lo < hi) {
        ll mid = (lo + hi + 1) / 2;
        ll X;
        bool safe = false;
        if (B >= mid) {
            X = mid;
        } else if (B > lo) {
            X = B;
        } else {
            safe = true;
            X = lo;
        }
        if (safe && X == 0) {
            // cannot happen since B>=1 and lo==0 gives B>lo
            break;
        }
        printf("? %lld\n", X);
        fflush(stdout);
        string s;
        if (!(cin >> s)) return 0;
        if (s == "Fired!") return 0;
        if (s == "Lucky!") {
            B += X;
            if (!safe) lo = max(lo, X);
        } else {
            B -= X;
            hi = X - 1;
        }
    }
    printf("! %lld\n", lo);
    fflush(stdout);
    return 0;
}
