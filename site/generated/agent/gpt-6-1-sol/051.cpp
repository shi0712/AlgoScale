#include <bits/stdc++.h>
using namespace std;

constexpr int MOD = 1000000007;
inline int addmod(int a, int b) {
    int s = a + b;
    return s >= MOD ? s - MOD : s;
}

struct State {
    int a = 0, b = 0, c = 0;
};

struct SuffixTable {
    int rows, cols, stride;
    vector<int> data;
    SuffixTable(int r, int c) : rows(r), cols(c), stride(c + 1),
                              data((r + 1) * (c + 1)) {}
    int& at(int a, int b) { return data[a * stride + b]; }
    int get(int a, int b) const {
        if (a >= rows || b >= cols) return 0;
        return data[a * stride + b];
    }
    void build() {
        for (int a = rows - 1; a >= 0; --a) {
            int rowSum = 0;
            for (int b = cols - 1; b >= 0; --b) {
                rowSum = addmod(rowSum, at(a, b));
                at(a, b) = addmod(rowSum, at(a + 1, b));
            }
        }
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, Q;
    cin >> n >> Q;
    string s;
    cin >> s;
    int fixedCount[3] = {}, m = 0;
    bool impossible = false;
    for (int i = 0; i < n; ++i) {
        if (s[i] == '?') ++m;
        else ++fixedCount[s[i] - 'a'];
        if (i && s[i] != '?' && s[i] == s[i - 1]) impossible = true;
    }
    int cap[3];
    for (int k = 0; k < 3; ++k) {
        cap[k] = min(m, (n + 1) / 2 - fixedCount[k]);
        if (cap[k] < 0) impossible = true;
    }
    if (impossible) {
        while (Q--) {
            int x, y, z;
            cin >> x >> y >> z;
            cout << 0 << '\n';
        }
        return 0;
    }

    // Padding at a = -1 and b = -1 makes all predecessor reads valid.
    const int stride = cap[1] + 2;
    const int cells = (cap[0] + 2) * stride;
    vector<State> dp(cells), next(cells);
    int prefixFixed[3] = {}, used = 0;
    for (int pos = 0; pos < n; ++pos) {
        if (s[pos] == '?') ++used;
        else ++prefixFixed[s[pos] - 'a'];
        int bound[3];
        for (int k = 0; k < 3; ++k)
            bound[k] = min(cap[k], (pos + 2) / 2 - prefixFixed[k]);

        fill(next.begin(), next.end(), State{});
        if (pos == 0) {
            for (int k = 0; k < 3; ++k) {
                if (s[pos] != '?' && s[pos] != 'a' + k) continue;
                int a = s[pos] == '?' && k == 0;
                int b = s[pos] == '?' && k == 1;
                int c = s[pos] == '?' && k == 2;
                if (a > bound[0] || b > bound[1] || c > bound[2]) continue;
                State& t = next[(a + 1) * stride + b + 1];
                if (k == 0) t.a = 1;
                if (k == 1) t.b = 1;
                if (k == 2) t.c = 1;
            }
        } else {
            for (int a = 0; a <= min(bound[0], used); ++a) {
                int lo = max(0, used - a - bound[2]);
                int hi = min(bound[1], used - a);
                int idx = (a + 1) * stride + lo + 1;
                if (s[pos] == '?') {
                    for (int b = lo; b <= hi; ++b, ++idx) {
                        const State& pa = dp[idx - stride];
                        const State& pb = dp[idx - 1];
                        const State& pc = dp[idx];
                        next[idx] = {addmod(pa.b, pa.c), addmod(pb.a, pb.c),
                                     addmod(pc.a, pc.b)};
                    }
                } else if (s[pos] == 'a') {
                    for (int b = lo; b <= hi; ++b, ++idx)
                        next[idx].a = addmod(dp[idx].b, dp[idx].c);
                } else if (s[pos] == 'b') {
                    for (int b = lo; b <= hi; ++b, ++idx)
                        next[idx].b = addmod(dp[idx].a, dp[idx].c);
                } else {
                    for (int b = lo; b <= hi; ++b, ++idx)
                        next[idx].c = addmod(dp[idx].a, dp[idx].b);
                }
            }
        }
        dp.swap(next);
    }

    SuffixTable ab(cap[0] + 1, cap[1] + 1);
    SuffixTable ac(cap[0] + 1, cap[2] + 1);
    SuffixTable bc(cap[1] + 1, cap[2] + 1);
    for (int a = 0; a <= cap[0]; ++a) {
        for (int b = max(0, m - a - cap[2]); b <= min(cap[1], m - a); ++b) {
            int c = m - a - b;
            const State& t = dp[(a + 1) * stride + b + 1];
            int ways = addmod(addmod(t.a, t.b), t.c);
            ab.at(a, b) = ways;
            ac.at(a, c) = ways;
            bc.at(b, c) = ways;
        }
    }
    ab.build();
    ac.build();
    bc.build();
    int total = ab.get(0, 0);
    while (Q--) {
        int x, y, z;
        cin >> x >> y >> z;
        // The intersection of all three violations is empty because x+y+z >= m.
        long long ans = total + 3LL * MOD
                      - ab.get(x + 1, 0) - ab.get(0, y + 1) - ac.get(0, z + 1)
                      + ab.get(x + 1, y + 1) + ac.get(x + 1, z + 1)
                      + bc.get(y + 1, z + 1);
        cout << ans % MOD << '\n';
    }
    return 0;
}
