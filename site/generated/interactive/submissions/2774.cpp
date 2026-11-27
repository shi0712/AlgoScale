#include <bits/stdc++.h>
using namespace std;

class Interactor {
public:
    int add(int id) {
        cout << "+ " << id << endl;
        return readResponse();
    }

    int remove() {
        cout << "-" << endl;
        return readResponse();
    }

private:
    int readResponse() {
        int response;
        if (!(cin >> response) || response < 0) {
            exit(0);
        }
        return response;
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int N;
    if (!(cin >> N) || N <= 0) {
        return 0;
    }

    vector<int> order(N);
    iota(order.begin(), order.end(), 1);

    mt19937 rng(712367821);
    shuffle(order.begin(), order.end(), rng);

    Interactor interactor;

    vector<vector<int>> groups;
    vector<int> representativeGroup(N + 1, -1);
    queue<int> belt;

    for (int id : order) {
        int distinctBefore = static_cast<int>(groups.size());
        int distinctAfter = interactor.add(id);

        if (distinctAfter == distinctBefore + 1) {
            int groupId = static_cast<int>(groups.size());
            groups.push_back({id});
            representativeGroup[id] = groupId;
            continue;
        }

        if (distinctAfter != distinctBefore) {
            exit(0);
        }

        vector<int> removedRepresentatives;
        int previousDistinct = distinctAfter;
        int matchingRepresentative = -1;

        while (true) {
            int representative = belt.front();
            belt.pop();

            int currentDistinct = interactor.remove();

            if (currentDistinct == previousDistinct) {
                matchingRepresentative = representative;
                break;
            }

            removedRepresentatives.push_back(representative);
            previousDistinct = currentDistinct;
        }

        int groupId = representativeGroup[matchingRepresentative];
        groups[groupId].push_back(id);
        representativeGroup[id] = groupId;

        for (int representative : removedRepresentatives) {
            interactor.add(representative);
        }
    }

    cout << "! " << groups.size() << endl;
    for (const auto& group : groups) {
        cout << group.size();
        for (int id : group) {
            cout << ' ' << id;
        }
        cout << endl;
    }

    return 0;
}
