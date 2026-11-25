#include <bits/stdc++.h>
using namespace std;
typedef long long ll;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int N;
    if (!(cin >> N)) return 0;

    vector<vector<ll>> dist(N, vector<ll>(N, -1));
    for (int i = 0; i < N; ++i) dist[i][i] = 0;

    auto query = [&](int i, int j) -> ll {
        if (i == j) return 0;
        if (dist[i][j] != -1) return dist[i][j];
        cout << "? " << i << " " << j << endl;
        ll res;
        if (!(cin >> res)) exit(0);
        dist[i][j] = dist[j][i] = res;
        return res;
    };

    // Find one endpoint A of a diameter.
    int A = 0;
    ll maxd = -1;
    for (int i = 1; i < N; ++i) {
        ll d = query(0, i);
        if (d > maxd) {
            maxd = d;
            A = i;
        }
    }

    // Find the other endpoint B.
    int B = 0;
    ll D = -1;
    for (int i = 0; i < N; ++i) {
        if (i == A) continue;
        ll d = query(A, i);
        if (d > D) {
            D = d;
            B = i;
        }
    }
    D = query(A, B);

    // Distances from B to all towns.
    for (int i = 0; i < N; ++i) {
        if (i != A && i != B) query(B, i);
    }

    // Projection coordinate and height for every town.
    vector<ll> p(N), h(N);
    p[A] = 0; h[A] = 0;
    p[B] = D; h[B] = 0;
    for (int i = 0; i < N; ++i) {
        if (i == A || i == B) continue;
        ll dA = dist[A][i];
        ll dB = dist[B][i];
        ll num = dA + D - dB;
        p[i] = num / 2;
        h[i] = dA - p[i];
    }

    // All internal vertices on the diameter path.
    set<ll> posSet;
    for (int i = 0; i < N; ++i) {
        if (i == A || i == B) continue;
        posSet.insert(p[i]);
    }

    ll R = LLONG_MAX;
    vector<ll> candidates;
    for (ll pos : posSet) {
        ll ecc = max(pos, D - pos);
        if (ecc < R) {
            R = ecc;
            candidates.clear();
            candidates.push_back(pos);
        } else if (ecc == R) {
            candidates.push_back(pos);
        }
    }

    int half = N / 2;

    for (ll pos : candidates) {
        int L = 0, Rg = 0;
        vector<int> M;
        for (int i = 0; i < N; ++i) {
            if (p[i] < pos) ++L;
            else if (p[i] > pos) ++Rg;
            else M.push_back(i);
        }

        bool balanced = (L <= half && Rg <= half);

        if (balanced && (int)M.size() > half) {
            // Majority vote for a component with more than half of all towns.
            int cand = -1;
            int cnt = 0;
            for (int x : M) {
                if (cnt == 0) {
                    cand = x;
                    cnt = 1;
                } else {
                    bool same;
                    if (cand == x) {
                        same = true;
                    } else {
                        ll dij = query(cand, x);
                        same = (dij < h[cand] + h[x]);
                    }
                    if (same) ++cnt;
                    else --cnt;
                }
            }

            if (cand != -1) {
                int real = 0;
                for (int x : M) {
                    if (x == cand) {
                        ++real;
                    } else {
                        ll dij = query(cand, x);
                        if (dij < h[cand] + h[x]) ++real;
                    }
                }
                if (real > half) balanced = false;
            }
        }

        if (balanced) {
            cout << "! " << R << endl;
            return 0;
        }
    }

    cout << "! " << -R << endl;
    return 0;
}
