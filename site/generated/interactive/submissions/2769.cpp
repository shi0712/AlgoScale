#include <bits/stdc++.h>
using namespace std;

int ask(const vector<int>& query) {
    cout << "? " << query.size();
    for (int x : query) cout << ' ' << x;
    cout << endl;

    int response;
    if (!(cin >> response) || response < 0) exit(0);
    return response;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n)) return 0;

    if (n == 2) {
        int response = ask({0, 1});
        if (response < 0 || response > 1) exit(0);
        cout << "! " << (response == 0 ? 2 : 1) << endl;
        return 0;
    }

    int d = 1;
    long long bestQueries = LLONG_MAX;

    for (int x = 1; 2 * x <= n - 1; ++x) {
        int remaining = n - 1 - 2 * x;
        int batches = (remaining + x - 1) / x;
        long long queries = 2LL * x + batches;

        if (queries <= bestQueries) {
            bestQueries = queries;
            d = x;
        }
    }

    vector<int> speciesA{0};
    vector<int> speciesB;

    int next = 1;
    while (static_cast<int>(speciesA.size()) < d + 1 &&
           static_cast<int>(speciesB.size()) < d + 1) {
        int response = ask({0, next});
        if (response < 0 || response > 1) exit(0);

        if (response == 0) {
            speciesA.push_back(next);
        } else {
            speciesB.push_back(next);
        }
        ++next;
    }

    int answerA = static_cast<int>(speciesA.size());

    while (next < n) {
        bool useA = speciesA.size() >= speciesB.size();
        vector<int>& target = useA ? speciesA : speciesB;
        vector<int>& other = useA ? speciesB : speciesA;

        int count = min(n - next, static_cast<int>(target.size()) - 1);
        if (count <= 0) exit(0);

        vector<int> query;
        query.reserve(2 * count + 1);

        for (int j = 0; j < count; ++j) {
            query.push_back(target[j]);
            query.push_back(next + j);
        }
        query.push_back(target[count]);

        int response = ask(query);
        if (response < 0 || (response & 1) || response > 2 * count) exit(0);

        int opposite = response / 2;

        if (useA) {
            answerA += count - opposite;
        } else {
            answerA += opposite;
        }

        if (opposite == 0) {
            for (int j = 0; j < count; ++j) {
                target.push_back(next + j);
            }
        } else if (opposite == count) {
            for (int j = 0; j < count; ++j) {
                other.push_back(next + j);
            }
        }

        next += count;
    }

    cout << "! " << answerA << endl;
    return 0;
}
