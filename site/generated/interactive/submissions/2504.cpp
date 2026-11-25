#include <iostream>
#include <vector>

using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int m;
    if (!(cin >> m)) return 0;

    int queryCount = 0;
    auto ask = [&](int p, int e) {
        ++queryCount;
        cout << "? " << p << ' ' << e << '\n' << flush;
        int answer;
        if (!(cin >> answer) || answer == -1) exit(0);
        return answer;
    };

    vector<int> representatives;
    vector<int> prefixQuery(1, 0);
    vector<int> owner(m + 1, 0);
    vector<int> cycleSize(m + 1, 0);
    vector<int> answer(m + 1, -1);
    int currentQuery = 0;
    bool bridgeSeen = false;

    for (int e = 1; e <= m; ++e) {
        int queryId = queryCount + 1;
        if (ask(currentQuery, e) == 1) {
            currentQuery = queryId;
            representatives.push_back(e);
            prefixQuery.push_back(queryId);
            owner[e] = e;
            cycleSize[e] = 1;
            continue;
        }

        if (representatives.empty()) continue;
        if (bridgeSeen && ask(0, e) == 0) continue;

        int low = 0;
        int high = static_cast<int>(representatives.size());
        while (high - low > 1) {
            int middle = (low + high) / 2;
            if (ask(prefixQuery[middle], e) == 1) {
                low = middle;
            } else {
                high = middle;
            }
        }

        if (low == 0 && !bridgeSeen) {
            if (ask(0, e) == 0) {
                bridgeSeen = true;
                continue;
            }
        }
        int representative = representatives[high - 1];
        owner[e] = representative;
        ++cycleSize[representative];
    }

    for (int e = 1; e <= m; ++e) {
        if (owner[e] != 0) {
            int length = cycleSize[owner[e]];
            answer[e] = length <= 14 ? length : 0;
        }
    }

    cout << '!';
    for (int e = 1; e <= m; ++e) cout << ' ' << answer[e];
    cout << '\n' << flush;

    int confirmation;
    if (!(cin >> confirmation)) return 0;
    return 0;
}
