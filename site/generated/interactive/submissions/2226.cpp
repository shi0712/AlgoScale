#include <bits/stdc++.h>
using namespace std;

struct PendingEdge {
    int edge;
    int prefix;
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int m;
    if (!(cin >> m)) return 0;

    vector<int> representatives;
    vector<int> prefixQuery(1, 0);
    vector<PendingEdge> pending;

    int currentQuery = 0;
    int queryCount = 0;

    for (int e = 1; e <= m; ++e) {
        cout << "? " << currentQuery << ' ' << e << '\n' << flush;
        int id = ++queryCount;

        int response;
        if (!(cin >> response) || response == -1) return 0;

        if (response == 1) {
            representatives.push_back(e);
            prefixQuery.push_back(id);
            currentQuery = id;
        } else {
            pending.push_back({e, (int)representatives.size()});
        }
    }

    int r = (int)representatives.size();
    vector<int> answer(m + 1, -2);
    vector<vector<int>> members(r + 1);

    // Indices of cycles whose length is not yet known to exceed 14.
    set<int> smallCycles;
    for (int i = 1; i <= r; ++i) smallCycles.insert(i);

    for (const auto &item : pending) {
        int e = item.edge;
        int s = item.prefix;

        if (s == 0) {
            answer[e] = -1;
            continue;
        }

        // The answer at prefix s is already known to be 0.  A virtual
        // prefix -1 has answer 1.  Find the first prefix with answer 0.
        int low = -1, high = s;

        while (true) {
            if (low >= 0) {
                // A positive answer at low proves that e is not a bridge.
                // If every remaining possible cycle is already big, its
                // exact identity is unnecessary.
                auto it = smallCycles.lower_bound(low + 1);
                if (it == smallCycles.end() || *it > high) {
                    answer[e] = 0;
                    break;
                }
            }

            if (high - low == 1) {
                int cycle = high;
                if (cycle == 0) {
                    answer[e] = -1;
                } else if (!smallCycles.count(cycle)) {
                    answer[e] = 0;
                } else {
                    members[cycle].push_back(e);
                    if ((int)members[cycle].size() + 1 >= 15) {
                        smallCycles.erase(cycle);
                    }
                }
                break;
            }

            int mid = (low + high) / 2;
            cout << "? " << prefixQuery[mid] << ' ' << e << '\n' << flush;
            ++queryCount;

            int response;
            if (!(cin >> response) || response == -1) return 0;

            if (response == 1) low = mid;
            else high = mid;
        }
    }

    for (int cycle = 1; cycle <= r; ++cycle) {
        int length = 1 + (int)members[cycle].size();
        int value = (length <= 14 ? length : 0);

        answer[representatives[cycle - 1]] = value;
        for (int e : members[cycle]) answer[e] = value;
    }

    cout << '!';
    for (int e = 1; e <= m; ++e) cout << ' ' << answer[e];
    cout << '\n' << flush;

    int verdict;
    if (!(cin >> verdict)) return 0;
    return 0;
}
