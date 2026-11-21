#include <bits/stdc++.h>
using namespace std;

int N;
vector<vector<long long>> memo;

long long query(int i, int j) {
    if (i == j) return 0;
    if (memo[i][j] != -1) return memo[i][j];

    cout << "? " << i << " " << j << '\n';
    cout.flush();

    long long x;
    cin >> x;
    if (!cin) exit(0);
    if (x < 0) exit(0);

    memo[i][j] = memo[j][i] = x;
    return x;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    cin >> N;
    if (!cin) return 0;

    memo.assign(N, vector<long long>(N, -1));
    for (int i = 0; i < N; ++i) memo[i][i] = 0;

    vector<long long> d0(N, 0);
    for (int i = 1; i < N; ++i) {
        d0[i] = query(0, i);
    }

    int A = 0;
    for (int i = 1; i < N; ++i) {
        if (d0[i] > d0[A]) A = i;
    }

    vector<long long> dA(N);
    for (int i = 0; i < N; ++i) {
        dA[i] = (i == A ? 0 : query(A, i));
    }

    int B = 0;
    for (int i = 1; i < N; ++i) {
        if (dA[i] > dA[B]) B = i;
    }

    long long D = dA[B];

    vector<long long> dB(N);
    for (int i = 0; i < N; ++i) {
        dB[i] = (i == B ? 0 : query(B, i));
    }

    vector<long long> p(N), h(N);
    map<long long, vector<int>> groups;

    for (int i = 0; i < N; ++i) {
        p[i] = (dA[i] + D - dB[i]) / 2;
        h[i] = (dA[i] + dB[i] - D) / 2;
        groups[p[i]].push_back(i);
    }

    long long best = LLONG_MAX;
    vector<long long> candidates;

    for (auto &entry : groups) {
        long long s = entry.first;
        if (s > 0 && s < D) {
            long long val = max(s, D - s);
            if (val < best) {
                best = val;
                candidates.clear();
                candidates.push_back(s);
            } else if (val == best) {
                candidates.push_back(s);
            }
        }
    }

    int half = N / 2;

    auto isBalanced = [&](long long s) -> bool {
        int left = 0, right = 0;
        vector<int> group;

        for (int i = 0; i < N; ++i) {
            if (p[i] < s) ++left;
            else if (p[i] > s) ++right;
            else group.push_back(i);
        }

        if (left > half || right > half) return false;

        if ((int)group.size() <= half) return true;

        auto sameComponent = [&](int x, int y) -> bool {
            if (x == y) return true;
            long long d = query(x, y);
            return d < h[x] + h[y];
        };

        int cand = -1;
        int cnt = 0;

        for (int x : group) {
            if (cnt == 0) {
                cand = x;
                cnt = 1;
            } else {
                if (sameComponent(x, cand)) ++cnt;
                else --cnt;
            }
        }

        if (cnt == 0) return true;

        int actual = 0;
        for (int x : group) {
            if (sameComponent(x, cand)) ++actual;
        }

        return actual <= half;
    };

    bool ok = false;
    for (long long s : candidates) {
        if (isBalanced(s)) {
            ok = true;
            break;
        }
    }

    cout << "! " << (ok ? best : -best) << '\n';
    cout.flush();

    return 0;
}
