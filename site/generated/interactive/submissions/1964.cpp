#include <bits/stdc++.h>
using namespace std;

static int N, F;
static long long mdist[130][130];
static bool have[130][130];

static long long ask(int i, int j) {
    if (i == j) return 0;
    if (have[i][j]) return mdist[i][j];
    cout << "? " << i << " " << j << endl;
    long long d;
    if (!(cin >> d)) exit(0);
    have[i][j] = have[j][i] = true;
    mdist[i][j] = mdist[j][i] = d;
    return d;
}

static vector<long long> gtau, gmu;
static long long gtaub;

// ---- balance test of one node -------------------------------------------------
// G is a set of leaves attached to the node h (all of them), off[x] = dist(x,h).
// Decides whether some neighbour-component of h that is represented by G holds
// more than F leaves.

// Boyer-Moore majority candidate + exact verification.
static bool groupOverBM(const vector<int>& G, const vector<long long>& off) {
    int m = (int)G.size();
    int cand = -1, cnt = 0;
    for (int i = 0; i < m; i++) {
        int x = G[i];
        if (cnt == 0) { cand = x; cnt = 1; }
        else if (ask(x, cand) < off[x] + off[cand]) cnt++;
        else cnt--;
    }
    long long total = 0;
    for (int x : G)
        if (x == cand || ask(x, cand) < off[x] + off[cand]) total++;
    return total > F;
}

static bool groupOver(const vector<int>& G, const vector<long long>& off) {
    int m = (int)G.size();
    if ((long long)m <= F) return false;
    // Cheap certificate: take a perfect matching of G and test every matched pair.
    // If all matched pairs lie in two different classes, every class can use at
    // most one element of each pair, hence at most ceil(m/2) <= F elements.
    // A few shifted matchings are tried; abort a matching at the first equal pair.
    for (int off0 = 0; off0 < 3; off0++) {
        vector<pair<int,int>> prs;
        prs.reserve(m / 2);
        for (int i = 0; i + 1 < m; i += 2)
            prs.push_back({G[(off0 + i) % m], G[(off0 + i + 1) % m]});
        bool allDiff = true;
        for (auto& pr : prs) {
            if (ask(pr.first, pr.second) < off[pr.first] + off[pr.second]) { allDiff = false; break; }
        }
        if (allDiff) {
            long long blocks = (long long)m - (long long)prs.size();
            if (blocks <= (long long)F) return false;
        }
    }
    return groupOverBM(G, off);
}

static bool checkBalanced(long long p) {
    long long a_side = 0;
    for (int x = 0; x < N; x++) if (gtau[x] < p) a_side++;
    if (a_side > F) return false;
    if (p < gtaub) {
        vector<int> G;
        for (int x = 0; x < N; x++) if (gtau[x] == p) G.push_back(x);
        long long m = (long long)G.size();
        long long b_side = (long long)N - a_side - m;
        if (b_side > F) return false;
        if (m > (long long)F && groupOver(G, gmu)) return false;
        return true;
    } else {
        // p == gtaub: the hub is the junction q where the diameter branch leaves
        long long gi = 0;
        for (int x = 0; x < N; x++) if (gtau[x] > gtaub) gi++;
        if (gi > F) return false;
        vector<int> U;
        for (int x = 0; x < N; x++) if (gtau[x] == gtaub) U.push_back(x);
        if ((long long)U.size() > (long long)F && groupOver(U, gmu)) return false;
        return true;
    }
}

int main() {
    if (!(cin >> N)) return 0;
    F = N / 2;

    vector<long long> d0(N, 0), da(N, 0);
    for (int x = 1; x < N; x++) d0[x] = ask(0, x);
    int a = 1;
    for (int x = 2; x < N; x++) if (d0[x] > d0[a]) a = x;
    long long L = d0[a];                 // a is a diameter endpoint
    for (int x = 0; x < N; x++) if (x != a) da[x] = ask(a, x);
    da[a] = 0;

    long long D = 0;
    for (int x = 0; x < N; x++) if (x != a) D = max(D, da[x]);

    gtau.assign(N, 0);
    gmu.assign(N, 0);
    for (int x = 0; x < N; x++) {
        gtau[x] = (L + da[x] - d0[x]) / 2;   // distance from a to projection on path (0,a)
        gmu[x]  = (d0[x] + da[x] - L) / 2;   // distance from x to path (0,a)
    }

    map<long long, int> cnt;
    for (int x = 0; x < N; x++) cnt[gtau[x]]++;

    // choose b (a leaf farthest from a) so that the expected extra queries are minimal
    vector<int> cands;
    for (int x = 0; x < N; x++) if (x != a && da[x] == D) cands.push_back(x);
    long long bestCost = LLONG_MAX;
    int b = cands[0];
    for (int bb : cands) {
        long long tb = gtau[bb];
        vector<long long> K;
        for (auto& kv : cnt)
            if (kv.first > 0 && kv.first < tb) K.push_back(kv.first);
        if (bb != 0) K.push_back(tb);
        long long R = LLONG_MAX;
        for (long long p : K) R = min(R, max(p, D - p));
        long long cost = 0;
        for (long long p : K) {
            if (max(p, D - p) != R) continue;
            if (p < tb) {
                int m = cnt[p];
                if (m > F) cost += 2LL * m;
            } else {
                int m = cnt[tb];
                if (m > F) cost += 2LL * m;
            }
        }
        if (cost < bestCost) { bestCost = cost; b = bb; }
    }

    long long taub = gtau[b];
    gtaub = taub;

    vector<long long> K;
    {
        for (auto& kv : cnt)
            if (kv.first > 0 && kv.first < taub) K.push_back(kv.first);
        if (b != 0) K.push_back(taub);
        sort(K.begin(), K.end());
    }

    long long R = LLONG_MAX;
    for (long long p : K) R = min(R, max(p, D - p));

    bool balanced = false;
    for (long long p : K) {
        if (max(p, D - p) == R) {
            if (checkBalanced(p)) { balanced = true; break; }
        }
    }

    cout << "! " << (balanced ? R : -R) << endl;
    return 0;
}
