#include <bits/stdc++.h>
using namespace std;

static const int MOD = 1000000007;

void applyQuestion(
    const int* d0, const int* d1, const int* d2,
    int* e0, int* e1, int* e2,
    int M, int t
) {
    int newt = t + 1;

    for (int a = 0; a <= newt; ++a) {
        int maxb = newt - a;
        int idx = a * M;

        for (int b = 0; b <= maxb; ++b, ++idx) {
            int v0 = 0, v1 = 0, v2 = 0;

            // Put 'a' at this question mark.
            if (a > 0) {
                int old = idx - M;
                v0 = d1[old] + d2[old];
                if (v0 >= MOD) v0 -= MOD;
            }

            // Put 'b' at this question mark.
            if (b > 0) {
                int old = idx - 1;
                v1 = d0[old] + d2[old];
                if (v1 >= MOD) v1 -= MOD;
            }

            // Put 'c' at this question mark.
            // c > 0 iff b < maxb, because c = newt - a - b.
            if (b < maxb) {
                v2 = d0[idx] + d1[idx];
                if (v2 >= MOD) v2 -= MOD;
            }

            e0[idx] = v0;
            e1[idx] = v1;
            e2[idx] = v2;
        }
    }
}

void applyFixedBlock(
    const int* d0, const int* d1, const int* d2,
    int* e0, int* e1, int* e2,
    int M, int t, int first, int last
) {
    for (int a = 0; a <= t; ++a) {
        int maxb = t - a;
        int idx = a * M;

        for (int b = 0; b <= maxb; ++b, ++idx) {
            int val;

            // Previous last color must differ from first fixed character.
            if (first == 0) {
                val = d1[idx] + d2[idx];
                if (val >= MOD) val -= MOD;
            } else if (first == 1) {
                val = d0[idx] + d2[idx];
                if (val >= MOD) val -= MOD;
            } else {
                val = d0[idx] + d1[idx];
                if (val >= MOD) val -= MOD;
            }

            e0[idx] = 0;
            e1[idx] = 0;
            e2[idx] = 0;

            if (last == 0) e0[idx] = val;
            else if (last == 1) e1[idx] = val;
            else e2[idx] = val;
        }
    }
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, Q;
    cin >> n >> Q;

    string s;
    cin >> s;

    int m = 0;
    for (char ch : s) {
        if (ch == '?') ++m;
    }

    int M = m + 1;
    int sz = M * M;

    vector<int> d0(sz, 0), d1(sz, 0), d2(sz, 0);
    vector<int> e0(sz, 0), e1(sz, 0), e2(sz, 0);

    int t = 0;
    bool started = false;
    bool possible = true;

    int i = 0;
    while (i < n && possible) {
        if (s[i] == '?') {
            if (!started) {
                // The first character is '?'.
                t = 1;
                d0[1 * M + 0] = 1; // last color a, counts (a,b)=(1,0)
                d1[0 * M + 1] = 1; // last color b, counts (a,b)=(0,1)
                d2[0 * M + 0] = 1; // last color c, counts (a,b)=(0,0)
                started = true;
                ++i;
            } else {
                applyQuestion(
                    d0.data(), d1.data(), d2.data(),
                    e0.data(), e1.data(), e2.data(),
                    M, t
                );

                d0.swap(e0);
                d1.swap(e1);
                d2.swap(e2);
                ++t;
                ++i;
            }
        } else {
            int j = i;
            while (j < n && s[j] != '?') ++j;

            // Check the fixed run itself.
            for (int k = i + 1; k < j; ++k) {
                if (s[k] == s[k - 1]) {
                    possible = false;
                    break;
                }
            }

            if (!possible) break;

            int first = s[i] - 'a';
            int last = s[j - 1] - 'a';

            if (!started) {
                // The string starts with a fixed run.
                t = 0;
                if (last == 0) d0[0] = 1;
                else if (last == 1) d1[0] = 1;
                else d2[0] = 1;
                started = true;
            } else {
                applyFixedBlock(
                    d0.data(), d1.data(), d2.data(),
                    e0.data(), e1.data(), e2.data(),
                    M, t, first, last
                );

                d0.swap(e0);
                d1.swap(e1);
                d2.swap(e2);
            }

            i = j;
        }
    }

    if (!possible) {
        for (int qi = 0; qi < Q; ++qi) {
            int x, y, z;
            cin >> x >> y >> z;
            cout << 0 << '\n';
        }
        return 0;
    }

    // coeff[a, b] = number of valid assignments where new 'a' count = a,
    // new 'b' count = b, and new 'c' count = m - a - b.
    vector<int> coeff(M * M, 0);

    for (int a = 0; a <= m; ++a) {
        int base = a * M;
        for (int b = 0; b <= m - a; ++b) {
            int idx = base + b;
            int val = d0[idx] + d1[idx];
            if (val >= MOD) val -= MOD;
            val += d2[idx];
            if (val >= MOD) val -= MOD;
            coeff[idx] = val;
        }
    }

    // 2D prefix sums of coeff.
    vector<int> pref(M * M, 0);

    for (int a = 0; a <= m; ++a) {
        int base = a * M;
        int prevBase = (a == 0 ? 0 : base - M);
        int rowSum = 0;

        for (int b = 0; b <= m; ++b) {
            rowSum += coeff[base + b];
            if (rowSum >= MOD) rowSum -= MOD;

            int val = rowSum;
            if (a > 0) {
                val += pref[prevBase + b];
                if (val >= MOD) val -= MOD;
            }

            pref[base + b] = val;
        }
    }

    vector<int> qx(Q), qy(Q), ans(Q, 0);
    vector<vector<int>> byL(m + 1);

    for (int qi = 0; qi < Q; ++qi) {
        int x, y, z;
        cin >> x >> y >> z;

        qx[qi] = x;
        qy[qi] = y;

        // c <= z  <=>  m - a - b <= z  <=>  a + b >= m - z.
        int L = m - z;
        if (L < 0) L = 0;

        if (L > m) {
            // This can only happen for negative z, which is not in constraints,
            // but keep it safe.
            continue;
        }

        byL[L].push_back(qi);
    }

    int B = m + 2;
    vector<int> bit(B * B, 0);

    auto bitUpdate = [&](int a, int b, int val) {
        for (int i = a + 1; i <= m + 1; i += i & -i) {
            int base = i * B;
            for (int j = b + 1; j <= m + 1; j += j & -j) {
                int &cell = bit[base + j];
                cell += val;
                if (cell >= MOD) cell -= MOD;
            }
        }
    };

    auto bitQuery = [&](int x, int y) {
        int res = 0;
        for (int i = x + 1; i > 0; i -= i & -i) {
            int base = i * B;
            for (int j = y + 1; j > 0; j -= j & -j) {
                res += bit[base + j];
                if (res >= MOD) res -= MOD;
            }
        }
        return res;
    };

    int nextD = 0;

    for (int L = 0; L <= m; ++L) {
        while (nextD < L) {
            int sum = nextD;
            int amin = max(0, sum - m);
            int amax = min(m, sum);

            for (int a = amin; a <= amax; ++a) {
                int b = sum - a;
                int val = coeff[a * M + b];
                if (val != 0) {
                    bitUpdate(a, b, val);
                }
            }

            ++nextD;
        }

        for (int qi : byL[L]) {
            int x = qx[qi];
            int y = qy[qi];

            if (x > m) x = m;
            if (y > m) y = m;

            int total = pref[x * M + y];
            int bad = bitQuery(x, y);

            int res = total - bad;
            if (res < 0) res += MOD;

            ans[qi] = res;
        }
    }

    for (int qi = 0; qi < Q; ++qi) {
        cout << ans[qi] << '\n';
    }

    return 0;
}
