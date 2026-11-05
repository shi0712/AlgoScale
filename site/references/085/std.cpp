#include <bits/stdc++.h>
using namespace std;

using int64 = long long;

// 内部使用 0-based 编号；返回值 n 表示环上的点 0。
template <class Ask>
pair<int64, int64> locate(int64 n, Ask ask) {
    const int64 m = (n - 1) / 2;
    const int64 k = (m + 1) / 2;
    const int64 u = ask(0);

    // n = 3 单独处理，保证最多两次询问。
    if (n == 3) {
        if (u == 2) return {1, 2};
        const int64 v = ask(1);
        if (v == 1) return {0, 1};
        return {0, 2};
    }

    const int64 v = ask(k);
    const int64 S = u + v;
    const int64 D = u - v;

    // 两点处在同一短段中，且已知展开坐标之和 p。
    auto same_segment = [&](int64 p) -> pair<int64, int64> {
        const int64 x = p / 2;
        int64 gap;
        if (x == 0) gap = u;
        else if (x == k) gap = v;
        else gap = ask(x);
        return {(p - gap) / 2, (p + gap) / 2};
    };

    // 必须先处理这些极值情况，再处理后面的 D == 0。
    if (S == 2 * k || D == 2 * k)
        return same_segment(u);
    if (S == 2 * (n - k) || D == -2 * k)
        return same_segment(2 * n - u);

    // 两点满足 b = a + n - u，只需在连续区间内确定 a。
    if (S == n || D == 0) {
        int64 l, r;
        if (S == n) {
            l = max<int64>(0, u - m);
            r = min(k, u - m + k - 1);
        } else {
            l = max(k, u - m + k);
            r = min(m, u);
        }

        while (l < r) {
            if (u <= m) {
                // f(x) == u 当且仅当 x <= a。
                const int64 mid = (l + r + 1) / 2;
                if (ask(mid) == u) l = mid;
                else r = mid - 1;
            } else {
                // f(x) == n-u 当且仅当 a <= x。
                const int64 mid = (l + r) / 2;
                if (ask(mid) == n - u) r = mid;
                else l = mid + 1;
            }
        }
        return {l, l + n - u};
    }

    // 其余情况下，两次回答已经唯一确定答案。
    if (S % 2 == 0) {
        if (D > 0)
            return {D / 2, S / 2};
        return {k + D / 2, n + k - S / 2};
    }
    if (D > 0)
        return {(S - n + 2 * k) / 2, (n + 2 * k - D) / 2};
    return {(n - D) / 2, (3 * n - S) / 2};
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int T;
    if (!(cin >> T)) return 0;
    while (T--) {
        int64 n;
        if (!(cin >> n) || n < 3) return 0;

        auto ask = [&](int64 x) -> int64 {
            x %= n;
            cout << "? " << x + 1 << endl;  // endl 会刷新缓冲区。
            int64 reply;
            if (!(cin >> reply) || reply < 0) exit(0);
            return reply;
        };

        auto [a, b] = locate(n, ask);
        a %= n;
        b %= n;
        cout << "! " << a + 1 << ' ' << b + 1 << endl;
    }
    return 0;
}
