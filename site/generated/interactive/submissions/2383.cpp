#include <bits/stdc++.h>
using namespace std;

int main() {
    int n;
    if (!(cin >> n)) return 0;
    for (int i = 0; i < n - 1; i++) {
        int a, b;
        cin >> a >> b;
    }
    auto query = [&](vector<int> s) -> long long {
        cout << "? 1 " << s.size();
        for (int x : s) cout << " " << x;
        cout << "\n";
        cout.flush();
        long long r;
        if (!(cin >> r)) exit(0);
        if (r == -1) exit(0);
        return r;
    };
    long long A = query({1});
    vector<long long> f(n + 1, 0);
    for (int u = 2; u <= n; u++) f[u] = query({u});
    vector<int> ans(n + 1, 0);
    if (A % 2 != 0) {
        ans[1] = (int)A;
        for (int u = 2; u <= n; u++) ans[u] = (int)(f[u] - A);
    } else {
        vector<int> d(n + 1, 0);
        for (int u = 2; u <= n; u++) d[u] = (int)(f[u] - A);
        for (int u = 2; u <= n; u++) ans[u] = d[u];
        if (A == 0) {
            bool allSame = true;
            for (int u = 3; u <= n; u++)
                if (d[u] != d[2]) allSame = false;
            int v1;
            if (allSame) {
                v1 = -d[2];
                ans[1] = v1;
            } else {
                long long S = 0;
                vector<int> all;
                for (int u = 2; u <= n; u++) {
                    S += f[u];
                    all.push_back(u);
                }
                cout << "? 2 1\n";
                cout.flush();
                long long T = query(all);
                long long diff = T - S;
                // diff = -2*v1*(n-2)
                v1 = (diff < 0) ? 1 : -1;
                ans[1] = -v1; // final value after flip
            }
        } else {
            int v1 = (int)(A / 2);
            vector<int> cand;
            for (int u = 2; u <= n; u++)
                if (d[u] == -v1) cand.push_back(u);
            int r;
            bool flipped = false;
            if (cand.size() == 1) {
                r = cand[0];
            } else {
                cout << "? 2 1\n";
                cout.flush();
                flipped = true;
                long long delta = -2LL * v1;
                while (cand.size() > 1) {
                    size_t h = cand.size() / 2;
                    vector<int> S(cand.begin(), cand.begin() + h);
                    long long old = 0;
                    for (int u : S) old += f[u];
                    long long T = query(S);
                    long long diff = T - old;
                    if (diff == delta * (long long)S.size()) {
                        cand = vector<int>(cand.begin() + h, cand.end());
                    } else {
                        cand = S;
                    }
                }
                r = cand[0];
            }
            ans[r] = -d[r];
            ans[1] = flipped ? -v1 : v1;
        }
    }
    cout << "!";
    for (int i = 1; i <= n; i++) cout << " " << ans[i];
    cout << "\n";
    cout.flush();
    return 0;
}
