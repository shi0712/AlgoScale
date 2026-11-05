#include <bits/stdc++.h>
using namespace std;

/*
  For an edge (i,j) and a key point k with i<=k<=j let
      t_k = h-y_k,  b_k = sqrt(x_j-x_k),  p_k = sqrt(x_k-x_i).
  The arch condition at k is  |p_k - b_k| <= sqrt(2 t_k), which is
  equivalent to
      L_k(j) <= x_i <= U_k(j),
      L_k(j) = x_k - (sqrt(x_j-x_k)+sqrt(2t_k))^2
      U_k(j) = x_k - max(0, sqrt(x_j-x_k)-sqrt(2t_k))^2.

  * Lower bounds:  since L_k(j) <= x_k, the condition x_i >= L_k(j)
    for all k in [i,j] is the same as x_i >= max_{k<=j} L_k(j) = M_j.
  * Upper bounds:  x_i <= U_k(j) is equivalent (for k>=i) to
    x_j <= C_k(i), where C_k(i)=x_k+(sqrt(x_k-x_i)+sqrt(2t_k))^2.
    Hence all upper conditions are equivalent to
    x_j <= D_i = min_{k>=i} C_k(i), which depends only on i.

  Therefore edge (i,j) is legal iff
       x_i >= M_j   and   x_j <= D_i.
  M_j is computed with a max-Li-Chao (prefix), D_i with a min-Li-Chao
  (suffix), both batched on a segment tree over indices.  Afterwards the
  DP is a standard CHT DP with the two independent restrictions.
*/

static const long long INFLL = (long long)4e18;
static const double INFDB = 1e18;
static const double EPS = 1e-7;

int n;
long long h, alpha, bet;
vector<int> xs;        // x coordinates
vector<int> tt;        // t_i = h - y_i
vector<double> ss;     // sqrt(2*t_i)

vector<vector<int>> qs;   // query lists attached to segment tree nodes

void addQuery(int node, int l, int r, int ql, int qr, int id) {
    if (qr < l || r < ql) return;
    if (ql <= l && r <= qr) { qs[node].push_back(id); return; }
    int mid = (l + r) >> 1;
    addQuery(node << 1, l, mid, ql, qr, id);
    addQuery(node << 1 | 1, mid + 1, r, ql, qr, id);
}

/* ---------------- Li Chao: minimum of C_k(u), valid u<=x_k --------------- */
struct LiChaoC {
    const vector<int>* crd;
    vector<int> tr;
    inline double val(int id, int u) const {
        double p = sqrt((double)xs[id] - u) + ss[id];
        return (double)xs[id] + p * p;
    }
    void init(const vector<int>& c) {
        crd = &c;
        tr.assign(4 * (int)c.size() + 4, -1);
    }
    void add(int v, int l, int r, int id) {
        int &cur = tr[v];
        if (cur == -1) { cur = id; return; }
        int mid = (l + r) >> 1;
        int um = (*crd)[mid], ul = (*crd)[l];
        bool midBetter  = val(id, um) < val(cur, um);
        bool leftBetter = val(id, ul) < val(cur, ul);
        if (midBetter) swap(cur, id);
        if (l == r) return;
        if (leftBetter != midBetter) add(v << 1, l, mid, id);
        else                         add(v << 1 | 1, mid + 1, r, id);
    }
    double query(int v, int l, int r, int u) const {
        double res = INFDB;
        if (tr[v] != -1) res = val(tr[v], u);
        if (l == r) return res;
        int mid = (l + r) >> 1;
        if (u <= (*crd)[mid]) return min(res, query(v << 1, l, mid, u));
        return min(res, query(v << 1 | 1, mid + 1, r, u));
    }
};

/* ---------------- Li Chao: maximum of L_k(u), valid u>=x_k --------------- */
struct LiChaoL {
    const vector<int>* crd;
    vector<int> tr;
    inline double val(int id, int u) const {
        double p = sqrt((double)u - xs[id]) + ss[id];
        return (double)xs[id] - p * p;
    }
    void init(const vector<int>& c) {
        crd = &c;
        tr.assign(4 * (int)c.size() + 4, -1);
    }
    void add(int v, int l, int r, int id) {
        int &cur = tr[v];
        if (cur == -1) { cur = id; return; }
        int mid = (l + r) >> 1;
        int um = (*crd)[mid], ul = (*crd)[l];
        bool midBetter  = val(id, um) > val(cur, um);
        bool leftBetter = val(id, ul) > val(cur, ul);
        if (midBetter) swap(cur, id);
        if (l == r) return;
        if (leftBetter != midBetter) add(v << 1, l, mid, id);
        else                         add(v << 1 | 1, mid + 1, r, id);
    }
    double query(int v, int l, int r, int u) const {
        double res = -INFDB;
        if (tr[v] != -1) res = val(tr[v], u);
        if (l == r) return res;
        int mid = (l + r) >> 1;
        if (u <= (*crd)[mid]) return max(res, query(v << 1, l, mid, u));
        return max(res, query(v << 1 | 1, mid + 1, r, u));
    }
};

/* ---------------- Li Chao: DP lines b + m*u, minimum (exact) ------------ */
struct LiChaoDP {
    const vector<int>* crd;
    const vector<long long> *mm, *bb;
    vector<int> tr;
    inline long long val(int id, int u) const {
        return (*bb)[id] + (*mm)[id] * (long long)u;
    }
    void init(const vector<int>& c, const vector<long long>& m,
              const vector<long long>& b) {
        crd = &c; mm = &m; bb = &b;
        tr.assign(4 * (int)c.size() + 4, -1);
    }
    void add(int v, int l, int r, int id) {
        int &cur = tr[v];
        if (cur == -1) { cur = id; return; }
        int mid = (l + r) >> 1;
        int um = (*crd)[mid], ul = (*crd)[l];
        bool midBetter  = val(id, um) < val(cur, um);
        bool leftBetter = val(id, ul) < val(cur, ul);
        if (midBetter) swap(cur, id);
        if (l == r) return;
        if (leftBetter != midBetter) add(v << 1, l, mid, id);
        else                         add(v << 1 | 1, mid + 1, r, id);
    }
    long long query(int v, int l, int r, int u) const {
        long long res = INFLL;
        if (tr[v] != -1) res = val(tr[v], u);
        if (l == r) return res;
        int mid = (l + r) >> 1;
        if (u <= (*crd)[mid]) return min(res, query(v << 1, l, mid, u));
        return min(res, query(v << 1 | 1, mid + 1, r, u));
    }
};

vector<double> Dval, Mval;

void processD(int v, int l, int r) {
    if (!qs[v].empty()) {
        vector<int> coords;
        coords.reserve(qs[v].size());
        for (int id : qs[v]) coords.push_back(xs[id]);
        sort(coords.begin(), coords.end());
        coords.erase(unique(coords.begin(), coords.end()), coords.end());
        LiChaoC lc;
        lc.init(coords);
        int m = (int)coords.size();
        for (int k = l; k <= r; ++k) lc.add(1, 0, m - 1, k);
        for (int id : qs[v]) {
            double cur = lc.query(1, 0, m - 1, xs[id]);
            if (cur < Dval[id]) Dval[id] = cur;
        }
    }
    if (l < r) {
        int mid = (l + r) >> 1;
        processD(v << 1, l, mid);
        processD(v << 1 | 1, mid + 1, r);
    }
}

void processM(int v, int l, int r) {
    if (!qs[v].empty()) {
        vector<int> coords;
        coords.reserve(qs[v].size());
        for (int id : qs[v]) coords.push_back(xs[id]);
        sort(coords.begin(), coords.end());
        coords.erase(unique(coords.begin(), coords.end()), coords.end());
        LiChaoL lc;
        lc.init(coords);
        int m = (int)coords.size();
        for (int k = l; k <= r; ++k) lc.add(1, 0, m - 1, k);
        for (int id : qs[v]) {
            double cur = lc.query(1, 0, m - 1, xs[id]);
            if (cur > Mval[id]) Mval[id] = cur;
        }
    }
    if (l < r) {
        int mid = (l + r) >> 1;
        processM(v << 1, l, mid);
        processM(v << 1 | 1, mid + 1, r);
    }
}

vector<int> segL, segR;
vector<int> Rreach;              // last j with edge(i,j) upper-valid

void collectDP(int v, int l, int r, vector<vector<int>>& byR) {
    if (!qs[v].empty()) {
        segL[v] = l; segR[v] = r;
        byR[r].push_back(v);     // all its lines are known at time r
    }
    if (l < r) {
        int mid = (l + r) >> 1;
        collectDP(v << 1, l, mid, byR);
        collectDP(v << 1 | 1, mid + 1, r, byR);
    }
}

void processDPNode(int v, vector<long long>& dp, vector<long long>& contrib,
                   const vector<long long>& lm, const vector<long long>& lbv) {
    int L = segL[v], R = segR[v];
    vector<int> lines;
    lines.reserve(R - L + 1);
    for (int i = L; i <= R; ++i)
        if (dp[i] < INFLL) lines.push_back(i);
    if (lines.empty()) return;

    sort(lines.begin(), lines.end(), [&](int a, int b) {
        if (Rreach[a] != Rreach[b]) return Rreach[a] > Rreach[b];
        return a > b;
    });

    vector<int> coords;
    coords.reserve(qs[v].size());
    for (int j : qs[v]) coords.push_back(xs[j]);
    sort(coords.begin(), coords.end());
    coords.erase(unique(coords.begin(), coords.end()), coords.end());

    LiChaoDP lc;
    lc.init(coords, lm, lbv);
    int qn = (int)coords.size();
    int ptr = 0, sz = (int)lines.size();

    // qs[v] is in increasing j; process decreasing j, adding lines as R_i>=j.
    for (int qi = (int)qs[v].size() - 1; qi >= 0; --qi) {
        int j = qs[v][qi];
        while (ptr < sz && Rreach[lines[ptr]] >= j) {
            lc.add(1, 0, qn - 1, lines[ptr]);
            ++ptr;
        }
        if (ptr == 0) continue;
        long long cur = lc.query(1, 0, qn - 1, xs[j]);
        if (cur < contrib[j]) contrib[j] = cur;
    }
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int alphai, betai;
    if (!(cin >> n >> h >> alphai >> betai)) return 0;
    alpha = alphai; bet = betai;

    xs.resize(n);
    vector<long long> yin(n);
    for (int i = 0; i < n; ++i) cin >> xs[i] >> yin[i];

    tt.resize(n);
    ss.resize(n);
    for (int i = 0; i < n; ++i) {
        tt[i] = (int)(h - yin[i]);
        ss[i] = sqrt(2.0 * (double)tt[i]);
    }

    int N4 = 4 * n + 5;
    qs.assign(N4, {});

    /* ---------- D_i = min_{k>=i} C_k(x_i) ---------- */
    Dval.assign(n, INFDB);
    for (int i = 0; i + 1 < n; ++i)
        addQuery(1, 0, n - 1, i + 1, n - 1, i);
    processD(1, 0, n - 1);

    Rreach.assign(n, n - 1);
    for (int i = 0; i < n; ++i) {
        double d = Dval[i];
        double own = (double)xs[i] + 2.0 * (double)tt[i];
        if (own < d) d = own;
        int r = (int)(upper_bound(xs.begin(), xs.end(), d + EPS) - xs.begin()) - 1;
        if (r < i) r = i;
        if (r > n - 1) r = n - 1;
        Rreach[i] = r;
    }

    for (auto& v : qs) v.clear();

    /* ---------- M_j = max_{k<=j} L_k(x_j) ---------- */
    Mval.assign(n, -INFDB);
    for (int j = 0; j < n; ++j)
        addQuery(1, 0, n - 1, 0, j, j);
    processM(1, 0, n - 1);

    vector<int> lowIdx(n);
    for (int j = 0; j < n; ++j) {
        int p = (int)(lower_bound(xs.begin(), xs.end(), Mval[j] - EPS) - xs.begin());
        if (p < 0) p = 0;
        if (p > j) p = j;
        lowIdx[j] = p;
    }

    for (auto& v : qs) v.clear();

    /* ---------- DP ---------- */
    vector<long long> dp(n, INFLL), contrib(n, INFLL);
    vector<long long> lm(n, 0), lb(n, 0);

    if (n == 1) {
        cout << (long long)alpha * tt[0] << '\n';
        return 0;
    }

    for (int j = 1; j < n; ++j) {
        int lo = lowIdx[j], hi = j - 1;
        if (lo <= hi) addQuery(1, 0, n - 1, lo, hi, j);
    }

    segL.assign(N4, 0); segR.assign(N4, 0);
    vector<vector<int>> byR(n);
    collectDP(1, 0, n - 1, byR);

    dp[0] = alpha * (long long)tt[0];
    lm[0] = -2LL * bet * xs[0];
    lb[0] = dp[0] + bet * 1LL * xs[0] * xs[0];

    for (int j = 1; j < n; ++j) {
        for (int v : byR[j - 1]) processDPNode(v, dp, contrib, lm, lb);
        if (contrib[j] < INFLL) {
            long long xx = xs[j];
            dp[j] = alpha * (long long)tt[j] + bet * xx * xx + contrib[j];
            lm[j] = -2LL * bet * xx;
            lb[j] = dp[j] + bet * xx * xx;
        }
    }

    if (dp[n - 1] >= INFLL) cout << "impossible\n";
    else cout << dp[n - 1] << '\n';
    return 0;
}
