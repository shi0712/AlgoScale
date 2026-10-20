#include <bits/stdc++.h>
using namespace std;
static const double INF = numeric_limits<double>::infinity();

struct FastIn {
    static const int S = 1 << 16;
    char buf[S]; int n = 0, p = 0;
    inline char gc() {
        if (p == n) { n = (int)fread(buf, 1, S, stdin); p = 0; if (n <= 0) return 0; }
        return buf[p++];
    }
    template <class T> inline bool readInt(T &out) {
        char c; T sign = 1, num = 0;
        c = gc(); if (!c) return false;
        while (c != '-' && (c < '0' || c > '9')) { c = gc(); if (!c) return false; }
        if (c == '-') { sign = -1; c = gc(); }
        for (; c >= '0' && c <= '9'; c = gc()) num = num * 10 + (c - '0');
        out = num * sign; return true;
    }
};

struct Ev { double g, dt, F; int t, q; };

struct alignas(64) Nd {
    double D, s, u;
    double mcVal;
    int mcCh, cntS, par, pos;
    double pad[2];
};

vector<Ev> ev;
vector<Nd> nd;
vector<int> deg, head, adj, childStart, childList, ptr, ea, eb;
vector<double> fminDeg, A, B;

static void processTree(int n, int s0, int tt) {
    deg.assign(n + 1, 0);
    for (int i = 0; i + 1 < n; i++) { deg[ea[i]]++; deg[eb[i]]++; }
    head.assign(n + 2, 0);
    for (int i = 1; i <= n; i++) head[i + 1] = head[i] + deg[i];
    adj.assign(2 * (n - 1) > 1 ? 2 * (n - 1) : 2, 0);
    {
        vector<int> ps(head.begin(), head.end());
        for (int i = 0; i + 1 < n; i++) {
            int x = ea[i], y = eb[i];
            adj[ps[x]++] = y; adj[ps[y]++] = x;
        }
    }
    nd.assign(n + 1, Nd());
    {
        vector<int> order; order.assign(1, 1); order.reserve(n);
        for (size_t qi = 0; qi < order.size(); qi++) {
            int x = order[qi]; int px = nd[x].par;
            for (int j = head[x]; j < head[x + 1]; j++) {
                int y = adj[j];
                if (y != px) { nd[y].par = x; order.push_back(y); }
            }
        }
    }
    childStart.assign(n + 2, 0);
    for (int v = 2; v <= n; v++) childStart[nd[v].par + 1]++;
    for (int v = 1; v <= n; v++) childStart[v + 1] += childStart[v];
    childList.assign(n - 1 > 0 ? n - 1 : 1, 0);
    {
        vector<int> ps(childStart.begin(), childStart.end());
        for (int v = 2; v <= n; v++) childList[ps[nd[v].par]++] = v;
        for (int v = 1; v <= n; v++)
            sort(childList.begin() + childStart[v], childList.begin() + childStart[v + 1],
                 [&](int x, int y) { return deg[x] < deg[y]; });
    }
    int L = 0;
    { int a2 = s0; while (a2) { L++; a2 = nd[a2].par; } }
    { int i = L, a2 = s0; while (a2) { nd[a2].pos = i--; a2 = nd[a2].par; } }
    fminDeg.assign(n + 1, INF);
    A.assign(L + 2, 0.0); B.assign(L + 2, 0.0);
    ptr.assign(n + 1, 0);

    nd[1].D = 1.0; nd[1].s = 1.0; nd[1].u = 0.0;
    nd[1].mcVal = INF; nd[1].mcCh = 0; nd[1].cntS = 0;
    fminDeg[1] = (childStart[2] > childStart[1]) ? (double)deg[childList[childStart[1]]] : INF;

    int LS = 0, dstart = 1;
    double tStore = 0.0, gPrev = INF;

    for (int step = 1; step < n; step++) {
        double curM = min(nd[1].mcVal, fminDeg[1]);
        int x = dstart, v;
        while (fminDeg[x] > nd[x].mcVal) { int y = nd[x].mcCh; if (!y) break; x = y; }
        v = (ptr[x] < childStart[x + 1] - childStart[x]) ? childList[childStart[x] + ptr[x]] : 0;
        if (!v) break;
        double g = 1.0 / curM;
        if (g > gPrev) g = gPrev;
        gPrev = g;
        Nd &nv = nd[v];
        nv.D = (double)deg[v];
        nv.s = 1.0 / nv.D;
        nv.u = 1.0;
        nv.mcVal = INF; nv.mcCh = 0; nv.cntS = 0;
        {
            int cs = childStart[v], len = childStart[v + 1] - cs;
            fminDeg[v] = (len > 0) ? (double)deg[childList[cs]] : INF;
        }
        int p = nv.par;
        Nd &np = nd[p];
        ptr[p]++;
        {
            int cs = childStart[p], len = childStart[p + 1] - cs, p0 = ptr[p];
            fminDeg[p] = (p0 < len) ? (double)deg[childList[cs + p0]] : INF;
        }
        np.cntS++;
        int iv = nv.pos;
        if (iv) { A[iv] = nv.s; B[iv] = nv.u; LS = iv; }

        double ds = nv.s, du = nv.u;
        double Dc = nv.D, uc = nv.u, Mc = fminDeg[v];
        int c = v, a = p, dnew = p;
        int ipa = np.pos;
        double AiC = 0.0, BiC = 0.0;
        bool cPath = (iv != 0);
        if (cPath) { AiC = nv.s; BiC = nv.u; }
        else if (ipa != 0 && ipa < LS) { AiC = A[ipa + 1]; BiC = B[ipa + 1]; }
        for (;;) {
            Nd &na = nd[a];
            double keyc = Dc * (uc + Mc);
            double fd = fminDeg[a];
            if (a != 1) {
                double Do = na.D, Dn = Do - ds;
                double sn = 1.0 / Dn;
                double un = sn * (na.u * Do + du);
                double nds = sn - na.s, ndu = un - na.u;
                na.D = Dn; na.s = sn; na.u = un;
                double mcv;
                if (na.cntS == 1) { mcv = keyc; na.mcVal = keyc; na.mcCh = c; }
                else if (c == na.mcCh) {
                    double best = INF; int bc = 0;
                    for (int j = childStart[a]; j < childStart[a + 1]; j++) {
                        int y = childList[j];
                        Nd &ny = nd[y];
                        if (ny.D == 0.0) continue;
                        double vv = ny.D * (ny.u + min(ny.mcVal, fminDeg[y]));
                        if (vv < best) { best = vv; bc = y; }
                    }
                    mcv = best; na.mcVal = best; na.mcCh = bc;
                } else if (keyc < na.mcVal) { mcv = keyc; na.mcVal = keyc; na.mcCh = c; }
                else mcv = na.mcVal;
                double Mm = min(mcv, fd);
                int ia = na.pos;
                if (ia) {
                    if (!cPath) {
                        if (ia < LS) { AiC = A[ia + 1]; BiC = B[ia + 1]; }
                        else { AiC = 1.0; BiC = 0.0; }
                    }
                    double nA = sn * AiC, nB = un * AiC + BiC;
                    A[ia] = nA; B[ia] = nB;
                    AiC = nA; BiC = nB; cPath = true;
                } else cPath = false;
                if (!(mcv < fd && na.mcCh == c)) dnew = a;
                c = a; Dc = Dn; uc = un; Mc = Mm;
                ds = nds; du = ndu;
                a = na.par;
            } else {
                double mcv;
                if (na.cntS == 1) { mcv = keyc; na.mcVal = keyc; na.mcCh = c; }
                else if (c == na.mcCh) {
                    double best = INF; int bc = 0;
                    for (int j = childStart[1]; j < childStart[2]; j++) {
                        int y = childList[j];
                        Nd &ny = nd[y];
                        if (ny.D == 0.0) continue;
                        double vv = ny.D * (ny.u + min(ny.mcVal, fminDeg[y]));
                        if (vv < best) { best = vv; bc = y; }
                    }
                    mcv = best; na.mcVal = best; na.mcCh = bc;
                } else if (keyc < na.mcVal) { mcv = keyc; na.mcVal = keyc; na.mcCh = c; }
                else mcv = na.mcVal;
                if (!(mcv < fd && na.mcCh == c)) dnew = a;
                break;
            }
        }
        dstart = dnew;
        double pv2 = 0.0, tv2 = 0.0;
        if (LS == L) { pv2 = A[2]; tv2 = B[2]; }
        double dt = tv2 - tStore; tStore = tv2;
        double F = 1.0 - pv2;
        if (F < 0.0) F = 0.0;
        ev.push_back({g, dt, F, tt, step});
    }
}

int main() {
    FastIn in;
    int k;
    if (!in.readInt(k)) return 0;
    ev.reserve(1 << 12);
    bool anyRoot = false;
    for (int idx = 0; idx < k; idx++) {
        int n, s0;
        in.readInt(n); in.readInt(s0);
        ea.assign(n - 1, 0); eb.assign(n - 1, 0);
        for (int j = 0; j + 1 < n; j++) in.readInt(ea[j]), in.readInt(eb[j]);
        if (s0 == 1) { anyRoot = true; continue; }
        processTree(n, s0, idx);
    }
    if (anyRoot) { printf("0.000000000000\n"); return 0; }
    sort(ev.begin(), ev.end(), [](const Ev &X, const Ev &Y) {
        if (X.g != Y.g) return X.g > Y.g;
        if (X.t != Y.t) return X.t < Y.t;
        return X.q < Y.q;
    });
    vector<double> Fcur(k, 1.0);
    int zeros = 0; double tnz = 1.0, V = 0.0;
    for (size_t i = 0; i < ev.size(); i++) {
        int ti = ev[i].t;
        double Fi = Fcur[ti], other;
        if (Fi != 0.0) other = (zeros >= 1) ? 0.0 : tnz / Fi;
        else other = (zeros >= 2) ? 0.0 : tnz;
        V += ev[i].dt * other;
        double nf = ev[i].F;
        if (Fi != 0.0) tnz /= Fi; else zeros--;
        if (nf != 0.0) tnz *= nf; else zeros++;
        Fcur[ti] = nf;
    }
    printf("%.12f\n", V);
    return 0;
}
