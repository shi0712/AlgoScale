#include <bits/stdc++.h>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int m;
    if (!(cin >> m)) return 0;

    vector<int> ans(m + 1);          // 0 – bridge, 1 – non‑bridge
    vector<int> q0(m + 1);           // query number of "? 0 i"
    int qcnt = 0;

    auto ask = [&](int p, int e) -> int {
        ++qcnt;
        cout << "? " << p << ' ' << e << "\n";
        cout.flush();
        int r;
        if (!(cin >> r)) exit(0);
        if (r == -1) exit(0);
        return r;
    };

    /* 1. bridges */
    for (int i = 1; i <= m; ++i) {
        int r = ask(0, i);
        ans[i] = r;          // 0 – bridge, 1 – not a bridge
        q0[i] = qcnt;
    }

    /* collect non‑bridge edges */
    vector<int> nb;
    for (int i = 1; i <= m; ++i)
        if (ans[i] == 1) nb.push_back(i);
    int nbCnt = (int)nb.size();

    if (nbCnt == 0) {                     // only bridges
        cout << "!";
        for (int i = 1; i <= m; ++i) cout << " -1";
        cout << "\n";
        cout.flush();
        int conf; if (cin >> conf) {}
        return 0;
    }

    const int B = 150;                    // block size, > 14
    int blocks = (nbCnt + B - 1) / B;
    if (blocks == 0) blocks = 1;

    vector<vector<int>> blk(blocks);
    for (int i = 0; i < nbCnt; ++i) {
        int b = i % blocks;               // round‑robin distribution
        blk[b].push_back(nb[i]);
    }

    /* 2. queries that remove a whole block */
    vector<int> blkQ(blocks, 0);
    for (int b = 0; b < blocks; ++b) {
        int cur = 0;
        for (int e : blk[b]) {
            ask(cur, e);                  // answer is irrelevant here
            cur = qcnt;
        }
        blkQ[b] = cur;                    // set after removing the whole block
    }

    /* 3. discover cycles */
    vector<int> comp(m + 1, 0);            // component id, 0 = unknown
    vector<int> compSize(1, 0);            // 1‑based, size of component
    int compCnt = 0;

    for (int e : nb) {
        if (comp[e]) continue;            // already assigned
        ++compCnt;
        comp[e] = compCnt;
        int sz = 1;
        bool big = false;

        for (int b = 0; b < blocks && !big; ++b) {
            int r = ask(blkQ[b], e);
            if (r == 0) {                 // block contains a partner edge
                for (int f : blk[b]) {
                    if (comp[f]) continue;
                    int r2 = ask(q0[e], f);
                    if (r2 == 0) {
                        comp[f] = compCnt;
                        ++sz;
                        if (sz > 14) {   // cycle is long enough
                            big = true;
                            break;
                        }
                    }
                }
            }
        }
        if (big) compSize.push_back(15); // any value >14
        else     compSize.push_back(sz);
    }

    /* 4. output */
    cout << "!";
    for (int i = 1; i <= m; ++i) {
        int out;
        if (ans[i] == 0) out = -1;                     // bridge
        else if (comp[i] == 0) out = 0;                // big cycle, never discovered
        else {
            int sz = compSize[comp[i]];
            out = (sz <= 14 ? sz : 0);
        }
        cout << ' ' << out;
    }
    cout << "\n";
    cout.flush();

    int conf;
    if (cin >> conf) {}   // read final confirmation
    return 0;
}
