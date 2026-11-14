#include <bits/stdc++.h>
using namespace std;

static constexpr int MOD = 1'000'000'007;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, q;
    string s;
    cin >> n >> q >> s;

    int m = count(s.begin(), s.end(), '?');
    vector<int> offset(m + 2);
    for (int a = 0; a <= m; ++a) {
        offset[a + 1] = offset[a] + (m - a + 1);
    }
    int size = offset[m + 1];

    vector<int> cur(4 * size), nxt(4 * size);
    cur[3 * size] = 1;
    int inserted = 0;

    auto add = [](int a, int b) {
        int sum = a + b;
        return sum >= MOD ? sum - MOD : sum;
    };

    for (char fixed : s) {
        fill(nxt.begin(), nxt.end(), 0);
        if (fixed == '?') {
            for (int a = 0; a <= inserted; ++a) {
                for (int b = 0; b <= inserted - a; ++b) {
                    int i = offset[a] + b;
                    int va = cur[i];
                    int vb = cur[size + i];
                    int vc = cur[2 * size + i];
                    int initial = cur[3 * size + i];
                    nxt[offset[a + 1] + b] = add(add(vb, vc), initial);
                    nxt[size + i + 1] = add(add(va, vc), initial);
                    nxt[2 * size + i] = add(add(va, vb), initial);
                }
            }
            ++inserted;
        } else {
            int c = fixed - 'a';
            int other1 = (c + 1) % 3;
            int other2 = (c + 2) % 3;
            for (int a = 0; a <= inserted; ++a) {
                for (int b = 0; b <= inserted - a; ++b) {
                    int i = offset[a] + b;
                    nxt[c * size + i] = add(add(cur[other1 * size + i], cur[other2 * size + i]), cur[3 * size + i]);
                }
            }
        }
        cur.swap(nxt);
    }

    vector<int> exact(size);
    for (int i = 0; i < size; ++i) {
        exact[i] = add(add(cur[i], cur[size + i]), cur[2 * size + i]);
    }
    int total = 0;
    for (int ways : exact) total = add(total, ways);

    vector<array<int, 3>> marginal(m + 1);
    for (int a = 0; a <= m; ++a) {
        for (int b = 0; b <= m - a; ++b) {
            int ways = exact[offset[a] + b];
            int c = m - a - b;
            marginal[a][0] = add(marginal[a][0], ways);
            marginal[b][1] = add(marginal[b][1], ways);
            marginal[c][2] = add(marginal[c][2], ways);
        }
    }
    for (int c = 0; c < 3; ++c) {
        for (int k = 1; k <= m; ++k) {
            marginal[k][c] = add(marginal[k][c], marginal[k - 1][c]);
        }
    }

    int width = m + 1;
    vector<vector<int>> rect(3, vector<int>(width * width));
    for (int pair = 0; pair < 3; ++pair) {
        for (int a = 0; a <= m; ++a) {
            for (int b = 0; b <= m; ++b) {
                int i = a * width + b;
                int value = 0;
                if (a + b <= m) {
                    if (pair == 0) value = exact[offset[a] + b];
                    else if (pair == 1) value = exact[offset[a] + (m - a - b)];
                    else value = exact[offset[m - a - b] + a];
                }

                value = add(value, b > 0 ? rect[pair][i - 1] : 0);
                value = add(value, a > 0 ? rect[pair][i - width] : 0);
                if (a > 0 && b > 0) {
                    value -= rect[pair][i - width - 1];
                    if (value < 0) value += MOD;
                }
                rect[pair][i] = value;
            }
        }
    }

    auto at_most = [&](int c, int k) {
        return marginal[min(k, m)][c];
    };
    auto pair_at_most = [&](int pair, int x, int y) {
        x = min(x, m);
        y = min(y, m);
        return rect[pair][x * width + y];
    };
    auto both_exceeded = [&](int pair, int x, int y) {
        int first = pair == 2 ? 1 : 0;
        int second = pair == 0 ? 1 : 2;
        long long result = total;
        result -= at_most(first, x);
        result -= at_most(second, y);
        result += pair_at_most(pair, x, y);
        result %= MOD;
        if (result < 0) result += MOD;
        return (int)result;
    };

    while (q--) {
        array<int, 3> limit;
        cin >> limit[0] >> limit[1] >> limit[2];
        long long answer = total;
        for (int c = 0; c < 3; ++c) {
            answer -= total - at_most(c, limit[c]);
        }
        answer += both_exceeded(0, limit[0], limit[1]);
        answer += both_exceeded(1, limit[0], limit[2]);
        answer += both_exceeded(2, limit[1], limit[2]);
        answer %= MOD;
        if (answer < 0) answer += MOD;
        cout << answer << '\n';
    }

    return 0;
}
