#include <bits/stdc++.h>
using namespace std;

static signed char cacheF[2005][2005];

int f(int l, int r) {
    if (l >= r) return 0;
    if (cacheF[l][r] != -1) return cacheF[l][r];
    printf("? %d %d\n", l, r);
    fflush(stdout);
    int x;
    if (scanf("%d", &x) != 1 || x == -1) exit(0);
    cacheF[l][r] = x;
    return x;
}

// returns 1 if p_i > p_r, for i < r
int gt(int i, int r) {
    if (r == i + 1) return f(i, r);
    return f(i, r) ^ f(i + 1, r) ^ f(i, r - 1) ^ f(i + 1, r - 1);
}

int main() {
    int n;
    if (scanf("%d", &n) != 1) return 0;
    memset(cacheF, -1, sizeof(cacheF));
    vector<int> order; // indices sorted by value ascending
    for (int r = 1; r <= n; r++) {
        int lo = 0, hi = order.size();
        while (lo < hi) {
            int mid = (lo + hi) / 2;
            if (gt(order[mid], r)) hi = mid;
            else lo = mid + 1;
        }
        order.insert(order.begin() + lo, r);
    }
    vector<int> p(n + 1);
    for (int k = 0; k < n; k++) p[order[k]] = k + 1;
    printf("!");
    for (int i = 1; i <= n; i++) printf(" %d", p[i]);
    printf("\n");
    fflush(stdout);
    return 0;
}
