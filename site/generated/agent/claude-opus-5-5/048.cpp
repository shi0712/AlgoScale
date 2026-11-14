#include <bits/stdc++.h>
using namespace std;

static int n, N;
static vector<int> A;

static char *ibuf; static size_t ipos = 0, ilen = 0;
static int readInt() {
    while (ipos < ilen && (ibuf[ipos] < '0' || ibuf[ipos] > '9')) ipos++;
    int x = 0;
    while (ipos < ilen && ibuf[ipos] >= '0' && ibuf[ipos] <= '9') x = x * 10 + (ibuf[ipos++] - '0');
    return x;
}

// Banded construction: points sorted by t descending, split into contiguous bands;
// earlier band = lower removal priority p; inside a band, larger s = lower p.
struct Solver {
    int K;
    const vector<int> &s, &t, &ordT;
    vector<int> E;            // E[l] = earliest time of committed card with L >= l (1-indexed)
    vector<int> fen, ss, Ls, Lt;
    vector<int> p;            // removal priority per rank
    Solver(int K, const vector<int> &s, const vector<int> &t, const vector<int> &ordT)
        : K(K), s(s), t(t), ordT(ordT) {}

    inline int Gq(int tau) { // max L of committed cards at time < tau
        return int(lower_bound(E.begin() + 1, E.begin() + K + 1, tau) - (E.begin() + 1));
    }
    // evaluate band ordT[l..r); return true if max L <= K
    bool evalBand(int l, int r, bool commit) {
        int m = r - l;
        ss.resize(m);
        for (int i = 0; i < m; i++) ss[i] = s[ordT[l + i]];
        sort(ss.begin(), ss.end());
        fen.assign(m + 1, 0);
        int ptr = 0;
        if (commit) { Ls.resize(m); Lt.resize(m); }
        for (int i = r - 1; i >= l; i--) {
            int y = ordT[i];
            int ty = t[y], sy = s[y];
            while (ptr < m && ss[ptr] < ty) ptr++;
            // ptr-1 is index of max s < ty (exists since sy < ty)
            int term2 = 1 + Gq(ss[ptr - 1]);
            int term1 = Gq(ty);
            int rk = int(lower_bound(ss.begin(), ss.end(), sy) - ss.begin());
            int j = m - rk; // suffix in s => prefix in reversed index; positions 1..m
            int term3 = 0;
            for (int q = j; q > 0; q -= q & -q) term3 = max(term3, fen[q]);
            int L = 1 + max(term1, max(term2, term3));
            if (L > K) return false;
            for (int q = j; q <= m; q += q & -q) fen[q] = max(fen[q], L);
            if (commit) {
                int idx = i - l;
                Ls[idx] = 1 + Gq(sy);
                Lt[idx] = L;
                p[y] = j - 1; // relative rank within band (larger s -> smaller)
            }
        }
        return true;
    }
    void commitCard(int c, int L) {
        for (int l = L; l >= 1; l--) {
            if (E[l] > c) E[l] = c; else break;
        }
    }
    bool run() {
        E.assign(K + 2, INT_MAX);
        p.assign(n + 1, 0);
        int pos = 0, base = 0;
        int g = max(1, n / max(1, K));
        while (pos < n) {
            if (!evalBand(pos, pos + 1, false)) return false;
            int lo = pos + 1, hi = n + 1; // lo good, hi bad (n+1 means none known)
            int r = min(n, pos + g);
            if (r > lo) {
                if (evalBand(pos, r, false)) {
                    lo = r;
                    int step = max(1, g / 4);
                    while (lo < n) {
                        int rr = min(n, lo + step);
                        if (evalBand(pos, rr, false)) { lo = rr; step *= 2; }
                        else { hi = rr; break; }
                    }
                } else {
                    hi = r;
                    int step = max(1, g / 4);
                    while (true) {
                        int rr = hi - step;
                        if (rr <= lo) break;
                        if (evalBand(pos, rr, false)) { lo = rr; break; }
                        hi = rr; step *= 2;
                    }
                }
            }
            if (hi > n) hi = n + 1;
            while (hi - lo > 1) {
                int mid = (lo + hi) / 2;
                if (evalBand(pos, mid, false)) lo = mid; else hi = mid;
            }
            int m = lo - pos;
            evalBand(pos, lo, true);
            for (int i = pos; i < lo; i++) {
                int y = ordT[i];
                p[y] += base;
                commitCard(s[y], Ls[i - pos]);
                commitCard(t[y], Lt[i - pos]);
            }
            base += m;
            pos = lo;
            g = m;
        }
        return true;
    }
};

// Given a sequence (ranks) and priorities, compute stack assignment; returns number of stacks.
static int assignStacks(const vector<int> &seq, const vector<int> &pr, vector<int> &out) {
    vector<int> tops; tops.reserve(256);
    out.resize(seq.size());
    for (size_t i = 0; i < seq.size(); i++) {
        int v = pr[seq[i]];
        auto it = upper_bound(tops.begin(), tops.end(), v);
        out[i] = int(it - tops.begin());
        if (it == tops.end()) tops.push_back(v); else *it = v;
    }
    return (int)tops.size();
}

// attempt on a sequence; returns priorities (empty if fail)
static bool attempt(const vector<int> &seq, int K, vector<int> &pr) {
    vector<int> s(n + 1, -1), t(n + 1, -1);
    for (int i = 0; i < N; i++) { int x = seq[i]; if (s[x] < 0) s[x] = i; else t[x] = i; }
    vector<int> ordT; ordT.reserve(n);
    for (int i = N - 1; i >= 0; i--) if (t[seq[i]] == i) ordT.push_back(seq[i]);
    Solver S(K, s, t, ordT);
    if (!S.run()) return false;
    pr = S.p;
    return true;
}

int main() {
    // read all input
    {
        size_t cap = 1 << 16; ibuf = (char *)malloc(cap);
        size_t r;
        while ((r = fread(ibuf + ilen, 1, cap - ilen, stdin)) > 0) {
            ilen += r;
            if (ilen == cap) { cap *= 2; ibuf = (char *)realloc(ibuf, cap); }
        }
    }
    n = readInt(); N = 2 * n;
    A.resize(N);
    for (int i = 0; i < N; i++) A[i] = readInt();
    vector<int> Arev(A.rbegin(), A.rend());

    auto Tstart = chrono::steady_clock::now();
    vector<int> best; int bestCnt = INT_MAX;
    vector<int> out;
    auto tryOne = [&](int K, bool rev) -> bool {
        vector<int> pr;
        if (!attempt(rev ? Arev : A, K, pr)) return false;
        if (rev) for (int x = 1; x <= n; x++) pr[x] = -pr[x];
        int c = assignStacks(A, pr, out);
        if (c < bestCnt) { bestCnt = c; best = out; }
        return c <= 180;
    };
    bool done = tryOne(180, false);
    if (!done && chrono::duration<double>(chrono::steady_clock::now() - Tstart).count() < 0.9) done = tryOne(180, true);
    auto T0 = chrono::steady_clock::now();
    auto elapsed = [&]() { return chrono::duration<double>(chrono::steady_clock::now() - Tstart).count(); };
    for (int K = 176; !done && K >= 1 && elapsed() < 0.8; K -= (K > 100 ? 8 : (K > 20 ? 4 : 1))) {
        done = tryOne(K, false);
        if (!done) done = tryOne(K, true);
    }
    if (best.empty()) {
        // fallback: priority by t descending (always produces something)
        vector<int> s(n + 1, -1), pr(n + 1);
        int c = 0;
        vector<int> seen(n + 1, 0);
        for (int i = N - 1; i >= 0; i--) if (!seen[A[i]]) { seen[A[i]] = 1; pr[A[i]] = c++; }
        assignStacks(A, pr, best);
    }
    // output
    string o; o.reserve((size_t)N * 4 + 16);
    char tmp[16];
    for (int i = 0; i < N; i++) {
        int v = best[i] + 1, l = 0;
        while (v) { tmp[l++] = char('0' + v % 10); v /= 10; }
        while (l) o.push_back(tmp[--l]);
        o.push_back(i + 1 < N ? ' ' : '\n');
    }
    fwrite(o.data(), 1, o.size(), stdout);
    return 0;
}
