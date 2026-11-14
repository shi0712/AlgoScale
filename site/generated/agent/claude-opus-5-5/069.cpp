#include <bits/stdc++.h>
using namespace std;
typedef long long ll;

// Gap node: fresh (list of positions) or composite (left gap, eliminated block value, right gap)
struct Gap {
    int type; // 0 fresh, 1 composite
    vector<int> pos;
    int L, R; ll p;
};
struct Blk { int l, r; ll v; int leftGap; };

template <class Q>
vector<ll> solveCase(int n, Q query) {
    vector<ll> a(n + 2, 0);
    vector<int> st(n + 2, 0); // 0 unknown/skipped, 1 positive, 2 nonpositive
    int i = 1;
    while (i <= n) {
        ll x = query(i, i);
        if (x <= 0) { st[i] = 2; i++; continue; }
        st[i] = 1; a[i] = x;
        // positive at i
        while (true) {
            if (i + 2 <= n) {
                ll y = query(i + 2, i + 2);
                if (y > 0) { st[i + 2] = 1; a[i + 2] = y; st[i + 1] = 0; i = i + 2; continue; }
                st[i + 2] = 2;
                ll z = query(i + 1, i + 1);
                if (z > 0) { st[i + 1] = 1; a[i + 1] = z; } else st[i + 1] = 2;
                i = i + 3;
                break;
            } else if (i + 1 == n) {
                ll z = query(n, n);
                if (z > 0) { st[n] = 1; a[n] = z; } else st[n] = 2;
                i = n + 1;
                break;
            } else { i = n + 1; break; }
        }
    }
    vector<ll> b(n + 1, 0);
    vector<Gap> gaps;
    vector<ll> gv; vector<char> gset;
    auto newGap = [&](Gap g) { gaps.push_back(g); gv.push_back(0); gset.push_back(0); return (int)gaps.size() - 1; };
    vector<Blk> stk;
    int pos = 1;
    // leading
    Gap lead; lead.type = 0;
    while (pos <= n && st[pos] != 1) { lead.pos.push_back(pos); pos++; }
    int leadId = newGap(lead);
    if (pos > n) return b;
    const ll BIG = -(ll)1e13;
    // read first block
    auto readBlock = [&](int &p) {
        Blk B; B.l = p; B.v = 0;
        while (p <= n && st[p] == 1) { B.v += a[p]; b[p] = a[p]; p++; }
        B.r = p - 1; return B;
    };
    {
        Blk B = readBlock(pos); B.leftGap = leadId; stk.push_back(B);
    }
    vector<int> leadingRoots = {leadId};
    while (pos <= n) {
        Gap g; g.type = 0;
        while (pos <= n && st[pos] != 1) { g.pos.push_back(pos); pos++; }
        if (pos > n) { for (int x : g.pos) b[x] = 0; break; }
        int curGap = newGap(g);
        Blk cur = readBlock(pos);
        while (true) {
            Blk top = stk.back();
            ll ans = query(top.l, cur.r);
            if (ans > max(top.v, cur.v)) {
                ll gval = ans - top.v - cur.v;
                gv[curGap] = gval; gset[curGap] = 1;
                Blk M; M.l = top.l; M.r = cur.r; M.v = top.v + gval + cur.v; M.leftGap = top.leftGap;
                stk.pop_back();
                cur = M;
                if (stk.empty()) { stk.push_back(cur); break; }
                curGap = cur.leftGap;
            } else if (top.v <= cur.v) {
                Gap c; c.type = 1; c.L = top.leftGap; c.R = curGap; c.p = top.v;
                int cid = newGap(c);
                stk.pop_back();
                if (stk.empty()) {
                    gv[cid] = BIG; gset[cid] = 1;
                    cur.leftGap = cid; stk.push_back(cur); break;
                }
                curGap = cid;
            } else {
                cur.leftGap = curGap; stk.push_back(cur); break;
            }
        }
    }
    // final stack gaps
    for (size_t k = 0; k < stk.size(); k++) {
        int gid = stk[k].leftGap;
        if (k == 0) { if (!gset[gid]) { gv[gid] = BIG; gset[gid] = 1; } }
        else { gv[gid] = -stk[k].v; gset[gid] = 1; }
    }
    // distribute: process roots (set) -- use explicit stack
    vector<char> done(gaps.size(), 0);
    for (int id = (int)gaps.size() - 1; id >= 0; id--) {
        if (!gset[id] || done[id]) continue;
        vector<int> s = {id};
        while (!s.empty()) {
            int u = s.back(); s.pop_back();
            done[u] = 1;
            Gap &G = gaps[u];
            if (G.type == 0) {
                for (size_t t = 0; t < G.pos.size(); t++) b[G.pos[t]] = (t == 0 ? gv[u] : 0);
            } else {
                gv[G.L] = -G.p; gset[G.L] = 1;
                gv[G.R] = gv[u]; gset[G.R] = 1;
                s.push_back(G.L); s.push_back(G.R);
            }
        }
    }
    return b;
}

#ifndef LOCAL_TEST
int main() {
    int T;
    if (scanf("%d", &T) != 1) return 0;
    bool dead = false;
    while (T-- && !dead) {
        int n;
        if (scanf("%d", &n) != 1) return 0;
        auto q = [&](int l, int r) -> ll {
            printf("? %d %d\n", l, r); fflush(stdout);
            ll x; if (scanf("%lld", &x) != 1 || x == -1) exit(0);
            return x;
        };
        vector<ll> b = solveCase(n, q);
        printf("!");
        for (int i = 1; i <= n; i++) printf(" %lld", b[i]);
        printf("\n"); fflush(stdout);
    }
}
#endif
