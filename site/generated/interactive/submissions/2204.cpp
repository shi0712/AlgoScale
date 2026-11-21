#include <bits/stdc++.h>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n)) return 0;

    if (n == 1) {
        cout << "!\n1\n0\n";
        cout.flush();
        return 0;
    }

    vector<int> R(n);
    for (int i = 0; i < n; ++i) {
        cout << "? " << i << " 0\n";
        cout.flush();
        int x;
        if (!(cin >> x)) return 0;
        if (x == -1) return 0;
        R[i] = x;
    }

    const int MAXV = 16384;

    vector<int> H;
    H.reserve(n);
    for (int c = 0; c < n; ++c) {
        bool ok = true;
        for (int x = 0; x < n; ++x) {
            if ((x ^ c) >= n) {
                ok = false;
                break;
            }
        }
        if (ok) H.push_back(c);
    }

    vector<int> C;
    C.reserve(H.size());
    for (int h : H) {
        bool ok = true;
        for (int i = 0; i < n; ++i) {
            if (R[i ^ h] != (R[i] ^ h)) {
                ok = false;
                break;
            }
        }
        if (ok) C.push_back(h);
    }

    vector<char> inIm(MAXV, 0);
    for (int i = 0; i < n; ++i) {
        int v = R[i];
        if (0 <= v && v < MAXV) inIm[v] = 1;
    }

    vector<int> cand;
    for (int t = 0; t < n; ++t) {
        if (R[t] != t) continue;
        bool ok = true;
        for (int x = 0; x < n; ++x) {
            int u = x ^ t;
            if (u >= MAXV || !inIm[u]) {
                ok = false;
                break;
            }
        }
        if (ok) cand.push_back(t);
    }

    if (cand.empty()) {
        for (int t = 0; t < n; ++t) {
            bool ok = true;
            for (int x = 0; x < n; ++x) {
                int u = x ^ t;
                if (u >= MAXV || !inIm[u]) {
                    ok = false;
                    break;
                }
            }
            if (ok) {
                cand.push_back(t);
                break;
            }
        }
    }

    vector<char> used(MAXV, 0);
    vector<int> reps;
    for (int t : cand) {
        if (!used[t]) {
            reps.push_back(t);
            for (int h : C) {
                int u = t ^ h;
                if (u < MAXV) used[u] = 1;
            }
        }
    }

    if (reps.empty()) reps.push_back(0);

    auto ask = [&](int i, int j) -> int {
        cout << "? " << i << " " << j << "\n";
        cout.flush();
        int x;
        if (!(cin >> x)) exit(0);
        if (x == -1) exit(0);
        return x;
    };

    if (reps.size() > 1) {
        vector<int> P(MAXV, -1);
        for (int i = 0; i < n; ++i) {
            int v = R[i];
            if (0 <= v && v < MAXV) P[v] = i;
        }
        vector<int> D(MAXV, -1);
        for (int u = 0; u < MAXV; ++u) {
            if (P[u] != -1) D[u] = u ^ P[u];
        }

        while (reps.size() > 1) {
            int a = reps[0];
            int b = reps[1];
            int j = -1;
            for (int jj = 1; jj < n; ++jj) {
                int va = D[jj ^ a];
                int vb = D[jj ^ b];
                if (va != vb) {
                    j = jj;
                    break;
                }
            }
            if (j == -1) {
                reps.erase(reps.begin() + 1);
                continue;
            }
            int y = ask(0, j);
            int obs = y ^ R[0] ^ j;
            vector<int> nr;
            nr.reserve(reps.size());
            for (int t : reps) {
                if (D[j ^ t] == obs) nr.push_back(t);
            }
            if (nr.empty()) break;
            reps.swap(nr);
        }
    }

    int t = reps[0];
    vector<int> p(n);
    for (int i = 0; i < n; ++i) p[i] = R[i] ^ t;

    cout << "!\n";
    cout << C.size() << "\n";
    for (int i = 0; i < n; ++i) {
        if (i) cout << ' ';
        cout << p[i];
    }
    cout << "\n";
    cout.flush();
    return 0;
}
