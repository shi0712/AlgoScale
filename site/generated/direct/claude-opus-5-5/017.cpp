#include <bits/stdc++.h>
using namespace std;
typedef long long ll;

int main() {
    int n, m;
    scanf("%d %d", &n, &m);
    vector<int> t(n);
    for (auto &x : t) scanf("%d", &x);
    vector<vector<ll>> a(n, vector<ll>(m)), d(n, vector<ll>(m));
    for (int i = 0; i < n; i++)
        for (int j = 0; j < m; j++) scanf("%lld", &a[i][j]);
    for (int i = 0; i < n; i++)
        for (int j = 0; j < m; j++) scanf("%lld", &d[i][j]);

    int full = 1 << m;
    vector<ll> A(full), D(full), F(full), sumU(full, 0), trumpNet(full, 0);
    vector<char> bad(full, 0);

    for (int i = 0; i < n; i++) {
        A[0] = D[0] = F[0] = 0;
        for (int S = 1; S < full; S++) {
            int j0 = __builtin_ctz(S);
            int Sp = S & (S - 1);
            A[S] = A[Sp] + a[i][j0];
            D[S] = D[Sp] + d[i][j0];
            F[S] = max(F[Sp], A[S] - D[Sp]);
            if (t[i]) {
                if (F[S] > 0) bad[S] = 1;
                trumpNet[S] += D[S] - A[S];
            } else {
                sumU[S] += F[S];
            }
        }
    }

    int covered = 0;
    for (int S = 1; S < full; S++) {
        if (!bad[S] && sumU[S] <= trumpNet[S]) covered |= S;
    }

    ll ans = 0;
    for (int j = 0; j < m; j++) {
        if (covered >> j & 1) continue;
        ll tot = 0;
        for (int i = 0; i < n; i++) tot += a[i][j];
        ans += tot;
    }
    printf("%lld\n", ans);
    return 0;
}
