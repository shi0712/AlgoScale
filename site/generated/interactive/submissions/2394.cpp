#include <bits/stdc++.h>
using namespace std;

int main() {
    int n;
    if (!(cin >> n)) return 0;
    vector<char> removed(n + 1, 0);
    vector<pair<int,int>> ops;
    int k = n;
    while (k > 2) {
        printf("? %d\n", k - 2);
        fflush(stdout);
        int v, u;
        if (!(cin >> v >> u)) return 0;
        if (v == -1) return 0;
        if (v == 0) return 0;
        removed[v] = 1;
        ops.push_back({v, u});
        k--;
    }
    deque<int> P;
    for (int i = 1; i <= n; i++) if (!removed[i]) P.push_back(i);
    for (int i = (int)ops.size() - 1; i >= 0; i--) {
        int v = ops[i].first, u = ops[i].second;
        if (P.front() != u) P.push_front(v);
        else P.push_back(v);
    }
    printf("!");
    for (int x : P) printf(" %d", x);
    printf("\n");
    fflush(stdout);
    return 0;
}
