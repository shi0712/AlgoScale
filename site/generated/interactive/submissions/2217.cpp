#include <bits/stdc++.h>
using namespace std;

static int ask(int i, int j) {
    cout << "? " << i << ' ' << j << '\n' << flush;
    int response;
    if (!(cin >> response) || response == -1) exit(0);
    return response;
}

[[noreturn]] static void answer(int x, int y) {
    cout << "! " << x << ' ' << y << '\n' << flush;
    int verdict;
    if (!(cin >> verdict)) exit(0);
    exit(0);
}

// The values in c are exactly 0, 2*m, 3*m, ..., |c|*m.
// Every index outside c is already known to be nonzero.
static void solveCandidates(vector<int> c, int m) {
    while (true) {
        int q = (int)c.size();

        if (q == 1) answer(c[0], c[0]);
        if (q == 2) answer(c[0], c[1]);

        if (q == 3) {
            int g = ask(c[0], c[1]);
            if (g == m) answer(c[2], c[2]);
            answer(c[0], c[1]);
        }

        if (q == 4) {
            int a = c[0], b = c[1], d = c[2], e = c[3];
            int g = ask(a, b);

            if (g == m) answer(d, e);
            if (2LL * g > 4LL * m) answer(a, b);

            // Here g = 2*m.
            int h = ask(a, d);
            if (h > 2 * m) answer(a, d);
            if (h == 2 * m) answer(b, d);
            answer(b, e);
        }

        int center = c[0];
        vector<int> others(c.begin() + 1, c.end());
        vector<int> results;
        results.reserve(others.size());

        int maximum = 0, argMaximum = -1;
        for (int index : others) {
            int g = ask(center, index);

            // Two distinct nonzero values here cannot have such a large gcd.
            if (2LL * g > 1LL * q * m) answer(center, index);

            results.push_back(g);
            if (g > maximum) {
                maximum = g;
                argMaximum = index;
            }
        }

        if (maximum == q * m) answer(center, argMaximum);

        // The center is nonzero and its value is maximum. Zero is precisely
        // among the other indices whose answer equals maximum.
        vector<int> next;
        for (int i = 0; i < (int)others.size(); ++i) {
            if (results[i] == maximum) next.push_back(others[i]);
        }

        c = move(next);
        m = maximum;
    }
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n)) return 0;

    if (n == 2) answer(1, 2);

    const int largest = n - 1;
    vector<int> others;
    for (int i = 2; i <= n; ++i) others.push_back(i);

    vector<int> results;
    results.reserve(others.size());

    int maximum = 0, argMaximum = -1;
    for (int index : others) {
        int g = ask(1, index);

        if (2 * g > largest) answer(1, index);

        results.push_back(g);
        if (g > maximum) {
            maximum = g;
            argMaximum = index;
        }
    }

    if (maximum == largest) answer(1, argMaximum);

    // Index 1 has value maximum. Its other multiples, together with zero,
    // are exactly the indices giving the maximum answer.
    vector<int> candidates;
    for (int i = 0; i < (int)others.size(); ++i) {
        if (results[i] == maximum) candidates.push_back(others[i]);
    }

    solveCandidates(move(candidates), maximum);
}
