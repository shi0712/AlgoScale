#include <bits/stdc++.h>
using namespace std;

struct Item {
    int L, U;
    int l, s;
    bool known;
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, m;
    cin >> n >> m;

    vector<vector<int>> G(n + 1, vector<int>(n + 1));
    for (int i = 1; i <= n; ++i) {
        for (int j = 1; j <= n; ++j) {
            cin >> G[i][j];
        }
    }

    const int INF = 1'000'000'000;
    const int Dmax = 2 * n - 2;

    // F[k] = maximum in row 1, columns 1..k
    vector<int> F(n + 1, 0);
    for (int i = 1; i <= n; ++i) {
        F[i] = max(F[i - 1], G[1][i]);
    }

    // M[s] = minimum possible maximum on P1..Ps (after the forced first move)
    vector<int> M(Dmax + 1, INF);
    M[0] = 0;

    vector<vector<int>> best(n + 1, vector<int>(n + 1, INF));
    for (int x = 2; x <= n; ++x) {
        for (int y = 1; y <= n; ++y) {
            if (x == 2 && y == 1) {
                best[x][y] = G[x][y];
            } else {
                int val = INF;
                if (x > 2) val = min(val, best[x - 1][y]);
                if (y > 1) val = min(val, best[x][y - 1]);
                if (val < INF) {
                    best[x][y] = max(G[x][y], val);
                }
            }

            int d = x + y - 2;
            if (best[x][y] < M[d]) M[d] = best[x][y];
        }
    }

    // Diagonal maxima using only rows >= 2
    vector<int> diagMax(Dmax + 1, -1);
    for (int x = 2; x <= n; ++x) {
        for (int y = 1; y <= n; ++y) {
            int d = x + y - 2;
            diagMax[d] = max(diagMax[d], G[x][y]);
        }
    }

    vector<int> prefDiag(Dmax + 1, 0);
    for (int d = 1; d <= Dmax; ++d) {
        prefDiag[d] = max(prefDiag[d - 1], diagMax[d]);
    }

    // Range maxima over diagonal labels
    vector<vector<int>> rmax(Dmax + 1, vector<int>(Dmax + 1, 0));
    for (int l = 1; l <= Dmax; ++l) {
        int cur = 0;
        for (int r = l; r <= Dmax; ++r) {
            cur = max(cur, diagMax[r]);
            rmax[l][r] = cur;
        }
    }

    vector<Item> items;
    items.reserve(n * (2 * n - 1));

    vector<vector<int>> dpPrev(n + 1, vector<int>(n + 1, INF));
    vector<vector<int>> dpCur(n + 1, vector<int>(n + 1, INF));

    for (int len = 1; len <= n; ++len) {
        vector<int> minD(Dmax + 1, INF);

        for (int x = 2; x <= n; ++x) {
            for (int y = 1; y <= n; ++y) {
                int v;
                if (len == 1) {
                    v = G[x][y];
                } else {
                    int val = INF;
                    if (x > 2) val = min(val, dpPrev[x - 1][y]);
                    if (y > 1) val = min(val, dpPrev[x][y - 1]);

                    if (val >= INF) v = INF;
                    else v = max(G[x][y], val);
                }

                dpCur[x][y] = v;
                int d = x + y - 2;
                if (v < minD[d]) minD[d] = v;
            }
        }

        int l = len;

        for (int s = 0; s <= Dmax; ++s) {
            int L, U;

            if (s == 0) {
                L = F[l];
                U = F[l];
            } else if (s < l - 1) {
                int k = l - s;
                L = max(F[k], M[s]);
                U = max(F[k], prefDiag[s]);
            } else if (s == l - 1) {
                L = max(G[1][1], M[s]);
                U = max(G[1][1], prefDiag[s]);
            } else {
                L = minD[s];
                int left = s - l + 1;
                U = rmax[left][s];
            }

            bool known = (L == U);
            items.push_back({L, U, l, s, known});
        }

        if (len < n) {
            swap(dpPrev, dpCur);
        }
    }

    sort(items.begin(), items.end(), [](const Item &a, const Item &b) {
        if (a.L != b.L) return a.L < b.L;
        if (a.known != b.known) return a.known > b.known;
        if (a.U != b.U) return a.U < b.U;
        if (a.l != b.l) return a.l < b.l;
        return a.s < b.s;
    });

    priority_queue<int> pq;

    auto addValue = [&](int val) {
        if ((int)pq.size() < m) {
            pq.push(val);
        } else if (val < pq.top()) {
            pq.pop();
            pq.push(val);
        }
    };

    for (const Item &it : items) {
        if ((int)pq.size() == m && it.L >= pq.top()) {
            break;
        }

        if (it.known) {
            addValue(it.L);
        } else {
            cout << "? " << it.l << ' ' << (it.s + 1) << '\n';
            cout.flush();

            int res;
            cin >> res;
            if (!cin || res < 0) return 0;

            addValue(res);
        }
    }

    vector<int> ans;
    ans.reserve(m);
    while (!pq.empty()) {
        ans.push_back(pq.top());
        pq.pop();
    }
    sort(ans.begin(), ans.end());

    cout << '!';
    for (int x : ans) {
        cout << ' ' << x;
    }
    cout << '\n';
    cout.flush();

    return 0;
}
