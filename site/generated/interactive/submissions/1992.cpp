// Solution for "Hidden Hierarchy Hints" (interactive).
//
// Query model.  With b_i = 1 the i-th edge is directed s_i -> f_i, otherwise
// f_i -> s_i.  Put beta_i = 2*b_i - 1 (in {-1,+1}) and let F_i be the component
// of f_i after deleting edge i.  The returned value is
//        A = (m + sum_i beta_i * sigma_i(v)) / 2 ,   sigma_i(v) = 2*[v in F_i]-1
// hence the "potential"
//        Phi(v) = (A - m/2 + B/2),  B = sum beta_i,
// equals  sum_{i : v in F_i} beta_i.   Its gradient across edge i is exactly
// beta_i:  Phi(f_i) - Phi(s_i) = beta_i.
//
// Strategy.  Use k = 1 + ceil(log2 m) potentials:
//   * coordinate 0 is beta = (+1,...,+1) (the level function) which fixes the
//     direction of every edge (Delta of a candidate pair must be +1 in it);
//   * coordinates 1..k-1 give every edge index a distinct binary code, hence
//     every edge gets a distinct signature Sigma_i = (beta_1[i],...,beta_k[i]).
// Querying potential t at every vertex yields Phi_t(v) for all v.  For index i
// the true edge {s_i,f_i} satisfies Phi(f_i) - Phi(s_i) = Sigma_i, so it is among
// the pairs with exactly that difference ("candidates").  Distinct signatures
// guarantee that once every index has a single candidate the labelling is
// forced.  Leftover ambiguity (rare "distance >= 3" pairs whose signatures
// coincide) is removed by cheap tree-constraint reasoning and, if needed, by
// extra random potential coordinates evaluated only at the involved vertices.
//
// Total: (1 + ceil(log2(n-1))) * n queries plus a small adaptive refinement.

#include <bits/stdc++.h>
using namespace std;

static int n, m;
static long long queriesUsed = 0;   // safety counter

static int askQuery(int v, const vector<int>& b) {
    ++queriesUsed;
    string s;
    s.reserve((size_t)m + 16);
    s += "? ";
    s += to_string(v);
    s += ' ';
    for (int i = 0; i < m; i++) s += (char)('0' + b[i]);
    s += '\n';
    fwrite(s.data(), 1, s.size(), stdout);
    fflush(stdout);
    int r;
    if (scanf("%d", &r) != 1) exit(0);
    return r;
}

int main() {
    if (scanf("%d", &n) != 1) return 0;
    m = n - 1;
    if (m <= 0) { printf("!\n"); fflush(stdout); return 0; }

    if (m == 1) {
        // n == 2: one edge.  Query root 1 with b="1" (direction s1->f1).
        vector<int> b(1, 1);
        int r = askQuery(1, b);
        if (r == 1) printf("! 2 1\n");   // edge points toward 1  => f1 = 1
        else        printf("! 1 2\n");
        fflush(stdout);
        return 0;
    }

    mt19937_64 rng(chrono::steady_clock::now().time_since_epoch().count() ^ ((uint64_t)n * 1000003ULL));

    // k potentials: coordinate 0 is "all +1" (level function),
    // the other (k-1) coordinates form a distinct binary code per edge index.
    int bits = 0;
    while ((1LL << bits) < (long long)m) bits++;
    int k = 1 + bits;

    vector<int> code(m);
    for (int i = 0; i < m; i++) code[i] = i;
    for (int i = m - 1; i > 0; i--) swap(code[i], code[rng() % (i + 1)]);

    vector<vector<int>> beta(k, vector<int>(m));
    for (int i = 0; i < m; i++) beta[0][i] = 1;
    for (int t = 1; t < k; t++)
        for (int i = 0; i < m; i++) beta[t][i] = ((code[i] >> (t - 1)) & 1) ? -1 : 1;

    vector<int> B(k, 0);
    for (int t = 0; t < k; t++) { int s = 0; for (int i = 0; i < m; i++) s += beta[t][i]; B[t] = s; }

    // Phi_t(v) = sum over edges i (with v on f_i's side) of beta[t][i]
    vector<vector<int>> phi(k, vector<int>(n + 1, 0));
    {
        vector<int> bt(m);
        for (int t = 0; t < k; t++) {
            for (int i = 0; i < m; i++) bt[i] = (beta[t][i] == 1) ? 1 : 0;
            for (int v = 1; v <= n; v++) {
                int ans = askQuery(v, bt);
                phi[t][v] = ans + (B[t] - m) / 2;
            }
        }
    }

    // ---- candidate edges: pair (u,w) with  Phi(w) - Phi(u) == Sigma_i  ----
    const int KS = 2 * k;
    auto makeKey = [&](int v) {
        string s; s.resize(KS);
        for (int t = 0; t < k; t++) { int16_t c = (int16_t)phi[t][v]; memcpy(&s[(size_t)2 * t], &c, 2); }
        return s;
    };
    unordered_map<string, vector<int>> pos;
    pos.reserve((size_t)n * 2 + 16);
    for (int v = 1; v <= n; v++) pos[makeKey(v)].push_back(v);

    vector<vector<pair<int,int>>> cands(m);
    {
        string kk; kk.resize(KS);
        for (int i = 0; i < m; i++) {
            for (int u = 1; u <= n; u++) {
                for (int t = 0; t < k; t++) {
                    int16_t c = (int16_t)(phi[t][u] + beta[t][i]);
                    memcpy(&kk[(size_t)2 * t], &c, 2);
                }
                auto it = pos.find(kk);
                if (it == pos.end()) continue;
                for (int w : it->second) cands[i].push_back({u, w});
            }
        }
    }

    // ---- cheap logical eliminations using the tree constraints (no queries) ----
    auto eliminate = [&]() {
        bool changed = true;
        while (changed) {
            changed = false;
            unordered_set<long long> forced;
            forced.reserve((size_t)m * 2 + 16);
            vector<int> par(n + 1);
            for (int v = 0; v <= n; v++) par[v] = v;
            function<int(int)> find = [&](int x) { while (par[x] != x) { par[x] = par[par[x]]; x = par[x]; } return x; };
            for (int i = 0; i < m; i++) if (cands[i].size() == 1) {
                int a = cands[i][0].first, b = cands[i][0].second;
                forced.insert(((long long)min(a, b) << 20) | max(a, b));
                int ra = find(a), rb = find(b);
                if (ra != rb) par[ra] = rb;
            }
            for (int i = 0; i < m; i++) {
                if (cands[i].size() <= 1) continue;
                vector<pair<int,int>> keep;
                keep.reserve(cands[i].size());
                for (auto& pr : cands[i]) {
                    int a = pr.first, b = pr.second;
                    if (forced.count(((long long)min(a, b) << 20) | max(a, b))) { changed = true; continue; }
                    if (find(a) == find(b)) { changed = true; continue; }
                    keep.push_back(pr);
                }
                if (keep.size() != cands[i].size()) cands[i].swap(keep);
            }
        }
    };

    eliminate();

    // ---- adaptive refinement: add random potential coordinates at needed vertices ----
    for (int round = 0; round < 100000 && queriesUsed < 30000; round++) {
        bool amb = false;
        size_t ambCnt = 0;
        for (int i = 0; i < m; i++) if (cands[i].size() > 1) { amb = true; ambCnt += cands[i].size(); }
        if (!amb) break;

        vector<int> nb(m);
        int nbB = 0;
        for (int i = 0; i < m; i++) { nb[i] = (int)(rng() & 1) ? 1 : -1; nbB += nb[i]; }

        vector<char> need(n + 1, 0);
        vector<int> needList;
        needList.reserve(ambCnt * 2);
        for (int i = 0; i < m; i++) {
            if (cands[i].size() <= 1) continue;
            for (auto& pr : cands[i]) {
                if (!need[pr.first]) { need[pr.first] = 1; needList.push_back(pr.first); }
                if (!need[pr.second]) { need[pr.second] = 1; needList.push_back(pr.second); }
            }
        }
        vector<int> bn(m);
        for (int i = 0; i < m; i++) bn[i] = (nb[i] == 1) ? 1 : 0;
        unordered_map<int,int> nphi;
        nphi.reserve(needList.size() * 2 + 16);
        for (int v : needList) {
            int ans = askQuery(v, bn);
            nphi[v] = ans + (nbB - m) / 2;
        }
        for (int i = 0; i < m; i++) {
            if (cands[i].size() <= 1) continue;
            vector<pair<int,int>> keep;
            keep.reserve(cands[i].size());
            for (auto& pr : cands[i])
                if (nphi[pr.second] - nphi[pr.first] == nb[i]) keep.push_back(pr);
            cands[i].swap(keep);
        }
        eliminate();
    }

    string out;
    out.reserve((size_t)m * 12 + 8);
    out += '!';
    for (int i = 0; i < m; i++) {
        int s = 1, f = 2;
        if (!cands[i].empty()) { s = cands[i][0].first; f = cands[i][0].second; }
        out += ' '; out += to_string(s);
        out += ' '; out += to_string(f);
    }
    out += '\n';
    fwrite(out.data(), 1, out.size(), stdout);
    fflush(stdout);
    return 0;
}
