#include <bits/stdc++.h>
using namespace std;
using ull = unsigned long long;

ull n;
vector<ull> ans;
const int BRUTE_LIMIT = 100;

bool query(ull l, ull r, ull &res) {
    cout << "xor " << l << ' ' << r << '\n';
    cout.flush();
    long long x;
    if (!(cin >> x)) return false;
    if (x < 0) return false;
    res = (ull)x;
    return true;
}

bool tryBrute(ull L, ull R, int c, ull X) {
    ull vl = max(L, 1ULL);
    ull vr = min(R, n);
    if (vl > vr) return false;
    ull mull = vr - vl + 1;
    if (mull > BRUTE_LIMIT) return false;
    int m = (int)mull;
    vector<ull> v(m);
    for (int i = 0; i < m; ++i) v[i] = vl + (ull)i;
    int found = 0;
    vector<ull> res;
    if (c == 2) {
        for (int i = 0; i < m && found < 2; ++i) {
            for (int j = i + 1; j < m && found < 2; ++j) {
                if ((v[i] ^ v[j]) == X) {
                    ++found;
                    if (found == 1) res = {v[i], v[j]};
                }
            }
        }
    } else if (c == 3) {
        for (int i = 0; i < m && found < 2; ++i) {
            for (int j = i + 1; j < m && found < 2; ++j) {
                for (int k = j + 1; k < m && found < 2; ++k) {
                    if ((v[i] ^ v[j] ^ v[k]) == X) {
                        ++found;
                        if (found == 1) res = {v[i], v[j], v[k]};
                    }
                }
            }
        }
    }
    if (found == 1) {
        ans.insert(ans.end(), res.begin(), res.end());
        return true;
    }
    return false;
}

void solve(ull L, ull R, int c, ull X) {
    if (c == 0) return;
    if (c == 1) {
        ans.push_back(X);
        return;
    }
    ull vl = max(L, 1ULL);
    ull vr = min(R, n);
    if (vl <= vr) {
        ull mull = vr - vl + 1;
        if (mull == (ull)c) {
            for (ull x = vl; x <= vr; ++x) ans.push_back(x);
            return;
        }
        if (mull <= BRUTE_LIMIT) {
            if (tryBrute(L, R, c, X)) return;
        }
    }
    if (L == R) return;
    ull mid = (L + R) >> 1;
    ull Lr = mid + 1;
    ull Y;
    if (Lr > n) {
        Y = 0;
    } else {
        ull rq = min(R, n);
        if (!query(Lr, rq, Y)) exit(0);
    }
    int cR;
    if (Y == 0) {
        cR = 0;
    } else if (Y < Lr || Y > R) {
        cR = 2;
    } else {
        if (c == 3 && Y == X) cR = 3;
        else cR = 1;
    }
    if (cR > c) cR = c; // safety
    int cL = c - cR;
    ull XL = X ^ Y;
    solve(L, mid, cL, XL);
    solve(Lr, R, cR, Y);
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    if (!(cin >> n)) return 0;
    if (n == 3) {
        cout << "ans 1 2 3\n";
        cout.flush();
        return 0;
    }
    ull S;
    if (!query(1, n, S)) return 0;
    int B = 0;
    ull M = 1;
    while (M <= n) {
        M <<= 1;
        ++B;
    }
    ull R = M - 1;
    solve(0, R, 3, S);
    sort(ans.begin(), ans.end());
    // In case of any unforeseen issue, ensure three values are printed.
    while (ans.size() < 3) ans.push_back(0);
    cout << "ans " << ans[0] << ' ' << ans[1] << ' ' << ans[2] << '\n';
    cout.flush();
    return 0;
}
