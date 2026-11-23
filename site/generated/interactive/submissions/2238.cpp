#include <bits/stdc++.h>
using namespace std;

using int64 = long long;

int n;

int64 ask(const vector<int>& nodes) {
    cout << "? 1 " << nodes.size();
    for (int u : nodes) cout << ' ' << u;
    cout << endl;

    int64 response;
    if (!(cin >> response)) exit(0);
    return response;
}

void flipNode1() {
    cout << "? 2 1" << endl;
}

void printAnswer(const vector<int>& a) {
    cout << "!";
    for (int i = 1; i <= n; ++i) cout << ' ' << a[i];
    cout << endl;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    if (!(cin >> n)) return 0;

    for (int i = 0; i < n - 1; ++i) {
        int u, v;
        cin >> u >> v;
    }

    int64 b = ask(vector<int>{1});

    int d = 0;
    while ((1LL << (d + 1)) - 1 <= n) ++d;

    vector<int> s(n + 1, 0);

    for (int left = 2; left <= n; left += d) {
        int g = min(d, n - left + 1);
        int W = (1 << g) - 1;

        vector<int> query;
        query.reserve(W);

        for (int j = 0; j < g; ++j) {
            int u = left + j;
            int weight = 1 << j;
            for (int rep = 0; rep < weight; ++rep) {
                query.push_back(u);
            }
        }

        int64 response = ask(query);
        int64 encoded = (response - (b - 1) * W) / 2;

        for (int j = 0; j < g; ++j) {
            int bit = encoded & 1;
            s[left + j] = bit ? 1 : -1;
            encoded >>= 1;
        }
    }

    vector<int> answer(n + 1);

    if (b % 2 != 0) {
        answer[1] = static_cast<int>(b);
        for (int u = 2; u <= n; ++u) answer[u] = s[u];
        printAnswer(answer);
        return 0;
    }

    if (b == 0) {
        bool mixed = false;
        for (int u = 3; u <= n; ++u) {
            if (s[u] != s[2]) {
                mixed = true;
                break;
            }
        }

        if (!mixed) {
            int z = s[2];
            answer[1] = -z;
            for (int u = 2; u <= n; ++u) answer[u] = s[u];
            printAnswer(answer);
            return 0;
        }

        flipNode1();
        int z = static_cast<int>(ask(vector<int>{1}) / 2);

        answer[1] = z;
        for (int u = 2; u <= n; ++u) answer[u] = s[u];
        printAnswer(answer);
        return 0;
    }

    int x = static_cast<int>(b / 2);
    vector<int> candidates;

    for (int u = 2; u <= n; ++u) {
        if (s[u] == -x) candidates.push_back(u);
    }

    if (candidates.size() == 1) {
        fill(answer.begin(), answer.end(), x);
        printAnswer(answer);
        return 0;
    }

    flipNode1();

    int m = static_cast<int>(candidates.size());
    int bestBase = 2;
    int bestUsed = INT_MAX;
    vector<int> bestSums;

    for (int base = 2; base <= m; ++base) {
        int digits = 1;
        int64 power = base;
        while (power < m) {
            power *= base;
            ++digits;
        }

        vector<int> sums(digits, 0);
        for (int code = 0; code < m; ++code) {
            int value = code;
            for (int pos = 0; pos < digits; ++pos) {
                sums[pos] += value % base;
                value /= base;
            }
        }

        bool feasible = true;
        int used = 0;
        for (int sum : sums) {
            if (sum > n) feasible = false;
            if (sum > 0) ++used;
        }

        if (feasible && used < bestUsed) {
            bestUsed = used;
            bestBase = base;
            bestSums = sums;
        }
    }

    int digits = static_cast<int>(bestSums.size());
    vector<int64> powers(digits + 1, 1);
    for (int i = 0; i < digits; ++i) {
        powers[i + 1] = powers[i] * bestBase;
    }

    int64 rootCode = 0;

    for (int pos = 0; pos < digits; ++pos) {
        int K = bestSums[pos];
        if (K == 0) continue;

        vector<int> query;
        query.reserve(K);

        for (int i = 0; i < m; ++i) {
            int digit = static_cast<int>((i / powers[pos]) % bestBase);
            for (int rep = 0; rep < digit; ++rep) {
                query.push_back(candidates[i]);
            }
        }

        int64 response = ask(query);
        int64 normalized = static_cast<int64>(x) * response;
        int rootDigit = static_cast<int>((normalized + K) / 2);
        rootCode += rootDigit * powers[pos];
    }

    int root = candidates[static_cast<int>(rootCode)];

    fill(answer.begin(), answer.end(), x);
    answer[1] = -x;
    for (int u : candidates) answer[u] = -x;
    answer[root] = x;

    printAnswer(answer);
    return 0;
}
