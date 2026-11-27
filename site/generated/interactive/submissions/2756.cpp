#include <bits/stdc++.h>
using namespace std;

int ask(int i, int j) {
    cout << "? " << i + 1 << ' ' << j + 1 << endl;

    int g;
    if (!(cin >> g) || g == -1) {
        exit(0);
    }
    return g;
}

void answer(int x, int y) {
    cout << "! " << x + 1 << ' ' << y + 1 << endl;

    int verdict;
    if (!(cin >> verdict) || verdict == -1) {
        exit(0);
    }
    exit(0);
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n) || n == -1) {
        return 0;
    }

    const int N = n - 1;

    mt19937 rng(static_cast<uint32_t>(
        chrono::steady_clock::now().time_since_epoch().count()
    ));

    int pivot = static_cast<int>(rng() % n);

    vector<int> order;
    order.reserve(n - 1);
    for (int i = 0; i < n; ++i) {
        if (i != pivot) {
            order.push_back(i);
        }
    }
    shuffle(order.begin(), order.end(), rng);

    vector<int> response(n, -1);
    int base = 0;

    for (int j : order) {
        int g = ask(pivot, j);

        // Two distinct nonzero permutation values cannot have gcd > N / 2.
        if (2 * g > N) {
            answer(pivot, j);
        }

        response[j] = g;
        base = max(base, g);
    }

    /*
       The pivot is nonzero. Its value is base, and the candidates below
       have values 0, 2*base, 3*base, ..., floor(N/base)*base.
    */
    vector<int> candidates;
    for (int j = 0; j < n; ++j) {
        if (j != pivot && response[j] == base) {
            candidates.push_back(j);
        }
    }

    while (true) {
        if (candidates.size() == 1) {
            // The only remaining candidate must contain zero.
            answer(candidates[0], candidates[0]);
        }

        shuffle(candidates.begin(), candidates.end(), rng);
        pivot = candidates.front();

        const int m = static_cast<int>(candidates.size());
        const int currentMax = base * m;

        vector<int> currentResponse(n, -1);
        int nextBase = 0;

        for (int k = 1; k < m; ++k) {
            int j = candidates[k];
            int g = ask(pivot, j);

            /*
               All positive candidate values are distinct multiples of base.
               Two such values have gcd at most currentMax / 2, so a larger
               response proves that one endpoint contains zero.
            */
            if (2 * g > currentMax) {
                answer(pivot, j);
            }

            currentResponse[j] = g;
            nextBase = max(nextBase, g);
        }

        vector<int> nextCandidates;
        nextCandidates.reserve(m);

        for (int k = 1; k < m; ++k) {
            int j = candidates[k];
            if (currentResponse[j] == nextBase) {
                nextCandidates.push_back(j);
            }
        }

        candidates = move(nextCandidates);
        base = nextBase;
    }
}
