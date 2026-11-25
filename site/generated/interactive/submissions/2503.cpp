#include <algorithm>
#include <cstdint>
#include <iostream>
#include <numeric>
#include <set>
#include <string>
#include <utility>
#include <vector>

using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n)) return 0;

    set<pair<int, int>> edges;
    if (n == 2) {
        cout << "!\n1 2\n" << flush;
        return 0;
    }

    auto codeLength = [](int m) {
        for (int p = 2; ; ++p) {
            bool prime = true;
            for (int d = 2; d * d <= p; ++d) {
                if (p % d == 0) prime = false;
            }
            if (!prime) continue;
            int power = 1, t = 0;
            while (power < m) {
                power *= p;
                ++t;
            }
            if (p > 2 * (t - 1)) return p * p;
        }
    };

    vector<int> worst(n + 1, 0);
    vector<char> useCodes(n + 1, false);
    for (int m = 2; m <= n; ++m) {
        worst[m] = m - 2;
        int remaining = (m - 2) / 2;
        int coded = codeLength(m) + worst[remaining];
        if (coded < worst[m]) {
            worst[m] = coded;
            useCodes[m] = true;
        }
    }

    vector<int> active(n);
    iota(active.begin(), active.end(), 0);

    auto addEdge = [&](int a, int b) {
        edges.emplace(min(a, b), max(a, b));
    };

    while (active.size() > 1) {
        int m = static_cast<int>(active.size());

        if (!useCodes[m]) {
            for (int q = 0; q + 2 < m; ++q) {
                cout << '?';
                for (int i = 0; i < n; ++i) {
                    cout << ' ' << (i == active[q] ? 1 : 0);
                }
                cout << '\n' << flush;

                string lamps;
                if (!(cin >> lamps)) return 0;
                for (int j = q + 1; j < m; ++j) {
                    if (lamps[active[j]] == '1') addEdge(active[q], active[j]);
                }
            }
            if (edges.size() == static_cast<size_t>(n - 2)) {
                addEdge(active[m - 2], active[m - 1]);
            }
            break;
        }

        int p = 2;
        while (p * p != codeLength(m)) ++p;
        int queries = p * p;
        vector<uint64_t> code(m), lamps(m, 0);
        for (int i = 0; i < m; ++i) {
            vector<int> coefficients;
            int value = i;
            while (value > 0) {
                coefficients.push_back(value % p);
                value /= p;
            }
            for (int x = 0; x < p; ++x) {
                int y = 0, place = 1;
                for (int coefficient : coefficients) {
                    y = (y + coefficient * place) % p;
                    place = place * x % p;
                }
                code[i] |= uint64_t{1} << (x * p + y);
            }
        }

        for (int column = 0; column < queries; ++column) {
            cout << '?';
            for (int vertex = 0; vertex < n; ++vertex) {
                int d = 0;
                auto it = lower_bound(active.begin(), active.end(), vertex);
                if (it != active.end() && *it == vertex) {
                    int i = static_cast<int>(it - active.begin());
                    d = ((code[i] >> column) & 1) ? 1 : 0;
                }
                cout << ' ' << d;
            }
            cout << '\n' << flush;

            string response;
            if (!(cin >> response)) return 0;
            for (int i = 0; i < m; ++i) {
                if (response[active[i]] == '1') {
                    lamps[i] |= uint64_t{1} << column;
                }
            }
        }

        vector<int> remaining;
        for (int i = 0; i < m; ++i) {
            vector<int> neighbors;
            for (int j = 0; j < m; ++j) {
                if ((code[j] & lamps[i]) == code[j]) neighbors.push_back(j);
            }
            if (neighbors.size() <= 2) {
                for (int j : neighbors) addEdge(active[i], active[j]);
            } else {
                remaining.push_back(active[i]);
            }
        }
        active.swap(remaining);
    }

    cout << "!\n";
    for (auto [a, b] : edges) cout << a + 1 << ' ' << b + 1 << '\n';
    cout << flush;
    return 0;
}
