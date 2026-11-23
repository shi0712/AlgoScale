#include <bits/stdc++.h>
using namespace std;

int n;
vector<int> st;      // 0 pending, 1 known, 2 dependent
vector<int> kval;    // known bit (1 => +1, 0 => -1)
vector<int> depX, depB0, depB1;
deque<int> pend;
vector<int> grp;

long long ask(const vector<pair<int,int>>& items) {
    long long k = 0;
    for (auto& p : items) k += p.second;
    string s = "? 1 " + to_string(k);
    for (auto& p : items)
        for (int i = 0; i < p.second; i++) s += " " + to_string(p.first);
    s += "\n";
    fputs(s.c_str(), stdout);
    fflush(stdout);
    long long r;
    if (scanf("%lld", &r) != 1) exit(0);
    return r;
}
void toggle(int u) {
    printf("? 2 %d\n", u);
    fflush(stdout);
}

int getBit(int u) {
    if (st[u] == 1) return kval[u];
    if (st[u] == 2) {
        int x = getBit(depX[u]);
        return x ? depB1[u] : depB0[u];
    }
    return -1;  // should not happen
}

// choose vars & weights for capacity K from pending queue
struct Plan {
    vector<int> vars;  // low vars then optional z
    vector<int> w;
    int p;             // number of power vars
    int e;             // extra weight (0 if none)
    long long W;
};
Plan makePlan(long long K) {
    Plan pl;
    pl.e = 0;
    int M = pend.size();
    int p = 0;
    while (p < 30 && ((1LL << (p + 1)) - 1) <= K) p++;
    if (M <= p) {
        p = M;
    }
    pl.p = p;
    for (int i = 0; i < p; i++) {
        pl.vars.push_back(pend.front());
        pend.pop_front();
        pl.w.push_back(1 << i);
    }
    long long used = (1LL << p) - 1;
    if (!pend.empty() && K - used >= 1) {
        long long e = K - used;
        if (e > (1LL << p)) e = (1LL << p);
        pl.e = (int)e;
        pl.vars.push_back(pend.front());
        pend.pop_front();
        pl.w.push_back(pl.e);
        used += e;
    }
    pl.W = used;
    return pl;
}
// decode S for plan
void decode(const Plan& pl, long long S) {
    int p = pl.p;
    if (pl.e == 0) {
        for (int i = 0; i < p; i++) { st[pl.vars[i]] = 1; kval[pl.vars[i]] = (S >> i) & 1; }
        return;
    }
    int z = pl.vars[p];
    long long e = pl.e;
    long long hi = (1LL << p) - 1;
    if (S < e || S > hi) {
        int zb = (S < e) ? 0 : 1;
        long long low = S - zb * e;
        st[z] = 1; kval[z] = zb;
        for (int i = 0; i < p; i++) { st[pl.vars[i]] = 1; kval[pl.vars[i]] = (low >> i) & 1; }
    } else {
        long long l0 = S, l1 = S - e;
        for (int i = 0; i < p; i++) {
            int u = pl.vars[i];
            int b0 = (l0 >> i) & 1, b1 = (l1 >> i) & 1;
            if (b0 == b1) { st[u] = 1; kval[u] = b0; }
            else { st[u] = 2; depX[u] = z; depB0[u] = b0; depB1[u] = b1; }
        }
        pend.push_back(z);
    }
}

int main() {
    if (scanf("%d", &n) != 1) return 0;
    for (int i = 0; i < n - 1; i++) { int a, b; if (scanf("%d %d", &a, &b) != 2) return 0; }
    st.assign(n + 1, 0); kval.assign(n + 1, 0);
    depX.assign(n + 1, 0); depB0.assign(n + 1, 0); depB1.assign(n + 1, 0);
    vector<int> leaves;
    for (int i = 2; i <= n; i++) leaves.push_back(i);
    mt19937 rng((unsigned)chrono::steady_clock::now().time_since_epoch().count() ^ 0x9e3779b9u);
    shuffle(leaves.begin(), leaves.end(), rng);
    for (int u : leaves) pend.push_back(u);

    // First query: node 1 with weight w1 >= 1, T = w1 + W odd, T <= n.
    long long K1 = (n % 2 == 1) ? n - 1 : n - 2;
    Plan pl;
    pl.p = 0; pl.e = 0; pl.W = 0;
    if (K1 >= 1) pl = makePlan(K1);
    long long W = pl.W;
    int w1 = (W % 2 == 0) ? 1 : 2;
    vector<pair<int,int>> items;
    items.push_back({1, w1});
    for (size_t i = 0; i < pl.vars.size(); i++) items.push_back({pl.vars[i], pl.w[i]});
    long long T = w1 + W;
    long long ans = ask(items);
    int a = 99;
    long long S = 0;
    for (int c = -2; c <= 2; c++) {
        long long x = ans + W - T * c;
        if (x < 0 || (x & 1)) continue;
        if (x / 2 > W) continue;
        a = c; S = x / 2;
        break;
    }
    if (a == 99) exit(0);
    if (!pl.vars.empty()) decode(pl, S);

    int finalV1 = 0;
    int mode;  // 0: case A, 1: a==0, 2: a==+-2
    if (a % 2 != 0) { mode = 0; finalV1 = a; }
    else if (a == 0) mode = 1;
    else mode = 2;

    int acur = a;
    if (mode == 2) {
        int safeBit = (a > 0) ? 1 : 0;
        // evaluate first-query vars under hypotheses
        vector<int> unsafe, amb;
        int z = -1;
        if (pl.e != 0 && st[pl.vars[pl.p]] == 0) z = pl.vars[pl.p];
        for (int u : pl.vars) {
            int b0, b1;
            if (u == z) { b0 = 0; b1 = 1; }
            else if (st[u] == 1) { b0 = b1 = kval[u]; }
            else { b0 = depB0[u]; b1 = depB1[u]; }
            if (b0 == safeBit && b1 == safeBit) { st[u] = 1; kval[u] = safeBit; }
            else if (b0 != b1) { st[u] = 0; amb.push_back(u); }
            else unsafe.push_back(u);
        }
        // remove z from pending (it was pushed back), rebuild pending
        if (z != -1) {
            deque<int> np;
            for (int u : pend) if (u != z) np.push_back(u);
            pend = np;
        }
        for (int u : amb) pend.push_back(u);
        if (pend.size() + unsafe.size() == 1) {
            int u = pend.empty() ? unsafe[0] : pend.front(); pend.clear();
            st[u] = 1; kval[u] = safeBit;
            finalV1 = a / 2;
        } else {
            toggle(1);
            finalV1 = -a / 2;
            acur = 0;
            if (unsafe.size() == 1 || n < 14) { for (int u : unsafe) { st[u] = 0; pend.push_back(u); } }
            else if (!unsafe.empty()) {
                grp = unsafe;
                for (int u : grp) { st[u] = 1; kval[u] = 1 - safeBit; }
            }
        }
    }

    while (!pend.empty() || !grp.empty()) {
        long long d = grp.size();
        bool useG = false;
        Plan q;
        if (d > 0 && d <= n) {
            deque<int> save = pend;
            long long K2 = (n - d) / 2;
            if (K2 >= 1 || pend.empty()) {
                if (pend.empty()) { q.p = 0; q.e = 0; q.W = 0; }
                else q = makePlan(K2);
                if (pend.empty()) useG = true;
                else { pend = save; }
            }
        }
        vector<pair<int,int>> it;
        long long Wtot;
        if (useG) {
            for (size_t i = 0; i < q.vars.size(); i++) it.push_back({q.vars[i], 2 * q.w[i]});
            for (int u : grp) it.push_back({u, 1});
            Wtot = 2 * q.W + d;
        } else {
            q = makePlan(n);
            for (size_t i = 0; i < q.vars.size(); i++) it.push_back({q.vars[i], q.w[i]});
            Wtot = q.W;
        }
        long long r = ask(it);
        // r = W*acur - W + 2S
        long long S2 = (r - Wtot * acur + Wtot) / 2;
        if (useG) {
            long long g, Sreg;
            if (a == 2) { g = S2 & 1; Sreg = (S2 - g) / 2; }
            else { long long t = S2 - d; g = ((t % 2) + 2) % 2; Sreg = (t + g) / 2; }
            if (!q.vars.empty()) decode(q, Sreg);
            if (g == 1) for (int u : grp) { st[u] = 0; pend.push_back(u); }
            grp.clear();
        } else decode(q, S2);
    }

    vector<int> val(n + 1);
    for (int u = 2; u <= n; u++) val[u] = getBit(u) ? 1 : -1;
    if (mode == 1) {
        bool allEq = true;
        for (int u = 3; u <= n; u++) if (val[u] != val[2]) allEq = false;
        if (allEq) finalV1 = -val[2];
        else {
            toggle(1);
            long long r = ask({{1, 1}});
            finalV1 = (int)(r / 2);
        }
    }
    val[1] = finalV1;
    string s = "!";
    for (int u = 1; u <= n; u++) s += " " + to_string(val[u]);
    s += "\n";
    fputs(s.c_str(), stdout);
    fflush(stdout);
    return 0;
}
