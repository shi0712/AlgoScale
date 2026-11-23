#include <bits/stdc++.h>
using namespace std;

const int MAXN = 800;
typedef bitset<MAXN + 1> BS;
int n;
mt19937 rng(123456789);

int ask(const vector<int>& idx) {
    printf("? %d", (int)idx.size());
    for (int x : idx) printf(" %d", x + 1);
    printf("\n");
    fflush(stdout);
    int r;
    if (scanf("%d", &r) != 1) exit(0);
    if (r == -1) exit(0);
    return r;
}

struct Basis {
    int n;
    vector<BS> row;
    vector<char> has;
    int rank = 0;
    Basis(int n_) : n(n_), row(n_), has(n_, 0) {}
    bool insert(BS v) {
        v[MAXN] = 0;
        while (true) {
            int i = (int)v._Find_first();
            if (i >= n) return false;
            if (has[i]) v ^= row[i];
            else { row[i] = v; has[i] = 1; rank++; return true; }
        }
    }
};

bool gaussSolve(vector<BS>& M, int nv, vector<int>& b) {
    int R = M.size();
    int r = 0;
    for (int col = 0; col < nv; col++) {
        int p = -1;
        for (int i = r; i < R; i++) if (M[i][col]) { p = i; break; }
        if (p < 0) return false;
        swap(M[p], M[r]);
        for (int i = 0; i < R; i++) if (i != r && M[i][col]) M[i] ^= M[r];
        r++;
    }
    b.assign(nv, 0);
    for (int c = 0; c < nv; c++) b[c] = M[c][MAXN] ? 1 : 0;
    return true;
}

int main() {
    if (scanf("%d", &n) != 1) return 0;
    if (n == 2) {
        printf("! 1 2\n");
        fflush(stdout);
        return 0;
    }
    // Level 1: parity classes
    vector<int> Ca, Cb;
    Ca.push_back(0);
    for (int x = 1; x < n; x++) {
        int r = ask({0, x});
        if (r) Ca.push_back(x); else Cb.push_back(x);
    }
    vector<int> res(n, 0);
    for (int x : Ca) res[x] = 1;
    int curMod = 2;

    int A = -1, Bp = -1;
    bool anchorsFound = false;
    auto findHit = [&](vector<int>& C) -> int {
        for (size_t i = 0; i + 1 < C.size(); i++) {
            vector<int> q;
            for (int j = 0; j < n; j++) if (j != C[i]) q.push_back(j);
            if (ask(q)) return C[i];
        }
        return C.back();
    };
    auto findAnchors = [&]() {
        A = findHit(Ca);
        Bp = findHit(Cb);
        anchorsFound = true;
    };
    if (n % 4 == 0) findAnchors();

    vector<int> perm(n);
    iota(perm.begin(), perm.end(), 0);

    for (int L = 2; (1 << L) <= n; L++) {
        int B = 1 << (L - 1), K = 1 << L;
        vector<vector<int>> cls(B);
        for (int x = 0; x < n; x++) cls[res[x]].push_back(x);
        vector<BS> rows;
        Basis bas(n);
        for (int r = 0; r < B; r++) {
            BS v;
            for (int x : cls[r]) v[x] = 1;
            int e = 0;
            for (int val = (r == 0 ? B : r); val <= n; val += B) if ((val / B) & 1) e++;
            v[MAXN] = e & 1;
            rows.push_back(v);
            bas.insert(v);
        }
        if (anchorsFound) {
            BS a; a[A] = 1; a[MAXN] = ((1 >> (L - 1)) & 1);
            rows.push_back(a); bas.insert(a);
            BS b2; b2[Bp] = 1; b2[MAXN] = ((n >> (L - 1)) & 1);
            rows.push_back(b2); bas.insert(b2);
        }
        auto gen = [&](vector<int>& S, int fx) -> bool {
            shuffle(perm.begin(), perm.end(), rng);
            S.clear();
            vector<char> in(n, 0);
            if (fx >= 0) { S.push_back(fx); in[fx] = 1; }
            for (int idx : perm) {
                if ((int)S.size() == K - 1) break;
                if (in[idx]) continue;
                S.push_back(idx); in[idx] = 1;
            }
            long long s = 0;
            for (int x : S) s += res[x];
            int target = (int)(((-s) % B + B) % B);
            vector<int> cands;
            for (int idx : perm) if (!in[idx] && res[idx] == target) cands.push_back(idx);
            if (cands.empty()) return false;
            int pick = cands[rng() % cands.size()];
            S.push_back(pick);
            return true;
        };
        vector<vector<int>> chosen;
        int fails = 0;
        while (bas.rank < n && fails < 3000) {
            int fx = -1;
            if (n - bas.rank <= 300 && rng() % 10 < 7) {
                vector<int> fr;
                for (int i = 0; i < n; i++) if (!bas.has[i]) fr.push_back(i);
                if (!fr.empty()) fx = fr[rng() % fr.size()];
            }
            vector<int> S;
            if (!gen(S, fx)) { fails++; continue; }
            BS vec;
            for (int x : S) vec[x] = 1;
            if (bas.insert(vec)) { chosen.push_back(S); fails = 0; }
            else fails++;
        }
        if (bas.rank < n) break;
        for (auto& S : chosen) {
            long long s = 0;
            for (int x : S) s += res[x];
            int c = (int)(s / B);
            int ans = ask(S);
            BS v;
            for (int x : S) v[x] = 1;
            v[MAXN] = (c + (ans ? 0 : 1)) & 1;
            rows.push_back(v);
        }
        vector<int> b;
        if (!gaussSolve(rows, n, b)) break;
        for (int x = 0; x < n; x++) res[x] += b[x] * B;
        curMod = K;
    }

    // Phase 3
    vector<int> val(n, 0);
    vector<char> known(n, 0), used(n + 2, 0);
    vector<int> Kn;
    auto assignV = [&](int x, int v) {
        val[x] = v; known[x] = 1; used[v] = 1; Kn.push_back(x);
    };
    if (anchorsFound) {
        assignV(A, 1);
        assignV(Bp, n);
    }
    auto cand = [&](int x) {
        vector<int> c;
        int v = res[x] == 0 ? curMod : res[x];
        for (; v <= n; v += curMod) if (!used[v]) c.push_back(v);
        return c;
    };
    auto propagate = [&]() {
        bool ch = true;
        while (ch) {
            ch = false;
            for (int x = 0; x < n; x++) if (!known[x]) {
                auto c = cand(x);
                if (c.size() == 1) { assignV(x, c[0]); ch = true; }
            }
        }
    };
    auto splitTest = [&](int x, vector<int>& c) -> bool {
        int m = c.size();
        int sz = Kn.size();
        int qmax = min(sz + 1, 40);
        struct Opt { int sc, q, rho; };
        vector<Opt> opts;
        for (int q = 2; q <= qmax; q++) {
            vector<int> cnt(q, 0);
            for (int v : c) cnt[v % q]++;
            for (int rho = 0; rho < q; rho++) {
                int y = cnt[rho];
                if (y >= 1 && y <= m - 1) opts.push_back({min(y, m - y), q, rho});
            }
        }
        sort(opts.begin(), opts.end(), [](const Opt& a, const Opt& b) {
            if (a.sc != b.sc) return a.sc > b.sc;
            return a.q < b.q;
        });
        int tried = 0;
        for (auto& o : opts) {
            if (tried++ > 80) break;
            int q = o.q, rho = o.rho;
            vector<int> idx(sz);
            iota(idx.begin(), idx.end(), 0);
            bool ok = false;
            vector<int> Kset;
            for (int att = 0; att < 20 && !ok; att++) {
                for (int i = 0; i < q - 2; i++) {
                    int j = i + rng() % (sz - i);
                    swap(idx[i], idx[j]);
                }
                long long s = 0;
                for (int i = 0; i < q - 2; i++) s += val[Kn[idx[i]]];
                long long t = rho + s;
                int need = (int)(((-t) % q + q) % q);
                for (int j = q - 2; j < sz; j++) {
                    if (val[Kn[idx[j]]] % q == need) {
                        ok = true;
                        Kset.clear();
                        for (int i = 0; i < q - 2; i++) Kset.push_back(Kn[idx[i]]);
                        Kset.push_back(Kn[idx[j]]);
                        break;
                    }
                }
            }
            if (!ok) continue;
            vector<int> Q;
            Q.push_back(x);
            for (int y : Kset) Q.push_back(y);
            int ans = ask(Q);
            vector<int> nc;
            for (int v : c) {
                bool yes = (v % q == rho);
                if (yes == (ans == 1)) nc.push_back(v);
            }
            c = nc;
            return true;
        }
        return false;
    };
    auto addAnchors = [&]() {
        findAnchors();
        if (!known[A]) assignV(A, 1);
        if (!known[Bp]) assignV(Bp, n);
    };

    propagate();
    while (true) {
        int bx = -1;
        size_t bs = SIZE_MAX;
        for (int x = 0; x < n; x++) if (!known[x]) {
            auto c = cand(x);
            if (c.size() < bs) { bs = c.size(); bx = x; }
        }
        if (bx < 0) break;
        auto c = cand(bx);
        while (c.size() > 1) {
            if (!splitTest(bx, c)) {
                if (!anchorsFound) {
                    addAnchors();
                    if (known[bx]) break;
                    c = cand(bx);
                    continue;
                } else break;
            }
        }
        if (known[bx]) { propagate(); continue; }
        if (c.empty()) {
            for (int v = 1; v <= n; v++) if (!used[v]) { c.push_back(v); break; }
        }
        assignV(bx, c[0]);
        propagate();
    }

    if (val[0] > n / 2) {
        for (int x = 0; x < n; x++) val[x] = n + 1 - val[x];
    }
    printf("!");
    for (int x = 0; x < n; x++) printf(" %d", val[x]);
    printf("\n");
    fflush(stdout);
    return 0;
}
