#include <bits/stdc++.h>
using namespace std;

using Bits = bitset<128>;

int n;

int ask(int p) {
    cout << "? " << p << '\n';
    cout.flush();
    int r;
    if (!(cin >> r)) exit(0);
    if (r == -1) exit(0);
    return r;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    cin >> n;

    auto finish = [&](const vector<int>& a) {
        int bestTarget = 0, bestCost = INT_MAX;
        for (int t = 0; t < n; ++t) {
            int cost = 0;
            for (int x : a) cost += (t - x + n) % n;
            if (cost < bestCost) {
                bestCost = cost;
                bestTarget = t;
            }
        }
        for (int i = 0; i < n; ++i) {
            int d = (bestTarget - a[i] + n) % n;
            while (d--) ask(i + 1);
        }
        cout << "!\n";
        cout.flush();
        exit(0);
    };

    // Let the initial value of dial 1 be coordinate 0.
    vector<int> S;
    for (int d = 1; d <= n; ++d) {
        if (ask(1) == 1) S.push_back(d % n);
    }
    sort(S.begin(), S.end());
    S.erase(unique(S.begin(), S.end()), S.end());

    const int N = n - 1;
    const int m = (int)S.size();

    if (m == 1) {
        vector<int> a(n, S[0]);
        a[0] = 0;
        finish(a);
    }

    // b=0: this dial was the only non-first dial with its value.
    // b=1: some other non-first dial had the same value.
    vector<pair<int,int>> baseCandidates;
    for (int x : S) {
        baseCandidates.push_back({x, 0});
        if (N > m) baseCandidates.push_back({x, 1});
    }

    static Bits pat[100][100][2];
    for (int y = 0; y < n; ++y) {
        for (int x : S) {
            for (int b = 0; b <= 1; ++b) {
                for (int z : S) {
                    if (b || z != x)
                        pat[y][x][b].set((z - x + n) % n);
                }
                pat[y][x][b].set((y - x + n) % n);
            }
        }
    }

    int y = -1;
    for (int testY = 0; testY < n && y == -1; ++testY) {
        bool identifying = true;
        for (int i = 0; i < (int)baseCandidates.size() && identifying; ++i) {
            for (int j = 0; j < i; ++j) {
                auto [x1, b1] = baseCandidates[i];
                auto [x2, b2] = baseCandidates[j];
                if (x1 != x2 &&
                    pat[testY][x1][b1] == pat[testY][x2][b2]) {
                    identifying = false;
                    break;
                }
            }
        }
        if (identifying) y = testY;
    }

    bool identifyingPosition = (y != -1);
    if (y == -1) y = 0;

    for (int i = 0; i < y; ++i) ask(1);

    if (identifyingPosition) {
        vector<int> original(n + 1, -1);
        vector<int> bMask(n + 1, 0);
        vector<char> scanned(n + 1, false);
        vector<int> remaining;
        for (int p = 2; p <= n; ++p) remaining.push_back(p);

        auto possibleRemainingValues = [&](int r) {
            vector<int> L(n, 0), U(n, 0);
            int sumL = 0, sumU = 0;
            bool feasible = true;

            for (int v : S) {
                int c = 0, mask = 0;
                for (int p = 2; p <= n; ++p) {
                    if (scanned[p] && original[p] == v) {
                        ++c;
                        mask = bMask[p];
                    }
                }

                if (c == 0) {
                    L[v] = 1;
                    U[v] = r;
                } else if (c == 1) {
                    bool allow0 = mask & 1;
                    bool allow1 = mask & 2;
                    if (allow0 && allow1) {
                        L[v] = 0;
                        U[v] = r;
                    } else if (allow0) {
                        L[v] = U[v] = 0;
                    } else if (allow1) {
                        L[v] = 1;
                        U[v] = r;
                    } else {
                        feasible = false;
                    }
                } else {
                    L[v] = 0;
                    U[v] = r;
                }
                sumL += L[v];
                sumU += U[v];
            }

            vector<int> result;
            if (!feasible || sumL > r || sumU < r) return result;

            for (int v : S) {
                if (U[v] < 1) continue;
                int need = max(1, L[v]);
                if (need > U[v]) continue;
                int newSumL = sumL - L[v] + need;
                if (newSumL <= r && sumU >= r) result.push_back(v);
            }
            return result;
        };

        while (!remaining.empty()) {
            int r = (int)remaining.size();
            vector<int> possible = possibleRemainingValues(r);

            if (possible.size() == 1) {
                for (int p : remaining) original[p] = possible[0];
                remaining.clear();
                break;
            }

            if (r == 1) {
                int pos = remaining[0];
                vector<int> count(n, 0);
                for (int p = 2; p <= n; ++p)
                    if (scanned[p]) ++count[original[p]];

                vector<pair<int,int>> candidates;
                for (auto [x, b] : baseCandidates) {
                    bool ok = (b == (count[x] > 0));
                    for (int v : S)
                        if (count[v] + (x == v) == 0) ok = false;

                    for (int p = 2; p <= n && ok; ++p) {
                        if (!scanned[p]) continue;
                        int v = original[p];
                        int actualB =
                            (count[v] + (x == v) >= 2) ? 1 : 0;
                        if (!(bMask[p] & (1 << actualB))) ok = false;
                    }
                    if (ok) candidates.push_back({x, b});
                }

                int identified = -1, used = n;
                for (int d = 1; d <= n; ++d) {
                    int response = ask(pos);
                    int residue = d % n;
                    vector<pair<int,int>> filtered;
                    for (auto [x, b] : candidates) {
                        if ((int)pat[y][x][b][residue] == response)
                            filtered.push_back({x, b});
                    }
                    candidates.swap(filtered);

                    int value = -1;
                    bool uniqueValue = true;
                    for (auto [x, b] : candidates) {
                        if (value == -1) value = x;
                        else if (value != x) {
                            uniqueValue = false;
                            break;
                        }
                    }
                    if (uniqueValue && value != -1) {
                        identified = value;
                        used = d;
                        break;
                    }
                }

                vector<int> current(n);
                current[0] = y;
                for (int p = 2; p <= n; ++p)
                    current[p - 1] = original[p];
                current[pos - 1] = (identified + used) % n;
                finish(current);
            }

            int pos = remaining.front();
            remaining.erase(remaining.begin());

            Bits observed;
            for (int d = 1; d <= n; ++d) {
                if (ask(pos) == 1) observed.set(d % n);
            }

            vector<pair<int,int>> filtered;
            for (auto [x, b] : baseCandidates) {
                if (pat[y][x][b] == observed)
                    filtered.push_back({x, b});
            }

            int x = filtered.front().first;
            int mask = 0;
            for (auto [v, b] : filtered) mask |= (1 << b);

            original[pos] = x;
            bMask[pos] = mask;
            scanned[pos] = true;
        }

        vector<int> current(n);
        current[0] = y;
        for (int p = 2; p <= n; ++p)
            current[p - 1] = original[p];
        finish(current);
    }

    // General fallback: combine observations at several positions of dial 1.
    vector<vector<pair<int,int>>> candidates(
        n + 1, baseCandidates);
    vector<int> value(n + 1, -1);
    int currentY = y;

    while (true) {
        for (int p = 2; p <= n; ++p) {
            if (value[p] != -1) continue;

            Bits observed;
            for (int d = 1; d <= n; ++d) {
                if (ask(p) == 1) observed.set(d % n);
            }

            vector<pair<int,int>> filtered;
            for (auto [x, b] : candidates[p]) {
                if (pat[currentY][x][b] == observed)
                    filtered.push_back({x, b});
            }
            candidates[p].swap(filtered);

            int x = candidates[p].front().first;
            bool uniqueValue = true;
            for (auto [v, b] : candidates[p])
                if (v != x) uniqueValue = false;
            if (uniqueValue) value[p] = x;
        }

        bool done = true;
        for (int p = 2; p <= n; ++p)
            if (value[p] == -1) done = false;

        if (done) {
            vector<int> current(n);
            current[0] = currentY;
            for (int p = 2; p <= n; ++p)
                current[p - 1] = value[p];
            finish(current);
        }

        ask(1);
        currentY = (currentY + 1) % n;
    }
}
