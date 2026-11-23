#include <bits/stdc++.h>
using namespace std;
typedef long long ll;

int n;
vector<ll> a, b;
struct Block { int l, r; ll S; };
struct Gap { int type; int l, r; int gl, q, gr; };
vector<Block> blocks;
vector<Gap> gaps;

ll query(int l, int r) {
    printf("? %d %d\n", l, r);
    fflush(stdout);
    ll x;
    if (scanf("%lld", &x) != 1) exit(0);
    if (x == -1) exit(0);
    return x;
}

void assignGap(int g, ll v) {
    // iterative along right spine
    while (true) {
        Gap &G = gaps[g];
        if (G.type == 0) {
            if (G.l <= G.r) {
                b[G.l] = v;
                for (int i = G.l + 1; i <= G.r; i++) b[i] = 0;
            }
            return;
        }
        int gl = G.gl, q = G.q, gr = G.gr;
        assignGap(gl, -blocks[q].S);
        g = gr;
    }
}

int main() {
    if (scanf("%d", &n) != 1) return 0;
    a.assign(n + 2, 0);
    b.assign(n + 2, 0);
    for (int i = 1; i <= n; i++) a[i] = query(i, i);
    for (int i = 1; i <= n; i++) b[i] = a[i] > 0 ? a[i] : 0;
    // find positive runs
    vector<pair<int,int>> runs;
    for (int i = 1; i <= n;) {
        if (a[i] > 0) {
            int j = i;
            while (j + 1 <= n && a[j + 1] > 0) j++;
            runs.push_back({i, j});
            i = j + 1;
        } else i++;
    }
    vector<pair<int,int>> st; // (blockIdx, leftGapIdx)
    int prevEnd = 0;
    for (auto [l, r] : runs) {
        ll S = 0;
        for (int i = l; i <= r; i++) S += a[i];
        blocks.push_back({l, r, S});
        int cur = blocks.size() - 1;
        gaps.push_back({0, prevEnd + 1, l - 1, -1, -1, -1});
        int curGap = gaps.size() - 1;
        prevEnd = r;
        while (true) {
            if (st.empty()) { st.push_back({cur, curGap}); break; }
            auto [T, TG] = st.back();
            ll ans = query(blocks[T].l, blocks[cur].r);
            ll TS = blocks[T].S, CS = blocks[cur].S;
            if (ans > max(TS, CS)) {
                ll X = ans - TS - CS;
                assignGap(curGap, X);
                blocks.push_back({blocks[T].l, blocks[cur].r, ans});
                cur = blocks.size() - 1;
                curGap = TG;
                st.pop_back();
            } else if (TS <= CS) {
                gaps.push_back({1, 0, -1, TG, T, curGap});
                curGap = gaps.size() - 1;
                st.pop_back();
            } else {
                st.push_back({cur, curGap});
                break;
            }
        }
    }
    for (auto [blk, g] : st) {
        assignGap(g, -blocks[blk].S);
    }
    // trailing nonpositives already 0
    printf("!");
    for (int i = 1; i <= n; i++) printf(" %lld", b[i]);
    printf("\n");
    fflush(stdout);
    return 0;
}
