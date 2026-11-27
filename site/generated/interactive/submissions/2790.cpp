#include <bits/stdc++.h>
using namespace std;

struct Group {
    vector<int> coins;
    int representative;
};

vector<int> counterfeit;
mt19937_64 rng(
    chrono::steady_clock::now().time_since_epoch().count() ^
    (uint64_t(random_device{}()) << 32)
);

int ask(int x, int y) {
    cout << "? " << x << ' ' << y << endl;
    int response;
    if (!(cin >> response) || response == -1) {
        exit(0);
    }
    if (response != 0 && response != 1) {
        exit(0);
    }
    return response;
}

// Every group is known to be homogeneous. Exactly k groups are counterfeit.
// Returns a coin known to be genuine, or externalGenuine if no such group exists.
int solve(vector<Group> items, int k, int externalGenuine) {
    int n = (int)items.size();

    if (n == 0) {
        return externalGenuine;
    }

    if (k == 0) {
        return items.front().representative;
    }

    if (k == n) {
        for (const Group& group : items) {
            counterfeit.insert(counterfeit.end(), group.coins.begin(), group.coins.end());
        }
        return externalGenuine;
    }

    shuffle(items.begin(), items.end(), rng);

    vector<Group> sameGroups;
    vector<pair<Group, Group>> mixedGroups;
    sameGroups.reserve(n / 2);
    mixedGroups.reserve(k);

    int different = 0;

    for (int i = 0; i + 1 < n; i += 2) {
        int result = ask(items[i].representative, items[i + 1].representative);

        if (result == 0) {
            Group merged;
            merged.representative = items[i].representative;
            merged.coins = move(items[i].coins);
            merged.coins.insert(
                merged.coins.end(),
                items[i + 1].coins.begin(),
                items[i + 1].coins.end()
            );
            sameGroups.push_back(move(merged));
        } else {
            mixedGroups.emplace_back(move(items[i]), move(items[i + 1]));
            ++different;

            if (different == k) {
                int genuine = externalGenuine;

                if (genuine == -1 && i + 2 < n) {
                    genuine = items[i + 2].representative;
                }
                if (genuine == -1 && !sameGroups.empty()) {
                    genuine = sameGroups.back().representative;
                }
                if (genuine == -1 && (n & 1)) {
                    genuine = items.back().representative;
                }
                if (genuine == -1) {
                    exit(0);
                }

                for (const auto& mixed : mixedGroups) {
                    int relation = ask(mixed.first.representative, genuine);
                    if (relation == 1) {
                        counterfeit.insert(
                            counterfeit.end(),
                            mixed.first.coins.begin(),
                            mixed.first.coins.end()
                        );
                    } else {
                        counterfeit.insert(
                            counterfeit.end(),
                            mixed.second.coins.begin(),
                            mixed.second.coins.end()
                        );
                    }
                }

                return genuine;
            }
        }
    }

    bool hasLeftover = (n & 1);
    int leftoverIsCounterfeit = (k - different) & 1;
    int sameCounterfeitGroups = (k - different - leftoverIsCounterfeit) / 2;

    int genuineForRecursion = externalGenuine;
    if (hasLeftover && !leftoverIsCounterfeit) {
        genuineForRecursion = items.back().representative;
    }

    int genuine = solve(
        move(sameGroups),
        sameCounterfeitGroups,
        genuineForRecursion
    );

    if (genuine == -1) {
        exit(0);
    }

    for (const auto& mixed : mixedGroups) {
        int relation = ask(mixed.first.representative, genuine);
        if (relation == 1) {
            counterfeit.insert(
                counterfeit.end(),
                mixed.first.coins.begin(),
                mixed.first.coins.end()
            );
        } else {
            counterfeit.insert(
                counterfeit.end(),
                mixed.second.coins.begin(),
                mixed.second.coins.end()
            );
        }
    }

    if (hasLeftover && leftoverIsCounterfeit) {
        counterfeit.insert(
            counterfeit.end(),
            items.back().coins.begin(),
            items.back().coins.end()
        );
    }

    return genuine;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int N, M;
    if (!(cin >> N >> M)) {
        return 0;
    }

    counterfeit.reserve(M);

    vector<Group> items(N);
    for (int i = 0; i < N; ++i) {
        items[i].coins.push_back(i + 1);
        items[i].representative = i + 1;
    }

    solve(move(items), M, -1);

    cout << "!";
    for (int coin : counterfeit) {
        cout << ' ' << coin;
    }
    cout << endl;

    return 0;
}
