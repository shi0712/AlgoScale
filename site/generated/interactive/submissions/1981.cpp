#include <bits/stdc++.h>
using namespace std;

static int n;

static int ask(int i, int j, int k) {
    printf("? %d %d %d\n", i, j, k);
    fflush(stdout);
    int r;
    if (scanf("%d", &r) != 1) exit(0);
    if (r < 0) exit(0);
    return r;
}

static void answer(int i, int j) {
    printf("! %d %d\n", i, j);
    fflush(stdout);
    exit(0);
}

/* ---------- deterministic solution: probe with pair (A,B), then tournament ---------- */
static void solveDet(int A, int B, vector<int> &R) {
    vector<int> others;
    for (int i = 1; i <= n; i++) if (i != A && i != B) others.push_back(i);
    for (int i : others) if (R[i] < 0) R[i] = ask(A, B, i);
    int mx = INT_MIN, mn = INT_MAX;
    for (int i : others) { mx = max(mx, R[i]); mn = min(mn, R[i]); }

    if (mx == mn) {
        int m = (int)others.size();
        for (int t = 0; t < m; t += 2) {
            int x = others[t];
            int y = (t + 1 < m) ? others[t + 1] : others[0];
            int q = ask(A, x, y);
            if (q > mx) answer(x, y);
        }
        answer(A, B);
    }

    int e = others[0];
    for (int i : others) if (R[i] == mx) { e = i; break; }

    int best = mx, bp1 = A, bp2 = B;   // free pair (A,B) has value diam(e,A,B)=R[e]=mx
    vector<int> items;
    for (int i : others) if (i != e) items.push_back(i);
    int m = (int)items.size();
    for (int t = 0; t < m; t += 2) {
        int x = items[t];
        int y = (t + 1 < m) ? items[t + 1] : A;
        int q = ask(e, x, y);
        if (q > best) { best = q; bp1 = x; bp2 = y; }
    }
    int z = -1;
    for (int i = 1; i <= n; i++) if (i != e && i != bp1 && i != bp2) { z = i; break; }
    int q = ask(e, bp1, z);
    if (q == best) answer(e, bp1);
    else answer(e, bp2);
}

/* Try to build a valid diameter pair of the 5-element init set.
   Returns true on success; R is filled for the probe-pair elements. */
static bool tryInit(const vector<int> &init, int &u, int &v, int &D) {
    int A = init[0], B = init[1];
    int r[3];
    for (int t = 0; t < 3; t++) r[t] = ask(A, B, init[2 + t]);
    int mx = max(r[0], max(r[1], r[2]));
    int mn = min(r[0], min(r[1], r[2]));

    if (mx == 0) { u = A; v = B; D = 0; return true; }
    if (mx == mn) {
        int R0 = mx;
        int o0 = init[2], o1 = init[3], o2 = init[4];
        int t = ask(o0, o1, o2);
        if (t > R0) {
            int cand[3][2] = {{o0,o1},{o0,o2},{o1,o2}};
            for (int cc = 0; cc < 3; cc++) {
                int x = cand[cc][0], y = cand[cc][1];
                if (ask(A, x, y) > R0) { u = x; v = y; D = t; return true; }
            }
            return false;
        } else if (t > 0) {
            u = A; v = B; D = R0; return true;
        } else {
            int q1 = ask(A, o0, o1);
            int q2 = ask(B, o0, o1);
            if ((long long)q1 + q2 == (long long)R0) { u = A; v = B; D = R0; return true; }
            if (q1 == R0) { u = A; v = o0; D = R0; return true; }
            if (q2 == R0) { u = B; v = o0; D = R0; return true; }
            return false;
        }
    }
    // mx > mn : e = argmax is an extreme of the init set
    int e = init[2];
    for (int t = 0; t < 3; t++) if (r[t] == mx) { e = init[2 + t]; break; }
    vector<int> oth;
    oth.push_back(A); oth.push_back(B);
    for (int t = 0; t < 3; t++) if (init[2 + t] != e) oth.push_back(init[2 + t]);
    int V1 = ask(e, oth[0], oth[1]);
    int V2 = ask(e, oth[2], oth[3]);
    if (V1 > V2) {
        int q = ask(e, oth[0], oth[2]);
        u = e; v = (q == V1) ? oth[0] : oth[1]; D = V1; return true;
    } else if (V2 > V1) {
        int q = ask(e, oth[2], oth[0]);
        u = e; v = (q == V2) ? oth[2] : oth[3]; D = V2; return true;
    } else {
        int V = V1;
        if (V == 0) { u = e; v = oth[0]; D = 0; return true; }
        int q = ask(e, oth[0], oth[2]);
        if (q < V) { u = e; v = oth[1]; D = V; return true; }
        int q2 = ask(e, oth[0], oth[3]);
        if (q2 < V) { u = e; v = oth[1]; D = V; return true; }
        return false;
    }
}

int main() {
    if (scanf("%d", &n) != 1) return 0;

    if (n == 4) {
        int A = ask(1, 2, 3);
        int B = ask(1, 2, 4);
        int C = ask(1, 3, 4);
        int i, j;
        if (A == B && B == C) { i = 1; j = 2; }
        else if (A == B) { if (B < C) { i = 3; j = 4; } else { i = 1; j = 2; } }
        else if (A == C) { if (A < B) { i = 2; j = 4; } else { i = 1; j = 3; } }
        else if (B == C) { if (B < A) { i = 2; j = 3; } else { i = 1; j = 4; } }
        else if (A < B && B < C) { i = 3; j = 4; }
        else if (A < C && C < B) { i = 2; j = 4; }
        else if (B < A && A < C) { i = 3; j = 4; }
        else if (B < C && C < A) { i = 2; j = 3; }
        else if (C < A && A < B) { i = 2; j = 4; }
        else { i = 2; j = 3; }
        answer(i, j);
    }

    // random order
    vector<int> perm(n);
    for (int i = 0; i < n; i++) perm[i] = i + 1;
    {
        uint64_t seed = chrono::steady_clock::now().time_since_epoch().count();
        seed ^= (uint64_t)(uintptr_t)&perm;
        seed *= 0x9E3779B97F4A7C15ULL;
        mt19937_64 rng(seed);
        shuffle(perm.begin(), perm.end(), rng);
    }

    int A0 = perm[0], B0 = perm[1];
    vector<int> R(n + 1, -1);

    const int THRESH = 20;
    if (n >= THRESH) {
        // first attempt uses perm[0..4] with probe pair (perm[0],perm[1]); R will be kept
        // for the deterministic fallback.
        bool got = false;
        int u = -1, v = -1, D = -1;
        int usedInit[5];
        int numAttempts = (n >= 50) ? 5 : (n >= 30 ? 2 : 1);
        int base = 0;
        for (int att = 0; att < numAttempts && !got; att++) {
            if (base + 5 > n) break;
            vector<int> init;
            for (int t = 0; t < 5; t++) init.push_back(perm[base + t]);
            if (att == 0) {
                for (int t = 2; t < 5; t++) R[init[t]] = ask(init[0], init[1], init[t]);
            }
            if (tryInit(init, u, v, D)) {
                for (int t = 0; t < 5; t++) usedInit[t] = init[t];
                got = true;
            }
            base += 5;
        }
        if (got) {
            for (int t = 0; t < n; t++) {
                int x = perm[t];
                bool inInit = false;
                for (int q = 0; q < 5; q++) if (usedInit[q] == x) { inInit = true; break; }
                if (inInit) continue;
                int r = ask(u, v, x);
                if (r == D) continue;
                int z = -1;
                for (int q = 0; q < 5; q++)
                    if (usedInit[q] != u && usedInit[q] != v) { z = usedInit[q]; break; }
                int qu = ask(x, u, z);
                int qv = ask(x, v, z);
                int nu, nv;
                if (qu == r && qv == r) { nu = x; nv = z; }
                else if (qu == r) { nu = x; nv = u; }
                else { nu = x; nv = v; }
                u = nu; v = nv; D = r;
            }
            answer(u, v);
        }
    }

    solveDet(A0, B0, R);
}
