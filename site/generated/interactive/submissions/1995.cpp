#include <bits/stdc++.h>
using namespace std;

static int N;
static int TRANS[2][2][8]; // TRANS[x][e][h] -> new h or -1

struct Node {
    int cnt[7];
    signed char lazy[7];
};

static vector<Node> seg;

static void init_node(int v) {
    for (int i = 0; i < 7; i++) { seg[v].cnt[i] = 0; seg[v].lazy[i] = (signed char)i; }
}

static void build(int v, int l, int r) {
    init_node(v);
    if (l == r) {
        seg[v].cnt[0] = 1;
        return;
    }
    int m = (l + r) >> 1;
    build(v << 1, l, m);
    build(v << 1 | 1, m + 1, r);
    for (int s = 0; s < 7; s++) seg[v].cnt[s] = seg[v << 1].cnt[s] + seg[v << 1 | 1].cnt[s];
}

static inline void apply_map(int v, const int mp[7]) {
    int nc[7] = {0,0,0,0,0,0,0};
    for (int s = 0; s < 7; s++) {
        int c = seg[v].cnt[s];
        if (!c) continue;
        int to = mp[s];
        if (to >= 0) nc[to] += c;
    }
    for (int s = 0; s < 7; s++) seg[v].cnt[s] = nc[s];
    signed char nl[7];
    for (int s = 0; s < 7; s++) {
        int mid = seg[v].lazy[s];
        nl[s] = (mid < 0) ? (signed char)-1 : (signed char)mp[mid];
    }
    for (int s = 0; s < 7; s++) seg[v].lazy[s] = nl[s];
}

static inline void push(int v) {
    bool id = true;
    for (int s = 0; s < 7; s++) if (seg[v].lazy[s] != s) { id = false; break; }
    if (id) return;
    int mp[7];
    for (int s = 0; s < 7; s++) mp[s] = seg[v].lazy[s];
    apply_map(v << 1, mp);
    apply_map(v << 1 | 1, mp);
    for (int s = 0; s < 7; s++) seg[v].lazy[s] = (signed char)s;
}

static inline void pull(int v) {
    for (int s = 0; s < 7; s++) seg[v].cnt[s] = seg[v << 1].cnt[s] + seg[v << 1 | 1].cnt[s];
}

static void update(int v, int l, int r, int ql, int qr, const int mp[7]) {
    if (ql > r || qr < l) return;
    if (ql <= l && r <= qr) { apply_map(v, mp); return; }
    push(v);
    int m = (l + r) >> 1;
    update(v << 1, l, m, ql, qr, mp);
    update(v << 1 | 1, m + 1, r, ql, qr, mp);
    pull(v);
}

static int total_alive() {
    int s = 0;
    for (int i = 0; i < 7; i++) s += seg[1].cnt[i];
    return s;
}

// 1-indexed rank among alive positions
static int kth(int k) {
    int v = 1, l = 1, r = N;
    while (l < r) {
        push(v);
        int m = (l + r) >> 1;
        int lc = 0;
        for (int s = 0; s < 7; s++) lc += seg[v << 1].cnt[s];
        if (k <= lc) { v = v << 1; r = m; }
        else { k -= lc; v = v << 1 | 1; l = m + 1; }
    }
    return l;
}

static int state_at(int pos) {
    int v = 1, l = 1, r = N;
    while (l < r) {
        push(v);
        int m = (l + r) >> 1;
        if (pos <= m) { v = v << 1; r = m; }
        else { v = v << 1 | 1; l = m + 1; }
    }
    for (int s = 0; s < 7; s++) if (seg[v].cnt[s]) return s;
    return 7;
}

static void apply_query(int l, int r, int e) {
    int mp1[7], mp0[7];
    for (int s = 0; s < 7; s++) { mp1[s] = TRANS[1][e][s]; mp0[s] = TRANS[0][e][s]; }
    update(1, 1, N, l, r, mp1);
    if (l > 1) update(1, 1, N, 1, l - 1, mp0);
    if (r < N) update(1, 1, N, r + 1, N, mp0);
}

static void apply_global_one(int e) {
    int mp1[7];
    for (int s = 0; s < 7; s++) mp1[s] = TRANS[1][e][s];
    update(1, 1, N, 1, N, mp1);
}

static int ask(int l, int r) {
    cout << "? " << l << " " << r << '\n' << flush;
    int y;
    if (!(cin >> y)) exit(0);
    int len = r - l + 1;
    int c = len - y;
    if (c < 0 || c > 1) c = 0; // should not happen
    return c;
}

static int last_bit_of(int h) {
    return (h == 2 || h == 4 || h == 6) ? 1 : 0;
}

// ---------- exact small solver for at most SMALL_M alive candidates ----------
static const int SMALL_M = 6;

struct SmallSolver {
    int m;
    vector<int> sp;
    int n;
    vector<int> memo; // key = code*32 + qleft, value -1 unknown, 0 false, 1 true

    SmallSolver(const vector<int>& sp_, int n_) : m((int)sp_.size()), sp(sp_), n(n_) {
        size_t codes = (size_t)1 << (3 * m);
        memo.assign(codes * 32, -1);
    }

    int decode_cnt(int code) {
        int c = 0;
        for (int i = 0; i < m; i++) if (((code >> (3 * i)) & 7) != 7) c++;
        return c;
    }

    bool win(int code, int qleft) {
        int alive = decode_cnt(code);
        if (alive <= 2) return true;
        if (qleft == 0) return false;
        int key = code * 32 + qleft;
        if (memo[key] != -1) return memo[key] != 0;

        int idx[8], A = 0;
        for (int i = 0; i < m; i++) if (((code >> (3 * i)) & 7) != 7) idx[A++] = i;

        int rankOf[8];
        for (int t = 0; t < A; t++) rankOf[idx[t]] = t;

        vector<pair<int,int>> opts;
        opts.reserve(A * (A + 1) / 2 + 1);
        for (int i = 0; i < A; i++) for (int j = i; j < A; j++) opts.push_back({i, j});
        if (n > alive) opts.push_back({-1, -1});

        int xm[8];
        for (auto &pr : opts) {
            int a = pr.first, b = pr.second;
            for (int t = 0; t < A; t++) xm[t] = (a >= 0 && t >= a && t <= b) ? 1 : 0;
            bool ok = true, anyE = false;
            for (int e = 0; e < 2; e++) {
                int ns = 0;
                bool aliveAfter = false;
                for (int i = 0; i < m; i++) {
                    int h = (code >> (3 * i)) & 7;
                    if (h == 7) { ns |= (7 << (3 * i)); continue; }
                    int x = xm[rankOf[i]];
                    int nh = TRANS[x][e][h];
                    if (nh < 0) ns |= (7 << (3 * i));
                    else { ns |= (nh << (3 * i)); aliveAfter = true; }
                }
                if (!aliveAfter) continue;
                anyE = true;
                if (!win(ns, qleft - 1)) { ok = false; break; }
            }
            if (ok && anyE) { memo[key] = 1; return true; }
        }
        memo[key] = 0;
        return false;
    }

    int value(int code) {
        for (int q = 0; q <= 31; q++) if (win(code, q)) return q;
        return 32;
    }

    void choose(int code, int &l, int &r) {
        int q = value(code);
        int idx[8], A = 0;
        for (int i = 0; i < m; i++) if (((code >> (3 * i)) & 7) != 7) idx[A++] = i;
        int rankOf[8];
        for (int t = 0; t < A; t++) rankOf[idx[t]] = t;

        vector<pair<int,int>> opts;
        opts.reserve(A * (A + 1) / 2 + 1);
        for (int i = 0; i < A; i++) for (int j = i; j < A; j++) opts.push_back({i, j});
        if (n > A) opts.push_back({-1, -1});

        int xm[8];
        for (auto &pr : opts) {
            int a = pr.first, b = pr.second;
            for (int t = 0; t < A; t++) xm[t] = (a >= 0 && t >= a && t <= b) ? 1 : 0;
            bool ok = true, anyE = false;
            for (int e = 0; e < 2; e++) {
                int ns = 0;
                bool aliveAfter = false;
                for (int i = 0; i < m; i++) {
                    int h = (code >> (3 * i)) & 7;
                    if (h == 7) { ns |= (7 << (3 * i)); continue; }
                    int x = xm[rankOf[i]];
                    int nh = TRANS[x][e][h];
                    if (nh < 0) ns |= (7 << (3 * i));
                    else { ns |= (nh << (3 * i)); aliveAfter = true; }
                }
                if (!aliveAfter) continue;
                anyE = true;
                if (!win(ns, q - 1)) { ok = false; break; }
            }
            if (ok && anyE) {
                if (a < 0) {
                    for (int p = 1; p <= n; p++) {
                        bool used = false;
                        for (int t = 0; t < A; t++) if (sp[idx[t]] == p) { used = true; break; }
                        if (!used) { l = r = p; return; }
                    }
                } else {
                    l = sp[idx[a]];
                    r = sp[idx[b]];
                    return;
                }
            }
        }
        l = r = sp[idx[0]]; // fallback (should not happen)
    }
};

static void extract_alive(int total, vector<int>& sp, vector<int>& hs) {
    sp.resize(total); hs.resize(total);
    for (int i = 0; i < total; i++) {
        sp[i] = kth(i + 1);
        hs[i] = state_at(sp[i]);
    }
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    if (!(cin >> N)) return 0;

    // build transitions
    for (int h = 0; h < 8; h++) {
        for (int x = 0; x < 2; x++) {
            for (int e = 0; e < 2; e++) {
                if (h == 7) { TRANS[x][e][h] = -1; continue; }
                int d2, d1; bool has2 = false, has1 = false;
                if (h == 0) { }
                else if (h == 1) { has1 = true; d1 = 0; }
                else if (h == 2) { has1 = true; d1 = 1; }
                else if (h == 3) { has1 = true; d2 = 0; d1 = 0; has2 = true; }
                else if (h == 4) { has1 = true; d2 = 0; d1 = 1; has2 = true; }
                else if (h == 5) { has1 = true; d2 = 1; d1 = 0; has2 = true; }
                else { has1 = true; d2 = 1; d1 = 1; has2 = true; }
                int d = e ^ x;
                if (has2 && d2 == d1 && d1 == d) { TRANS[x][e][h] = -1; continue; }
                int nh;
                if (!has1) nh = (d == 0 ? 1 : 2);
                else nh = ((d1 << 1) | d) + 3;
                TRANS[x][e][h] = nh;
            }
        }
    }

    seg.assign(4 * N + 5, Node());
    build(1, 1, N);

    while (true) {
        int total = total_alive();
        if (total <= 2) break;
        if (total <= SMALL_M) {
            vector<int> sp, hs;
            extract_alive(total, sp, hs);
            int m = total;
            SmallSolver ss(sp, N);
            int code = 0;
            for (int i = 0; i < m; i++) code |= (hs[i] << (3 * i));
            while (total > 2) {
                int l, r;
                ss.choose(code, l, r);
                int e = ask(l, r);
                apply_query(l, r, e);
                code = 0;
                for (int i = 0; i < m; i++) {
                    int h = state_at(sp[i]);
                    code |= (h << (3 * i));
                }
                total = total_alive();
            }
            break;
        }

        // ---- synchronization: make all alive share the same full history ----
        int nz = 0, hcommon = -1, lastmask = 0;
        for (int s = 0; s < 7; s++) if (seg[1].cnt[s]) {
            nz++;
            hcommon = s;
            lastmask |= (last_bit_of(s) ? 2 : 1);
        }
        int need = 0;
        if (nz == 1 && hcommon >= 3) need = 0;
        else if (nz == 1) need = (hcommon == 0 ? 2 : 1);
        else need = (lastmask == 1 || lastmask == 2) ? 1 : 2;

        for (int t = 0; t < need; t++) {
            int L = kth(1), R = kth(total);
            int e = ask(L, R);
            apply_global_one(e);
            total = total_alive();
            if (total <= SMALL_M) break;
        }
        if (total <= SMALL_M) continue;

        // all alive now have the same full history; find it
        int h = -1;
        for (int s = 3; s < 7; s++) if (seg[1].cnt[s]) { h = s; break; }
        if (h < 0) {
            // fallback: run one more global query
            int L = kth(1), R = kth(total);
            int e = ask(L, R);
            apply_global_one(e);
            continue;
        }

        int cnt0 = total;
        int left_size = cnt0 / 2;
        int pivot = kth(left_size);
        int lsplit = kth(1);
        int e = ask(lsplit, pivot);
        apply_query(lsplit, pivot, e);
        total = total_alive();
        if (total <= SMALL_M) continue;

        if (h == 3 || h == 6) {
            // the split query was already a clean cut
            continue;
        }

        int b = last_bit_of(h);
        bool left_rigid = (e == (1 - b));
        int rigid_size = left_rigid ? left_size : (cnt0 - left_size);
        int a = rigid_size / 2;
        int k2rank = left_rigid ? a : (left_size + a);
        int k2 = kth(k2rank);
        int lcut = kth(1);
        int e3 = ask(lcut, k2);
        apply_query(lcut, k2, e3);
    }

    // ---- nominate ----
    int total = total_alive();
    int cand1 = kth(1);
    cout << "! " << cand1 << '\n' << flush;
    int res;
    if (!(cin >> res)) return 0;
    if (res == 0 && total >= 2) {
        int cand2 = kth(2);
        cout << "! " << cand2 << '\n' << flush;
        if (!(cin >> res)) return 0;
    }
    cout << "#\n" << flush;
    return 0;
}
