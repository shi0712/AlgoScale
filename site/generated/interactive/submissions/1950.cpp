// Probing the Disk - interactive solution.
//
// Facts used:
//  * The box is [0,1e5]^2, the disk has integer centre (cx,cy) and integer
//    radius r >= 100 and is fully inside the box.  Hence r <= cx,cy <= 1e5-r.
//  * A query on the horizontal line y=Y returns the full chord
//        L = 2*sqrt(r^2-(cy-Y)^2)          (if |cy-Y| < r),
//    because the disk is inside the box and the segment spans the whole box.
//  * For integer Y the quantity A = L^2/4 = r^2-(cy-Y)^2 is an *integer*,
//    so the returned L is either 0 (tangent included) or >= 2.  Rounding
//    L^2/4 recovers A exactly (error < 0.05).
//  * Two horizontal chords give
//        cy = (((y1+y2)*(y1-y2)) + (A1-A2)) / (2*(y1-y2))     (exact integer)
//    and then r^2 = A1 + (cy-y1)^2.
//  * Once cy and r are known the chord on the line y=cy is [cx-r, cx+r].
//    The segment (0,cy)-(m,cy) returns clamp(m+r-cx, 0, 2r): when it is
//    partial it gives cx outright, otherwise it halves the candidate
//    interval, i.e. a plain binary search.
//  * Localisation probes horizontal lines in "largest gap midpoint" order
//    over [100,99900].  Every prefix keeps the points evenly spread, so the
//    expected number of probes is the information-theoretic optimum
//    ~(1e5)/(4r), while even the worst case (r=100 in a corner) needs only
//    ~500 probes.  A miss at y0+d with d^2 < A0 moreover reveals on which
//    side of y0 the centre lies, so the mandatory second chord costs exactly
//    one probe.
//  * The last unknown cx is obtained with a single probe whenever possible
//    (see the phase-3 comment): the shallow line through (0,cy-100) and
//    (1e5,cy) is hit by the disk for every admissible cx and its chord is
//    always contained in that segment.  Otherwise (disk hugging the right
//    border, or r >= 1e5/4) a centre-line cut binary search finishes it.
//  Hence the typical cost is (localisation probes) + 2, which is optimal
//  within this scheme.

#include <bits/stdc++.h>
using namespace std;
using ll = long long;

static const ll LIM = 100000;

static double read_val() {
    char buf[512];
    if (scanf("%511s", buf) != 1) exit(0);
    char *end = nullptr;
    double v = strtod(buf, &end);
    if (end == buf || !isfinite(v)) exit(0);
    if (v < -1e-7) exit(0); // failure response
    return v;
}

static double ask(ll x1, ll y1, ll x2, ll y2) {
    printf("query %lld %lld %lld %lld\n", x1, y1, x2, y2);
    fflush(stdout);
    return read_val();
}

[[noreturn]] static void give(ll x, ll y, ll r) {
    printf("answer %lld %lld %lld\n", x, y, r);
    fflush(stdout);
    exit(0);
}

// integer square root, -1 when not a perfect square
static ll isqrt_exact(ll v) {
    if (v < 0) return -1;
    ll s = (ll)llround(sqrt((long double)v));
    while (s > 0 && s * s > v) --s;
    while ((s + 1) * (s + 1) <= v) ++s;
    return (s * s == v) ? s : -1;
}

int main() {
    vector<pair<ll, ll>> hits; // (y, A) of horizontal chords with A > 0
    set<ll> used;

    auto probe_h = [&](ll y) -> bool {
        if (y < 0 || y > LIM) return false;
        if (used.count(y)) return false;
        used.insert(y);
        double L = ask(0, y, LIM, y);
        if (L > 0.5) {
            hits.push_back({y, (ll)llround(L * L / 4.0)});
            return true;
        }
        return false;
    };

    // ---- localisation: greedy covering of [100,99900] ----
    // Always split the gap whose current covering contribution is maximal
    // (interior gap: half length, boundary gap: full distance to the border).
    {
        const ll A = 100, B = 99900;
        // heap item: (contribution*2, type, a, b)
        priority_queue<tuple<ll, int, ll, ll>> pq;
        ll first = (A + B) / 2;
        pq.push(make_tuple(2 * (first - A), 1, A, first));
        pq.push(make_tuple(2 * (B - first), 2, first, B));
        probe_h(first);
        while (hits.empty() && !pq.empty()) {
            auto [c, typ, a, b] = pq.top();
            pq.pop();
            ll m;
            if (typ == 0) {
                m = (a + b) / 2;
                if (m <= a || m >= b) continue;
                pq.push(make_tuple(m - a, 0, a, m));
                pq.push(make_tuple(b - m, 0, m, b));
            } else if (typ == 1) {
                m = (A + b) / 2;
                if (m <= A || m >= b) continue;
                pq.push(make_tuple(2 * (m - A), 1, A, m));
                pq.push(make_tuple(b - m, 0, m, b));
            } else {
                m = (a + B) / 2;
                if (m <= a || m >= B) continue;
                pq.push(make_tuple(m - a, 0, a, m));
                pq.push(make_tuple(2 * (B - m), 2, m, B));
            }
            probe_h(m);
        }
        // absolute fallback (never needed)
        for (ll y = 0; y <= LIM && hits.empty(); ++y) probe_h(y);
    }

    // ---- second horizontal chord ----
    // A miss at y0+d (d>0) implies s=cy-y0 <= d-r; because A0=r^2-s^2 and s<=d-r
    // this also gives r >= (A0+d^2)/(2d).  Hence d^2 < A0  =>  r > d  =>  s < 0.
    // Symmetrically a miss at y0-d with d^2 < A0 implies s > 0.  Those bounds
    // (already paid for) tell which neighbouring line hits.
    if (!hits.empty()) {
        ll y0 = hits[0].first, A0 = hits[0].second;
        ll dup = -1, ddn = -1;
        auto it = used.upper_bound(y0);
        if (it != used.end()) dup = *it - y0;
        if (it != used.begin()) { auto jt = prev(it); if (*jt != y0) ddn = y0 - *jt; }
        int order[2] = {1, -1};
        if (dup > 0 && dup * dup < A0) { order[0] = -1; order[1] = 1; }
        else if (ddn > 0 && ddn * ddn < A0) { order[0] = 1; order[1] = -1; }
        for (int k = 0; k < 2 && hits.size() < 2; ++k) probe_h(y0 + order[k]);
    }
    if (hits.size() < 2) {
        // mathematically unreachable (at least one of y0+-1 always cuts the
        // disk); keep probing until a second chord is found.
        for (ll y = 0; y <= LIM && hits.size() < 2; ++y) probe_h(y);
    }

    // ---- solve cy and r from two chords ----
    ll cy = -1, r = -1;
    {
        int n = (int)hits.size();
        for (int i = 0; i < n && cy < 0; ++i)
            for (int j = i + 1; j < n && cy < 0; ++j) {
                ll yA = hits[i].first, AA = hits[i].second;
                ll yB = hits[j].first, AB = hits[j].second;
                if (yA == yB) continue;
                ll dy = yA - yB;
                ll num = (AA - AB) + (yA + yB) * dy;
                ll den = 2 * dy;
                if (den < 0) { den = -den; num = -num; }
                if (num % den != 0) continue;
                ll cyc = num / den;
                ll r2 = AA + (cyc - yA) * (cyc - yA);
                ll rc = isqrt_exact(r2);
                if (rc < 100 || cyc < rc || cyc > LIM - rc) continue;
                bool ok = true;
                for (auto &h : hits) {
                    ll v = rc * rc - (cyc - h.first) * (cyc - h.first);
                    if (v != h.second) { ok = false; break; }
                }
                if (!ok) continue;
                cy = cyc;
                r = rc;
            }
    }
    if (cy < 0) {
        // cannot happen mathematically
        if (hits.empty()) give(50000, 50000, 100);
        give(50000, hits[0].first, 100);
    }

    // ---- cx ----
    // (a) r >= 1e5/4 : the whole feasible x-interval is shorter than the chord
    //     diameter, so the first centre-line cut already contains cx.
    // (b) otherwise the near-horizontal line through (1e5,cy) and (0,cy-100)
    //     (direction (1000,1)) is within distance < r of the centre for every
    //     possible cx and its chord always lies strictly inside that segment,
    //     so one probe returns the full chord and
    //        cx = 1e5 - sqrt((r^2 - L^2/4)*(1000^2+1)).
    //     The amplification of the 1e-6 reading error is h/(2t) (t=1e5-cx>=r,
    //     h=L/2<=r); it is < 0.4 unless the disk hugs the right border, in
    //     which case we fall back to the cut binary search.
    ll cx = -1;
    if (2 * r >= LIM - 2 * r) {
        // handled by the binary search below
    } else {
        double L = ask(0, cy - 100, LIM, cy);
        if (L > 0.5) {
            const double DD = 1000001.0; // 1000^2+1
            double h = L / 2.0;
            double V = ((double)r * (double)r - h * h) * DD;
            if (V < 0) V = 0;
            double t = sqrt(V);
            double eps = DD * h * 1.2e-6 / (2.0 * t); // 1.2e-6: safety margin over the stated 1e-6
            if (eps < 0.4) {
                ll c = (ll)llround((double)LIM - t);
                if (c >= r && c <= LIM - r) cx = c;
            }
        }
    }

    // ---- fallback / large r: binary search with cuts on the centre line ----
    ll lo = r, hi = LIM - r;
    while (cx < 0 && lo < hi) {
        ll m = (lo + hi) / 2;
        double L = ask(0, cy, m, cy);
        if (L < 0.5) {
            lo = m + r;
        } else if (L > 2.0 * r - 0.5) {
            hi = m - r;
        } else {
            cx = (ll)llround((double)(m + r) - L);
            break;
        }
    }
    if (cx < 0) cx = lo;
    if (cx < r) cx = r;
    if (cx > LIM - r) cx = LIM - r;
    give(cx, cy, r);
}
