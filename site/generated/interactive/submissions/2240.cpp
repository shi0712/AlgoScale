#include <bits/stdc++.h>
using namespace std;

int n;

int ask(int i, int j, int k) {
    cout << "? " << i << ' ' << j << ' ' << k << '\n';
    cout.flush();
    int response;
    if (!(cin >> response)) exit(0);
    if (response == -1) exit(0);
    return response;
}

void answer(int i, int j) {
    cout << "! " << i << ' ' << j << '\n';
    cout.flush();
    exit(0);
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    if (!(cin >> n)) return 0;

    vector<int> q(n + 1, 0);
    int D = 0;
    vector<int> S;

    for (int i = 3; i <= n; ++i) {
        q[i] = ask(1, 2, i);
        D = max(D, q[i]);
    }
    for (int i = 3; i <= n; ++i) {
        if (q[i] == D) S.push_back(i);
    }

    // All queries (1,2,i) gave the same answer.  Either the zero is
    // in {1,2}, or every outside element is either zero or a maximum.
    if ((int)S.size() == n - 2) {
        vector<int> outside;
        for (int i = 3; i <= n; ++i) outside.push_back(i);

        int pos = 0;
        while (pos + 3 <= (int)outside.size()) {
            int x = outside[pos], y = outside[pos + 1], z = outside[pos + 2];
            pos += 3;
            int r = ask(x, y, z);

            if (r > D) {
                // The zero is in {x,y,z}; test whether it is in {x,y}.
                int r2 = ask(1, x, y);
                if (r2 > D) answer(x, y);
                answer(z, z);
            }
            if (r > 0) {
                // Outside elements are not just zero/maximum values.
                answer(1, 2);
            }
        }

        int rem = (int)outside.size() - pos;
        if (rem == 0) answer(1, 2);

        if (rem == 1) {
            int x = outside[pos];
            int y = outside[0]; // An already scanned, distinct index.
            int r = ask(1, x, y);
            if (r > D) answer(x, x);
            answer(1, 2);
        }

        int x = outside[pos], y = outside[pos + 1];
        int r = ask(1, x, y);
        if (r > D) answer(x, y);
        answer(1, 2);
    }

    // With at least two maximizers (and not all equal), after one query
    // the zero is either a known fixed index c or in the remaining set R.
    if ((int)S.size() >= 2) {
        int u = S[0], v = S[1];
        int r = ask(1, u, v);

        if (r > D) answer(u, v);

        int c = (r == D ? 1 : 2);
        vector<int> R;
        for (int i = 2; i < (int)S.size(); ++i) R.push_back(S[i]);

        int pos = 0;
        while (pos + 3 <= (int)R.size()) {
            int x = R[pos], y = R[pos + 1], z = R[pos + 2];
            pos += 3;
            int cur = ask(x, y, z);

            if (cur > D) {
                int cur2 = ask(c, x, y);
                if (cur2 > D) answer(x, y);
                answer(z, z);
            }
        }

        int rem = (int)R.size() - pos;
        if (rem == 0) answer(c, c);
        if (rem == 1) answer(c, R[pos]);

        int x = R[pos], y = R[pos + 1];
        int cur = ask(c, x, y);
        if (cur > D) answer(x, y);
        answer(c, c);
    }

    // Exactly one maximizer: its index p is either the zero or a global
    // maximum.  Query pairs together with p and inspect the maximum answers.
    int p = S[0];
    vector<int> other;
    for (int i = 1; i <= n; ++i) {
        if (i != p) other.push_back(i);
    }

    vector<pair<int, int>> groups;
    int m = (int)other.size();
    for (int i = 0; i + 1 < m; i += 2) {
        groups.push_back({other[i], other[i + 1]});
    }
    if (m & 1) {
        groups.push_back({other.back(), other[0]});
    }

    vector<int> val;
    int E = 0;
    for (auto [x, y] : groups) {
        int cur = ask(p, x, y);
        val.push_back(cur);
        E = max(E, cur);
    }

    vector<int> top;
    for (int i = 0; i < (int)groups.size(); ++i) {
        if (val[i] == E) top.push_back(i);
    }

    if ((int)top.size() == 1) {
        auto [x, y] = groups[top[0]];

        auto occurrences = [&](int a) {
            int cnt = 0;
            for (auto [u, v] : groups) cnt += (u == a || v == a);
            return cnt;
        };

        // If the repeated index were the zero, both groups containing it
        // would have the maximum answer.
        if (occurrences(x) > 1) answer(p, y);
        if (occurrences(y) > 1) answer(p, x);

        int h = 1;
        while (h == p || h == x || h == y) ++h;
        int cur = ask(p, x, h);
        if (cur == E) answer(p, x);
        answer(p, y);
    }

    // If p is a maximum, the zero lies in every maximum-answer group.
    // Otherwise p itself is the zero.
    auto [x, y] = groups[top[0]];
    for (int w : {x, y}) {
        bool inAll = true;
        for (int id : top) {
            auto [u, v] = groups[id];
            if (u != w && v != w) {
                inAll = false;
                break;
            }
        }
        if (inAll) answer(p, w);
    }
    answer(p, p);
}
