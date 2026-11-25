#include <bits/stdc++.h>
using namespace std;

bool query(long long x, long long y) {
    cout << "? " << x << " " << y << endl;
    string s;
    if (!(cin >> s)) exit(0);
    if (s == "true") return true;
    if (s == "false") return false;
    exit(0);
}

long long find_left(long long x0, long long y, long long N) {
    long long step = 1;
    long long last_true = x0;
    while (last_true - step >= 1) {
        if (!query(last_true - step, y)) {
            long long lo = last_true - step + 1;
            long long hi = last_true;
            while (lo < hi) {
                long long mid = (lo + hi) / 2;
                if (query(mid, y)) hi = mid;
                else lo = mid + 1;
            }
            return lo;
        }
        last_true -= step;
        step <<= 1;
        if (step > x0) break;
    }
    return 1;
}

long long find_right(long long x0, long long y, long long N) {
    long long step = 1;
    long long last_true = x0;
    while (last_true + step <= N) {
        if (!query(last_true + step, y)) {
            long long lo = last_true;
            long long hi = last_true + step - 1;
            while (lo < hi) {
                long long mid = (lo + hi + 1) / 2;
                if (query(mid, y)) lo = mid;
                else hi = mid - 1;
            }
            return lo;
        }
        last_true += step;
        step <<= 1;
        if (step > N) break;
    }
    return N;
}

long long find_bottom(long long x, long long y0, long long N) {
    long long step = 1;
    long long last_true = y0;
    while (last_true - step >= 1) {
        if (!query(x, last_true - step)) {
            long long lo = last_true - step + 1;
            long long hi = last_true;
            while (lo < hi) {
                long long mid = (lo + hi) / 2;
                if (query(x, mid)) hi = mid;
                else lo = mid + 1;
            }
            return lo;
        }
        last_true -= step;
        step <<= 1;
        if (step > y0) break;
    }
    return 1;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    long long N, X0, Y0;
    if (!(cin >> N >> X0 >> Y0)) return 0;

    long long L = find_left(X0, Y0, N);
    long long R = find_right(X0, Y0, N);
    long long M = R - L + 1;

    long long A = L;
    while (A - 2 * M >= 1 && query(A - 2 * M, Y0)) A -= 2 * M;

    long long B = find_bottom(X0, Y0, N);

    long long Btm = B;
    while (Btm - 2 * M >= 1 && query(X0, Btm - 2 * M)) Btm -= 2 * M;

    bool even;
    if (A - M >= 1 && Btm - M >= 1) {
        even = query(A - M, Btm - M);
    } else {
        even = false;
    }

    long long a, b;
    if (even) {
        a = A - M;
        b = Btm - M;
    } else {
        a = A;
        b = Btm;
    }

    long long offset = (5 * M - 1) / 2;
    long long cx = a + offset;
    long long cy = b + offset;

    cout << "! " << cx << " " << cy << endl;
    return 0;
}
