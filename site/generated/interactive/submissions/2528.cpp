#include <bits/stdc++.h>
using namespace std;

long long ask(long long l, long long r) {
    cout << "xor " << l << " " << r << endl;
    long long res;
    if (!(cin >> res)) exit(0);
    if (res == -1) exit(0);
    return res;
}

long long first_nonzero_monotone(long long n) {
    long long lo = 1, hi = n, ans = n;
    while (lo <= hi) {
        long long mid = (lo + hi) / 2;
        long long val = ask(1, mid);
        if (val != 0) {
            ans = mid;
            hi = mid - 1;
        } else {
            lo = mid + 1;
        }
    }
    return ans;
}

long long first_nonzero_in_prefix(long long r) {
    long long lo = 1, hi = r, ans = r;
    while (lo <= hi) {
        long long mid = (lo + hi) / 2;
        long long val = ask(1, mid);
        if (val != 0) {
            ans = mid;
            hi = mid - 1;
        } else {
            lo = mid + 1;
        }
    }
    return ans;
}

long long find_nonzero(long long L, long long R) {
    if (L > R) return -1;
    if (L == R) {
        long long val = ask(L, L);
        return val != 0 ? L : -1;
    }
    long long mid = (L + R) / 2;
    long long val = ask(L, mid);
    if (val != 0) return mid;
    long long valR = ask(mid + 1, R);
    if (valR != 0) return mid + 1;
    long long res = find_nonzero(L, mid);
    if (res != -1) return res;
    return find_nonzero(mid + 1, R);
}

pair<long long, long long> solve2(long long L, long long R, long long Y) {
    if (R - L + 1 == 2) return {L, R};
    long long mid = (L + R) / 2;
    long long Z = ask(L, mid);
    if (Z == 0) {
        return solve2(mid + 1, R, Y);
    } else if (Z == Y) {
        return solve2(L, mid, Y);
    } else {
        long long x = Z;
        long long y = Y ^ Z;
        if (x > y) swap(x, y);
        return {x, y};
    }
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    long long n;
    cin >> n;

    long long X = ask(1, n);
    long long a;

    if (X != 0) {
        a = first_nonzero_monotone(n);
    } else {
        long long r = find_nonzero(1, n);
        a = first_nonzero_in_prefix(r);
    }

    long long Y = X ^ a;
    auto [b, c] = solve2(a + 1, n, Y);

    cout << "ans " << a << " " << b << " " << c << endl;
    return 0;
}
