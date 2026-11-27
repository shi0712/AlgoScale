#include <bits/stdc++.h>
using namespace std;

static bool ask(int cafe) {
    cout << "? " << cafe << endl;

    string response;
    if (!(cin >> response)) {
        exit(0);
    }

    if (response == "Y") return true;
    if (response == "N") return false;

    exit(0);
}

static void resetMemory() {
    cout << "R" << endl;
}

static long long chunkCost(int chunkSize, int unresolved, int k) {
    int capacity = k - chunkSize + 1;
    int batches = (unresolved + capacity - 1) / capacity;
    return 1LL * chunkSize * batches + unresolved;
}

static int chooseChunkSize(int remaining, int unresolved, int k) {
    const long long INF = (1LL << 60);

    vector<long long> dp(remaining + 1, INF);
    dp[0] = 0;

    for (int r = 1; r <= remaining; ++r) {
        int limit = min(k, r);
        for (int x = 1; x <= limit; ++x) {
            dp[r] = min(dp[r], dp[r - x] + chunkCost(x, unresolved, k));
        }
    }

    long long bestCost = INF;
    int bestSize = 1;

    for (int x = min(k, remaining); x >= 1; --x) {
        long long current =
            dp[remaining - x] + chunkCost(x, unresolved, k);

        if (current < bestCost) {
            bestCost = current;
            bestSize = x;
        }
    }

    return bestSize;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, k;
    if (!(cin >> n >> k)) {
        return 0;
    }

    vector<int> representatives;
    representatives.reserve(n);

    for (int left = 0; left < n; left += k) {
        int right = min(n, left + k);

        vector<int> localRepresentatives;
        localRepresentatives.reserve(right - left);

        resetMemory();
        for (int cafe = left + 1; cafe <= right; ++cafe) {
            if (!ask(cafe)) {
                localRepresentatives.push_back(cafe);
            }
        }

        if (representatives.empty()) {
            representatives = move(localRepresentatives);
            continue;
        }

        vector<int> unresolved = move(localRepresentatives);
        int globalPosition = 0;

        while (!unresolved.empty() &&
               globalPosition < static_cast<int>(representatives.size())) {
            int remaining =
                static_cast<int>(representatives.size()) - globalPosition;

            int chunkSize = chooseChunkSize(
                remaining, static_cast<int>(unresolved.size()), k
            );

            int capacity = k - chunkSize + 1;
            vector<int> nextUnresolved;
            nextUnresolved.reserve(unresolved.size());

            int index = 0;
            while (index < static_cast<int>(unresolved.size())) {
                int take = min(
                    capacity,
                    static_cast<int>(unresolved.size()) - index
                );

                resetMemory();

                for (int j = 0; j < chunkSize; ++j) {
                    ask(representatives[globalPosition + j]);
                }

                for (int j = 0; j < take; ++j) {
                    int cafe = unresolved[index + j];
                    if (!ask(cafe)) {
                        nextUnresolved.push_back(cafe);
                    }
                }

                index += take;
            }

            unresolved = move(nextUnresolved);
            globalPosition += chunkSize;
        }

        for (int cafe : unresolved) {
            representatives.push_back(cafe);
        }
    }

    cout << "! " << representatives.size() << endl;
    return 0;
}
