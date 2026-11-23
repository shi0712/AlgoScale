#include <bits/stdc++.h>
using namespace std;

using int64 = long long;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int N;
    if (!(cin >> N)) return 0;

    vector<vector<int64>> memo(N, vector<int64>(N, -1));
    for (int i = 0; i < N; ++i) memo[i][i] = 0;

    auto getDist = [&](int a, int b) -> int64 {
        if (memo[a][b] != -1) return memo[a][b];
        cout << "? " << a << ' ' << b << '\n';
        cout.flush();

        int64 value;
        if (!(cin >> value)) exit(0);
        if (value == -1) exit(0);

        memo[a][b] = memo[b][a] = value;
        return value;
    };

    vector<int64> d0(N, 0), da(N, 0), db(N, 0);

    for (int i = 1; i < N; ++i) d0[i] = getDist(0, i);

    int A = 0;
    for (int i = 1; i < N; ++i) {
        if (d0[i] > d0[A]) A = i;
    }

    da[A] = 0;
    da[0] = d0[A];
    for (int i = 1; i < N; ++i) {
        if (i != A) da[i] = getDist(A, i);
    }

    int B = 0;
    for (int i = 1; i < N; ++i) {
        if (da[i] > da[B]) B = i;
    }

    const int64 D = da[B];
    db[B] = 0;
    db[A] = D;

    if (B == 0) {
        db = d0;
    } else {
        db[0] = d0[B];
        for (int i = 1; i < N; ++i) {
            if (i != A && i != B) db[i] = getDist(B, i);
        }
    }

    vector<int64> x(N), h(N);
    set<int64> positions;

    for (int i = 0; i < N; ++i) {
        x[i] = (D + da[i] - db[i]) / 2;
        h[i] = da[i] - x[i];
        if (h[i] > 0) positions.insert(x[i]);
    }

    int64 R = LLONG_MAX;
    for (int64 c : positions) R = min(R, max(c, D - c));

    struct HubInfo {
        int64 c;
        int left = 0, right = 0;
        vector<int> middle;
    };

    vector<HubInfo> hubs;
    for (int64 c : positions) {
        if (max(c, D - c) != R) continue;

        HubInfo info;
        info.c = c;
        for (int i = 0; i < N; ++i) {
            if (x[i] < c) ++info.left;
            else if (x[i] > c) ++info.right;
            else info.middle.push_back(i);
        }

        auto it = find(info.middle.begin(), info.middle.end(), 0);
        if (it != info.middle.end()) iter_swap(info.middle.begin(), it);

        hubs.push_back(std::move(info));
    }

    const int T = N / 2;
    bool balanced = false;

    for (const auto &info : hubs) {
        if (info.left <= T && info.right <= T &&
            (int)info.middle.size() <= T) {
            balanced = true;
            break;
        }
    }

    auto sameComponent = [&](int a, int b) -> bool {
        return getDist(a, b) < h[a] + h[b];
    };

    auto noLargeComponent = [&](const vector<int> &towns) -> bool {
        int candidate = -1;
        int count = 0;

        for (int v : towns) {
            if (count == 0) {
                candidate = v;
                count = 1;
            } else if (sameComponent(candidate, v)) {
                ++count;
            } else {
                --count;
            }
        }

        if (count == 0) return true;

        int matches = 1;
        int remaining = (int)towns.size() - 1;

        for (int v : towns) {
            if (v == candidate) continue;

            if (sameComponent(candidate, v)) ++matches;
            --remaining;

            if (matches > T) return false;
            if (matches + remaining <= T) return true;
        }

        return true;
    };

    if (!balanced) {
        for (const auto &info : hubs) {
            if (info.left > T || info.right > T) continue;
            if ((int)info.middle.size() <= T) continue;

            if (noLargeComponent(info.middle)) {
                balanced = true;
                break;
            }
        }
    }

    cout << "! " << (balanced ? R : -R) << '\n';
    cout.flush();
    return 0;
}
