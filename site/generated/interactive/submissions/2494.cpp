#include <algorithm>
#include <array>
#include <iostream>
#include <vector>

using namespace std;

int query(int a, int b, int c) {
    cout << "? " << a << ' ' << b << ' ' << c << endl;
    int response;
    if (!(cin >> response) || response == -1) exit(0);
    return response;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n)) return 0;

    int a = 1, b = 2, c = 3;
    int d = query(a, b, c);

    for (int x = 4; x <= n; ++x) {
        int e = query(a, b, x);
        int f = query(a, c, x);

        if (x == n) {
            vector<pair<int, array<int, 3>>> triples = {
                {d, {a, b, c}},
                {e, {a, b, x}},
                {f, {a, c, x}}
            };
            int maximum = max({d, e, f});
            vector<array<int, 3>> best;
            for (auto [value, triple] : triples) {
                if (value == maximum) best.push_back(triple);
            }
            if (best.size() == 1) best.push_back({b, c, x});

            vector<int> answer;
            for (int i : best[0]) {
                for (int j : best[1]) {
                    if (i == j) answer.push_back(i);
                }
            }
            cout << "! " << answer[0] << ' ' << answer[1] << endl;
            return 0;
        }

        if (e <= d && f <= d) {
            continue;
        }
        if (e >= f) {
            c = x;
            d = e;
        } else {
            b = x;
            d = f;
        }
    }

    return 0;
}
