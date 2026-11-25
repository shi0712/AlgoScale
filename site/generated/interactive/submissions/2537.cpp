#include <bits/stdc++.h>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int m;
    if (!(cin >> m)) return 0;

    vector<int> delIndex(m + 1, 0); // 0 unknown, -1 bridge, >0 cycle index
    vector<int> upper(m + 1, 0);
    vector<char> kept(m + 1, 0);
    vector<int> ids;
    ids.push_back(0); // query 0 = all edges
    int D = 0;
    int qcount = 0;

    // Build a spanning tree by deleting non-bridges in label order.
    for (int e = 1; e <= m; ++e) {
        cout << "? " << ids.back() << " " << e << "\n" << flush;
        ++qcount;
        int ans;
        if (!(cin >> ans) || ans == -1) return 0;
        if (ans == 1) {
            ++D;
            delIndex[e] = D;
            ids.push_back(qcount);
        } else {
            kept[e] = 1;
            upper[e] = D;
        }
    }

    if (D == 0) {
        cout << "!";
        for (int e = 1; e <= m; ++e) cout << " -1";
        cout << "\n" << flush;
        int conf;
        if (!(cin >> conf) || conf == -1) return 0;
        return 0;
    }

    // Determine the deletion index (cycle identifier) for every kept edge.
    for (int e = 1; e <= m; ++e) {
        if (!kept[e]) continue;

        if (upper[e] == 0) {
            // No deletion happened before this edge: it must be an original bridge.
            delIndex[e] = -1;
            continue;
        }

        int lo = 1, hi = upper[e];
        while (lo < hi) {
            int mid = (lo + hi) >> 1;
            cout << "? " << ids[mid] << " " << e << "\n" << flush;
            ++qcount;
            int ans;
            if (!(cin >> ans) || ans == -1) return 0;
            if (ans == 0)
                hi = mid;
            else
                lo = mid + 1;
        }

        int t = lo;
        if (t == 1) {
            // Distinguish an original bridge from an edge in the first cycle.
            cout << "? 0 " << e << "\n" << flush;
            ++qcount;
            int ans0;
            if (!(cin >> ans0) || ans0 == -1) return 0;
            if (ans0 == 0)
                delIndex[e] = -1;
            else
                delIndex[e] = t;
        } else {
            delIndex[e] = t;
        }
    }

    vector<int> cnt(D + 1, 0);
    for (int e = 1; e <= m; ++e) {
        if (delIndex[e] > 0)
            ++cnt[delIndex[e]];
    }

    cout << "!";
    for (int e = 1; e <= m; ++e) {
        int val;
        if (delIndex[e] == -1) {
            val = -1;
        } else {
            int L = cnt[delIndex[e]];
            val = (L <= 14 ? L : 0);
        }
        cout << " " << val;
    }
    cout << "\n" << flush;

    int conf;
    if (!(cin >> conf) || conf == -1) return 0;
    return 0;
}
