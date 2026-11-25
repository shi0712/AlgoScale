#include <algorithm>
#include <iostream>
#include <numeric>
#include <vector>

using namespace std;

int n;
vector<vector<int>> members;
vector<int> representative;
vector<vector<int>> bestCost;
vector<vector<unsigned char>> useRecursion;

int addMineral(int id) {
    cout << "+ " << id << '\n' << flush;
    int distinct;
    if (!(cin >> distinct)) exit(0);
    return distinct;
}

int removeMineral() {
    cout << "-" << '\n' << flush;
    int distinct;
    if (!(cin >> distinct)) exit(0);
    return distinct;
}

void clearQueue(int size) {
    for (int i = 0; i < size; ++i) removeMineral();
}

void classifyDirectly(const vector<int>& items, vector<int> classes) {
    sort(classes.begin(), classes.end(), [](int a, int b) {
        return members[a].size() > members[b].size();
    });

    vector<int> online;
    vector<int> offline = classes;
    for (int item : items) {
        int onlineCount = online.size();
        int response = addMineral(item);
        int matched = -1;

        if (response == onlineCount) {
            int distinct = onlineCount;
            int i = 0;
            for (; i < onlineCount; ++i) {
                int current = online[i];
                response = removeMineral();
                if (response == distinct) {
                    matched = current;
                    break;
                }
                offline.push_back(current);
                distinct = response;
            }

            vector<int> remaining(online.begin() + i + 1, online.end());
            remaining.push_back(matched);
            online.swap(remaining);
            representative[matched] = item;
        } else {
            int offlineCount = offline.size();
            for (int current : online) {
                removeMineral();
                offline.push_back(current);
            }
            online.clear();

            vector<int> scanned;
            int distinct = 1;
            int matchedIndex = offlineCount - 1;
            for (int i = 0; i + 1 < offlineCount; ++i) {
                int current = offline[i];
                response = addMineral(representative[current]);
                scanned.push_back(current);
                if (response == distinct) {
                    matched = current;
                    matchedIndex = i;
                    break;
                }
                distinct = response;
            }
            bool inferred = matched == -1;
            if (inferred) matched = offline[matchedIndex];

            removeMineral();
            online.swap(scanned);
            vector<int> remaining;
            for (int i = matchedIndex + !inferred; i < (int)offline.size(); ++i) {
                remaining.push_back(offline[i]);
            }
            offline.swap(remaining);
        }

        members[matched].push_back(item);
    }
    clearQueue(online.size());
}

void classify(const vector<int>& items, const vector<int>& classes) {
    if (items.empty() || classes.size() == 1) {
        if (!classes.empty()) {
            members[classes[0]].insert(members[classes[0]].end(), items.begin(), items.end());
        }
        return;
    }

    int m = items.size();
    int k = classes.size();
    if (!useRecursion[m][k]) {
        classifyDirectly(items, classes);
        return;
    }

    for (int c : classes) addMineral(representative[c]);
    for (int item : items) addMineral(item);

    vector<int> present;
    int distinct = k;
    for (int c : classes) {
        int response = removeMineral();
        if (response == distinct) present.push_back(c);
        distinct = response;
    }
    clearQueue(m);

    if (present.size() == 1) {
        members[present[0]].insert(members[present[0]].end(), items.begin(), items.end());
        return;
    }

    int middle = m / 2;
    vector<int> left(items.begin(), items.begin() + middle);
    vector<int> right(items.begin() + middle, items.end());
    classify(left, present);
    classify(right, present);
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    if (!(cin >> n)) return 0;

    bestCost.assign(n + 1, vector<int>(n + 1, 0));
    useRecursion.assign(n + 1, vector<unsigned char>(n + 1, 0));
    for (int m = 1; m <= n; ++m) {
        for (int k = 1; k <= n; ++k) {
            bestCost[m][k] = m * (k + 1) + k;
            if (m == 1) continue;
            int left = m / 2;
            int right = m - left;
            int present = min(k, m);
            int recursiveCost = 2 * m + 2 * k
                + bestCost[left][present]
                + bestCost[right][present];
            if (recursiveCost < bestCost[m][k]) {
                bestCost[m][k] = recursiveCost;
                useRecursion[m][k] = 1;
            }
        }
    }

    vector<int> firstClass(n + 1, -1);
    vector<int> duplicates;
    int distinct = 0;

    for (int id = 1; id <= n; ++id) {
        int response = addMineral(id);
        if (response > distinct) {
            firstClass[id] = members.size();
            representative.push_back(id);
            members.push_back({id});
        } else {
            duplicates.push_back(id);
        }
        distinct = response;
    }

    if (!duplicates.empty() && members.size() > 1) {
        vector<int> active;
        for (int id = 1; id <= n; ++id) {
            int response = removeMineral();
            if (firstClass[id] != -1 && response == distinct) {
                active.push_back(firstClass[id]);
            }
            distinct = response;
        }
        classify(duplicates, active);
    } else if (members.size() == 1) {
        members[0].insert(members[0].end(), duplicates.begin(), duplicates.end());
    }

    cout << "! " << members.size() << '\n';
    for (const auto& group : members) {
        cout << group.size();
        for (int id : group) cout << ' ' << id;
        cout << '\n';
    }
    cout << flush;
    return 0;
}
