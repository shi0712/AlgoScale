// Cave: recover the wiring (switch -> door) and the correct switch positions.
//
// Model: switch i sits behind door p[i] and has correct position c[i].
// For a probe string s (one bit per switch) a door is closed iff the switch's
// correct position differs from s at that switch, so the reply is the smallest
// closed door, min{ p[i] : s[i] != c[i] }, or -1 when s == c.
//
// Strategy: peel the doors in increasing order.  Every switch behind the doors
// [0,k) is known and set correctly, hence a probe reveals the smallest door >= k
// whose switch disagrees with the guess.  A probe that is constant on all still
// unknown switches is correct on every door of the current run of equal labels
// and stops at the run's end, so one probe reveals the next run boundary.  The
// switches of the current run are then recovered either
//   * in bulk: flip every still unknown switch once -- the reply is that
//     switch's door whenever the door lies inside the run, so one probe per
//     switch resolves the entire run (and gives the next boundary for free), or
//   * per door by a binary search over the unknown switches, which is far
//     cheaper for short runs.
// When all remaining doors carry the same label every remaining switch is
// identified with a single probe each.
#include <bits/stdc++.h>
#include <unistd.h>
using namespace std;

static int N;
static vector<int> cpos;    // correct position of each switch (filled when resolved)
static vector<int> doorSw;  // switch behind each door, -1 = unknown
static vector<int> doorLb;  // correct position of the switch behind each door
static vector<char> taken;  // switch already resolved
static vector<int> unres;   // switches not resolved yet

static char *lineBuf;       // "? " + probe + "\n"
static char *probe;         // = lineBuf + 2, the probe bits

static char *ibbuf;
static int ibpos = 0, iblen = 0;

static inline int nextChar() {
    if (ibpos == iblen) {
        iblen = (int)read(0, ibbuf, 1 << 16);
        ibpos = 0;
        if (iblen <= 0) exit(0);
    }
    return ibbuf[ibpos++];
}

static inline int rdInt() {
    int c = nextChar();
    while (c != '-' && (c < '0' || c > '9')) c = nextChar();
    bool neg = false;
    if (c == '-') { neg = true; c = nextChar(); }
    int v = 0;
    while (c >= '0' && c <= '9') {
        v = v * 10 + (c - '0');
        if (ibpos == iblen) {
            iblen = (int)read(0, ibbuf, 1 << 16);
            ibpos = 0;
            if (iblen <= 0) break;
        }
        c = ibbuf[ibpos++];
    }
    return neg ? -v : v;
}

static inline void sendOut(const char *p, int len) {
    while (len > 0) {
        ssize_t w = write(1, p, len);
        if (w <= 0) exit(0);
        p += w;
        len -= (int)w;
    }
}

// issue the probe currently stored in probe[]
static int ask() {
    lineBuf[N + 2] = '\n';
    sendOut(lineBuf, N + 3);
    return rdInt();
}

static int ceilLog2(int m) {
    if (m <= 1) return 0;
    int b = 0;
    while ((1LL << b) < m) ++b;
    return b;
}

// probe := resolved switches carry their label, unknown switches carry 0
static void buildBase() {
    memset(probe, '0', N);
    for (int d = 0; d < N; ++d)
        if (doorSw[d] >= 0) probe[doorSw[d]] = char('0' + doorLb[d]);
}

// Every still unknown switch has the known label lbl: identify each with a probe.
static void singleton_finish(int lbl) {
    int U = (int)unres.size();
    if (U == 0) return;
    buildBase();
    for (int i : unres) probe[i] = char('0' + lbl);
    for (int t = 0; t + 1 < U; ++t) {
        int i = unres[t];
        probe[i] = char('0' + (1 - lbl));
        int r = ask();
        probe[i] = char('0' + lbl);
        if (r < 0) exit(0);
        doorSw[r] = i;
        doorLb[r] = lbl;
        cpos[i] = lbl;
        taken[i] = 1;
    }
    int last = unres[U - 1];
    int rd = -1;
    for (int d = 0; d < N; ++d)
        if (doorSw[d] < 0) { rd = d; break; }
    if (rd < 0) exit(0);
    doorSw[rd] = last;
    doorLb[rd] = lbl;
    cpos[last] = lbl;
    unres.clear();
}

static void answer_out() {
    for (int d = 0; d < N; ++d) {
        if (doorSw[d] < 0) exit(0);
        cpos[doorSw[d]] = doorLb[d];
    }
    vector<int> mp(N, -1);
    for (int d = 0; d < N; ++d) mp[doorSw[d]] = d;
    vector<char> out;
    out.reserve((size_t)N * 7 + 16);
    out.push_back('!');
    out.push_back(' ');
    for (int i = 0; i < N; ++i) out.push_back(char('0' + cpos[i]));
    for (int i = 0; i < N; ++i) {
        out.push_back(' ');
        int v = mp[i];
        char tmp[12]; int t = 0;
        if (v == 0) tmp[t++] = '0';
        while (v > 0) { tmp[t++] = char('0' + v % 10); v /= 10; }
        while (t > 0) out.push_back(tmp[--t]);
    }
    out.push_back('\n');
    sendOut(out.data(), (int)out.size());
    exit(0);
}

int main() {
    ibbuf = (char *)malloc(1 << 16);
    N = rdInt();
    if (N < 1) exit(0);
    cpos.assign(N, -1);
    doorSw.assign(N, -1);
    doorLb.assign(N, -1);
    taken.assign(N, 0);
    unres.resize(N);
    for (int i = 0; i < N; ++i) unres[i] = i;

    lineBuf = (char *)malloc((size_t)N + 8);
    lineBuf[0] = '?';
    lineBuf[1] = ' ';
    probe = lineBuf + 2;
    memset(probe, '0', N);

    int knownCnt = 0;  // doorLb known for all doors < knownCnt

    int a0 = ask();
    if (a0 == -1) {
        for (int d = 0; d < N; ++d) doorLb[d] = 0;
        knownCnt = N;
        singleton_finish(0);
        answer_out();
    }
    if (a0 > 0) {
        for (int d = 0; d < a0; ++d) doorLb[d] = 0;
        doorLb[a0] = 1;
        knownCnt = a0 + 1;
    } else {
        // door 0 carries label 1
        if (N == 1) {
            doorLb[0] = 1;
            knownCnt = 1;
        } else {
            memset(probe, '1', N);
            int z = ask();
            if (z == -1) {
                for (int d = 0; d < N; ++d) doorLb[d] = 1;
                knownCnt = N;
                singleton_finish(1);
                answer_out();
            }
            for (int d = 0; d < z; ++d) doorLb[d] = 1;
            doorLb[z] = 0;
            knownCnt = z + 1;
        }
    }

    while (!unres.empty()) {
        int curn = N;
        for (int d = 0; d < N; ++d)
            if (doorSw[d] < 0) { curn = d; break; }
        if (curn >= N) break;
        int lbl = doorLb[curn];
        if (lbl < 0) exit(0);

        int d = -1;  // first door > curn with a different (known) label
        for (int e = curn + 1; e < knownCnt; ++e)
            if (doorLb[e] != lbl) { d = e; break; }
        if (d < 0) {
            // No known door beyond curn carries a different label.  Normally the
            // whole suffix is known here (knownCnt == N) and every remaining door
            // carries label lbl; otherwise extend the known labels first.
            if (knownCnt < N) {
                buildBase();
                for (int i : unres) probe[i] = char('0' + lbl);
                int m = ask();  // first door >= knownCnt that is not lbl
                if (m == -1) {
                    for (int e = knownCnt; e < N; ++e) doorLb[e] = lbl;
                    knownCnt = N;
                } else {
                    for (int e = knownCnt; e < m; ++e) doorLb[e] = lbl;
                    doorLb[m] = 1 - lbl;
                    knownCnt = m + 1;
                    continue;
                }
            }
            singleton_finish(lbl);
            break;
        }

        int U = (int)unres.size();
        int L = d - curn;
        int bsCost = 1;  // plus the probe that finds the next run boundary
        for (int p = 0; p < L; ++p) bsCost += ceilLog2(U - p);

        if (U <= bsCost) {
            // ---------- bulk round: resolves doors [curn, d] ----------
            buildBase();
            for (int i : unres) probe[i] = char('0' + lbl);
            int nextBoundary = -2;
            for (size_t idx = 0; idx < unres.size(); ++idx) {
                int i = unres[idx];
                probe[i] = char('0' + (1 - lbl));
                int a = ask();
                probe[i] = char('0' + lbl);
                if (a >= 0 && a < d) {
                    doorSw[a] = i;
                    doorLb[a] = lbl;
                    cpos[i] = lbl;
                    taken[i] = 1;
                } else if (a == -1 || a > d) {
                    doorSw[d] = i;
                    doorLb[d] = 1 - lbl;
                    cpos[i] = 1 - lbl;
                    taken[i] = 1;
                    nextBoundary = a;
                }
            }
            if (nextBoundary == -2) exit(0);
            {
                vector<int> nu;
                for (int i : unres)
                    if (!taken[i]) nu.push_back(i);
                unres.swap(nu);
            }
            if (nextBoundary == -1) {
                for (int e = d + 1; e < N; ++e) doorLb[e] = lbl;
                knownCnt = N;
            } else if (nextBoundary > d + 1) {
                for (int e = d + 1; e < nextBoundary; ++e) doorLb[e] = lbl;
                doorLb[nextBoundary] = 1 - lbl;
                knownCnt = nextBoundary + 1;
            } else {  // nextBoundary == d+1: door d+1 carries label 1-lbl
                doorLb[d + 1] = 1 - lbl;
                buildBase();
                for (int i : unres) probe[i] = char('0' + (1 - lbl));
                int m = ask();
                if (m == -1) {
                    for (int e = d + 2; e < N; ++e) doorLb[e] = 1 - lbl;
                    knownCnt = N;
                } else {
                    for (int e = d + 2; e < m; ++e) doorLb[e] = 1 - lbl;
                    doorLb[m] = lbl;
                    knownCnt = m + 1;
                }
            }
        } else {
            // ---------- binary search for each door of the run ----------
            buildBase();
            for (int i : unres) probe[i] = char('0' + (1 - lbl));
            for (int p = curn; p < d; ++p) {
                size_t lo = 0, hi = unres.size();  // candidates = unres[lo..hi)
                while (hi - lo > 1) {
                    size_t mid = lo + (hi - lo) / 2;
                    for (size_t q = lo; q < mid; ++q)
                        probe[unres[q]] = char('0' + lbl);
                    int a = ask();
                    for (size_t q = lo; q < mid; ++q)
                        probe[unres[q]] = char('0' + (1 - lbl));
                    if (a == p) lo = mid;  // door p's switch is outside the group
                    else hi = mid;         // it is inside the group
                }
                if (hi - lo != 1) exit(0);
                int j = unres[lo];
                doorSw[p] = j;
                doorLb[p] = lbl;
                cpos[j] = lbl;
                taken[j] = 1;
                probe[j] = char('0' + lbl);
                unres.erase(unres.begin() + lo);
            }
            // next door is d with label 1-lbl; locate the next run of label lbl
            int m = ask();
            doorLb[d] = 1 - lbl;
            if (m == -1) {
                for (int e = d + 1; e < N; ++e) doorLb[e] = 1 - lbl;
                knownCnt = N;
            } else {
                for (int e = d + 1; e < m; ++e) doorLb[e] = 1 - lbl;
                doorLb[m] = lbl;
                knownCnt = m + 1;
            }
        }
    }
    answer_out();
}
