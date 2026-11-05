// 星网配能
//
// Maximise  F = sum_{i<j} e_i e_j w_ij   subject to  e_i >= 0, sum e_i = 1,
// i.e. the standard quadratic program   max (1/2) x^T W x   on the simplex.
//
// Theory used
// -----------
// At a global maximiser x* with support S (x*_i > 0 for i in S) the KKT
// conditions give W_SS x* = lambda * 1.  If W_SS is invertible then
// y = W_SS^{-1} 1 = x*/lambda > 0 and the optimal value is 1/(2 * 1^T y).
// If W_SS is singular, every null vector n satisfies 1^T n = 0 (otherwise
// sliding x* along n changes the value, contradicting optimality), so the
// solution can be slid to a *smaller* support while keeping the same value.
// Hence the answer equals
//        max { 1/(2 1^T y) : S,  W_SS invertible, y = W_SS^{-1} 1 > 0 }.
// Every such candidate corresponds to an explicitly feasible point, so it is
// never an overestimate.  Moreover the optimum can always be attained on a
// clique of the graph of positive weights (if w_ij = 0 the objective is exactly
// flat along e_i - e_j), which is used for pruning.
//
// Algorithm
// ---------
// DFS over all subsets (in increasing index order), maintaining the inverse of
// W_SS together with u = W_SS^{-1} 1 incrementally through the Schur complement
// of the bordered matrix (O(k^2) per subset, O(2^n n^2) overall).  A tiny eps*I
// is added to make every intermediate matrix invertible; this perturbs the
// answer by at most eps/2 (generically only O(eps^2), since the optimum is a
// stationary point).  Since W'u = 1, one has u^T W u = s - eps*||u||^2 with
// s = 1^T u, so 1/(2s) is an upper bound for the value of the feasible point
// x = u/s; it serves as a trigger, and every candidate reaching it is evaluated
// exactly in O(k^2) at that genuine feasible point (never an overestimate).
// Finally the most promising supports are re-solved from scratch without
// perturbation in extended precision, which removes the perturbation error.

#include <cstdio>
#include <cmath>
#include <cstring>
#include <algorithm>
using namespace std;

static const int MAXN = 20;
static const int TOPK = 64;

int n;
int A[MAXN][MAXN];       // original weights
int adjm[MAXN];          // bitmask of positive-weight neighbours

static const double EPS = 1e-7;

double WD[MAXN][MAXN];   // W + EPS*I

// ---- DFS state (indexed by |S|) ----
double invst[MAXN + 1][MAXN][MAXN];  // inverse of WD[S,S]
double uarr[MAXN + 1][MAXN];         // inverse * ones
int slist[MAXN];
int curmask;

double bestVal = 0.0;                // best verified achievable value

// top candidate supports discovered so far
int topCnt = 0;
double topVal[TOPK];
int topMask[TOPK];
int worstIdx = 0;

void recordCandidate(double val, int mask) {
    if (topCnt < TOPK) {
        topVal[topCnt] = val;
        topMask[topCnt] = mask;
        topCnt++;
        if (topCnt == TOPK) {
            worstIdx = 0;
            for (int i = 1; i < TOPK; i++) if (topVal[i] < topVal[worstIdx]) worstIdx = i;
        }
        return;
    }
    if (val <= topVal[worstIdx]) return;
    topVal[worstIdx] = val;
    topMask[worstIdx] = mask;
    worstIdx = 0;
    for (int i = 1; i < TOPK; i++) if (topVal[i] < topVal[worstIdx]) worstIdx = i;
}

// Exact (unperturbed) value of the stationary point on support `mask`, using
// extended precision and Gaussian elimination with partial pivoting.
bool exactValue(int mask, long double& out) {
    int idx[MAXN], k = 0;
    for (int i = 0; i < n; i++) if (mask >> i & 1) idx[k++] = i;
    if (k == 0) return false;
    long double M[MAXN][MAXN + 1];
    for (int i = 0; i < k; i++) {
        for (int j = 0; j < k; j++) M[i][j] = (long double)A[idx[i]][idx[j]];
        M[i][k] = 1.0L;
    }
    for (int col = 0; col < k; col++) {
        int piv = col;
        long double bv = fabsl(M[col][col]);
        for (int r = col + 1; r < k; r++)
            if (fabsl(M[r][col]) > bv) { bv = fabsl(M[r][col]); piv = r; }
        if (!(bv > 1e-15L)) return false;     // singular
        if (piv != col) for (int j = col; j <= k; j++) swap(M[piv][j], M[col][j]);
        long double d = M[col][col];
        for (int r = col + 1; r < k; r++) {
            long double f = M[r][col] / d;
            if (f == 0) continue;
            for (int j = col; j <= k; j++) M[r][j] -= f * M[col][j];
        }
    }
    long double y[MAXN], s = 0;
    for (int i = k - 1; i >= 0; i--) {
        long double v = M[i][k];
        for (int j = i + 1; j < k; j++) v -= M[i][j] * y[j];
        y[i] = v / M[i][i];
        if (!(y[i] > 0)) return false;
        s += y[i];
    }
    if (!(s > 0)) return false;
    out = 1.0L / (2.0L * s);
    return out >= 0 && out < 1e9L;
}

void dfs(int k, int last, int common) {
    double* u = uarr[k];
    if (k > 0) {
        bool pos = true;
        double s = 0.0;
        for (int i = 0; i < k; i++) { if (!(u[i] > 0.0)) { pos = false; break; } s += u[i]; }
        if (pos && s > 0.0) {
            // u solves W'u = 1, hence u^T W u = s - EPS*|u|^2 and the value of the
            // feasible point x = u/s is (s - EPS*|u|^2) / (2 s^2) <= 1/(2s).
            // 1/(2s) is therefore a safe upper bound used as the trigger.
            if (0.5 / s > bestVal) {
                double iv = 1.0 / s, exact = 0.0;
                for (int i = 0; i < k; i++) {
                    double xi = u[i] * iv;
                    int si = slist[i];
                    const int* Ai = A[si];
                    for (int j = i + 1; j < k; j++)
                        exact += xi * (u[j] * iv) * (double)Ai[slist[j]];
                }
                if (exact >= bestVal) {   // >= also keeps tied (equally good) supports
                    bestVal = max(bestVal, exact);
                    recordCandidate(exact, curmask);
                }
            }
        }
    }
    if (k == n) return;

    for (int j = last + 1; j < n; j++) {
        if (!((common >> j) & 1)) continue;

        double b[MAXN], c[MAXN];
        for (int i = 0; i < k; i++) b[i] = WD[slist[i]][j];
        for (int i = 0; i < k; i++) {
            const double* row = invst[k][i];
            double s = 0.0;
            for (int t = 0; t < k; t++) s += row[t] * b[t];
            c[i] = s;
        }
        double bc = 0.0;
        for (int i = 0; i < k; i++) bc += b[i] * c[i];

        double sigma = EPS - bc;               // Schur complement
        if (!(fabs(sigma) > 1e-300)) continue; // only an exact zero is hopeless
        double invs = 1.0 / sigma;

        for (int i = 0; i < k; i++) {
            double ci = c[i] * invs;
            const double* prow = invst[k][i];
            double* row = invst[k + 1][i];
            for (int t = 0; t < k; t++) row[t] = prow[t] + ci * c[t];
            row[k] = -ci;
            invst[k + 1][k][i] = -ci;
        }
        invst[k + 1][k][k] = invs;

        double c1 = 0.0;
        for (int i = 0; i < k; i++) c1 += c[i];
        double f = (c1 - 1.0) * invs;
        double* nu = uarr[k + 1];
        const double* pu = uarr[k];
        for (int i = 0; i < k; i++) nu[i] = pu[i] + c[i] * f;
        nu[k] = -f;

        slist[k] = j;
        curmask |= (1 << j);
        dfs(k + 1, j, k == 0 ? adjm[j] : (common & adjm[j]));
        curmask &= ~(1 << j);
    }
}

int main() {
    if (scanf("%d", &n) != 1) return 0;
    bool allZero = true;
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++) {
            if (scanf("%d", &A[i][j]) != 1) A[i][j] = 0;
            if (i != j && A[i][j] != 0) allZero = false;
        }
    if (n <= 1 || allZero) {
        printf("0.000000000000\n");
        return 0;
    }
    for (int i = 0; i < n; i++) {
        adjm[i] = 0;
        for (int j = 0; j < n; j++) {
            WD[i][j] = (double)A[i][j];
            if (i != j && A[i][j] > 0) adjm[i] |= (1 << j);
        }
        WD[i][i] = EPS;
    }

    bestVal = 0.0;
    curmask = 0;
    dfs(0, -1, (1 << n) - 1);

    // remove the perturbation error: re-solve the most promising supports exactly
    for (int i = 0; i < topCnt; i++) {
        long double v;
        if (exactValue(topMask[i], v) && (double)v > bestVal) bestVal = (double)v;
    }

    if (!(bestVal > 0.0)) bestVal = 0.0;
    printf("%.12f\n", bestVal);
    return 0;
}
