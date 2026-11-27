#include <bits/stdc++.h>
using namespace std;

char ask(int cafe) {
    cout << "? " << cafe << '\n' << flush;

    char response;
    if (!(cin >> response)) exit(0);

    if (response == 'E' || response == '-') exit(0);
    if (response != 'Y' && response != 'N') exit(0);

    return response;
}

void resetMemory() {
    cout << "R\n" << flush;
}

int chooseChunkSize(int globalSize, int localSize, int k) {
    long long bestCost = LLONG_MAX;
    int bestChunkSize = 1;

    for (int x = 1; x <= localSize && x < k; ++x) {
        long long cost = 1LL * ((localSize + x - 1) / x) * globalSize;

        for (int offset = 0; offset < localSize; offset += x) {
            int remaining = min(x, localSize - offset);
            int capacity = k - remaining;
            int groups = (globalSize + capacity - 1) / capacity;
            cost += 1LL * remaining * groups;
        }

        if (cost <= bestCost) {
            bestCost = cost;
            bestChunkSize = x;
        }
    }

    return bestChunkSize;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, k;
    if (!(cin >> n >> k)) return 0;

    if (k == 1) {
        vector<int> representatives;

        for (int cafe = 1; cafe <= n; ++cafe) {
            bool found = false;

            for (int representative : representatives) {
                resetMemory();
                ask(representative);
                if (ask(cafe) == 'Y') {
                    found = true;
                    break;
                }
            }

            if (!found) representatives.push_back(cafe);
        }

        cout << "! " << representatives.size() << '\n' << flush;
        return 0;
    }

    vector<int> globalRepresentatives;

    for (int start = 0; start < n; start += k) {
        int finish = min(n, start + k);
        vector<int> localRepresentatives;

        resetMemory();
        for (int cafe = start + 1; cafe <= finish; ++cafe) {
            if (ask(cafe) == 'N') {
                localRepresentatives.push_back(cafe);
            }
        }

        if (globalRepresentatives.empty()) {
            globalRepresentatives = move(localRepresentatives);
            continue;
        }

        int globalSize = static_cast<int>(globalRepresentatives.size());
        int localSize = static_cast<int>(localRepresentatives.size());
        int chunkSize = chooseChunkSize(globalSize, localSize, k);

        vector<char> matched(localSize, false);

        for (int offset = 0; offset < localSize; offset += chunkSize) {
            int chunkEnd = min(localSize, offset + chunkSize);
            vector<int> candidates(localRepresentatives.begin() + offset,
                                   localRepresentatives.begin() + chunkEnd);

            int globalPosition = 0;

            while (globalPosition < globalSize && !candidates.empty()) {
                int candidateCount = static_cast<int>(candidates.size());
                int groupCapacity = k - candidateCount;
                int groupSize = min(groupCapacity, globalSize - globalPosition);

                resetMemory();
                for (int j = 0; j < groupSize; ++j) {
                    ask(globalRepresentatives[globalPosition + j]);
                }
                globalPosition += groupSize;

                vector<int> remaining;
                remaining.reserve(candidates.size());

                for (int cafe : candidates) {
                    if (ask(cafe) == 'Y') {
                        matched[cafe - localRepresentatives.front()] = true;
                    } else {
                        remaining.push_back(cafe);
                    }
                }

                candidates.swap(remaining);
            }
        }

        for (int i = 0; i < localSize; ++i) {
            if (!matched[i]) {
                globalRepresentatives.push_back(localRepresentatives[i]);
            }
        }
    }

    cout << "! " << globalRepresentatives.size() << '\n' << flush;
    return 0;
}
