// Aliens - interactive: find the center cell of a 5x5 checkerboard crop formation
//
// The meadow is an N x N grid.  There is a 5x5 chessboard (each square M x M
// meadow cells, M odd, M >= 3) placed inside the meadow.  Its lower left
// square is red (13 red, 12 white squares).  Cell (x,y) is flattened ("red")
// iff  (i+j) is even, where i = floor((x-a)/M), j = floor((y-b)/M)  and
// (a,b) is the lower left corner of the board.  Everything outside the board
// is not flattened.
//
// The middle chessboard square is red and its centre cell equals the centre of
// the whole 5M x 5M formation, i.e.  (a + (5M-1)/2, b + (5M-1)/2).
//
// Facts used below:
//  * Along the row Y0 the flattened cells form blocks of exactly M columns;
//    consecutive blocks alternate red / white (period 2M).
//  * The leftmost red cell of row Y0 is a + (j0 mod 2)*M and the rightmost one
//    is a+5M-1 - (j0 mod 2)*M, hence the board's centre column is the midpoint
//    of those two extremes (the same holds vertically for column X0).  This
//    makes it enough to know the board's block grid, never the colour parity.
//
// Query plan:
//   A) gallop left/right along row Y0 to bracket both ends of the anchor's
//      block, then binary search them  ->  block length M.
//   B) with M known, binary search the start of the anchor's y-block.
//   C) at most 2+2 more probes give how many blocks lie to each side, hence
//      the two extreme red cells, hence the answer.
#include <bits/stdc++.h>
using namespace std;
typedef long long ll;

static ll N, X0, Y0;

static bool ask(ll x, ll y) {
    printf("? %lld %lld\n", x, y);
    fflush(stdout);
    char buf[16];
    if (scanf("%15s", buf) != 1) exit(0);
    return buf[0] == 't';
}

// flattened status; cells outside the meadow are never flattened
static bool red(ll x, ll y) {
    if (x < 1 || x > N || y < 1 || y > N) return false;
    return ask(x, y);
}

int main() {
    if (scanf("%lld %lld %lld", &N, &X0, &Y0) != 3) return 0;

    // ------------------------------------------------------------------
    // Phase A: the red x-block containing (X0,Y0).
    // aL/aR: offsets from X0 known to be red, dL/dR: offsets known to be white.
    // A jump to the left is safe (still inside the anchor block or the white
    // block right after it) iff  jump <= p+M ; since p+q+1 = M and p >= aL,
    // q >= aR we have p+M >= 2aL+aR+1, and symmetrically for the right.
    // ------------------------------------------------------------------
    ll aL = 0, aR = 0;
    ll dL = -1, dR = -1;
    bool okL = false, okR = false;
    while (!okL || !okR) {
        if (!okL) {
            ll j = 2 * aL + aR + 1;
            if (red(X0 - j, Y0)) aL = j; else { dL = j; okL = true; }
        }
        if (!okR) {
            ll j = aL + 2 * aR + 1;
            if (red(X0 + j, Y0)) aR = j; else { dR = j; okR = true; }
        }
    }
    while (dL - aL > 1) {                   // exact left end of the block
        ll mid = aL + (dL - aL) / 2;
        if (red(X0 - mid, Y0)) aL = mid; else dL = mid;
    }
    while (dR - aR > 1) {                   // exact right end of the block
        ll mid = aR + (dR - aR) / 2;
        if (red(X0 + mid, Y0)) aR = mid; else dR = mid;
    }
    ll p = dL - 1, q = dR - 1;
    ll M = p + q + 1;
    ll Lx = X0 - p, Rx = X0 + q;

    // ------------------------------------------------------------------
    // Phase B: start of the y-block containing Y0.  red(X0,.) is monotone on
    // either window of length <= M next to Y0, use the shorter one.
    // ------------------------------------------------------------------
    ll By, Ty;
    ll botSize = min(Y0, M);
    ll topSize = min(N - Y0 + 1, M);
    if (botSize <= topSize) {
        ll lo = Y0 - botSize + 1, hi = Y0;  // first red row
        while (lo < hi) {
            ll mid = (lo + hi) >> 1;
            if (red(X0, mid)) hi = mid; else lo = mid + 1;
        }
        By = lo;
        Ty = By + M - 1;
    } else {
        ll lo = Y0, hi = Y0 + topSize - 1;  // last red row
        while (lo < hi) {
            ll mid = (lo + hi + 1) >> 1;
            if (red(X0, mid)) lo = mid; else hi = mid - 1;
        }
        Ty = lo;
        By = Ty - (M - 1);
    }

    // ------------------------------------------------------------------
    // Phase C: how many blocks lie left/right (below/above); the extremes of
    // the flattened run and their midpoint give the centre.
    // ------------------------------------------------------------------
    ll xl = Lx, xr = Rx;
    if (red(Lx - 2 * M, Y0)) {
        xl = Lx - 2 * M;
        if (red(Lx - 4 * M, Y0)) xl = Lx - 4 * M;
    }
    if (red(Rx + 2 * M, Y0)) {
        xr = Rx + 2 * M;
        if (red(Rx + 4 * M, Y0)) xr = Rx + 4 * M;
    }
    ll yb = By, yt = Ty;
    if (red(X0, By - 2 * M)) {
        yb = By - 2 * M;
        if (red(X0, By - 4 * M)) yb = By - 4 * M;
    }
    if (red(X0, Ty + 2 * M)) {
        yt = Ty + 2 * M;
        if (red(X0, Ty + 4 * M)) yt = Ty + 4 * M;
    }

    ll cx = (xl + xr) / 2;
    ll cy = (yb + yt) / 2;
    printf("! %lld %lld\n", cx, cy);
    fflush(stdout);
    return 0;
}
