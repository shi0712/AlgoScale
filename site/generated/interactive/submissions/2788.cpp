#include <bits/stdc++.h>
using namespace std;

struct Candidate {
    int x, y;
};

long long ask(int i, int j, int k) {
    cout << "? " << i << ' ' << j << ' ' << k << endl;
    long long response;
    if (!(cin >> response) || response == -1) {
        exit(0);
    }
    return response;
}

// The four indices must be distinct and must contain the unique zero.
// Returns a pair containing that zero.
Candidate reduceFour(const array<int, 4>& v) {
    // Query triples omitting v[0], v[1], and v[2], respectively.
    long long r0 = ask(v[1], v[2], v[3]);
    long long r1 = ask(v[0], v[2], v[3]);
    long long r2 = ask(v[0], v[1], v[3]);

    long long maximum = max({r0, r1, r2});
    vector<int> low;

    if (r0 < maximum) low.push_back(v[0]);
    if (r1 < maximum) low.push_back(v[1]);
    if (r2 < maximum) low.push_back(v[2]);

    if (low.size() >= 2) {
        return {low[0], low[1]};
    }
    if (low.size() == 1) {
        return {low[0], v[3]};
    }
    return {v[3], v[0]};
}

Candidate mergeCandidates(const Candidate& a, const Candidate& b) {
    return reduceFour({a.x, a.y, b.x, b.y});
}

Candidate reduceEvenIndices(int count) {
    vector<Candidate> current;
    for (int i = 1; i <= count; i += 2) {
        current.push_back({i, i + 1});
    }

    while (current.size() > 1) {
        vector<Candidate> next;
        size_t i = 0;

        while (i + 1 < current.size()) {
            next.push_back(mergeCandidates(current[i], current[i + 1]));
            i += 2;
        }
        if (i < current.size()) {
            next.push_back(current[i]);
        }

        current = move(next);
    }

    return current.front();
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n) || n == -1) {
        return 0;
    }

    Candidate answer;

    if (n % 2 == 0) {
        answer = reduceEvenIndices(n);
    } else {
        Candidate prefix = reduceEvenIndices(n - 1);

        int helper = -1;
        for (int i = 1; i <= n - 1; ++i) {
            if (i != prefix.x && i != prefix.y) {
                helper = i;
                break;
            }
        }

        answer = reduceFour({prefix.x, prefix.y, n, helper});
    }

    cout << "! " << answer.x << ' ' << answer.y << endl;
    return 0;
}
