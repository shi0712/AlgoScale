#include <bits/stdc++.h>
using namespace std;

static int n;
static const int T = 64;
static double Cb[2 * T + 8][2 * T + 8];

static bool feas(int k, int z, int a, int w) {
    if (z > k || a < 0 || w < 0) return false;
    int kk = k - z, d = w - a;
    if (d > kk || -d > kk) return false;
    return ((kk + d) & 1) == 0;
}
static double cnt0(int k, int a, int w) {
    if (a < 0 || w < 0) return 0;
    int d = w - a;
    if (d > k || -d > k || ((k + d) & 1)) return 0;
    double r = Cb[k][(k + d) / 2];
    int up2 = (k + w + a + 2) / 2;
    if (up2 <= k) r -= Cb[k][up2];
    return r;
}
static double cntz(int k, int z, int a, int w) {
    if (z > k) return 0;
    return Cb[k][z] * cnt0(k - z, a, w);
}

struct Res { int worst; double ex; int m1; int t; };  // t: 0=F,1=G,2=N
// memo
static const int DD = 2 * T + 1;
static signed char mw[2][2][2][T + 1][T + 1][DD];
static float mex[2][2][2][T + 1][T + 1][DD];
static unsigned char mm[2][2][2][T + 1][T + 1][DD], mt[2][2][2][T + 1][T + 1][DD];

static Res get(int L, int a, int b, int eL, int eR, int s) {
    if (s == 1 && L == 1) return {0, 0, 0, 0};
    if (s == 2 && L == 2) return {1, 1, 0, 2};
    int A = min(a, L);
    int d = b - a;
    b = A + d; a = A;
    int di = d + L;
    signed char &W = mw[s - 1][eL][eR][L][a][di];
    if (W > 0) return {W, mex[s - 1][eL][eR][L][a][di], mm[s - 1][eL][eR][L][a][di], mt[s - 1][eL][eR][L][a][di]};
    int bestW = 1000; double bestE = 1e18; int bm = 1, bt = 0;
    int VM = 3 * L + 8;
    vector<double> vw(VM);
    vector<int> vworst(VM), vcnt(VM);
    vector<double> vsum(VM);
    for (int m1 = 1; m1 < L; m1++) {
        int k1 = m1, k2 = L - m1;
        // precompute per hypothesis/w sub costs
        for (int t = 0; t < 2; t++) {
            bool freeF = (eL && m1 == 1), freeG = (eR && m1 == L - 1);
            int q1 = (t == 0 ? (freeF ? 0 : 1) : (freeG ? 0 : 1));
            int q2 = (t == 0 ? (freeG ? 0 : 1) : (freeF ? 0 : 1));
            fill(vworst.begin(), vworst.end(), 0);
            fill(vcnt.begin(), vcnt.end(), 0);
            fill(vsum.begin(), vsum.end(), 0.0);
            fill(vw.begin(), vw.end(), 0.0);
            // entries
            for (int z1 = 0; z1 <= s; z1++) {
                int z2 = s - z1;
                if (z1 > k1 || z2 > k2) continue;
                int lo = max(0, a - (k1 - z1)), hi = a + (k1 - z1);
                for (int w = lo; w <= hi; w++) {
                    if (!feas(k1, z1, a, w) || !feas(k2, z2, w, b)) continue;
                    double wt = cntz(k1, z1, a, w) * cntz(k2, z2, w, b);
                    if (wt <= 0) continue;
                    int sw; double se;
                    if (s == 1) {
                        Res r = (z1 == 1) ? get(k1, a, w, eL, 0, 1) : get(k2, w, b, 0, eR, 1);
                        sw = r.worst; se = r.ex;
                    } else if (z1 == 2) {
                        Res r = get(k1, a, w, eL, 0, 2); sw = r.worst; se = r.ex;
                    } else if (z1 == 0) {
                        Res r = get(k2, w, b, 0, eR, 2); sw = r.worst; se = r.ex;
                    } else {
                        Res r1 = get(k1, a, w, eL, 0, 1), r2 = get(k2, w, b, 0, eR, 1);
                        sw = 1 + max(r1.worst, r2.worst); se = 1 + 0.5 * (r1.ex + r2.ex);
                    }
                    int v = w + (t == 0 ? z1 : z2);
                    vcnt[v]++;
                    vworst[v] = max(vworst[v], sw);
                    vsum[v] += wt * se;
                    vw[v] += wt;
                }
            }
            int worst = 0; double tot = 0, ex = 0;
            for (int v = 0; v < VM; v++) {
                if (!vcnt[v]) continue;
                int qc = q1 + (vcnt[v] > 1 ? q2 : 0);
                worst = max(worst, qc + vworst[v]);
                ex += vsum[v] + qc * vw[v];
                tot += vw[v];
            }
            if (tot <= 0) continue;
            ex /= tot;
            if (worst < bestW || (worst == bestW && ex < bestE - 1e-9)) {
                bestW = worst; bestE = ex; bm = m1; bt = t;
            }
        }
    }
    W = (signed char)bestW;
    mex[s - 1][eL][eR][L][a][di] = (float)bestE;
    mm[s - 1][eL][eR][L][a][di] = bm;
    mt[s - 1][eL][eR][L][a][di] = bt;
    return {bestW, bestE, bm, bt};
}

static const char* TREES = "Q 1 2 2 1:1 2:2|L 3|Q 2 3 2 1:3 2:4|L 1|L 2#Q 1 2 4 1:1 2:8|Q 1 2 2 1:2 2:5|Q 1 3 4 1:3 2:4|L 3|L 4|Q 2 4 2 2:6 3:7|L 1|L 2|Q 3 4 2 1:9 2:12|Q 2 3 4 1:10 2:11|L 2|L 1|Q 1 3 2 2:13 3:14|L 4|L 3#Q 1 3 2 2:1 3:10|Q 4 5 2 1:2 2:7|Q 1 2 2 1:3 2:4|L 3|Q 2 3 2 1:5 2:6|L 1|L 2|Q 1 4 2 2:8 3:9|L 5|L 4|Q 2 5 2 2:11 3:12|L 1|Q 3 5 2 2:13 3:14|L 2|L 3#Q 1 3 2 2:1 3:22|Q 4 6 2 2:2 3:13|Q 1 3 6 1:3 2:8|Q 1 2 2 1:4 2:5|L 3|Q 2 3 2 1:6 2:7|L 1|L 2|Q 1 5 2 3:9 4:10|L 6|Q 5 6 2 1:11 2:12|L 4|L 5|Q 1 4 2 2:14 3:17|Q 1 5 3 2:15 3:16|L 5|L 6|Q 1 4 6 1:18 2:19|L 4|Q 1 5 2 3:20 4:21|L 6|L 5|Q 4 6 2 2:23 3:32|Q 1 2 6 1:24 2:27|Q 2 6 2 3:25 4:26|L 1|L 2|Q 3 6 2 2:28 3:31|Q 2 3 6 1:29 2:30|L 2|L 1|L 3|Q 1 3 6 1:33 2:38|Q 2 6 2 3:34 4:35|L 1|Q 3 6 2 3:36 4:37|L 2|L 3|Q 1 4 2 3:39 4:42|Q 1 5 2 3:40 4:41|L 6|L 5|L 4#";

static int ask(int l, int r, int k);
static void answer(int y);
static void runTree() {
    // optimal decision trees for small n (precomputed by exhaustive search)
    string all = TREES;
    vector<string> secs; { string cur; for (char c : all) { if (c == '#') { secs.push_back(cur); cur.clear(); } else cur += c; } }
    string sec = secs[n - 3];
    vector<string> nodes; { string cur; for (char c : sec) { if (c == '|') { nodes.push_back(cur); cur.clear(); } else cur += c; } nodes.push_back(cur); }
    int id = 0;
    while (true) {
        istringstream is(nodes[id]);
        string ty; is >> ty;
        if (ty == "L") { int y; is >> y; answer(y); }
        int l, r, k; is >> l >> r >> k;
        int v = ask(l, r, k);
        string tok; int nxt = -1;
        while (is >> tok) { auto p = tok.find(':'); if (stoi(tok.substr(0, p)) == v) nxt = stoi(tok.substr(p + 1)); }
        if (nxt < 0) exit(0);
        id = nxt;
    }
}

static int ask(int l, int r, int k) {
    printf("? %d %d %d\n", l, r, k);
    fflush(stdout);
    int x;
    if (scanf("%d", &x) != 1 || x < 0) exit(0);
    return x;
}
static int askF(int m) {  // h(m) + singletons in [1,m]
    if (m == 1) return 1;
    return 2 * ask(1, m, 2) - m;
}
static int askG(int m) {  // h(m) + singletons in [m+1,n]
    if (m == n - 1) return 1;
    return 2 * ask(m + 1, n, 2) - (n - m);
}
static bool zInPrefix(int m) {  // is position of n in [1,m]
    if (m >= 2) return ask(1, m, n) == 2;
    return ask(m + 1, n, n) != 2;
}
static void answer(int y) {
    printf("! %d\n", y);
    fflush(stdout);
    exit(0);
}

int main() {
    if (scanf("%d", &n) != 1) return 0;
    for (int i = 0; i < 2 * T + 8; i++) {
        Cb[i][0] = 1;
        for (int j = 1; j <= i; j++) Cb[i][j] = Cb[i - 1][j - 1] + (j < i ? Cb[i - 1][j] : 0);
    }
    if (n <= 6) runTree();
    int s = (n % 2 == 0) ? 2 : 1;
    int l = 1, r = n, a = 0, b = 0, zl = 0, zr = 0;
    while (true) {
        int L = r - l + 1;
        if (s == 1 && L == 1) answer(l);
        if (s == 2 && L == 2) {
            bool zl_ = zInPrefix(l);
            answer(zl_ ? l + 1 : l);
        }
        int m1, t;
        if (L <= T) {
            Res res = get(L, a, b, l == 1, r == n, s);
            m1 = res.m1; t = res.t;
        } else {
            m1 = L / 2; t = 0;
        }
        int m = l + m1 - 1;
        int k1 = m1, k2 = L - m1;
        int v1 = (t == 0) ? askF(m) - zl : askG(m) - zr;
        vector<pair<int, int>> hyp;  // (z1, w)
        for (int z1 = 0; z1 <= s; z1++) {
            int z2 = s - z1;
            int w = v1 - (t == 0 ? z1 : z2);
            if (feas(k1, z1, a, w) && feas(k2, z2, w, b)) hyp.push_back({z1, w});
        }
        int z1, w;
        if (hyp.size() == 1) {
            z1 = hyp[0].first; w = hyp[0].second;
        } else {
            int v2 = (t == 0) ? askG(m) - zr : askF(m) - zl;
            int Fr = t == 0 ? v1 : v2, Gr = t == 0 ? v2 : v1;
            z1 = (Fr - Gr + s) / 2;
            w = Fr - z1;
        }
        if (s == 1) {
            if (z1 == 1) { r = m; b = w; }
            else { l = m + 1; a = w; }
        } else {
            if (z1 == 2) { r = m; b = w; }
            else if (z1 == 0) { l = m + 1; a = w; }
            else {
                bool zp = zInPrefix(m);
                s = 1;
                if (zp) { l = m + 1; a = w; zl++; }
                else { r = m; b = w; zr++; }
            }
        }
    }
}
