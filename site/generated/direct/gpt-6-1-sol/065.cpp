#include <bits/stdc++.h>
using namespace std;

struct Matrix {
    int a00, a01, a10, a11;
};

// Min-plus matrix multiplication.
static inline Matrix multiply(const Matrix& a, const Matrix& b) {
    return {
        min(a.a00 + b.a00, a.a01 + b.a10),
        min(a.a00 + b.a01, a.a01 + b.a11),
        min(a.a10 + b.a00, a.a11 + b.a10),
        min(a.a10 + b.a01, a.a11 + b.a11)
    };
}

int solve(int n, int m, const vector<int>& a) {
    int mn = a[1], mx = a[1];
    int descents = 0;

    for (int i = 1; i <= n; ++i) {
        mn = min(mn, a[i]);
        mx = max(mx, a[i]);
        if (i > 1) {
            descents += max(0, a[i - 1] - a[i]);
        }
    }

    // Making every element equal to mx.
    int answer = mx - a[1] + descents;
    if (m == 1 || answer == 0) {
        return answer;
    }

    vector<int> order(n);
    iota(order.begin(), order.end(), 1);
    sort(order.begin(), order.end(),
         [&](int x, int y) { return a[x] < a[y]; });

    int distinct = 0;
    for (int i = 0; i < n; ++i) {
        if (i == 0 || a[order[i]] != a[order[i - 1]]) {
            ++distinct;
        }
    }
    if (distinct <= 2) {
        return 0;
    }

    const int V = mx - mn;

    vector<int> p(n + 1), q(n + 1);
    vector<unsigned char> active(n + 2);
    vector<int> leftEnd(n + 2), rightEnd(n + 2);
    vector<int> contribution(n + 2);
    vector<Matrix> product(n + 2);

    // The two final values are L and L + d.
    for (int d = 1; d <= V; ++d) {
        for (int i = 1; i < n; ++i) {
            int delta = a[i + 1] - a[i];
            p[i] = d - min(d, max(delta, 0));
            q[i] = -min(d, max(-delta, 0));
        }
        p[n] = 0;

        fill(active.begin(), active.end(), 0);
        int totalContribution = 0;

        auto activate = [&](int i) {
            int l = i, r = i;

            Matrix current;
            if (i == 1) {
                current = {-d, 0, -d, 0};
            } else {
                current = {0, p[i - 1], q[i - 1], 0};
            }

            if (active[i - 1]) {
                l = leftEnd[i - 1];
                totalContribution -= contribution[l];
                current = multiply(product[l], current);
            }

            if (active[i + 1]) {
                int rightComponent = i + 1;
                r = rightEnd[rightComponent];
                totalContribution -= contribution[rightComponent];
                current = multiply(current, product[rightComponent]);
            }

            active[i] = 1;
            leftEnd[r] = l;
            rightEnd[l] = r;
            product[l] = current;

            contribution[l] = min(current.a10 + p[r], current.a11);
            totalContribution += contribution[l];
        };

        // L + d must be at least mx.
        int L = mx - d;
        int pos = 0;

        while (pos < n && a[order[pos]] <= L) {
            activate(order[pos]);
            ++pos;
        }

        answer = min(answer,
                     L + d - a[1] + descents + totalContribution);
        if (answer == 0) {
            return 0;
        }

        // Only values already occurring in a need to be considered for L.
        while (pos < n) {
            L = a[order[pos]];

            do {
                activate(order[pos]);
                ++pos;
            } while (pos < n && a[order[pos]] == L);

            answer = min(answer,
                         L + d - a[1] + descents + totalContribution);

            if (answer == 0) {
                return 0;
            }
        }
    }

    return answer;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int T;
    cin >> T;

    while (T--) {
        int n, m;
        cin >> n >> m;

        vector<int> a(n + 1);
        for (int i = 1; i <= n; ++i) {
            cin >> a[i];
        }

        cout << solve(n, m, a) << '\n';
    }

    return 0;
}
