#include <bits/stdc++.h>
using namespace std;

struct PairPositions {
    int a, b;
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n)) return 0;
    if (n < 2 || (n & 1)) return 0;

    const int m = n / 2;

    auto send = [](const string& s) {
        if (fwrite(s.data(), 1, s.size(), stdout) == 0) exit(0);
        if (fflush(stdout) != 0) exit(0);
    };

    auto readResponse = []() -> int {
        int response;
        if (!(cin >> response)) exit(0);
        if (response == -1) exit(0);
        if (response != 0 && response != 1) exit(0);
        return response;
    };

    auto askExcept = [&](const vector<int>& positions, int skipped) -> int {
        string query = "? " + to_string(positions.size() - 1);
        query.reserve(query.size() + 5 * positions.size());

        for (int i = 0; i < (int)positions.size(); ++i) {
            if (i == skipped) continue;
            query.push_back(' ');
            query += to_string(positions[i]);
        }
        query.push_back('\n');
        send(query);
        return readResponse();
    };

    auto askTwo = [&](int x, int y) -> int {
        string query = "? 2 " + to_string(x) + " " + to_string(y) + "\n";
        send(query);
        return readResponse();
    };

    mt19937_64 rng(
        chrono::steady_clock::now().time_since_epoch().count()
    );

    vector<int> remaining(n);
    iota(remaining.begin(), remaining.end(), 1);

    vector<PairPositions> pairs(m + 1);

    for (int layer = 1; layer <= m; ++layer) {
        shuffle(remaining.begin(), remaining.end(), rng);

        vector<int> yes;
        yes.reserve(2);

        for (int i = 0; i + 1 < (int)remaining.size(); ++i) {
            if (askExcept(remaining, i) == 1) {
                yes.push_back(remaining[i]);
                if (yes.size() == 2) break;
            }
        }

        if (yes.size() == 1) {
            yes.push_back(remaining.back());
        }

        if (yes.size() != 2) exit(0);

        pairs[layer] = {yes[0], yes[1]};

        remaining.erase(find(remaining.begin(), remaining.end(), yes[0]));
        remaining.erase(find(remaining.begin(), remaining.end(), yes[1]));
    }

    int anchorLayer = -1;
    for (int layer = 1; layer <= m; ++layer) {
        if (pairs[layer].a == 1 || pairs[layer].b == 1) {
            anchorLayer = layer;
            break;
        }
    }
    if (anchorLayer == -1) exit(0);

    vector<int> representative(m + 1);
    vector<int> counterpart(m + 1);

    for (int layer = 1; layer <= m; ++layer) {
        if (layer == anchorLayer) {
            representative[layer] = 1;
            counterpart[layer] =
                (pairs[layer].a == 1 ? pairs[layer].b : pairs[layer].a);
        } else {
            representative[layer] = pairs[layer].a;
            counterpart[layer] = pairs[layer].b;
        }
    }

    vector<int> flipped(m + 1, 0);

    for (int layer = 1; layer <= m; ++layer) {
        if (layer == anchorLayer) continue;

        int response = askTwo(1, representative[layer]);
        int expectedParity = (anchorLayer & 1) ^ (layer & 1);

        if (response == 1) {
            flipped[layer] = expectedParity;
        } else {
            flipped[layer] = expectedParity ^ 1;
        }
    }

    vector<int> permutation(n + 1);

    for (int layer = 1; layer <= m; ++layer) {
        int lowValue = layer;
        int highValue = n + 1 - layer;

        if (flipped[layer] == 0) {
            permutation[representative[layer]] = lowValue;
            permutation[counterpart[layer]] = highValue;
        } else {
            permutation[representative[layer]] = highValue;
            permutation[counterpart[layer]] = lowValue;
        }
    }

    string answer = "!";
    answer.reserve(5 * n + 2);
    for (int i = 1; i <= n; ++i) {
        answer.push_back(' ');
        answer += to_string(permutation[i]);
    }
    answer.push_back('\n');
    send(answer);

    return 0;
}
