#include <bits/stdc++.h>
using namespace std;

// Hash map value -> id (open addressing)
struct HMap {
    vector<unsigned> keys; vector<int> vals; unsigned mask = 0;
    void init(int cnt) {
        unsigned sz = 1; while (sz < (unsigned)cnt * 2 + 2) sz <<= 1;
        keys.assign(sz, 0); vals.assign(sz, -1); mask = sz - 1;
    }
    static unsigned h(unsigned x) { x ^= x >> 16; x *= 0x7feb352dU; x ^= x >> 15; x *= 0x846ca68bU; x ^= x >> 16; return x; }
    void insert(unsigned k, int v) {
        unsigned i = h(k) & mask;
        while (vals[i] != -1 && keys[i] != k) i = (i + 1) & mask;
        keys[i] = k; vals[i] = v;
    }
    int get(unsigned k) const {
        unsigned i = h(k) & mask;
        while (vals[i] != -1) { if (keys[i] == k) return vals[i]; i = (i + 1) & mask; }
        return -1;
    }
};

static vector<uint16_t> RUNbuf;

// Try pattern (a). Returns true and fills ops (forward ops, 1/2) if found.
bool solveA(const vector<unsigned>& A, const vector<unsigned>& B, vector<int>& ops) {
    int n = A.size(), m = B.size();
    vector<unsigned> P(n + 1, 0);
    for (int i = 0; i < n; i++) P[i + 1] = P[i] ^ A[i];
    // ids
    HMap hm; hm.init(n + 1);
    vector<int> id(n + 1);
    int cnt = 0;
    for (int i = 0; i <= n; i++) {
        int g = hm.get(P[i]);
        if (g < 0) { g = cnt++; hm.insert(P[i], g); }
        id[i] = g;
    }
    // CSR by key = id*3 + (pos%3)
    int K = cnt * 3;
    vector<int> st(K + 1, 0), lst(n + 1);
    for (int i = 0; i <= n; i++) st[id[i] * 3 + i % 3 + 1]++;
    for (int k = 0; k < K; k++) st[k + 1] += st[k];
    {
        vector<int> fill(st.begin(), st.end() - 1);
        for (int i = 0; i <= n; i++) lst[fill[id[i] * 3 + i % 3]++] = i;
    }
    // B prefix / suffix (1-indexed b_k = B[k-1])
    vector<unsigned> pre(m + 1, 0), suf(m + 2, 0);
    for (int k = 1; k <= m; k++) pre[k] = pre[k - 1] ^ B[k - 1];
    for (int k = m; k >= 1; k--) suf[k] = suf[k + 1] ^ B[k - 1];
    // RUN[q][k]
    size_t W = m + 1;
    RUNbuf.assign((size_t)(n + 1) * W, 0);
    auto RUN = [&](int q, int k) -> int { return RUNbuf[(size_t)q * W + k]; };
    for (int q = 1; q <= n; q++)
        for (int k = 1; k <= m; k++)
            if (A[q - 1] == B[k - 1]) RUNbuf[(size_t)q * W + k] = RUNbuf[(size_t)(q - 1) * W + (k - 1)] + 1;

    auto longQ = [&](int q, int vid) -> int {
        // largest q' <= q-4 with q' % 3 == (q-1)%3 and id == vid
        if (q < 4 || vid < 0) return -1;
        int key = vid * 3 + (q - 1) % 3;
        int lo = st[key], hi = st[key + 1];
        int x = q - 4;
        int pos = upper_bound(lst.begin() + lo, lst.begin() + hi, x) - lst.begin();
        if (pos == lo) return -1;
        return lst[pos - 1];
    };

    vector<int> E0(m + 1), E1(m + 1), E2(m + 1), ent1(m + 1), par2(m + 1), S(m + 1);
    vector<pair<int, int>> qu; qu.reserve(m + 1);
    int pres = ((2 * n - m) % 3 + 3) % 3;
    int fp = -1, fj = -1, fst = -1, fE = -1;
    for (int p = pres; p <= n && fp < 0; p += 3) {
        // right chain
        S[0] = p;
        int rmax = 0; // S[0..rmax] valid
        for (int k = 1; k <= m - 1; k++) {
            int vid = hm.get(P[p] ^ pre[k]);
            if (vid < 0) break;
            int x = S[k - 1] + 2;
            if (x > n) break;
            int key = vid * 3 + x % 3;
            int lo = st[key], hi = st[key + 1];
            int pos = lower_bound(lst.begin() + lo, lst.begin() + hi, x) - lst.begin();
            if (pos == hi) break;
            S[k] = lst[pos];
            rmax = k;
        }
        // left chain
        E0[m] = p; E1[m] = -1; E2[m] = -1;
        qu.clear(); size_t qf = 0;
        int lmin = m; // indices lmin..m valid
        for (int k = m; k >= 1; k--) {
            int vid = hm.get(P[p] ^ suf[k]);
            if (E0[k] >= 1 && A[E0[k] - 1] == B[k - 1]) qu.push_back({k, E0[k]});
            while (qf < qu.size() && RUN(qu[qf].second, qu[qf].first) < qu[qf].first - k + 1) qf++;
            if (qf < qu.size()) { E1[k - 1] = qu[qf].second - (qu[qf].first - k + 1); ent1[k - 1] = qu[qf].first; }
            else E1[k - 1] = -1;
            E0[k - 1] = (E0[k] >= 0) ? longQ(E0[k], vid) : -1;
            int a2 = (E2[k] >= 0) ? longQ(E2[k], vid) : -1;
            int a1 = (E1[k] >= 0) ? longQ(E1[k], vid) : -1;
            if (a2 >= a1) { E2[k - 1] = a2; par2[k - 1] = 2; } else { E2[k - 1] = a1; par2[k - 1] = 1; }
            if (E0[k - 1] < 0 && E1[k - 1] < 0 && E2[k - 1] < 0 && qf >= qu.size()) { lmin = k; break; }
            lmin = k - 1;
        }
        // combine
        for (int j = max(1, lmin); j <= m && j - 1 <= rmax; j++) {
            int Sv = S[j - 1];
            int mE = max(E0[j], max(E1[j], E2[j]));
            if (mE < 0) continue;
            int s = mE + n - Sv;
            if (s < 1) continue;
            int chosen = -1;
            if (s == 1) {
                if (E0[j] == mE) chosen = 0;
                else if (E1[j] == mE) chosen = 1;
                else continue;
            } else {
                if (E0[j] == mE) chosen = 0;
                else if (E1[j] == mE) chosen = 1;
                else chosen = 2;
            }
            fp = p; fj = j; fst = chosen; fE = mE;
            break;
        }
    }
    if (fp < 0) return false;
    int p = fp, j = fj;
    // reconstruct left positions pos[k], k=j..m
    vector<int> pos(m + 1, -1);
    {
        int k = j, stt = fst;
        while (k <= m) {
            if (stt == 0) { pos[k] = E0[k]; k++; }
            else if (stt == 2) { pos[k] = E2[k]; int pr = par2[k]; k++; stt = pr; }
            else { // state 1
                int kp = ent1[k];
                int q = E1[k] + (kp - k);  // = E0[kp]
                for (int t = k; t < kp; t++) pos[t] = q - (kp - t);
                k = kp; stt = 0;
            }
        }
    }
    // boundaries z[1..m+1]
    vector<long long> z(m + 2);
    for (int i = 1; i <= j; i++) z[i] = S[i - 1];
    for (int i = j + 1; i <= m + 1; i++) z[i] = (long long)pos[i - 1] + n;
    vector<int> L(m + 1);
    for (int i = 1; i <= m; i++) L[i] = (int)(z[i + 1] - z[i]);
    (void)fE;

    // reverse construction
    vector<char> rev; rev.reserve(n);
    vector<int> Ls, Rs;
    long long w, T = n;
    int u1 = -1, u2 = -1;
    for (int i = 1; i <= m; i++) if (L[i] == 1) { if (u1 < 0) u1 = i; u2 = i; }
    if (u1 >= 0) {
        for (int i = u1 - 1; i >= 1; i--) Rs.push_back(L[i]);
        for (int i = u2 + 1; i <= m; i++) Ls.push_back(L[i]);
        if (j <= u2) w = p; else w = (long long)p + n;
    } else {
        long long e0 = T - z[j];
        if (j > 1 || e0 > 0) {
            int t = L[1];
            rev.push_back('R');
            w = (long long)p + t;
            if (t - 1 >= 2) Ls.push_back(t - 1);
            for (int i = m; i >= 2; i--) Rs.push_back(L[i]);
        } else {
            int t = L[1];
            // cross right with (x2=t-2, x1=2), then cross left the 2 with (1,1)
            rev.push_back('R');
            rev.push_back('L');
            w = (long long)p + t - 2;
            for (int i = 2; i <= m; i++) Ls.push_back(L[i]);
            Ls.push_back(t - 2);
        }
    }
    while (!Ls.empty() || !Rs.empty()) {
        long long d = T - w;
        bool modeA = d > 0 || (d == 0 && !Ls.empty());
        if (modeA) {
            if (!Ls.empty()) {
                int g = Ls.back(); Ls.pop_back();
                w -= g; Rs.push_back(g - 2); Rs.push_back(2); rev.push_back('L');
            } else {
                int f = Rs.back(); Rs.pop_back();
                w += f; if (f - 1 >= 2) Ls.push_back(f - 1); rev.push_back('R');
            }
        } else {
            if (!Rs.empty()) {
                int f = Rs.back(); Rs.pop_back();
                w += f; Ls.push_back(f - 2); Ls.push_back(2); rev.push_back('R');
            } else {
                int g = Ls.back(); Ls.pop_back();
                w -= g; if (g - 1 >= 2) Rs.push_back(g - 1); rev.push_back('L');
            }
        }
        if ((int)rev.size() > n) break;
    }
    ops.clear();
    for (int i = (int)rev.size() - 1; i >= 0; i--) ops.push_back(rev[i] == 'L' ? 1 : 2);
    return true;
}

bool check(const vector<unsigned>& A, const vector<unsigned>& B, const vector<int>& ops) {
    if ((int)ops.size() != (int)A.size() - (int)B.size()) return false;
    deque<unsigned> d(A.begin(), A.end());
    for (int o : ops) {
        if (d.size() < 2) return false;
        if (o == 1) { unsigned x = d.front(); d.pop_front(); unsigned y = d.front(); d.pop_front(); d.push_back(x ^ y); }
        else { unsigned y = d.back(); d.pop_back(); unsigned x = d.back(); d.pop_back(); d.push_front(x ^ y); }
    }
    for (size_t i = 0; i < B.size(); i++) if (d[i] != B[i]) return false;
    return true;
}

int main() {
    int c, t;
    if (scanf("%d %d", &c, &t) != 2) return 0;
    string out;
    while (t--) {
        int n, m; scanf("%d %d", &n, &m);
        vector<unsigned> A(n), B(m);
        for (auto& x : A) scanf("%u", &x);
        for (auto& x : B) scanf("%u", &x);
        unsigned xa = 0, xb = 0;
        for (auto x : A) xa ^= x;
        for (auto x : B) xb ^= x;
        vector<int> ops;
        bool ok = false;
        if (xa == xb) {
            if (solveA(A, B, ops)) ok = true;
            else {
                vector<unsigned> rA(A.rbegin(), A.rend()), rB(B.rbegin(), B.rend());
                if (solveA(rA, rB, ops)) { ok = true; for (auto& o : ops) o = 3 - o; }
            }
        }
#ifdef DEBUG
        if (ok && !check(A, B, ops)) { fprintf(stderr, "CHECK FAILED\n"); }
#endif
        if (ok) {
            out += "Yes\n";
            for (size_t i = 0; i < ops.size(); i++) { if (i) out += ' '; out += char('0' + ops[i]); }
            out += '\n';
        } else out += "No\n";
    }
    fputs(out.c_str(), stdout);
}
