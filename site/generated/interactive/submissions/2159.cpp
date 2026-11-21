#include <bits/stdc++.h>
using namespace std;

int n;

long long ask(int i, int j, int k) {
    cout << "? " << i << ' ' << j << ' ' << k << '\n' << flush;
    long long r;
    if (!(cin >> r)) exit(0);
    if (r == -1) exit(0);
    return r;
}

// Reduces four indices a,b,c,d to a pair.
// If the zero is among these four, the returned pair contains it.
vector<int> reduce4(int a, int b, int c, int d) {
    long long A = ask(a, b, c);
    if (A == 0) return {d, a};

    long long B = ask(a, b, d);
    if (B == 0) return {c, a};

    long long C = ask(a, c, d);

    if (A == B && A >= C) return {a, b};
    if (A == C && A >= B) return {a, c};
    if (B == C && B >= A) return {a, d};

    if (A > B && A > C) return {b, c};
    if (B > A && B > C) return {b, d};
    return {c, d};
}

// Combines two candidate sets of size at most 2 into one set of size at most 2.
vector<int> reduceSets(const vector<int>& s, const vector<int>& t) {
    vector<int> u;
    auto add = [&](int x) {
        for (int y : u) if (y == x) return;
        u.push_back(x);
    };

    for (int x : s) add(x);
    for (int x : t) add(x);

    if (u.size() <= 2) return u;

    if (u.size() == 3) {
        int h = -1;
        for (int i = 1; i <= n; ++i) {
            bool in = false;
            for (int x : u) {
                if (x == i) {
                    in = true;
                    break;
                }
            }
            if (!in) {
                h = i;
                break;
            }
        }

        // n >= 4, so such h always exists.
        if (h == -1) return {u[0], u[1]};

        vector<int> r = reduce4(u[0], u[1], u[2], h);
        vector<int> res;
        for (int x : r) {
            if (x != h) res.push_back(x);
        }

        if (res.empty()) res = {u[0]};
        return res;
    }

    return reduce4(u[0], u[1], u[2], u[3]);
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    if (!(cin >> n)) return 0;

    vector<vector<int>> sets;
    for (int i = 1; i <= n; i += 2) {
        if (i + 1 <= n) sets.push_back({i, i + 1});
        else sets.push_back({i});
    }

    while (sets.size() > 1) {
        vector<vector<int>> nxt;
        for (size_t i = 0; i + 1 < sets.size(); i += 2) {
            nxt.push_back(reduceSets(sets[i], sets[i + 1]));
        }
        if (sets.size() % 2 == 1) {
            nxt.push_back(sets.back());
        }
        sets.swap(nxt);
    }

    int x = sets[0][0];
    int y = (sets[0].size() > 1 ? sets[0][1] : sets[0][0]);

    cout << "! " << x << ' ' << y << '\n' << flush;
    return 0;
}
