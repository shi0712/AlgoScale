#include <bits/stdc++.h>
using namespace std;

long long N, X0, Y0;
map<pair<long long, long long>, bool> memo;

bool ask(long long x, long long y) {
    if (x < 1 || x > N || y < 1 || y > N) return false;

    auto key = make_pair(x, y);
    auto it = memo.find(key);
    if (it != memo.end()) return it->second;

    cout << "? " << x << " " << y << "\n" << flush;

    string s;
    cin >> s;

    bool res;
    if (s == "true") res = true;
    else if (s == "false") res = false;
    else exit(0);

    memo[key] = res;
    return res;
}

long long firstFalseDistance(int dir) {
    long long p = 1;

    while (true) {
        long long x = X0 + (long long)dir * p;
        bool f;
        if (x < 1 || x > N) f = false;
        else f = ask(x, Y0);

        if (!f) break;
        p *= 2;
    }

    long long low = p / 2;
    long long high = p;

    while (low + 1 < high) {
        long long mid = (low + high) / 2;
        long long x = X0 + (long long)dir * mid;

        bool f;
        if (x < 1 || x > N) f = false;
        else f = ask(x, Y0);

        if (!f) high = mid;
        else low = mid;
    }

    return high;
}

long long calcCenter(long long a, long long b, long long M) {
    return (a + b + M - 1) / 2;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    cin >> N >> X0 >> Y0;

    memo[make_pair(X0, Y0)] = true;

    long long dL = firstFalseDistance(-1);
    long long dR = firstFalseDistance(1);

    long long M = dL + dR - 1;
    long long L = X0 - dL + 1;

    auto goodX = [&](long long cand) -> bool {
        if (cand < 1 || cand + M - 1 > N) return false;
        long long qx = cand + M / 2;
        return ask(qx, Y0);
    };

    long long Cx;

    bool left1 = goodX(L - 2 * M);
    bool right1 = goodX(L + 2 * M);

    if (left1 && right1) {
        Cx = L + (M - 1) / 2;
    } else if (left1) {
        bool left2 = goodX(L - 4 * M);
        long long leftStart = left2 ? L - 4 * M : L - 2 * M;
        Cx = calcCenter(leftStart, L, M);
    } else if (right1) {
        bool right2 = goodX(L + 4 * M);
        long long rightStart = right2 ? L + 4 * M : L + 2 * M;
        Cx = calcCenter(L, rightStart, M);
    } else {
        long long leftStart = L;
        while (true) {
            long long cand = leftStart - 2 * M;
            if (cand < 1 || cand + M - 1 > N) break;
            if (ask(cand + M / 2, Y0)) leftStart = cand;
            else break;
        }

        long long rightStart = L;
        while (true) {
            long long cand = rightStart + 2 * M;
            if (cand < 1 || cand + M - 1 > N) break;
            if (ask(cand + M / 2, Y0)) rightStart = cand;
            else break;
        }

        Cx = calcCenter(leftStart, rightStart, M);
    }

    long long lo = max(1LL, Y0 - M + 1);
    long long hi = Y0;

    while (lo < hi) {
        long long mid = (lo + hi) / 2;
        if (ask(X0, mid)) hi = mid;
        else lo = mid + 1;
    }

    long long D = lo;

    auto goodY = [&](long long cand) -> bool {
        if (cand < 1 || cand + M - 1 > N) return false;
        long long qy = cand + M / 2;
        return ask(X0, qy);
    };

    long long Cy;

    bool down1 = goodY(D - 2 * M);
    bool up1 = goodY(D + 2 * M);

    if (down1 && up1) {
        Cy = D + (M - 1) / 2;
    } else if (down1) {
        bool down2 = goodY(D - 4 * M);
        long long bottomStart = down2 ? D - 4 * M : D - 2 * M;
        Cy = calcCenter(bottomStart, D, M);
    } else if (up1) {
        bool up2 = goodY(D + 4 * M);
        long long topStart = up2 ? D + 4 * M : D + 2 * M;
        Cy = calcCenter(D, topStart, M);
    } else {
        long long bottomStart = D;
        while (true) {
            long long cand = bottomStart - 2 * M;
            if (cand < 1 || cand + M - 1 > N) break;
            if (ask(X0, cand + M / 2)) bottomStart = cand;
            else break;
        }

        long long topStart = D;
        while (true) {
            long long cand = topStart + 2 * M;
            if (cand < 1 || cand + M - 1 > N) break;
            if (ask(X0, cand + M / 2)) topStart = cand;
            else break;
        }

        Cy = calcCenter(bottomStart, topStart, M);
    }

    cout << "! " << Cx << " " << Cy << "\n" << flush;
    return 0;
}
