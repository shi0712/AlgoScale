#include <bits/stdc++.h>
using namespace std;

int main() {
    int n;
    if (scanf("%d", &n) != 1) return 0;
    if (n <= 0) return 0;
    if (n == 1) {
        printf("! 1\n");
        fflush(stdout);
        return 0;
    }
    int B = 16;
    if (B > n) B = n;
    vector<signed char> S(n*n, -1), cmp(n*n, -1);
    auto get = [&](int l, int r) -> int {
        if (l >= r) return 0;
        int id = l * n + r;
        signed char v = S[id];
        if (v != -1) return (int)v;
        printf("? %d %d\n", l + 1, r + 1);
        fflush(stdout);
        int ans;
        if (scanf("%d", &ans) != 1) exit(0);
        if (ans == -1) exit(0);
        S[id] = (signed char)ans;
        return ans;
    };
    auto greater = [&](int i, int j) -> int {
        if (i == j) return 0;
        if (i > j) swap(i, j);
        int id = i * n + j;
        signed char v = cmp[id];
        if (v != -1) return (int)v;
        int ans = get(i, j) ^ get(i, j - 1) ^ get(i + 1, j) ^ get(i + 1, j - 1);
        cmp[id] = (signed char)ans;
        return ans;
    };
    auto less = [&](int i, int j) -> bool {
        if (i == j) return false;
        if (i < j) return !greater(i, j);
        return greater(j, i);
    };
    vector<vector<int>> runs;
    runs.reserve((n + B - 1) / B);
    for (int s = 0; s < n; s += B) {
        int e = min(n, s + B);
        for (int l = s; l < e; ++l)
            for (int r = l + 1; r < e; ++r)
                get(l, r);
        vector<int> block(e - s);
        iota(block.begin(), block.end(), s);
        sort(block.begin(), block.end(), less);
        runs.push_back(move(block));
    }
    auto merge_runs = [&](const vector<int>& a, const vector<int>& b) {
        vector<int> c;
        c.reserve(a.size() + b.size());
        size_t i = 0, j = 0;
        while (i < a.size() && j < b.size()) {
            if (less(a[i], b[j])) c.push_back(a[i++]);
            else c.push_back(b[j++]);
        }
        while (i < a.size()) c.push_back(a[i++]);
        while (j < b.size()) c.push_back(b[j++]);
        return c;
    };
    while (runs.size() > 1) {
        vector<vector<int>> next;
        next.reserve((runs.size() + 1) / 2);
        for (size_t i = 0; i < runs.size(); i += 2) {
            if (i + 1 < runs.size()) {
                next.push_back(merge_runs(runs[i], runs[i + 1]));
            } else {
                next.push_back(move(runs[i]));
            }
        }
        runs.swap(next);
    }
    vector<int> p(n);
    for (int rank = 0; rank < n; ++rank) {
        p[runs[0][rank]] = rank + 1;
    }
    printf("!");
    for (int i = 0; i < n; ++i) {
        printf(" %d", p[i]);
    }
    printf("\n");
    fflush(stdout);
    return 0;
}
