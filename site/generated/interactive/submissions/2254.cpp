#include <bits/stdc++.h>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n)) return 0;

    if (n == 1) {
        cout << "!\n1\n0\n" << flush;
        return 0;
    }
    if (n == 2) {
        cout << "!\n2\n0 1\n" << flush;
        return 0;
    }

    auto ask = [&](int i, int j) -> int {
        cout << "? " << i << ' ' << j << '\n' << flush;
        int response;
        if (!(cin >> response) || response < 0) exit(0);
        return response;
    };

    vector<int> a(n, -1);
    const int last = n - 1;

    for (int i = 0; i < last; ++i) {
        a[i] = ask(i, 0);
    }

    const bool powerOfTwo = (n & (n - 1)) == 0;
    int knownC = -1;

    if (powerOfTwo) {
        vector<char> seen(n, 0);
        for (int i = 0; i < last; ++i) seen[a[i]] = 1;
        for (int x = 0; x < n; ++x) {
            if (!seen[x]) {
                a[last] = x;
                break;
            }
        }
    } else {
        vector<pair<int, int>> options;
        vector<int> mark(n, 0);

        for (int c = 0; c < n; ++c) {
            if (c < last && a[c] != c) continue;

            const int stamp = c + 1;
            bool ok = true;

            for (int i = 0; i < last; ++i) {
                int value = a[i] ^ c;
                if (value >= n || mark[value] == stamp) {
                    ok = false;
                    break;
                }
                mark[value] = stamp;
            }
            if (!ok) continue;

            int missing = -1;
            for (int x = 0; x < n; ++x) {
                if (mark[x] != stamp) {
                    missing = x;
                    break;
                }
            }

            if (c == last && missing != 0) continue;
            options.push_back({c, missing ^ c});
        }

        bool sameLast = !options.empty();
        for (auto [c, value] : options) {
            if (value != options[0].second) {
                sameLast = false;
                break;
            }
        }

        if (sameLast) {
            a[last] = options[0].second;
        } else {
            a[last] = ask(last, 0);
        }

        if (options.size() == 1) knownC = options[0].first;
    }

    const int lowbit = n & -n;
    vector<int> symmetries;

    for (int t = 0; t < lowbit; ++t) {
        bool ok = true;
        for (int i = 0; i < n; ++i) {
            if (a[i ^ t] != (a[i] ^ t)) {
                ok = false;
                break;
            }
        }
        if (ok) symmetries.push_back(t);
    }

    auto printAnswer = [&](int c) {
        cout << "!\n" << symmetries.size() << '\n';
        for (int i = 0; i < n; ++i) {
            cout << (a[i] ^ c) << (i + 1 == n ? '\n' : ' ');
        }
        cout << flush;
    };

    if (knownC >= 0) {
        printAnswer(knownC);
        return 0;
    }

    vector<int> candidates;
    for (int c = 0; c < n; ++c) {
        if (a[c] != c) continue;

        bool ok = true;
        if (!powerOfTwo) {
            for (int i = 0; i < n; ++i) {
                if ((a[i] ^ c) >= n) {
                    ok = false;
                    break;
                }
            }
        }
        if (ok) candidates.push_back(c);
    }

    int mSize = 1;
    while (mSize < n) mSize <<= 1;

    vector<int> inverse(mSize, -1);
    for (int i = 0; i < n; ++i) inverse[a[i]] = i;

    vector<int> label(n, -1), representatives;
    for (int c : candidates) {
        if (label[c] >= 0) continue;

        int id = (int)representatives.size();
        representatives.push_back(c);
        for (int d : symmetries) label[c ^ d] = id;
    }

    auto predicted = [&](int c, int v) -> int {
        return a[0] ^ c ^ inverse[v ^ c];
    };

    vector<char> used(n, 0);
    used[0] = 1;

    vector<int> count(mSize, 0), touched;
    long long work = 0;

    while (representatives.size() > 1) {
        const int m = (int)representatives.size();
        int bestV = -1;
        int bestMaximum = INT_MAX;
        long long bestSumSquares = LLONG_MAX;

        auto evaluate = [&](int v) {
            touched.clear();

            for (int c : representatives) {
                int signature = predicted(c, v);
                if (count[signature] == 0) touched.push_back(signature);
                ++count[signature];
            }

            int maximum = 0;
            long long sumSquares = 0;
            for (int signature : touched) {
                maximum = max(maximum, count[signature]);
                sumSquares += 1LL * count[signature] * count[signature];
                count[signature] = 0;
            }

            if (maximum < bestMaximum ||
                (maximum == bestMaximum && sumSquares < bestSumSquares)) {
                bestMaximum = maximum;
                bestSumSquares = sumSquares;
                bestV = v;
            }
        };

        long long cost = 1LL * n * m;
        if (work + cost <= 60000000LL) {
            work += cost;
            for (int v = 1; v < n; ++v) {
                if (!used[v]) evaluate(v);
            }
        } else {
            int sampled = 0;
            for (int v = 1; v < n && sampled < 12; ++v) {
                if (!used[v]) {
                    evaluate(v);
                    ++sampled;
                }
            }
        }

        if (bestMaximum >= m) {
            int splittingV = -1;
            for (int v = 1; v < n; ++v) {
                if (!used[v] &&
                    predicted(representatives[0], v) !=
                    predicted(representatives[1], v)) {
                    splittingV = v;
                    break;
                }
            }
            if (splittingV < 0) break;
            bestV = splittingV;
        }

        if (bestV < 0) break;

        int response = ask(0, bestV);
        used[bestV] = 1;

        vector<int> remaining;
        for (int c : representatives) {
            if (predicted(c, bestV) == response) remaining.push_back(c);
        }
        representatives.swap(remaining);

        if (representatives.empty()) return 0;
    }

    printAnswer(representatives[0]);
    return 0;
}
