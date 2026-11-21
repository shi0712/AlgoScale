#include <bits/stdc++.h>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n)) return 0;

    if (n == 1) {
        cout << "!\n1\n0\n" << flush;
        return 0;
    }

    const int MAXV = 16384;
    vector<int> d(n);

    // Query d[i] = p[i] xor b[0] for all i.
    for (int i = 0; i < n; ++i) {
        cout << "? " << i << " 0\n" << flush;
        int x;
        cin >> x;
        if (x == -1 || !cin) return 0;
        d[i] = x;
    }

    vector<int> posD(MAXV, -1);
    for (int i = 0; i < n; ++i) {
        if (0 <= d[i] && d[i] < MAXV) posD[d[i]] = i;
    }

    // All shifts c such that {d[i] xor c} = [0, n-1].
    vector<int> C;
    for (int c = 0; c < n; ++c) {
        bool ok = true;
        for (int i = 0; i < n; ++i) {
            int v = d[i] ^ c;
            if (v < 0 || v >= n) {
                ok = false;
                break;
            }
        }
        if (ok) C.push_back(c);
    }

    // Keep shifts consistent with the already queried column j = 0.
    vector<int> C0;
    for (int c : C) {
        if (c >= 0 && c < MAXV && posD[c] == c) {
            C0.push_back(c);
        }
    }

    // G = {h : [0,n-1] xor h = [0,n-1]}.
    int sz = 1;
    while (sz * 2 <= n && n % (sz * 2) == 0) sz *= 2;

    // H = symmetries of the hidden permutation under xor.
    vector<int> H;
    for (int h = 0; h < sz; ++h) {
        bool ok = true;
        for (int i = 0; i < n; ++i) {
            if (d[i ^ h] != (d[i] ^ h)) {
                ok = false;
                break;
            }
        }
        if (ok) H.push_back(h);
    }

    // Merge candidates that are already equivalent under H.
    vector<int> cand;
    for (int c : C0) {
        int canonical = c;
        for (int h : H) {
            canonical = min(canonical, c ^ h);
        }
        if (canonical == c) cand.push_back(c);
    }

    vector<char> queried(n, false);
    queried[0] = true; // Column j = 0 has already been queried for every i.

    vector<int> cnt(MAXV, 0);
    vector<int> touched;
    touched.reserve(cand.size());

    const int ADAPTIVE_LIMIT = 10;

    // Identify the correct H-coset using a few well-chosen row queries.
    for (int it = 0; it < ADAPTIVE_LIMIT && cand.size() > 1; ++it) {
        int bestJ = -1;
        int bestMax = INT_MAX;
        int bestDistinct = 0;

        for (int j = 0; j < n; ++j) {
            if (queried[j]) continue;

            touched.clear();
            int distinct = 0;
            int mx = 0;

            for (int c : cand) {
                int idx = j ^ c;
                int pos = posD[idx];
                int key = c ^ pos;

                if (cnt[key] == 0) {
                    touched.push_back(key);
                    ++distinct;
                }
                ++cnt[key];
                if (cnt[key] > mx) mx = cnt[key];
            }

            for (int key : touched) cnt[key] = 0;

            if (mx < bestMax || (mx == bestMax && distinct > bestDistinct)) {
                bestMax = mx;
                bestDistinct = distinct;
                bestJ = j;
            }

            if (bestMax == 1) break;
        }

        if (bestJ == -1) break;

        cout << "? 0 " << bestJ << "\n" << flush;
        int ans;
        cin >> ans;
        if (ans == -1 || !cin) return 0;

        queried[bestJ] = true;
        int target = ans ^ d[0];

        vector<int> nextCand;
        for (int c : cand) {
            int idx = bestJ ^ c;
            int pos = posD[idx];
            int key = c ^ pos;
            if (key == target) nextCand.push_back(c);
        }

        cand.swap(nextCand);
    }

    // If still ambiguous, use the remaining row queries.
    for (int j = 0; j < n && cand.size() > 1; ++j) {
        if (queried[j]) continue;

        cout << "? 0 " << j << "\n" << flush;
        int ans;
        cin >> ans;
        if (ans == -1 || !cin) return 0;

        queried[j] = true;
        int target = ans ^ d[0];

        vector<int> nextCand;
        for (int c : cand) {
            int idx = j ^ c;
            int pos = posD[idx];
            int key = c ^ pos;
            if (key == target) nextCand.push_back(c);
        }

        cand.swap(nextCand);
    }

    int chosen;
    if (!cand.empty()) chosen = cand[0];
    else if (!C0.empty()) chosen = C0[0];
    else chosen = 0;

    vector<int> p(n);
    for (int i = 0; i < n; ++i) {
        p[i] = d[i] ^ chosen;
    }

    cout << "!\n";
    cout << H.size() << "\n";
    for (int i = 0; i < n; ++i) {
        if (i) cout << ' ';
        cout << p[i];
    }
    cout << "\n" << flush;

    return 0;
}
