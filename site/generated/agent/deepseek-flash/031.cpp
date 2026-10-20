#include "ballmachine.h"
#include <vector>
#include <algorithm>
#include <numeric>
using namespace std;

namespace {

struct Decoded {
    int N = 0;
    vector<int> idAt;    // chain id per preorder position
    bool ok = false;
};

// Reconstruct the tree from the preorder (values) given exact chain ids.
// Returns R of length N-1 (labels: leaves 0..M-1, root N-1, other internals M..N-2).
vector<int> reconstruct(int M, const vector<int>& V, int Kp, bool exactIds) {
    int N = (int)V.size();
    vector<int> id(N), isTop(N);
    for (int p = 0; p < N; p++) { id[p] = V[p] % Kp; isTop[p] = (V[p] < Kp) ? 1 : 0; }

    // chain membership.  exactIds: chain == id.
    // merged case: split each class into right-spine chains (heuristic below).
    vector<int> chainOf(N, -1);
    vector<int> leafOf;   // leaf label of chain
    int nch = 0;

    if (exactIds) {
        vector<int> mapId(Kp, -1);
        for (int p = 0; p < N; p++) {
            if (mapId[id[p]] < 0) { mapId[id[p]] = nch; leafOf.push_back(id[p]); nch++; }
            chainOf[p] = mapId[id[p]];
        }
    } else {
        // Heuristic: each top starts a chain; a rank>=2 node joins the most recent
        // chain of the same class that is still "open".
        vector<vector<int>> openByClass(Kp);
        vector<int> firstPos, lastPos;
        for (int p = 0; p < N; p++) {
            if (isTop[p]) {
                int c = nch++; chainOf[p] = c;
                firstPos.push_back(p); lastPos.push_back(p);
                openByClass[id[p]].push_back(c);
            } else {
                auto &v = openByClass[id[p]];
                if (v.empty()) { chainOf[p] = -1; }
                else {
                    int c = v.back(); chainOf[p] = c; lastPos[c] = p;
                }
            }
        }
        // assign leaf labels: per class, sort chains by first position, leaves ascending
        leafOf.assign(nch, -1);
        vector<vector<int>> cls(Kp);
        for (int c = 0; c < nch; c++) cls[id[firstPos[c]]].push_back(c);
        for (int r = 0; r < Kp; r++) {
            vector<int> &g = cls[r];
            sort(g.begin(), g.end(), [&](int a, int b){ return firstPos[a] < firstPos[b]; });
            vector<int> leaves;
            for (int i = r; i < M; i += Kp) leaves.push_back(i);
            if ((int)g.size() != (int)leaves.size()) { /* mismatch */ }
            for (size_t i = 0; i < g.size(); i++)
                leafOf[g[i]] = (i < leaves.size()) ? leaves[i] : (int)i;
        }
    }

    // drop invalid nodes
    for (int p = 0; p < N; p++) if (chainOf[p] < 0) return {};
    // per chain: first/last position
    vector<int> firstP(nch, N + 1), lastP(nch, -1);
    for (int p = 0; p < N; p++) {
        int c = chainOf[p];
        firstP[c] = min(firstP[c], p);
        lastP[c] = max(lastP[c], p);
    }
    if (firstP[0] != 0) return {};   // root chain must start at position 0

    // parent per position
    vector<int> par(N, -1);
    // within chain: previous element of the chain
    for (int p = 1; p < N; p++) {
        int c = chainOf[p];
        if (firstP[c] != p) {
            // find previous node of same chain
            int q = p - 1;
            while (q >= 0 && chainOf[q] != c) q--;
            if (q < 0) return {};
            par[p] = q;
        }
    }
    // chain tops: deepest ancestor = argmax position among nodes with lastP[chain] >= lastP[c]
    for (int c = 0; c < nch; c++) {
        int t = firstP[c];
        if (t == 0) continue;
        int need = lastP[c];
        int best = -1, bestPos = -1;
        for (int q = 0; q < t; q++) {
            if (lastP[chainOf[q]] >= need && q > bestPos) { bestPos = q; best = q; }
        }
        if (best < 0) return {};
        par[t] = best;
    }

    // labels
    vector<int> lab(N, -1);
    vector<char> isLeaf(N, 0);
    for (int c = 0; c < nch; c++) { int p = lastP[c]; isLeaf[p] = 1; lab[p] = leafOf[c]; }
    int nxt = M;
    for (int p = 0; p < N; p++) if (!isLeaf[p]) {
        if (p == 0) lab[p] = N - 1;
        else lab[p] = nxt++;
    }
    if (nxt != N - 1) return {};
    vector<int> R(N - 1, -1);
    for (int p = 1; p < N; p++) {
        if (par[p] < 0) return {};
        R[lab[p]] = lab[par[p]];
    }
    return R;
}

} // namespace

vector<int> find_structure(int M) {
    int Kp = (M <= 13) ? M : 13;
    int cap = 27 / Kp;
    if (cap < 2) cap = 2;
    // epoch: fill leaves 0..M-1 in order
    vector<int> ks(M, 0);
    for (int i = 0; i < M; i++) {
        int j = 0;
        while (true) {
            int v = min(j, cap - 1) * Kp + (i % Kp);
            if (!insert(i, v)) break;
            j++;
        }
        ks[i] = j;
    }
    vector<int> V = collect();
    int N = (int)V.size();
    if (N == 0) return vector<int>(0);
    // sanity: sum of chain lengths
    long long s = 0; for (int i = 0; i < M; i++) s += ks[i];
    if (s != N) { /* still try */ }
    bool exact = (M <= 13);
    vector<int> R = reconstruct(M, V, Kp, exact);
    if ((int)R.size() != N - 1) return vector<int>(0);
    return R;
}
