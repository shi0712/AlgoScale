#include <bits/stdc++.h>
using namespace std;
typedef signed char i8;
typedef long long ll;
typedef unsigned long long u64e;

// ---------------------------------------------------------------------------
// Scheme: Lindstrom-style recursive +-1 detecting matrix. Some mid-size nodes
// ("base" nodes) get E extra random columns, decoded by brute force over the
// 2^E extra bits (wrong branches fail fast).
// ---------------------------------------------------------------------------
static int gval[400];
static void initG(int maxM) {
    gval[1] = 1;
    for (int M = 2; M <= maxM; M++) {
        int best = -1;
        for (int mp = 1; mp <= M / 2; mp++) best = max(best, gval[M - mp] + gval[mp] + mp - 1);
        gval[M] = best;
    }
}
static int gsplitPure(int M) {
    int best = -1, bs = 1;
    for (int mp = 1; mp <= M / 2; mp++) {
        int v = gval[M - mp] + gval[mp] + mp - 1;
        if (v > best) { best = v; bs = mp; }
    }
    return bs;
}

struct Node {
    int M = 0, m = 0, mp = 0, N = 0, NA = 0, NB = 0, NC = 0;
    int E = 0;             // extras (base node)
    bool pure = true;      // no base node in subtree
    Node *A = nullptr, *B = nullptr, *core = nullptr;
    vector<vector<i8>> rows; // M x N
    vector<vector<ll>> X;    // base: E columns, each length M (column-major)
};

static map<int, Node*> pureMemo;
static Node* buildPure(int M) {
    auto it = pureMemo.find(M);
    if (it != pureMemo.end()) return it->second;
    Node* nd = new Node();
    nd->M = M;
    if (M == 1) {
        nd->m = 1; nd->mp = 0; nd->N = 1; nd->NA = 1;
        nd->rows = {vector<i8>{1}};
    } else {
        int mp = gsplitPure(M), m = M - mp;
        nd->m = m; nd->mp = mp;
        nd->A = buildPure(m); nd->B = buildPure(mp);
        nd->NA = nd->A->N; nd->NB = nd->B->N; nd->NC = mp - 1;
        nd->N = nd->NA + nd->NB + nd->NC;
    }
    pureMemo[M] = nd;
    return nd;
}

// fill rows for split node from children rows
static void fillSplitRows(Node* nd) {
    int mp = nd->mp, m = nd->m;
    nd->rows.assign(nd->M, vector<i8>(nd->N));
    for (int i = 0; i < mp; i++) {
        auto& u = nd->rows[2 * i]; auto& v = nd->rows[2 * i + 1];
        for (int k = 0; k < nd->NA; k++) u[k] = v[k] = nd->A->rows[i][k];
        for (int k = 0; k < nd->NB; k++) { u[nd->NA + k] = nd->B->rows[i][k]; v[nd->NA + k] = -nd->B->rows[i][k]; }
        for (int j = 0; j < nd->NC; j++) {
            int idx = nd->NA + nd->NB + j;
            u[idx] = 1; v[idx] = (j == i) ? 1 : -1;
        }
    }
    for (int i = mp; i < m; i++) {
        auto& u = nd->rows[mp + i];
        for (int k = 0; k < nd->NA; k++) u[k] = nd->A->rows[i][k];
        for (int k = nd->NA; k < nd->N; k++) u[k] = 1;
    }
}
static void ensureRows(Node* nd) {
    if (!nd->rows.empty()) return;
    if (nd->A) ensureRows(nd->A);
    if (nd->B) ensureRows(nd->B);
    if (nd->core) ensureRows(nd->core);
    if (nd->core) {
        nd->rows.assign(nd->M, vector<i8>(nd->N));
        for (int r = 0; r < nd->M; r++) {
            for (int k = 0; k < nd->core->N; k++) nd->rows[r][k] = nd->core->rows[r][k];
            for (int e = 0; e < nd->E; e++) nd->rows[r][nd->core->N + e] = (i8)nd->X[e][r];
        }
    } else fillSplitRows(nd);
}

static Node* buildBase(int M, int E, uint64_t seed) {
    Node* nd = new Node();
    nd->M = M; nd->E = E; nd->pure = false;
    nd->core = buildPure(M);
    nd->N = nd->core->N + E;
    mt19937_64 rng(seed);
    nd->X.assign(E, vector<ll>(M));
    for (int e = 0; e < E; e++)
        for (int r = 0; r < M; r++) nd->X[e][r] = (r == 0) ? 1 : ((rng() & 1) ? 1 : -1);
    return nd;
}

// fast exact decode of a pure node. fixed may be null.
static bool decLin(const Node* nd, const ll* vals, i8* out) {
    if (nd->M == 1) {
        ll v = vals[0];
        if (v != 0 && v != 1) return false;
        out[0] = (i8)v;
        return true;
    }
    int mp = nd->mp, m = nd->m, NC = nd->NC;
    ll S[400], D[400], bv[400], av[400];
    for (int i = 0; i < mp; i++) {
        ll s = vals[2 * i] + vals[2 * i + 1], d = vals[2 * i] - vals[2 * i + 1];
        if (s & 1) return false;
        S[i] = s / 2; D[i] = d / 2;
    }
    int p = (int)(S[mp - 1] & 1);
    ll sumC = 0;
    i8* cout_ = out + nd->NA + nd->NB;
    for (int j = 0; j < NC; j++) { int c = (int)((S[j] - p) & 1); cout_[j] = (i8)c; sumC += c; }
    for (int i = 0; i < mp; i++) bv[i] = D[i] - (sumC - (i < mp - 1 ? cout_[i] : 0));
    if (!decLin(nd->B, bv, out + nd->NA)) return false;
    ll nb = 0;
    for (int k = 0; k < nd->NB; k++) nb += out[nd->NA + k];
    for (int i = 0; i < mp; i++) av[i] = S[i] - (i < mp - 1 ? cout_[i] : 0);
    for (int i = mp; i < m; i++) {
        av[i] = vals[mp + i] - nb - sumC;
        if (((av[i] - p) & 1) != 0) return false;
    }
    return decLin(nd->A, av, out);
}

// fast decoder variant with int arrays
static bool decLin2(const Node* nd, const int* vals, i8* out) {
    if (nd->M == 1) {
        int v = vals[0];
        if ((unsigned)v > 1u) return false;
        out[0] = (i8)v;
        return true;
    }
    const int mp = nd->mp, m = nd->m, NC = nd->NC;
    int S[mp], D[mp], bv[mp], av[m];
    for (int i = 0; i < mp; i++) {
        int a = vals[2 * i], b = vals[2 * i + 1];
        if ((a + b) & 1) return false;
        S[i] = (a + b) >> 1; D[i] = (a - b) >> 1;
    }
    const int p = S[mp - 1] & 1;
    int sumC = 0;
    i8* co = out + nd->NA + nd->NB;
    for (int j = 0; j < NC; j++) { int c = (S[j] - p) & 1; co[j] = (i8)c; sumC += c; }
    for (int i = 0; i < mp - 1; i++) bv[i] = D[i] - sumC + co[i];
    bv[mp - 1] = D[mp - 1] - sumC;
    if (!decLin2(nd->B, bv, out + nd->NA)) return false;
    int nb = 0;
    for (int k = 0; k < nd->NB; k++) nb += out[nd->NA + k];
    for (int i = 0; i < mp - 1; i++) av[i] = S[i] - co[i];
    av[mp - 1] = S[mp - 1];
    for (int i = mp; i < m; i++) {
        av[i] = vals[mp + i] - nb - sumC;
        if ((av[i] - p) & 1) return false;
    }
    return decLin2(nd->A, av, out);
}

static long long decodeWork;
struct Amb { int offset; vector<vector<i8>> alts; };
static int AMB_CAP = 2000000;
// decode with all rows known; base-node ambiguities are recorded in amb (product structure)
static bool decodeRep(const Node* nd, const ll* vals, i8* out, int offset, vector<Amb>& amb) {
    if (nd->pure) { decodeWork++; return decLin(nd, vals, out); }
    if (nd->core) {
        int M = nd->M, E = nd->E;
        const Node* co = nd->core;
        vector<i8> buf(nd->N);
        Amb a; a.offset = offset;
        // build GF(2) linear conditions on e (E <= 63)
        vector<pair<u64e, int>> conds; // (mask, rhs)
        if (co->M >= 2) {
            int mp = co->mp, m = co->m;
            auto Smask = [&](int i) { u64e mk = 0; for (int k = 0; k < E; k++) if (nd->X[k][2 * i] == nd->X[k][2 * i + 1]) mk |= 1ULL << k; return mk; };
            auto Dmask = [&](int i) { u64e mk = 0; for (int k = 0; k < E; k++) if (nd->X[k][2 * i] != nd->X[k][2 * i + 1]) mk |= 1ULL << k; return mk; };
            auto S0 = [&](int i) { return (int)((((vals[2 * i] + vals[2 * i + 1]) / 2) % 2 + 2) % 2); };
            auto D0 = [&](int i) { return (int)((((vals[2 * i] - vals[2 * i + 1]) / 2) % 2 + 2) % 2); };
            for (int i = 0; i < mp; i++) if ((vals[2 * i] + vals[2 * i + 1]) & 1) return false;
            // c_j = S_j + p, p = S_{mp-1}; represent affine forms as (mask, const)
            auto Sf = [&](int i) { return make_pair(Smask(i), S0(i)); };
            auto Df = [&](int i) { return make_pair(Dmask(i), D0(i)); };
            pair<u64e,int> P = Sf(mp - 1);
            auto Cf = [&](int j) -> pair<u64e,int> {
                if (j >= mp - 1) return {0, 0};
                auto s = Sf(j); return {s.first ^ P.first, s.second ^ P.second};
            };
            auto add = [](pair<u64e,int> x, pair<u64e,int> y) { return make_pair(x.first ^ y.first, x.second ^ y.second); };
            const Node* B = co->B;
            if (B->M >= 2) {
                for (int k = 0; k < B->mp; k++) {
                    auto f = add(add(Df(2 * k), Cf(2 * k)), add(Df(2 * k + 1), Cf(2 * k + 1)));
                    conds.push_back({f.first, f.second}); // f == 0
                }
            }
            u64e allm = (E == 64) ? ~0ULL : ((1ULL << E) - 1);
            for (int i = mp; i < m; i++) {
                int r = mp + i;
                pair<u64e,int> wf = {allm, (int)(((vals[r] % 2) + 2) % 2)};
                auto f = add(add(wf, Df(0)), add(Cf(0), P));
                conds.push_back({f.first, f.second});
            }
        }
        // gaussian elimination: find e with mask.e == rhs for all conds
        vector<u64e> rowm; vector<int> rowr; vector<int> piv;
        for (auto& cd : conds) {
            u64e mk = cd.first; int rh = cd.second;
            for (size_t t = 0; t < rowm.size(); t++) if (mk >> piv[t] & 1) { mk ^= rowm[t]; rh ^= rowr[t]; }
            if (mk == 0) { if (rh) return false; continue; }
            int pv = __builtin_ctzll(mk);
            for (size_t t = 0; t < rowm.size(); t++) if (rowm[t] >> pv & 1) { rowm[t] ^= mk; rowr[t] ^= rh; }
            rowm.push_back(mk); rowr.push_back(rh); piv.push_back(pv);
        }
        u64e pivMask = 0;
        for (int pv : piv) pivMask |= 1ULL << pv;
        // particular solution: free vars = 0, pivots = rhs
        u64e e0 = 0;
        for (size_t t = 0; t < rowm.size(); t++) if (rowr[t]) e0 |= 1ULL << piv[t];
        // basis vectors for free vars
        vector<u64e> basis;
        for (int k = 0; k < E; k++) if (!(pivMask >> k & 1)) {
            u64e b = 1ULL << k;
            for (size_t t = 0; t < rowm.size(); t++) if (rowm[t] >> k & 1) b |= 1ULL << piv[t];
            basis.push_back(b);
        }
        vector<int> w(M);
        for (int r = 0; r < M; r++) {
            ll x = vals[r];
            for (int k = 0; k < E; k++) if (e0 >> k & 1) x -= nd->X[k][r];
            w[r] = (int)x;
        }
        vector<vector<int>> Xi(E, vector<int>(M));
        for (int k = 0; k < E; k++) for (int r = 0; r < M; r++) Xi[k][r] = (int)nd->X[k][r];
        u64e e = e0;
        ll total = 1LL << basis.size();
        for (ll g = 0; g < total; g++) {
            if (g > 0) {
                u64e bv = basis[__builtin_ctzll(g)];
                int* wp = w.data();
                while (bv) {
                    int k = __builtin_ctzll(bv); bv &= bv - 1;
                    const int* col = Xi[k].data();
                    if (e >> k & 1) { for (int r = 0; r < M; r++) wp[r] += col[r]; }
                    else { for (int r = 0; r < M; r++) wp[r] -= col[r]; }
                    e ^= 1ULL << k;
                }
            }
            decodeWork++;
            if (decLin2(co, w.data(), buf.data())) {
                for (int k = 0; k < E; k++) buf[co->N + k] = (i8)(e >> k & 1);
                if ((int)a.alts.size() < AMB_CAP) a.alts.push_back(buf);
            }
        }
        if (a.alts.empty()) return false;
        copy(a.alts[0].begin(), a.alts[0].end(), out);
        if (a.alts.size() > 1) amb.push_back(std::move(a));
        return true;
    }
    int mp = nd->mp, m = nd->m, NC = nd->NC;
    vector<ll> S(mp), D(mp), bv(mp), av(m);
    for (int i = 0; i < mp; i++) {
        ll s = vals[2 * i] + vals[2 * i + 1], d = vals[2 * i] - vals[2 * i + 1];
        if (s & 1) return false;
        S[i] = s / 2; D[i] = d / 2;
    }
    int p = (int)(S[mp - 1] & 1);
    ll sumC = 0;
    i8* co = out + nd->NA + nd->NB;
    for (int j = 0; j < NC; j++) { co[j] = (i8)((S[j] - p) & 1); sumC += co[j]; }
    for (int i = 0; i < mp; i++) bv[i] = D[i] - (sumC - (i < mp - 1 ? co[i] : 0));
    for (int i = 0; i < mp; i++) av[i] = S[i] - (i < mp - 1 ? co[i] : 0);
    if (!decodeRep(nd->B, bv.data(), out + nd->NA, offset + nd->NA, amb)) return false;
    ll nb = 0;
    for (int k = 0; k < nd->NB; k++) nb += out[nd->NA + k];
    for (int i = mp; i < m; i++) {
        av[i] = vals[mp + i] - nb - sumC;
        if (((av[i] - p) & 1) != 0) return false;
    }
    return decodeRep(nd->A, av.data(), out, offset, amb);
}

// ---------------------------------------------------------------------------
// Planner: h[M] = max coins; base nodes allowed for sizes with Emax(M).
// ---------------------------------------------------------------------------
static const float LTAB[33][23] = {
{0.000,0.149,0.399,0.650,0.893,1.362,1.890,2.465,2.961,3.680,4.228,4.968,5.714,6.359,7.062,7.944,8.702,9.637,10.384,11.546,12.261,12.659,14.264},
{0.000,0.091,0.254,0.517,0.691,0.949,1.484,1.797,2.332,2.828,3.357,4.001,4.775,5.268,6.122,6.878,7.444,8.171,9.005,9.708,10.599,11.695,12.128},
{0.000,0.082,0.164,0.358,0.557,0.878,1.163,1.591,1.971,2.562,3.050,3.456,4.122,4.713,5.303,6.024,6.955,7.529,8.238,9.075,9.916,10.049,10.943},
{0.000,0.048,0.114,0.308,0.418,0.578,0.862,1.227,1.527,1.988,2.524,2.982,3.588,4.147,4.905,5.495,5.951,6.852,7.583,8.528,8.845,9.734,11.121},
{0.000,0.037,0.116,0.213,0.369,0.555,0.680,1.016,1.366,1.654,2.040,2.653,3.173,3.672,4.374,5.084,5.825,6.422,7.315,8.066,8.795,9.108,9.694},
{0.000,0.027,0.053,0.155,0.277,0.472,0.706,0.811,1.091,1.554,1.955,2.449,3.066,3.645,4.034,4.706,5.138,5.992,7.056,7.056,8.156,9.233,10.318},
{0.000,0.071,0.117,0.227,0.282,0.405,0.595,0.743,0.908,1.366,1.719,2.098,2.547,2.991,3.548,4.329,4.698,5.715,6.518,7.551,8.380,8.380,10.088},
{0.000,0.022,0.059,0.116,0.186,0.268,0.404,0.544,0.729,1.096,1.481,1.724,2.230,2.603,3.268,3.649,4.323,4.867,5.430,6.519,7.204,7.794,7.794},
{0.000,0.032,0.045,0.093,0.171,0.286,0.396,0.593,0.830,1.069,1.514,1.901,2.209,2.731,3.352,3.944,4.416,4.980,5.559,6.713,7.514,8.409,8.723},
{0.000,0.024,0.041,0.079,0.123,0.155,0.235,0.365,0.487,0.664,0.920,1.066,1.444,1.802,2.177,2.649,3.352,3.675,4.611,5.212,5.920,5.920,6.978},
{0.000,0.026,0.026,0.056,0.088,0.103,0.175,0.215,0.363,0.474,0.673,0.890,1.155,1.479,1.785,2.261,2.794,3.250,3.818,3.950,4.692,5.563,5.563},
{0.000,0.008,0.016,0.044,0.074,0.095,0.131,0.150,0.238,0.317,0.449,0.587,0.751,0.947,1.301,1.637,2.072,2.496,3.071,3.444,3.943,4.556,5.681},
{0.000,0.003,0.019,0.022,0.051,0.066,0.090,0.125,0.192,0.250,0.342,0.523,0.679,0.868,1.151,1.497,2.001,2.450,2.789,2.789,3.463,4.204,4.478},
{0.000,0.002,0.013,0.053,0.059,0.076,0.106,0.110,0.167,0.252,0.301,0.412,0.512,0.771,1.087,1.171,1.509,2.089,2.131,2.763,3.285,3.661,4.318},
{0.000,0.003,0.004,0.009,0.012,0.023,0.049,0.100,0.104,0.135,0.205,0.296,0.411,0.527,0.748,0.957,1.187,1.521,1.703,2.368,2.417,2.777,3.408},
{0.000,0.002,0.013,0.016,0.027,0.039,0.039,0.054,0.065,0.127,0.169,0.236,0.345,0.444,0.629,0.819,1.014,1.448,1.758,1.758,2.229,3.693,3.693},
{0.000,0.002,0.004,0.011,0.013,0.017,0.017,0.056,0.069,0.107,0.137,0.221,0.309,0.422,0.556,0.865,0.996,1.291,1.636,1.636,2.610,2.861,3.542},
{0.000,0.001,0.013,0.024,0.024,0.024,0.059,0.063,0.063,0.067,0.141,0.151,0.208,0.307,0.466,0.539,0.766,0.946,1.297,1.564,1.851,2.364,3.104},
{0.000,0.002,0.002,0.006,0.007,0.012,0.016,0.022,0.040,0.075,0.092,0.114,0.190,0.268,0.339,0.472,0.639,0.784,1.073,1.282,1.706,1.706,1.819},
{0.000,0.000,0.002,0.002,0.003,0.011,0.018,0.025,0.032,0.049,0.059,0.078,0.121,0.194,0.271,0.338,0.515,0.650,0.833,0.966,1.171,1.216,1.437},
{0.000,0.000,0.001,0.001,0.008,0.010,0.016,0.021,0.021,0.037,0.052,0.081,0.106,0.147,0.257,0.387,0.469,0.535,0.692,0.866,1.470,2.047,99.000},
{0.000,0.000,0.001,0.002,0.003,0.007,0.009,0.013,0.014,0.022,0.025,0.063,0.087,0.109,0.159,0.235,0.350,0.386,0.968,0.968,1.424,1.424,99.000},
{0.000,0.001,0.001,0.001,0.006,0.006,0.006,0.017,0.017,0.017,0.026,0.062,0.062,0.078,0.137,0.223,0.317,0.446,0.446,0.811,1.037,1.675,99.000},
{0.000,0.000,0.001,0.001,0.001,0.004,0.004,0.006,0.011,0.022,0.031,0.041,0.056,0.081,0.109,0.168,0.228,0.481,0.497,0.793,0.793,1.277,99.000},
{0.000,0.000,0.003,0.003,0.003,0.003,0.003,0.004,0.013,0.019,0.036,0.046,0.050,0.077,0.132,0.171,0.232,0.319,0.322,0.572,0.597,1.184,99.000},
{0.000,0.000,0.000,0.002,0.002,0.002,0.002,0.002,0.007,0.007,0.017,0.017,0.040,0.047,0.050,0.059,0.107,0.114,0.175,0.462,0.601,0.712,0.783},
{0.000,0.001,0.001,0.001,0.001,0.001,0.003,0.003,0.003,0.009,0.009,0.014,0.014,0.021,0.033,0.044,0.076,0.113,0.180,0.220,0.298,0.298,0.489},
{0.000,0.000,0.001,0.001,0.001,0.003,0.003,0.003,0.003,0.003,0.008,0.008,0.013,0.013,0.026,0.026,0.039,0.094,0.094,0.094,0.213,0.213,0.390},
{0.000,0.000,0.000,0.001,0.001,0.001,0.001,0.002,0.002,0.002,0.004,0.009,0.009,0.014,0.014,0.028,0.031,0.049,0.073,0.146,0.146,0.200,0.230},
{0.000,0.000,0.000,0.000,0.002,0.002,0.002,0.002,0.002,0.002,0.002,0.002,0.004,0.009,0.013,0.017,0.018,0.018,0.053,0.123,0.228,0.228,0.372},
{0.000,0.000,0.000,0.000,0.000,0.000,0.001,0.001,0.001,0.001,0.001,0.002,0.002,0.008,0.008,0.011,0.018,0.018,0.018,0.080,0.089,0.146,0.172},
{0.000,0.000,0.000,0.000,0.000,0.001,0.001,0.001,0.001,0.001,0.002,0.002,0.003,0.003,0.005,0.011,0.011,0.022,0.022,0.063,0.067,0.067,0.111},
{0.000,0.000,0.001,0.001,0.001,0.001,0.001,0.001,0.001,0.001,0.001,0.001,0.001,0.001,0.004,0.004,0.004,0.009,0.014,0.018,0.131,0.131,0.131}};

static double ENDGAME_DIV = 6.0;
static double ACAP = 16.0;
static int EMAXG = 22;
static int hhow[400], hE[400]; // hhow: -1 leaf, 0 base, >0 split mp
static double planCost(int n, double lam, int& outM, bool commit) {
    static double val[400], amb[400];
    static int coins[400], how[400], eb[400];
    val[1] = 1; amb[1] = 0; coins[1] = 1; how[1] = -1; eb[1] = 0;
    int maxM = 300;
    for (int M = 2; M <= maxM; M++) {
        double bv = -1e18; int bh = 0, be = 0; int bc = 0; double ba = 0;
        if (M >= 8 && M <= 40) {
            for (int E = 1; E <= EMAXG; E++) {
                double l = LTAB[M - 8][E];
                if (l > 50) break;
                double v = gval[M] + E - lam * l;
                if (v > bv) { bv = v; bh = 0; be = E; bc = gval[M] + E; ba = l; }
            }
        }
        for (int mp = 1; mp <= M / 2; mp++) {
            int m = M - mp;
            double v = val[m] + val[mp] + mp - 1;
            if (v > bv + 1e-9) { bv = v; bh = mp; be = 0; bc = coins[m] + coins[mp] + mp - 1; ba = amb[m] + amb[mp]; }
        }
        val[M] = bv; how[M] = bh; eb[M] = be; coins[M] = bc; amb[M] = ba;
        if (bc >= n) { maxM = M; break; }
    }
    int M = 1;
    while (coins[M] < n) M++;
    outM = M;
    // collect base nodes
    vector<pair<int,int>> bases; // (M0, E)
    function<void(int)> walk = [&](int x) {
        if (how[x] == -1) return;
        if (how[x] == 0) { bases.push_back({x, eb[x]}); return; }
        walk(x - how[x]); walk(how[x]);
    };
    walk(M);
    int slack = coins[M] - n;
    while (slack > 0) {
        int bi = -1; double bd = -1;
        for (size_t t = 0; t < bases.size(); t++) if (bases[t].second > 0) {
            double d = LTAB[bases[t].first - 8][bases[t].second] - LTAB[bases[t].first - 8][bases[t].second - 1];
            if (d > bd) { bd = d; bi = t; }
        }
        if (bi < 0) break;
        bases[bi].second--; slack--;
    }
    double A = 0;
    for (auto& b : bases) A += LTAB[b.first - 8][b.second];
    if (commit) { for (int x = 1; x <= M; x++) { hhow[x] = how[x]; hE[x] = eb[x]; } }
    if (A > ACAP) return 1e9 + A;
    return M + 1 + A / ENDGAME_DIV;
}
static int planBest(int n) {
    double bestC = 1e18, bestL = 0; int Mout;
    for (int t = 0; t <= 80; t++) {
        double lam = t * 0.25;
        double c = planCost(n, lam, Mout, false);
        if (c < bestC - 1e-9) { bestC = c; bestL = lam; }
    }
    planCost(n, bestL, Mout, true);
    return Mout;
}

static vector<Node*> baseNodes;
static uint64_t seedCounter = 1;
static Node* buildPlan(int M) {
    if (hhow[M] == -1) return buildPure(1);
    if (hhow[M] == 0) {
        Node* b = buildBase(M, hE[M], 0x9E3779B97F4A7C15ULL * (seedCounter++));
        baseNodes.push_back(b);
        return b;
    }
    int mp = hhow[M], m = M - mp;
    Node* A = buildPlan(m);
    Node* B = buildPlan(mp);
    if (A->pure && B->pure) {
        // keep fully pure subtree (memoized)
        return buildPure(M);
    }
    Node* nd = new Node();
    nd->M = M; nd->m = m; nd->mp = mp; nd->A = A; nd->B = B;
    nd->NA = A->N; nd->NB = B->N; nd->NC = mp - 1;
    nd->N = nd->NA + nd->NB + nd->NC;
    nd->pure = A->pure && B->pure;
    return nd;
}
// recompute N after E reductions
static void recomputeN(Node* nd) {
    if (nd->core) { nd->N = nd->core->N + nd->E; return; }
    if (nd->pure) return;
    recomputeN(nd->A); recomputeN(nd->B);
    nd->NA = nd->A->N; nd->NB = nd->B->N; nd->N = nd->NA + nd->NB + nd->NC;
}
// ---- main solution (appended after core.h) ----
static int n;
static int queriesMade = 0;
typedef bitset<1024> BS;

static void finish() { exit(0); }

// ask sheet (bitset of T positions), return matches
static int ask(const BS& s) {
    string out(n, 'F');
    for (int i = 0; i < n; i++) if (s[i]) out[i] = 'T';
    printf("%s\n", out.c_str());
    fflush(stdout);
    queriesMade++;
    int r;
    if (scanf("%d", &r) != 1) finish();
    if (r < 0) finish();
    if (r == n) finish();
    return r;
}

static int hamm(const BS& a, const BS& b) { return (int)(a ^ b).count(); }

static double Eest(size_t K) {
    if (K <= 1) return 1.0;
    if (K == 2) return 1.5;
    return 2.0 - 1.0 / K + 0.55 * log2(K / 2.0);
}

// Resolve among explicit candidate sheets (each candidate is also the query sheet).
static mt19937 egRng(12345);
static BS resolveSet(vector<BS> cand, bool canWin, BS neutral = BS()) {
    while (true) {
        if (cand.empty()) finish(); // should not happen
        if (cand.size() == 1) {
            if (canWin) { ask(cand[0]); finish(); }
            return cand[0];
        }
        size_t K = cand.size();
        vector<int> optIdx, evalIdx;
        if (K <= 400) { for (size_t i = 0; i < K; i++) optIdx.push_back(i); }
        else { for (int t = 0; t < 400; t++) optIdx.push_back(egRng() % K); }
        if (K <= 2000) { for (size_t i = 0; i < K; i++) evalIdx.push_back(i); }
        else { for (int t = 0; t < 2000; t++) evalIdx.push_back(egRng() % K); }
        double scale = (double)K / evalIdx.size();
        double best = 1e18; int bi = optIdx[0];
        vector<int> cnt(n + 1);
        for (int o : optIdx) {
            fill(cnt.begin(), cnt.end(), 0);
            BS sh = cand[o] | neutral;
            for (int idx : evalIdx) cnt[n - hamm(sh, cand[idx])]++;
            double cost = 1.0;
            for (int r = 0; r <= n; r++) if (cnt[r]) {
                if (r == n && canWin) continue;
                double sz = cnt[r] * scale;
                cost += (sz / K) * Eest((size_t)llround(max(1.0, sz)));
            }
            if (cost < best - 1e-12) { best = cost; bi = o; }
        }
        BS q = cand[bi] | neutral;
        int r = ask(q);
        vector<BS> nc;
        for (auto& c : cand) if (n - hamm(q, c) == r) nc.push_back(c);
        cand.swap(nc);
    }
}

struct Block { vector<int> pos; vector<vector<char>> alts; };
static const size_t FULL_CAP = 200000;

struct EGState {
    BS rep;
    vector<Block> blocks;
    vector<BS> sheets; vector<int> target; // target = r - base matches outside blocks
    vector<vector<vector<int>>> contrib;   // [b][a][j]
};
static BS buildCand(const EGState& st, const vector<int>& ch) {
    BS x = st.rep;
    for (size_t b = 0; b < st.blocks.size(); b++) {
        const auto& blk = st.blocks[b];
        const auto& alt = blk.alts[ch[b]];
        for (size_t t = 0; t < blk.pos.size(); t++) x[blk.pos[t]] = alt[t];
    }
    return x;
}
static void addConstraint(EGState& st, const BS& sh, int r) {
    // base matches outside blocks
    BS inBlk;
    for (auto& b : st.blocks) for (int p : b.pos) inBlk[p] = 1;
    int base = 0;
    for (int i = 0; i < n; i++) if (!inBlk[i] && sh[i] == st.rep[i]) base++;
    st.sheets.push_back(sh); st.target.push_back(r - base);
    for (size_t b = 0; b < st.blocks.size(); b++) {
        auto& blk = st.blocks[b];
        for (size_t a = 0; a < blk.alts.size(); a++) {
            int c = 0;
            for (size_t t = 0; t < blk.pos.size(); t++) if (sh[blk.pos[t]] == (bool)blk.alts[a][t]) c++;
            st.contrib[b][a].push_back(c);
        }
    }
}
// DFS enumerate consistent choices; returns false if > cap
static bool enumConsistent(const EGState& st, size_t cap, vector<vector<int>>& out) {
    int B = st.blocks.size(), J = st.target.size();
    // suffix min/max
    vector<vector<int>> smin(B + 1, vector<int>(J, 0)), smax(B + 1, vector<int>(J, 0));
    for (int b = B - 1; b >= 0; b--) for (int j = 0; j < J; j++) {
        int mn = INT_MAX, mx = INT_MIN;
        for (auto& c : st.contrib[b]) { mn = min(mn, c[j]); mx = max(mx, c[j]); }
        smin[b][j] = smin[b + 1][j] + mn; smax[b][j] = smax[b + 1][j] + mx;
    }
    vector<int> ch(B), rem(st.target);
    bool over = false;
    function<void(int)> dfs = [&](int b) {
        if (over) return;
        if (b == B) {
            for (int j = 0; j < J; j++) if (rem[j] != 0) return;
            out.push_back(ch);
            if (out.size() > cap) over = true;
            return;
        }
        for (int j = 0; j < J; j++) if (rem[j] < smin[b][j] || rem[j] > smax[b][j]) return;
        for (size_t a = 0; a < st.blocks[b].alts.size() && !over; a++) {
            ch[b] = a;
            for (int j = 0; j < J; j++) rem[j] -= st.contrib[b][a][j];
            dfs(b + 1);
            for (int j = 0; j < J; j++) rem[j] += st.contrib[b][a][j];
        }
    };
    dfs(0);
    return !over;
}

static void endgameBlocks(BS rep, vector<Block> blocks) {
    if (blocks.empty()) { ask(rep); finish(); }
    EGState st;
    st.rep = rep;
    sort(blocks.begin(), blocks.end(), [](const Block& a, const Block& b) { return a.alts.size() > b.alts.size(); });
    st.blocks = blocks;
    st.contrib.resize(blocks.size());
    for (size_t b = 0; b < blocks.size(); b++) st.contrib[b].resize(blocks[b].alts.size());
    mt19937_64 rng(4242);
    int B = st.blocks.size();
    while (true) {
        double P = 1;
        for (auto& b : st.blocks) P *= b.alts.size();
        // estimate consistent count via sampling acceptance
        bool tryEnum = P <= FULL_CAP;
        vector<vector<int>> samples;
        if (!tryEnum) {
            int draws = 0, acc = 0;
            vector<int> ch(B);
            while (draws < 400000 && samples.size() < 2000) {
                draws++;
                for (int b = 0; b < B; b++) ch[b] = rng() % st.blocks[b].alts.size();
                bool ok = true;
                for (size_t j = 0; j < st.target.size() && ok; j++) {
                    int s2 = 0;
                    for (int b = 0; b < B; b++) s2 += st.contrib[b][ch[b]][j];
                    if (s2 != st.target[j]) ok = false;
                }
                if (ok) { acc++; samples.push_back(ch); }
            }
            double est = P * (double)acc / draws;
            if (est <= FULL_CAP * 0.5 || samples.size() < 200) tryEnum = true;
        }
        if (tryEnum) {
            vector<vector<int>> all;
            if (enumConsistent(st, FULL_CAP, all)) {
                vector<BS> cand;
                cand.reserve(all.size());
                for (auto& ch : all) cand.push_back(buildCand(st, ch));
                resolveSet(cand, true);
                return; // not reached
            }
            if (samples.empty()) {
                // fallback: take first FULL_CAP solutions as a sample
                samples.assign(all.begin(), all.begin() + min(all.size(), (size_t)2000));
            }
        }
        // choose query among sampled candidates, evaluate on samples
        size_t S = samples.size();
        vector<BS> sc(S);
        for (size_t i = 0; i < S; i++) sc[i] = buildCand(st, samples[i]);
        size_t nopt = min<size_t>(S, 300);
        double best = 1e18; size_t bi = 0;
        vector<int> cnt(n + 1);
        for (size_t o = 0; o < nopt; o++) {
            fill(cnt.begin(), cnt.end(), 0);
            for (size_t e = 0; e < S; e++) cnt[n - hamm(sc[o], sc[e])]++;
            double cost = 0;
            for (int r = 0; r < n; r++) if (cnt[r]) { double p = (double)cnt[r] / S; cost -= p * log2(p); }
            cost = -cost;
            if (cost < best) { best = cost; bi = o; }
        }
        BS q = sc[bi];
        int r = ask(q);
        addConstraint(st, q, r);
    }
}

// ---------------- sparse strategy -----------------
// target bit value tb; count of positions with key==tb is k.
struct Grp { vector<int> idx; int t; };
static double binomLog2(int s, int t) { return (lgamma(s + 1.0) - lgamma(t + 1.0) - lgamma(s - t + 1.0)) / log(2.0); }

static int simulateSparse(int nn, int k, mt19937& rng) {
    // returns number of split queries until remaining candidate product <= 64
    vector<pair<int,int>> st; // (size, count)  but need actual positions; simulate counts via hypergeometric
    // simulate using explicit random key
    vector<int> key(nn, 0);
    for (int i = 0; i < k; i++) key[i] = 1;
    shuffle(key.begin(), key.end(), rng);
    vector<Grp> gs; Grp g0; for (int i = 0; i < nn; i++) g0.idx.push_back(i); g0.t = k; gs.push_back(g0);
    int q = 0;
    while (true) {
        double lg = 0; int bi = -1; size_t bs = 0;
        for (size_t i = 0; i < gs.size(); i++) {
            if (gs[i].t == 0 || gs[i].t == (int)gs[i].idx.size()) continue;
            lg += binomLog2(gs[i].idx.size(), gs[i].t);
            if (gs[i].idx.size() > bs) { bs = gs[i].idx.size(); bi = i; }
        }
        if (bi < 0 || lg <= 6.0) break;
        Grp g = gs[bi];
        size_t h = g.idx.size() / 2;
        Grp L, R;
        L.idx.assign(g.idx.begin(), g.idx.begin() + h);
        R.idx.assign(g.idx.begin() + h, g.idx.end());
        L.t = 0; for (int x : L.idx) L.t += key[x];
        R.t = g.t - L.t;
        q++;
        gs[bi] = L; gs.push_back(R);
    }
    return q;
}

static vector<BS> enumerateGroups(const vector<Grp>& gs, int tb, const vector<int>& base) {
    // base: known values for positions in resolved groups; enumerate combos over mixed groups
    vector<BS> res;
    BS b0;
    for (int i = 0; i < n; i++) if (base[i] == 1) b0[i] = 1;
    res.push_back(b0);
    for (auto& g : gs) {
        int s = g.idx.size();
        if (g.t == 0 || g.t == s) continue;
        vector<BS> nr;
        // all subsets of size t of g.idx
        vector<int> sel(s, 0);
        fill(sel.begin(), sel.begin() + g.t, 1);
        sort(sel.begin(), sel.end());
        do {
            for (auto& r : res) {
                BS x = r;
                for (int j = 0; j < s; j++) {
                    int v = sel[j] ? tb : 1 - tb;
                    x[g.idx[j]] = v;
                }
                nr.push_back(x);
            }
        } while (next_permutation(sel.begin(), sel.end()));
        res.swap(nr);
    }
    return res;
}

static void runSparse(int k1) {
    // k1 = number of T in key
    int tb = (k1 <= n - k1) ? 1 : 0;
    int k = tb ? k1 : n - k1;
    vector<Grp> gs; Grp g0; for (int i = 0; i < n; i++) g0.idx.push_back(i); g0.t = k; gs.push_back(g0);
    while (true) {
        double lg = 0; int bi = -1; size_t bs = 0;
        for (size_t i = 0; i < gs.size(); i++) {
            if (gs[i].t == 0 || gs[i].t == (int)gs[i].idx.size()) continue;
            lg += binomLog2(gs[i].idx.size(), gs[i].t);
            if (gs[i].idx.size() > bs) { bs = gs[i].idx.size(); bi = i; }
        }
        if (bi < 0 || lg <= 6.0) break;
        Grp g = gs[bi];
        size_t h = g.idx.size() / 2;
        Grp L, R;
        L.idx.assign(g.idx.begin(), g.idx.begin() + h);
        R.idx.assign(g.idx.begin() + h, g.idx.end());
        // query L: sheet T on L
        BS s; for (int x : L.idx) s[x] = 1;
        int r = ask(s);
        // ones(T) in L: x ; matches = x + (n - |L|) - (k1 - x)
        int x = (r - n + (int)L.idx.size() + k1) / 2;
        L.t = tb ? x : (int)L.idx.size() - x;
        R.t = g.t - L.t;
        gs[bi] = L; gs.push_back(R);
    }
    vector<int> base(n, 0);
    for (auto& g : gs) {
        if (g.t == 0) for (int x : g.idx) base[x] = 1 - tb;
        else if (g.t == (int)g.idx.size()) for (int x : g.idx) base[x] = tb;
    }
    resolveSet(enumerateGroups(gs, tb, base), true);
}


// ---------------- small n (<= 44): explicit candidates -----------------
typedef unsigned long long u64;
static int SMALL_MAX = 44;
static double smallEest(size_t K) {
    if (K <= 1) return 1.0;
    if (K == 2) return 1.5;
    return 2.0 - 1.0 / K + 0.5 * log2(K / 2.0);
}
static u64 fullMask() { return n == 64 ? ~0ULL : ((1ULL << n) - 1); }
static int askMask(u64 s) {
    BS b;
    for (int i = 0; i < n; i++) if (s >> i & 1) b[i] = 1;
    return ask(b);
}
static void adaptiveSmall(vector<u64> cand, mt19937_64& rng) {
    u64 FM = fullMask();
    while (true) {
        if (cand.empty()) finish();
        if (cand.size() == 1) { askMask(cand[0]); finish(); }
        size_t K = cand.size();
        vector<u64> opts;
        if (K <= 300) opts = cand;
        else for (int t = 0; t < 300; t++) opts.push_back(cand[rng() % K]);
        int nRand = K > 50 ? 100 : 0;
        for (int t = 0; t < nRand; t++) opts.push_back(rng() & FM);
        vector<u64> ev;
        if (K <= 3000) ev = cand;
        else for (int t = 0; t < 3000; t++) ev.push_back(cand[rng() % K]);
        double scale = (double)K / ev.size();
        double best = 1e18; u64 bq = opts[0];
        vector<int> cnt(n + 1);
        for (u64 o : opts) {
            fill(cnt.begin(), cnt.end(), 0);
            for (u64 c : ev) cnt[n - __builtin_popcountll((o ^ c) & FM)]++;
            double cost = 1.0;
            for (int r = 0; r <= n; r++) if (cnt[r]) {
                if (r == n) continue; // o itself is the key -> solved
                double sz = cnt[r] * scale;
                cost += (sz / K) * smallEest((size_t)llround(max(1.0, sz)));
            }
            if (cost < best - 1e-12) { best = cost; bq = o; }
        }
        int r = askMask(bq);
        vector<u64> nc;
        for (u64 c : cand) if (n - __builtin_popcountll((bq ^ c) & FM) == r) nc.push_back(c);
        cand.swap(nc);
    }
}
static double confEst(int m, int N) {
    double s = 0;
    for (int w = 2; w <= N; w += 2) {
        double lp = lgamma(w + 1.0) - 2 * lgamma(w / 2 + 1.0) - w * log(2.0);
        double lc = lgamma(N + 1.0) - lgamma(w + 1.0) - lgamma(N - w + 1.0);
        s += exp(lc + m * lp);
    }
    return s;
}
// enumerate all x in {0,1}^n with  popcount(x)==k1 and for each query j: matches(q_j, x) == ans_j
static vector<u64> mitmEnumerate(const vector<u64>& qs, const vector<int>& ans, size_t cap, bool& over) {
    over = false;
    u64 FM = fullMask();
    int h = n / 2, hr = n - h;
    int m = qs.size();
    // matches(q,x) = n - popcount(q^x). Use values: for row j, contribution of a half = count of positions in half where q==x.
    // key over up to 9 rows, 7 bits each (values 0..32)
    int mk = min(m, 9);
    vector<pair<u64, uint32_t>> L, R;
    L.reserve(1u << h); R.reserve(1u << hr);
    u64 lm = (1ULL << h) - 1;
    for (u64 x = 0; x < (1ULL << h); x++) {
        u64 key = 0;
        for (int j = 0; j < mk; j++) {
            int c = h - __builtin_popcountll((qs[j] ^ x) & lm);
            key = key * 128 + c;
        }
        L.push_back({key, (uint32_t)x});
    }
    for (u64 y = 0; y < (1ULL << hr); y++) {
        u64 xr = y << h;
        u64 key = 0;
        bool ok = true;
        for (int j = 0; j < mk; j++) {
            int c = hr - __builtin_popcountll(((qs[j] ^ xr) & FM) >> h);
            int need = ans[j] - c;
            if (need < 0 || need > h) { ok = false; break; }
            key = key * 128 + need;
        }
        if (ok) R.push_back({key, (uint32_t)y});
    }
    sort(L.begin(), L.end());
    sort(R.begin(), R.end());
    vector<u64> res;
    size_t i = 0, j = 0;
    while (i < L.size() && j < R.size()) {
        if (L[i].first < R[j].first) i++;
        else if (L[i].first > R[j].first) j++;
        else {
            size_t i2 = i, j2 = j;
            while (i2 < L.size() && L[i2].first == L[i].first) i2++;
            while (j2 < R.size() && R[j2].first == R[j].first) j2++;
            for (size_t a = i; a < i2; a++) for (size_t b = j; b < j2; b++) {
                u64 x = (u64)L[a].second | ((u64)R[b].second << h);
                bool ok = true;
                for (int t = mk; t < m && ok; t++) if (n - __builtin_popcountll((qs[t] ^ x) & FM) != ans[t]) ok = false;
                if (ok) {
                    res.push_back(x);
                    if (res.size() > cap) { over = true; return res; }
                }
            }
            i = i2; j = j2;
        }
    }
    return res;
}
static void solveSmall(int k1) {
    mt19937_64 rng(0xC0FFEE + n);
    u64 FM = fullMask();
    vector<u64> qs{FM};
    vector<int> ans{k1};
    if (n <= 22) {
        vector<u64> cand;
        for (u64 x = 0; x < (1ULL << n); x++) if (__builtin_popcountll(x) == k1) cand.push_back(x);
        adaptiveSmall(cand, rng);
        return;
    }
    // random rows until expected candidates small enough
    const double TARGET = 20000;
    while (true) {
        int m = qs.size();
        if (confEst(m, n) <= TARGET) {
            bool over;
            vector<u64> cand = mitmEnumerate(qs, ans, 300000, over);
            if (!over) { adaptiveSmall(cand, rng); return; }
        }
        u64 q = rng() & FM;
        qs.push_back(q);
        ans.push_back(askMask(q));
    }
}
// ---------------- Lindstrom strategy -----------------
static Node* root;
static int M, N;

static BS rowSheet(int r) {
    BS s;
    for (int i = 0; i < n; i++) if (root->rows[r][i] > 0) s[i] = 1;
    return s;
}
static int rowNeg(int r) {
    int c = 0;
    for (int i = 0; i < n; i++) if (root->rows[r][i] < 0) c++;
    return c;
}

int main() {
    if (scanf("%d", &n) != 1) return 0;
    initG(399);
#ifdef EXPER
    if (getenv("EGDIV")) ENDGAME_DIV = atof(getenv("EGDIV"));
    if (getenv("EMAXG")) EMAXG = atoi(getenv("EMAXG"));
    if (getenv("ACAP")) ACAP = atof(getenv("ACAP"));
#endif
    M = planBest(n);
    root = buildPlan(M);
    recomputeN(root);
    {
        int slack = root->N - n;
        while (slack > 0) {
            Node* b = nullptr; double bd = -1;
            for (Node* x : baseNodes) if (x->E > 0) {
                double d = LTAB[x->M - 8][x->E] - LTAB[x->M - 8][x->E - 1];
                if (d > bd) { bd = d; b = x; }
            }
            if (!b) break;
            b->E--; b->X.pop_back(); slack--;
        }
        recomputeN(root);
    }
    ensureRows(root);
    N = root->N;
#ifdef EXPER
    { fprintf(stderr,"M=%d N=%d bases:",M,N); for(Node*b:baseNodes) fprintf(stderr," %d/%d",b->M,b->E); fprintf(stderr,"\n"); }
#endif

    vector<ll> vals(M, 0);
    // row 0 is all T
    int k1 = ask(rowSheet(0));
    vals[0] = k1 - rowNeg(0);
    if (k1 == 0) { BS z; ask(z); finish(); }
    if (n <= SMALL_MAX) { solveSmall(k1); return 0; }

    double lindCost = (M - 1) + 1.3;
    {
        int k = min(k1, n - k1);
        if (k <= 150) {
            mt19937 rng(777);
            double tot = 0; int T = 30;
            for (int t = 0; t < T; t++) tot += simulateSparse(n, k, rng);
            double sparseCost = tot / T + 2.0;
            if (sparseCost < lindCost) { runSparse(k1); return 0; }
        }
    }
    for (int r = 1; r < M; r++) {
        int ans = ask(rowSheet(r));
        vals[r] = ans - rowNeg(r);
    }
    vector<i8> outv(N, 0);
    vector<Amb> amb;
    if (!decodeRep(root, vals.data(), outv.data(), 0, amb)) {
        // should not happen; fall back to guessing nothing sensible
        BS z; ask(z); finish();
    }
    BS rep;
    for (int i = 0; i < n; i++) if (outv[i]) rep[i] = 1;
    vector<Block> blocks;
    for (auto& a : amb) {
        int len = a.alts[0].size();
        vector<vector<i8>> alts;
        for (auto& v : a.alts) {
            bool ok = true;
            for (int k = 0; k < len; k++) if (a.offset + k >= n && v[k]) ok = false;
            if (ok) alts.push_back(v);
        }
        Block b;
        for (int k = 0; k < len && a.offset + k < n; k++) {
            bool diff = false;
            for (auto& v : alts) if (v[k] != alts[0][k]) diff = true;
            if (diff) b.pos.push_back(a.offset + k);
        }
        set<vector<char>> uniq;
        for (auto& v : alts) {
            vector<char> x;
            for (int p : b.pos) x.push_back(v[p - a.offset]);
            uniq.insert(x);
        }
        for (int k = 0; k < len && a.offset + k < n; k++) rep[a.offset + k] = alts[0][k];
        if (uniq.size() > 1) { b.alts.assign(uniq.begin(), uniq.end()); blocks.push_back(b); }
    }
#ifdef EXPER
    { double P=1; for(auto&b:blocks) P*=b.alts.size(); fprintf(stderr,"blocks=%zu P=%g work=%lld\n",blocks.size(),P,decodeWork); }
#endif
    endgameBlocks(rep, blocks);
    return 0;
}
