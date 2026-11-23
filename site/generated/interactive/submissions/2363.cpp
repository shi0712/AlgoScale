#include <bits/stdc++.h>
using namespace std;

int main() {
    int n, t;
    if (scanf("%d %d", &n, &t) != 2) return 0;
    vector<int> lz(4 * (size_t)n + 4, -1);
    for (int round = 0; round < t; round++) {
        int k;
        if (scanf("%d", &k) != 1) return 0;
        int node = 1, l = 1, r = n;
        vector<int> wentLeft;
        while (l < r) {
            int mid = (l + r) / 2;
            if (lz[node] < 0) {
                printf("? %d %d\n", l, mid);
                fflush(stdout);
                int s;
                if (scanf("%d", &s) != 1) return 0;
                if (s < 0) return 0;
                lz[node] = (mid - l + 1) - s;
            }
            if (k <= lz[node]) {
                wentLeft.push_back(node);
                node = node * 2;
                r = mid;
            } else {
                k -= lz[node];
                node = node * 2 + 1;
                l = mid + 1;
            }
        }
        for (int v : wentLeft) lz[v]--;
        printf("! %d\n", l);
        fflush(stdout);
    }
    return 0;
}
