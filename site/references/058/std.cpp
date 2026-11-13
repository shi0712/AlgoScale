#include <bits/stdc++.h>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, q;
    if (!(cin >> n >> q)) return 0;
    vector<int> a(n);
    int M = 0;
    for (int &x : a) {
        cin >> x;
        M = max(M, x);
    }
    vector<unsigned long long> query(q);
    for (auto &x : query) cin >> x;

    vector<int> suffix(M + 2, 0);
    for (int x : a) ++suffix[x];
    for (int x = M - 1; x >= 1; --x) suffix[x] += suffix[x + 1];

    vector<long long> blocks(M + 1, 0);
    for (int b = 1; b <= M; ++b)
        for (int x = b; x <= M; x += b)
            blocks[b] += suffix[x];

    for (unsigned long long k : query) {
        unsigned long long b = (static_cast<unsigned long long>(M) + k - 1) / k;
        cout << (blocks[static_cast<int>(b)] >= static_cast<long long>(k) ? "YES\n" : "NO\n");
    }
    return 0;
}
