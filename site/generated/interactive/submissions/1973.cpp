#include <bits/stdc++.h>
using namespace std;

// ---------------------------------------------------------------------------
// Baudelaire (easy version) -- interactive, star tree centred at node 1.
//
// Let f(v) = sum of values on the path root -> v.
//   root == 1 :  f(1) = a1 (odd),  f(u) = a1 + au  (leaf u)      (even)
//   root == L :  f(1) = a1 + aL (even)
//                f(L) = aL
//                f(u) = a1 + aL + au   (leaf u != L)
//
// One query uses non-negative multiplicities w_u with sum(w) <= n and returns
// sum w_u f(u).  Giving the leaves of a group the weights 1,2,4,... makes the
// map (a_1..a_k) -> sum 2^i a_i injective, so a whole group of leaf values is
// recovered from a single query (balanced binary).  With a weight budget of n
// a group of at most floor(log2(n+1)) leaves can be handled this way.
//
// The first query also contains node 1 with a weight larger than the sum of
// the group weights; that determines f(1) (and thus the case) at no cost.
// ---------------------------------------------------------------------------

static int n;

static long long askRaw(const vector<int>& nodes) {
    cout << "? 1 " << nodes.size();
    for (int u : nodes) cout << ' ' << u;
    cout << '\n';
    cout.flush();
    long long r;
    if (!(cin >> r)) exit(0);
    return r;
}

static void doToggle(int u) {
    cout << "? 2 " << u << '\n';
    cout.flush();
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    if (!(cin >> n)) return 0;
    for (int i = 0; i < n - 1; i++) {
        int u, v;
        cin >> u >> v;   // star, edges carry no extra information
    }

    vector<int> val(n + 1, 0);

    auto finish = [&]() {
        cout << "!";
        for (int i = 1; i <= n; i++) cout << ' ' << val[i];
        cout << '\n';
        cout.flush();
    };

    // ---------------- tiny case n = 2 ----------------
    if (n == 2) {
        long long g = askRaw({1});
        long long f2 = askRaw({2});
        if (g & 1LL) {                 // root == 1 :  f1 = a1, f2 = a1 + a2
            val[1] = (int)g;
            val[2] = (int)(f2 - g);
        } else {                        // root == 2 :  f2 = a2, f1 = a1 + a2
            val[2] = (int)f2;
            val[1] = (int)(g - f2);
        }
        finish();
        return 0;
    }

    vector<int> leaves;
    leaves.reserve(n - 1);
    for (int u = 2; u <= n; u++) leaves.push_back(u);
    int m = n - 1;

    // largest block size b with 2^b - 1 <= n
    int bsz = 1;
    while ((1LL << (bsz + 1)) - 1 <= (long long)n) bsz++;

    // -------- first (merged) query: node 1 + (bsz-1) leaves --------
    int s0 = min(m, bsz - 1);
    long long Wp = (1LL << s0) - 1;     // sum of the leaf weights
    long long w1 = Wp + 1;              // weight of node 1
    long long g;                        // f(1)
    vector<int> mf(s0);                 // f-values of the first s0 leaves (g = +-2)
    {
        vector<int> q;
        q.reserve((size_t)(w1 + Wp));
        for (long long c = 0; c < w1; c++) q.push_back(1);
        for (int i = 0; i < s0; i++)
            for (int c = 0; c < (1 << i); c++) q.push_back(leaves[i]);
        long long R = askRaw(q);

        auto decodeSigma = [&](long long sigma) {     // sigma = sum 2^i a_i
            long long t = (sigma + Wp) / 2;           // in [0, Wp]
            for (int i = 0; i < s0; i++)
                val[leaves[i]] = (((t >> i) & 1LL) ? 1 : -1);
        };
        auto decodeF = [&](long long E, int sign) {    // f = sign*(1+2e)
            for (int i = 0; i < s0; i++)
                mf[i] = sign * (1 + 2 * (int)((E >> i) & 1LL));
        };
        if (R >= -Wp && R <= Wp) {                       // f(1) == 0
            g = 0;
            decodeSigma(R);
        } else if (R >= Wp + 1 && R <= 3 * Wp + 1) {      // f(1) == +1 : root 1
            g = 1;
            decodeSigma(R - (2 * Wp + 1));
        } else if (R >= -3 * Wp - 1 && R <= -Wp - 1) {    // f(1) == -1 : root 1
            g = -1;
            decodeSigma(R + (2 * Wp + 1));
        } else if (R >= 3 * Wp + 2 && R <= 5 * Wp + 2) {
            g = 2;
            decodeF((R - 3 * Wp - 2) / 2, 1);
        } else {
            g = -2;
            decodeF((-R - 3 * Wp - 2) / 2, -1);
        }
    }

    // A query with weight 1 on every leaf yields sum f(u) over the leaves, i.e.
    //   root 1 :  m*g      + sum a_u
    //   leaf   :  (m-1)*g  + sum a_u
    // If |sum a_u| == m then every leaf has the same value and we are done
    // immediately.  It is worth one query only if the already decoded prefix
    // is compatible with that situation.
    if (s0 >= 2 && s0 < m) {
        bool cond;
        if (g & 1LL || g == 0) {
            cond = true;
            for (int i = 1; i < s0; i++)
                if (val[leaves[i]] != val[leaves[0]]) { cond = false; break; }
        } else {
            long long hi = 3 * g / 2, lo = g / 2;
            int cntLo = 0;
            cond = true;
            for (int i = 0; i < s0; i++) {
                if (mf[i] == lo) cntLo++;
                else if (mf[i] != hi) { cond = false; break; }
            }
            if (cntLo > 1) cond = false;
        }
        if (cond) {
            long long R2 = askRaw(leaves);       // weight 1 on every leaf
            long long T = R2 - (g & 1LL ? (long long)m * g
                                        : (long long)(m - 1) * g);
            if (T == m || T == -m) {
                int v = (int)(T / m);
                bool ok = true;
                int a1;
                if (g & 1LL) a1 = (int)g;            // root is node 1
                else if (g == 0) a1 = -v;            // root leaf has value v
                else {                               // must have v == f(1)/2
                    if (v != (int)(g / 2)) ok = false;
                    a1 = (int)(g / 2);
                }
                if (ok) {
                    for (int u : leaves) val[u] = v;
                    val[1] = a1;
                    finish();
                    return 0;
                }
            }
        }
    }

    // queries one group of leaves [st, st+s) with weights 1,2,4,...
    // offsetMode: response == g*W + sum 2^i a_i  (root is node 1)
    // otherwise : response == sum 2^i a_i
    auto blockQuery = [&](int st, int s, bool offsetMode, long long gOff) {
        int W = (1 << s) - 1;
        vector<int> q;
        q.reserve(W);
        for (int i = 0; i < s; i++)
            for (int c = 0; c < (1 << i); c++) q.push_back(leaves[st + i]);
        long long R = askRaw(q);
        long long sigma = offsetMode ? (R - gOff * (long long)W) : R;
        long long t = (sigma + (long long)W) / 2;
        for (int i = 0; i < s; i++)
            val[leaves[st + i]] = (((t >> i) & 1LL) ? 1 : -1);
    };

    if (g & 1LL) {
        // ---- root is node 1 ----
        val[1] = (int)g;
        for (int st = s0; st < m; st += bsz) {
            int s = min(bsz, m - st);
            blockQuery(st, s, true, g);
        }
    } else if (g == 0) {
        // ---- root is a leaf and a1 + aL == 0 : every leaf has f(u) == a_u ----
        for (int st = s0; st < m; st += bsz) {
            int s = min(bsz, m - st);
            blockQuery(st, s, false, 0);
        }
        bool allsame = true;
        for (int i = 1; i < m; i++)
            if (val[leaves[i]] != val[leaves[0]]) { allsame = false; break; }
        if (allsame) {
            val[1] = -val[leaves[0]];
        } else {
            // only a1 is still ambiguous -> flip node 1 and read f(1) once
            doToggle(1);
            long long g2 = askRaw({1});   // g2 = -2 * (a1 before the flip)
            val[1] = (int)(g2 / 2);       // value of node 1 after the flip
        }
    } else {
        // ---- root is a leaf and a1 == aL == +-1, hence f(1) = +-2 ----
        // Flipping node 1 makes a1 = -aL, so f(1) becomes 0 and every leaf
        // satisfies f(u) == a_u; one sweep decodes everything and the former
        // ambiguity (which leaf is the root) disappears.
        long long A = g / 2;
        doToggle(1);
        for (int st = 0; st < m; st += bsz) {
            int s = min(bsz, m - st);
            blockQuery(st, s, false, 0);
        }
        val[1] = (int)(-A);               // node 1 after the flip
    }

    finish();
    return 0;
}
