#include <bits/stdc++.h>
using namespace std;

static int n, H, BASE;
static vector<vector<int>> childs;
static vector<int> sz, ht, parentArr;
static vector<unique_ptr<unordered_map<int, long long>>> memo;
static long long answer;

long long dfs(int v, int maxSeen, int curRun) {
    // If even continuing this heavy path cannot exceed maxSeen,
    // every vertex in this subtree keeps value maxSeen.
    if (maxSeen >= curRun + ht[v]) {
        return 1LL * maxSeen * sz[v];
    }

    // Only one child: it is always optimal to continue the heavy path.
    if (childs[v].size() == 1) {
        int c = childs[v][0];
        int nr = curRun + 1;
        int nm = max(maxSeen, nr);
        return maxSeen + dfs(c, nm, nr);
    }

    if (childs[v].empty()) {
        return maxSeen;
    }

    int key = maxSeen * BASE + curRun;

    if (!memo[v]) {
        memo[v] = make_unique<unordered_map<int, long long>>();
        memo[v]->reserve(16);
    }
    auto it = memo[v]->find(key);
    if (it != memo[v]->end()) return it->second;

    long long sumLight = 0;
    long long bestDiff = 0;

    int nr = curRun + 1;
    int nm = max(maxSeen, nr);

    for (int c : childs[v]) {
        long long light = dfs(c, maxSeen, 1);
        sumLight += light;

        long long heavy = dfs(c, nm, nr);
        long long diff = heavy - light;
        if (diff > bestDiff) bestDiff = diff;
    }

    long long res = (long long)maxSeen + sumLight + bestDiff;
    (*memo[v])[key] = res;
    return res;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int T;
    cin >> T;

    while (T--) {
        int m;
        cin >> n >> m;

        childs.assign(n + 1, {});
        parentArr.assign(n + 1, 0);
        sz.assign(n + 1, 1);
        ht.assign(n + 1, 0);

        for (int i = 2; i <= n; ++i) {
            int p;
            cin >> p;
            parentArr[i] = p;
            childs[p].push_back(i);
        }

        for (int i = n; i >= 2; --i) {
            int p = parentArr[i];
            sz[p] += sz[i];
            ht[p] = max(ht[p], ht[i] + 1);
        }

        H = ht[1];
        BASE = max(3, H + 3);

        memo.clear();
        memo.resize(n + 1);

        // Root has heavy-path position 1, so maxSeen = current run = 1.
        answer = dfs(1, 1, 1);

        cout << answer << '\n';
    }

    return 0;
}
