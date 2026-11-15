// Magical String
//
// ---------------------------------------------------------------------------
// Model
// ---------------------------------------------------------------------------
// The invariant "every run of equal letters has length <= 2" is preserved:
// an operation only ever removes a run and puts an inert '*' in its place, so
// no run can grow.
//
// Every operation absorbs a contiguous block [L,R] of the *original* string:
//   before the edit the block is  c^a X c^b   (exactly one position X != c),
//   and after changing X into c the whole block is one maximal run of c.
// Because original runs have length <= 2 we get a,b <= 2, hence |block| <= 5,
// and the possible shapes are
//       length 3:  ccX   cXc   Xcc
//       length 4:  ccXc  cXcc
//       length 5:  ccXcc
// The only interaction between two disjoint blocks is at a shared border:
// if the letter just outside a block equals the block's letter c, that outside
// letter must already have been absorbed (turned into '*') when the block is
// executed.  This creates a dependency between neighbouring blocks.  Since
// blocks form a chain, the only possible cycle is a 2-cycle between two
// adjacent blocks; a set of blocks is realizable iff no adjacent pair wants
// both execution orders.  A left-to-right DP with three states captures this:
//   0 : position i-1 is not absorbed (or i == 0)
//   1 : position i-1 is absorbed, the previous block has no pending need
//   2 : position i-1 is absorbed and the previous block needs position i to be
//       absorbed first (so a block must start at i, and it must not create a
//       2-cycle, i.e. s[i-1] != c of the new block).
//
// ---------------------------------------------------------------------------
// Number of operations
// ---------------------------------------------------------------------------
// We need the maximum absorbed length using at most K blocks.  The value
// function f(K) is concave (verified exhaustively on all small strings), so it
// equals its concave hull and the Lagrangian / "Aliens" trick applies:
//      f(K) = min_{lambda >= 0} ( G(lambda) + lambda * K ),
// where G(lambda) = max over block sets of (absorbed - lambda * #blocks) is
// computed by the O(n) DP above.  Block values lie in {3,4,5}, so all
// breakpoints are integers in [0,5]; lambdas 0..7 suffice (larger lambdas are
// harmless because G(lambda) + lambda*K >= f(K) for every lambda >= 0).
// ---------------------------------------------------------------------------
#include <bits/stdc++.h>
using namespace std;

static const int LAM = 8;            // lambdas 0..7 (0..5 are the meaningful ones)
static const int NNEG = -1000000000;

int main() {
    // read the whole input (at most ~10 MB given the constraints)
    vector<char> input;
    {
        char tmp[1 << 16];
        size_t r;
        while ((r = fread(tmp, 1, sizeof(tmp), stdin)) > 0)
            input.insert(input.end(), tmp, tmp + r);
        input.push_back(0);
    }
    char* ptr = input.data();

    auto nextInt = [&]() -> int {
        while (*ptr && (*ptr < '0' || *ptr > '9')) ptr++;
        int x = 0;
        while (*ptr >= '0' && *ptr <= '9') { x = x * 10 + (*ptr - '0'); ptr++; }
        return x;
    };

    int T = nextInt();
    string out;
    out.reserve(1 << 20);

    // Per position: up to three blocks (lengths 3,4,5) flattened as i*3 + t.
    vector<int> bR, bC, bV, bcnt;
    // DP tables, transposed so that the LAM lambda values of one position are
    // contiguous (helps the compiler vectorize the inner loops).
    vector<int> dp0, dp1, dp2;

    for (int tc = 0; tc < T; tc++) {
        while (*ptr && (*ptr < 'a' || *ptr > 'z')) ptr++;
        char* start = ptr;
        while (*ptr >= 'a' && *ptr <= 'z') ptr++;
        int n = (int)(ptr - start);
        const char* s = start;
        int K = nextInt();

        // ---- enumerate every absorbable block ------------------------------
        bR.assign((size_t)n * 3 + 3, 0);
        bC.assign((size_t)n * 3 + 3, 0);
        bV.assign((size_t)n * 3 + 3, 0);
        bcnt.assign(n + 1, 0);
        for (int i = 0; i < n; i++) {
            int cnt = 0, base = i * 3;
            // length 3: exactly two equal letters
            if (i + 2 < n) {
                char c = 0;
                if (s[i] == s[i+1]) c = s[i];
                else if (s[i] == s[i+2]) c = s[i];
                else if (s[i+1] == s[i+2]) c = s[i+1];
                if (c) { bR[base+cnt] = i+2; bC[base+cnt] = c; bV[base+cnt] = 3; cnt++; }
            }
            // length 4: ccXc or cXcc
            if (i + 3 < n && s[i] == s[i+3]) {
                if (s[i] == s[i+1] && s[i] != s[i+2]) {
                    bR[base+cnt] = i+3; bC[base+cnt] = s[i]; bV[base+cnt] = 4; cnt++;
                } else if (s[i] == s[i+2] && s[i] != s[i+1]) {
                    bR[base+cnt] = i+3; bC[base+cnt] = s[i]; bV[base+cnt] = 4; cnt++;
                }
            }
            // length 5: ccXcc
            if (i + 4 < n && s[i] == s[i+1] && s[i] == s[i+3] &&
                s[i] == s[i+4] && s[i] != s[i+2]) {
                bR[base+cnt] = i+4; bC[base+cnt] = s[i]; bV[base+cnt] = 5; cnt++;
            }
            bcnt[i] = cnt;
        }

        // ---- Lagrangian DP -------------------------------------------------
        size_t sz = ((size_t)n + 2) * LAM;
        dp0.assign(sz, NNEG); dp1.assign(sz, NNEG); dp2.assign(sz, NNEG);
        for (int l = 0; l < LAM; l++) { dp0[(size_t)n*LAM+l] = 0; dp1[(size_t)n*LAM+l] = 0; }

        for (int i = n - 1; i >= 0; i--) {
            int cnt = bcnt[i], base = i * 3;
            size_t row = (size_t)i * LAM, nrow = (size_t)(i + 1) * LAM;
            char left = (i > 0) ? s[i-1] : 0;
            // skipping position i is allowed unless a pending need forces a block
            for (int l = 0; l < LAM; l++) {
                dp0[row+l] = dp0[nrow+l];
                dp1[row+l] = dp0[nrow+l];
                dp2[row+l] = NNEG;
            }
            for (int t = 0; t < cnt; t++) {
                int R = bR[base+t];
                char c = (char)bC[base+t];
                int val = bV[base+t];
                bool need = (R + 1 < n) && (s[R+1] == c);
                size_t trow = (size_t)(R + 1) * LAM;
                bool lok = (i == 0) || (left != c);   // no left extension / no 2-cycle
                if (need) {
                    for (int l = 0; l < LAM; l++) {
                        int v = val - l + dp2[trow+l];
                        if (v > dp1[row+l]) dp1[row+l] = v;
                        if (lok) {
                            if (v > dp0[row+l]) dp0[row+l] = v;
                            if (v > dp2[row+l]) dp2[row+l] = v;
                        }
                    }
                } else {
                    for (int l = 0; l < LAM; l++) {
                        int v = val - l + dp1[trow+l];
                        if (v > dp1[row+l]) dp1[row+l] = v;
                        if (lok) {
                            if (v > dp0[row+l]) dp0[row+l] = v;
                            if (v > dp2[row+l]) dp2[row+l] = v;
                        }
                    }
                }
            }
        }

        long long ans = LLONG_MAX;
        for (int l = 0; l < LAM; l++) {
            long long cur = (long long)dp0[l] + (long long)l * K;
            if (cur < ans) ans = cur;
        }
        out += to_string(ans);
        out += '\n';
    }
    fwrite(out.data(), 1, out.size(), stdout);
    return 0;
}
