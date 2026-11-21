#include <bits/stdc++.h>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    int M;
    if (!(cin >> M)) return 0;

    // IDs: leaves 0..M-1, internal vertices start from M
    int nextInternal = M;
    int root = nextInternal++;               // root id
    vector<int> parent;                      // parent for internal nodes
    parent.resize(2005, -1);
    vector<int> leafParent(M, -1);
    vector<int> cur(M, root);                // deepest known vertex on leaf path
    vector<char> leafDone(M, 0);

    int nextVal = 1;                         // ball values start from 1

    // occupy the root
    cout << "? 1 0 " << nextVal << "\n" << flush;
    int ans;
    if (!(cin >> ans) || ans == -1) return 0; // should be 1

    // process each leaf
    for (int u = 0; u < M; ++u) {
        while (!leafDone[u]) {
            ++nextVal;
            cout << "? 1 " << u << " " << nextVal << "\n" << flush;
            if (!(cin >> ans) || ans == -1) return 0; // must succeed

            // create a new vertex as child of cur[u]
            int v = nextInternal++;
            parent[v] = cur[u];
            // test whether leaf u became occupied
            cout << "? 1 " << u << " 0\n" << flush;
            int r;
            if (!(cin >> r) || r == -1) return 0;
            if (r == 0) { // leaf is now occupied
                leafParent[u] = cur[u];
                leafDone[u] = 1;
                // the vertex we just created is actually the leaf,
                // discard it from internal set
                --nextInternal;
            } else {
                // leaf still free → the new vertex is internal
                cur[u] = v;
            }

            // update cur for other leaves
            for (int w = 0; w < M; ++w) if (!leafDone[w] && w != u) {
                // climb from leaf w using known parents
                int x = w;
                // if leaf w already has a known parent, climb up
                while (!leafDone[x] && leafParent[x] != -1) {
                    x = leafParent[x];
                }
                // now climb through internal nodes
                while (x >= M && parent[x] != -1) {
                    x = parent[x];
                }
                if (x == cur[w]) continue; // already deepest
                // check if the deepest known vertex is the new one
                // walk up from w until we meet a known vertex
                int y = w;
                while (!leafDone[y] && leafParent[y] != -1) {
                    y = leafParent[y];
                }
                while (y >= M && parent[y] != -1) {
                    y = parent[y];
                }
                if (y == v) cur[w] = v;
            }
        }
    }

    // final collection to empty the machine
    cout << "? 2\n" << flush;
    int s;
    if (!(cin >> s) || s == -1) return 0;
    for (int i = 0; i < s; ++i) {
        int dummy;
        cin >> dummy;
    }

    // Build final parent array
    int N = M + (nextInternal - M);
    vector<int> internalToFinal(nextInternal, -1);
    int rootId = N - 1;
    internalToFinal[root] = rootId;
    int curId = M;
    for (int id = M; id < nextInternal; ++id) {
        if (id == root) continue;
        internalToFinal[id] = curId++;
    }
    vector<int> R(N - 1, -1);
    // leaves
    for (int u = 0; u < M; ++u) {
        R[u] = internalToFinal[leafParent[u]];
    }
    // internal nodes except root
    for (int id = M; id < nextInternal; ++id) {
        if (id == root) continue;
        R[internalToFinal[id]] = internalToFinal[parent[id]];
    }

    cout << "! " << N;
    for (int i = 0; i < N - 1; ++i) cout << " " << R[i];
    cout << "\n" << flush;
    return 0;
}
