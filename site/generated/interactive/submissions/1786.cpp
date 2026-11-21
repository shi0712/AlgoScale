#include <bits/stdc++.h>
using namespace std;

static unsigned char invpar[2005][2005];
static int qcache[2005];
int n;

int ask(int l, int r) {
    printf("? %d %d\n", l, r);
    fflush(stdout);
    int x;
    if (scanf("%d", &x) != 1 || x == -1) exit(0);
    return x;
}

int main() {
    if (scanf("%d", &n) != 1) return 0;

    vector<int> val(n + 1, 0);
    vector<int> order;
    order.reserve(n);

    order.push_back(1);
    val[1] = 0;
    invpar[1][1] = 0;

    for (int k = 2; k <= n; ++k) {
        fill(qcache, qcache + k + 1, -1);

        auto getQ = [&](int l) -> int {
            if (l >= k) return 0;
            if (qcache[l] != -1) return qcache[l];
            int res = ask(l, k);
            qcache[l] = res;
            return res;
        };

        int lo = 0, hi = k - 1;

        while (lo < hi) {
            int mid = (lo + hi) / 2;
            int m = order[mid];

            int q1 = getQ(m);
            int q2 = getQ(m + 1);

            int a = invpar[m][k - 1];
            int b = (m + 1 <= k - 1) ? invpar[m + 1][k - 1] : 0;

            int greater = q1 ^ q2 ^ a ^ b;

            if (greater) hi = mid;
            else lo = mid + 1;
        }

        for (int i = 1; i < k; ++i) {
            if (val[i] >= lo) ++val[i];
        }

        val[k] = lo;
        order.insert(order.begin() + lo, k);

        int parity = 0;
        for (int i = k - 1; i >= 1; --i) {
            parity ^= (val[i] > val[k]) ? 1 : 0;
            invpar[i][k] = invpar[i][k - 1] ^ (unsigned char)parity;
        }
        invpar[k][k] = 0;
    }

    printf("!");
    for (int i = 1; i <= n; ++i) {
        printf(" %d", val[i] + 1);
    }
    printf("\n");
    fflush(stdout);

    return 0;
}
