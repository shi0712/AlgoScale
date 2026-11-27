#include <bits/stdc++.h>
using namespace std;

long long ask(const vector<int>& nodes) {
    cout << "? 1 " << nodes.size();
    for (int u : nodes) cout << ' ' << u;
    cout << '\n';
    cout.flush();

    long long response;
    if (!(cin >> response)) exit(0);
    if (response == -1) exit(0);
    return response;
}

void flip(int u) {
    cout << "? 2 " << u << '\n';
    cout.flush();
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n)) return 0;

    for (int i = 0; i < n - 1; ++i) {
        int u, v;
        if (!(cin >> u >> v)) return 0;
    }

    /*
       Each weight set is superincreasing, so all subset sums are distinct.
       The total weight and the number of odd weights are even, ensuring
       that every valid response is even and cannot be confused with -1.
    */
    const vector<vector<int>> weights = {
        {},
        {2},
        {1, 3},
        {1, 3, 6},
        {1, 3, 5, 11},
        {1, 3, 5, 11, 22},
        {1, 3, 5, 11, 22, 44},
        {1, 3, 5, 11, 22, 43, 87},
        {1, 3, 5, 11, 22, 43, 87, 174},
        {1, 3, 5, 11, 22, 43, 87, 174, 348}
    };

    vector<int> totalWeight(weights.size(), 0);
    for (int t = 1; t < (int)weights.size(); ++t) {
        for (int w : weights[t]) totalWeight[t] += w;
    }

    int maxBlock = 1;
    for (int t = 1; t < (int)weights.size(); ++t) {
        if (t <= n - 1 && totalWeight[t] <= n) {
            maxBlock = t;
        }
    }

    // Querying node 1 twice makes the response even.
    long long centerResponse = ask(vector<int>{1, 1});
    long long C = centerResponse / 2;

    vector<int> answer(n + 1);

    auto decodeLeaves = [&](bool rootIsOne) {
        for (int first = 2; first <= n; first += maxBlock) {
            int count = min(maxBlock, n - first + 1);
            const vector<int>& w = weights[count];
            int total = totalWeight[count];

            vector<int> query;
            query.reserve(total);

            for (int j = 0; j < count; ++j) {
                for (int k = 0; k < w[j]; ++k) {
                    query.push_back(first + j);
                }
            }

            long long response = ask(query);

            /*
               If the root is 1:
                   f(leaf) = C + value(leaf),
               so response = C * total + sum(w[j] * value[j]).

               Otherwise, after the appropriate transformation:
                   f(leaf) = value(leaf),
               so response = sum(w[j] * value[j]).
            */
            long long encoded = rootIsOne ? response - C * total : response;

            // encoded = total - 2 * P, where P is the sum of weights
            // corresponding to vertices whose value is -1.
            long long P = (total - encoded) / 2;

            for (int j = count - 1; j >= 0; --j) {
                if (P >= w[j]) {
                    P -= w[j];
                    answer[first + j] = -1;
                } else {
                    answer[first + j] = 1;
                }
            }
        }
    };

    if (C % 2 != 0) {
        // An odd value of f(1) means that node 1 itself is the root.
        answer[1] = static_cast<int>(C);
        decodeLeaves(true);
    } else if (C == 0) {
        // Initially, every leaf's root-path sum equals its own value.
        decodeLeaves(false);

        flip(1);

        // After the flip, the response is 4 times the final value of node 1.
        long long response = ask(vector<int>{1, 1});
        answer[1] = static_cast<int>(response / 4);
    } else {
        // C is 2 or -2. Flipping node 1 makes every leaf-path sum
        // equal to that leaf's own value.
        flip(1);
        decodeLeaves(false);
        answer[1] = static_cast<int>(-C / 2);
    }

    cout << '!';
    for (int u = 1; u <= n; ++u) {
        cout << ' ' << answer[u];
    }
    cout << '\n';
    cout.flush();

    return 0;
}
