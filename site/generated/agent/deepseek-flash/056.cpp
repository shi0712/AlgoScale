#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <vector>
#include <queue>
using namespace std;
typedef long long ll;
typedef __int128 lll;

static char *inbuf; static size_t inpos = 0, inlen = 0;
static inline int gc() { return (inpos >= inlen) ? -1 : (int)(unsigned char)inbuf[inpos++]; }
static ll rd() {
    int c = gc();
    while (c != -1 && (c < '0' || c > '9')) c = gc();
    ll x = 0;
    while (c >= '0' && c <= '9') { x = x * 10 + (c - '0'); c = gc(); }
    return x;
}

static const ll BIG = (ll)4e18;
static const int MAXT = 1400000;

static int N, K;
static ll L, T;
static vector<ll> cuts, gslot;

/* ---------- per-cut static sorted distinct-P lists ---------- */
static vector<int> cutSt, cutCnt, cutPre;
static vector<ll> keyP, keyH, preHarr, preHParr;

/* ---------- treap node pool (dynamic "extras") ---------- */
static int tlc[MAXT], trc[MAXT];
static ll tkey[MAXT], th[MAXT], tsh[MAXT], tshp[MAXT];
static unsigned tpri[MAXT];
static int freels[MAXT], freeTop = 0, bump = 1;
static unsigned long long rngst = 0x9E3779B97F4A7C15ULL;
static inline unsigned rnd() { rngst ^= rngst << 13; rngst ^= rngst >> 7; rngst ^= rngst << 17; return (unsigned)(rngst >> 11); }
static inline void tupd(int u) { tsh[u] = tsh[tlc[u]] + tsh[trc[u]] + th[u]; tshp[u] = tshp[tlc[u]] + tshp[trc[u]] + th[u] * tkey[u]; }
static inline int newNode(ll k, ll h) {
    int u = freeTop ? freels[--freeTop] : bump++;
    tlc[u] = trc[u] = 0; tkey[u] = k; th[u] = h; tsh[u] = h; tshp[u] = k * h; tpri[u] = rnd();
    return u;
}
static void trotR(int &t) { int u = tlc[t]; tlc[t] = trc[u]; trc[u] = t; tupd(t); tupd(u); t = u; }
static void trotL(int &t) { int u = trc[t]; trc[t] = tlc[u]; tlc[u] = t; tupd(t); tupd(u); t = u; }
static void tins(int &t, int nd) {
    if (!t) { t = nd; return; }
    if (tkey[nd] == tkey[t]) { th[t] += th[nd]; tupd(t); freels[freeTop++] = nd; return; }
    if (tkey[nd] < tkey[t]) { tins(tlc[t], nd); if (tpri[tlc[t]] < tpri[t]) trotR(t); else tupd(t); }
    else { tins(trc[t], nd); if (tpri[trc[t]] < tpri[t]) trotL(t); else tupd(t); }
}
static void freeRoot(int u) {
    if (!u) return;
    static vector<int> stk; stk.clear(); stk.push_back(u);
    while (!stk.empty()) {
        int t = stk.back(); stk.pop_back();
        if (tlc[t]) stk.push_back(tlc[t]);
        if (trc[t]) stk.push_back(trc[t]);
        freels[freeTop++] = t;
    }
}
static vector<int> trav;
static void collectAll(int u) {
    trav.clear(); static vector<int> stk; stk.clear();
    int x = u;
    while (x || !stk.empty()) {
        while (x) { stk.push_back(x); x = tlc[x]; }
        x = stk.back(); stk.pop_back(); trav.push_back(x); x = trc[x];
    }
}
/* prefix sums of H / H*P over treap keys <= c */
static ll exPreH(int u, ll c) { ll s = 0; while (u) { if (tkey[u] <= c) { s += th[u] + tsh[tlc[u]]; u = trc[u]; } else u = tlc[u]; } return s; }
static ll exPreHP(int u, ll c) { ll s = 0; while (u) { if (tkey[u] <= c) { s += th[u] * tkey[u] + tshp[tlc[u]]; u = trc[u]; } else u = tlc[u]; } return s; }
static inline int ubCut(int k, ll c) {
    const ll *beg = keyP.data() + cutSt[k], *end = beg + cutCnt[k];
    return (int)(upper_bound(beg, end, c) - beg);
}

/* ---------- components: base cut + extras treap ---------- */
struct Comp { int bcut, root; ll totH, totHP; int cnt; };
static vector<Comp> comps;
static vector<int> compFree;
static int newComp() {
    int id;
    if (!compFree.empty()) { id = compFree.back(); compFree.pop_back(); }
    else { id = (int)comps.size(); comps.push_back(Comp()); }
    Comp &c = comps[id]; c.bcut = -1; c.root = 0; c.totH = 0; c.totHP = 0; c.cnt = 0;
    return id;
}
static void delComp(int id) { freeRoot(comps[id].root); comps[id].root = 0; compFree.push_back(id); }

static long long stRep=0, stCuts=0, stCopy=0, stEv=0, stPool=0, stNosp=0, stSp=0;
static void getLevel(const Comp &cp, ll G, ll x, ll &lev, ll &W, ll &Q) {
    if ((lll)x * (lll)cp.totH <= (lll)G) { lev = 0; W = cp.totH; Q = 0; return; }
    ll best = BIG;
    if (cp.bcut >= 0) {
        int k = cp.bcut, lo = 0, hi = cutCnt[k] - 1, ans = -1;
        while (lo <= hi) {
            int mid = (lo + hi) >> 1;
            ll c = keyP[cutSt[k] + mid];
            ll w = cp.totH - preHarr[cutPre[k] + mid + 1] - exPreH(cp.root, c);
            if ((lll)x * (lll)w <= (lll)G) { ans = mid; hi = mid - 1; } else lo = mid + 1;
        }
        if (ans >= 0) best = keyP[cutSt[k] + ans];
    }
    {
        int u = cp.root; ll acc = 0;
        while (u) {
            ll c = tkey[u];
            ll pre = acc + tsh[tlc[u]] + th[u];
            ll bp = (cp.bcut >= 0) ? preHarr[cutPre[cp.bcut] + ubCut(cp.bcut, c)] : 0;
            ll w = cp.totH - bp - pre;
            if ((lll)x * (lll)w <= (lll)G) { if (c < best) best = c; u = tlc[u]; }
            else { acc += tsh[tlc[u]] + th[u]; u = trc[u]; }
        }
    }
    if (best == BIG) {
        ll mx = -1;
        if (cp.bcut >= 0) mx = keyP[cutSt[cp.bcut] + cutCnt[cp.bcut] - 1];
        if (cp.root) { int u = cp.root; while (trc[u]) u = trc[u]; if (tkey[u] > mx) mx = tkey[u]; }
        best = mx;
    }
    lev = best;
    ll ph = 0, php = 0;
    if (cp.bcut >= 0) { int ub = ubCut(cp.bcut, lev); ph += preHarr[cutPre[cp.bcut] + ub]; php += preHParr[cutPre[cp.bcut] + ub]; }
    ph += exPreH(cp.root, lev); php += exPreHP(cp.root, lev);
    W = cp.totH - ph; Q = php;
}

static void absorb(int dst, int src) {
    Comp &D = comps[dst], &S = comps[src];
    stCopy += S.cnt;
    stCopy += S.cnt;
    if (S.bcut >= 0) {
        int k = S.bcut;
        for (int i = 0; i < cutCnt[k]; i++) tins(D.root, newNode(keyP[cutSt[k] + i], keyH[cutSt[k] + i]));
    }
    if (S.root) {
        collectAll(S.root);
        for (size_t i = 0; i < trav.size(); i++) { int nd = trav[i]; tins(D.root, newNode(tkey[nd], th[nd])); }
    }
    D.totH += S.totH; D.totHP += S.totHP; D.cnt += S.cnt;
    delComp(src);
}

/* ---------- blocks ---------- */
struct Blk { int a, b, comp; ll lev, W, Q, G; };
static vector<Blk> blks;
static vector<int> bprev, bnext, bver;
static vector<char> balive;
static vector<int> belong;
static lll sumS = 0, sumC = 0;

struct Ent { int a, b, comp; ll lev, W, Q, G; };

static void pushBlkEv(int b);
static void poolRange(int lo, int hi, ll x, vector<Ent> &out) {
    stRep++; stCuts += hi-lo+1;
    { static int bq[26]={0}; int w=hi-lo+1, d=0, t=w; while(t>1){t>>=1;d++;} if(d>25)d=25; bq[d]++;
      if (getenv("DBGQ")) { }
      static int cnt=0; if (getenv("DBGR") && w > 5000) { cnt++; fprintf(stderr, "bigrep %d w=%d\n", cnt, w); } }
    out.clear();
    static vector<Ent> st; st.clear();
    for (int k = lo; k <= hi; k++) {
        int c = newComp();
        comps[c].bcut = k; comps[c].cnt = cutCnt[k];
        comps[c].totH = preHarr[cutPre[k] + cutCnt[k]];
        comps[c].totHP = preHParr[cutPre[k] + cutCnt[k]];
        Ent e; e.a = e.b = k; e.comp = c; e.G = gslot[k];
        getLevel(comps[c], e.G, x, e.lev, e.W, e.Q);
        st.push_back(e);
        while (st.size() >= 2 && st[st.size() - 2].lev > st.back().lev) {
            Ent r = st.back(); st.pop_back();
            Ent l = st.back(); st.pop_back();
            int keep, drop;
            if (comps[l.comp].cnt >= comps[r.comp].cnt) { keep = l.comp; drop = r.comp; }
            else { keep = r.comp; drop = l.comp; }
            absorb(keep, drop);
            Ent m; m.a = l.a; m.b = r.b; m.comp = keep; m.G = l.G + r.G;
            getLevel(comps[keep], m.G, x, m.lev, m.W, m.Q);
            st.push_back(m);
        }
    }
    out.assign(st.begin(), st.end());
}

static inline void addC(const Blk &b, int sign) {
    sumS += sign * ((lll)b.lev * (lll)b.W + (lll)b.Q);
    sumC += sign * ((lll)b.lev * (lll)b.G);
}

static void commitBlocks(int lo, int hi, vector<Ent> &nb) {
    int first = belong[lo];
    int Lb = bprev[first], Rb = -1, b = first;
    while (true) {
        addC(blks[b], -1);
        delComp(blks[b].comp);
        balive[b] = 0; bver[b]++;
        int nx = bnext[b];
        if (blks[b].b >= hi) { Rb = nx; break; }
        b = nx;
    }
    int prevId = Lb;
    if (nb.size()==1) stNosp++; else stSp++;
    for (size_t i = 0; i < nb.size(); i++) {
        int id = (int)blks.size();
        Blk b2; b2.a = nb[i].a; b2.b = nb[i].b; b2.comp = nb[i].comp;
        b2.lev = nb[i].lev; b2.W = nb[i].W; b2.Q = nb[i].Q; b2.G = nb[i].G;
        blks.push_back(b2); bprev.push_back(prevId); bnext.push_back(-1); bver.push_back(0); balive.push_back(1);
        if (prevId >= 0) bnext[prevId] = id;
        if (i + 1 == nb.size()) { bnext[id] = Rb; if (Rb >= 0) bprev[Rb] = id; }
        prevId = id;
        for (int k = b2.a; k <= b2.b; k++) belong[k] = id;
        addC(b2, 1);
        pushBlkEv(id);
    }
}

static void repair(int lo, int hi, ll x) {
    static vector<Ent> out;
    while (true) {
        poolRange(lo, hi, x, out);
        bool ext = false;
        if (lo > 0) { int bl = belong[lo - 1]; if (blks[bl].lev > out.front().lev) { lo = blks[bl].a; ext = true; } }
        if (!ext && hi < K - 1) { int br = belong[hi + 1]; if (out.back().lev > blks[br].lev) { hi = blks[br].b; ext = true; } }
        if (!ext) break;
        for (size_t i = 0; i < out.size(); i++) delComp(out[i].comp);
    }
    commitBlocks(lo, hi, out);
}

static vector<ll> gk, gW, gQ, gtau;
static vector<int> gver;
struct Ev { ll tau; int type, id, ver; };
struct CmpEv { bool operator()(const Ev &a, const Ev &b) const { return a.tau > b.tau; } };
static priority_queue<Ev, vector<Ev>, CmpEv> pq;
static inline void pushCutEv(int k) {
    gver[k]++;
    ll tau = (gW[k] > 0) ? (gslot[k] / gW[k] + 1) : BIG;
    gtau[k] = tau; pq.push(Ev{tau, 0, k, gver[k]});
}
static inline void pushBlkEv(int b) {
    bver[b]++;
    ll tau = (blks[b].W > 0) ? (blks[b].G / blks[b].W + 1) : BIG;
    pq.push(Ev{tau, 1, b, bver[b]});
}

int main() {
    {
        size_t cap = 1 << 20, len = 0;
        char *b = (char *)malloc(cap);
        while (true) {
            if (len == cap) { cap *= 2; b = (char *)realloc(b, cap); }
            size_t got = fread(b + len, 1, cap - len, stdin);
            if (!got) break;
            len += got;
        }
        inbuf = b; inlen = len; inpos = 0;
    }
    N = (int)rd(); L = rd(); T = rd();
    vector<ll> S(N), H(N), P(N);
    for (int i = 0; i < N; i++) { S[i] = rd(); H[i] = rd(); P[i] = rd(); }

    vector<ll> cs(S.begin(), S.end());
    sort(cs.begin(), cs.end());
    cs.erase(unique(cs.begin(), cs.end()), cs.end());
    K = (int)cs.size();
    cuts = cs;
    gslot.assign(K, 0);
    for (int k = 0; k < K; k++) gslot[k] = (k + 1 < K ? cuts[k + 1] : T) - cuts[k];

    /* build per-cut sorted distinct-P lists */
    {
        vector<int> ord(N);
        for (int i = 0; i < N; i++) ord[i] = i;
        vector<int> cnt(K, 0);
        vector<int> cid(N);
        for (int i = 0; i < N; i++) {
            cid[i] = (int)(lower_bound(cuts.begin(), cuts.end(), S[i]) - cuts.begin());
            cnt[cid[i]]++;
        }
        cutSt.assign(K, 0); cutCnt = cnt; cutPre.assign(K, 0);
        int tot = 0;
        for (int k = 0; k < K; k++) { cutSt[k] = tot; tot += cnt[k]; }
        keyP.assign(tot, 0); keyH.assign(tot, 0);
        vector<int> pos = cutSt;
        for (int i = 0; i < N; i++) { int k = cid[i]; int pp = pos[k]++; keyP[pp] = P[i]; keyH[pp] = H[i]; }
        int tot2 = 0;
        for (int k = 0; k < K; k++) { cutPre[k] = tot2; tot2 += cutCnt[k] + 1; }
        preHarr.assign(tot2, 0); preHParr.assign(tot2, 0);
        vector<pair<ll,ll>> tmp;
        for (int k = 0; k < K; k++) {
            int s0 = cutSt[k], n = cutCnt[k];
            tmp.clear();
            for (int j = 0; j < n; j++) tmp.push_back({keyP[s0 + j], keyH[s0 + j]});
            sort(tmp.begin(), tmp.end());
            int w = 0;
            for (int j = 0; j < n; j++) {
                if (w > 0 && keyP[s0 + w - 1] == tmp[j].first) keyH[s0 + w - 1] += tmp[j].second;
                else { keyP[s0 + w] = tmp[j].first; keyH[s0 + w] = tmp[j].second; w++; }
            }
            cutCnt[k] = w;
            ll ah = 0, ahp = 0;
            preHarr[cutPre[k]] = 0; preHParr[cutPre[k]] = 0;
            for (int j = 0; j < w; j++) {
                ah += keyH[s0 + j]; ahp += keyH[s0 + j] * keyP[s0 + j];
                preHarr[cutPre[k] + j + 1] = ah; preHParr[cutPre[k] + j + 1] = ahp;
            }
        }
    }

    gk.assign(K, 0); gW.assign(K, 0); gQ.assign(K, 0); gtau.assign(K, BIG); gver.assign(K, 0);
    belong.assign(K, 0);
    blks.clear(); bprev.clear(); bnext.clear(); bver.clear(); balive.clear();

    {
        static vector<Ent> out;
        poolRange(0, K - 1, 1, out);
        int prevId = -1;
        for (size_t i = 0; i < out.size(); i++) {
            int id = (int)blks.size();
            Blk b2; b2.a = out[i].a; b2.b = out[i].b; b2.comp = out[i].comp;
            b2.lev = out[i].lev; b2.W = out[i].W; b2.Q = out[i].Q; b2.G = out[i].G;
            blks.push_back(b2); bprev.push_back(prevId); bnext.push_back(-1); bver.push_back(0); balive.push_back(1);
            if (prevId >= 0) bnext[prevId] = id;
            prevId = id;
            for (int k = b2.a; k <= b2.b; k++) belong[k] = id;
            addC(b2, 1);
        }
    }
    for (int k = 0; k < K; k++) {
        int c = newComp();
        comps[c].bcut = k; comps[c].cnt = cutCnt[k];
        comps[c].totH = preHarr[cutPre[k] + cutCnt[k]];
        comps[c].totHP = preHParr[cutPre[k] + cutCnt[k]];
        getLevel(comps[c], gslot[k], 1, gk[k], gW[k], gQ[k]);
        delComp(c);
    }
    for (int k = 0; k < K; k++) pushCutEv(k);
    for (size_t b = 0; b < blks.size(); b++) pushBlkEv((int)b);

    vector<ll> plo; vector<lll> pS, pC;
    ll cur = 1;
    static vector<int> aff;
    while (true) {
        ll tauMin = BIG;
        while (!pq.empty()) {
            Ev e = pq.top();
            bool ok = (e.type == 0) ? (gver[e.id] == e.ver) : (balive[e.id] && bver[e.id] == e.ver);
            if (!ok) { pq.pop(); continue; }
            tauMin = e.tau; break;
        }
        if (getenv("DBG2")) {
            fprintf(stderr, "x=%lld:", cur);
            for (size_t b = 0; b < blks.size(); b++) if (balive[b]) fprintf(stderr, " [%d,%d]lev=%lld,W=%lld,Q=%lld,G=%lld", blks[b].a, blks[b].b, blks[b].lev, blks[b].W, blks[b].Q, blks[b].G);
            fprintf(stderr, "\n");
        }
        if (tauMin > L) { plo.push_back(cur); pS.push_back(sumS); pC.push_back(sumC); break; }
        plo.push_back(cur); pS.push_back(sumS); pC.push_back(sumC);
        cur = tauMin;
        aff.clear();
        while (!pq.empty()) {
            Ev e = pq.top();
            if (e.tau > cur) break;
            pq.pop(); stEv++; stEv++;
            if (getenv("DBG2")) fprintf(stderr, "  ev tau=%lld type=%d id=%d\n", e.tau, e.type, e.id);
            if (e.type == 0) {
                if (gver[e.id] != e.ver) continue;
                int k = e.id;
                int c = newComp();
                comps[c].bcut = k; comps[c].cnt = cutCnt[k];
                comps[c].totH = preHarr[cutPre[k] + cutCnt[k]];
                comps[c].totHP = preHParr[cutPre[k] + cutCnt[k]];
                getLevel(comps[c], gslot[k], cur, gk[k], gW[k], gQ[k]);
                delComp(c);
                pushCutEv(k);
                aff.push_back(k); aff.push_back(k);
            } else {
                if (!balive[e.id] || bver[e.id] != e.ver) continue;
                int b = e.id;
                aff.push_back(blks[b].a); aff.push_back(blks[b].b);
            }
        }
        if (!aff.empty()) {
            static vector<pair<int,int>> ivs; ivs.clear();
            for (size_t i = 0; i + 1 < aff.size(); i += 2) {
                int l = aff[i], r = aff[i + 1];
                int bl = belong[l], br = belong[r];
                ivs.push_back({blks[bl].a, blks[br].b});
            }
            sort(ivs.begin(), ivs.end());
            static vector<pair<int,int>> mg; mg.clear();
            for (size_t i = 0; i < ivs.size(); i++) {
                if (!mg.empty() && ivs[i].first <= mg.back().second + 1) mg.back().second = max(mg.back().second, ivs[i].second);
                else mg.push_back(ivs[i]);
            }
            size_t i = 0;
            while (i < mg.size()) {
                int l = blks[belong[mg[i].first]].a, r = blks[belong[mg[i].second]].b;
                size_t j = i + 1;
                while (j < mg.size() && blks[belong[mg[j].first]].a <= r + 1) { r = max(r, blks[belong[mg[j].second]].b); j++; }
                repair(l, r, cur);
                i = j;
            }
        }
    }
    if (getenv("DBGH")) { int bq[26]={0}; }
    if (getenv("DBGSTAT")) fprintf(stderr, "pieces=%d K=%d repairs=%lld cutsScanned=%lld copies=%lld events=%lld blocks=%d\n", (int)plo.size(), K, stRep, stCuts, stCopy, stEv, (int)blks.size());
    if (getenv("DBGSTAT")) fprintf(stderr, "pieces=%d K=%d repairs=%lld cutsScanned=%lld copies=%lld events=%lld nosplit=%lld split=%lld blocks=%d\n", (int)plo.size(), K, stRep, stCuts, stCopy, stEv, stNosp, stSp, (int)blks.size());
    int np = (int)plo.size();
    if (getenv("DBGP")) {
        for (int i = 0; i < np; i++) {
            ll hi = (i + 1 < np) ? plo[i + 1] - 1 : L;
            fprintf(stderr, "piece %d: [%lld,%lld] S=%lld C=%lld f=%lld\n", i, plo[i], hi, (ll)pS[i], (ll)pC[i], (ll)((lll)pS[i] * (lll)plo[i] - (lll)pC[i]));
        }
    }

    int Q = (int)rd();
    size_t outcap = (size_t)Q * 13 + 32;
    char *outb = (char *)malloc(outcap); size_t olen = 0;
    int p = 0;
    for (int qi = 0; qi < Q; qi++) {
        ll M = rd();
        while (p + 1 < np && (lll)pS[p + 1] * (lll)plo[p + 1] - pC[p + 1] <= (lll)M) p++;
        ll ans;
        if ((lll)pS[p] * (lll)plo[p] - pC[p] > (lll)M) ans = 0;
        else {
            ll hi = (p + 1 < np) ? plo[p + 1] - 1 : L;
            if (pS[p] == 0) ans = hi;
            else {
                lll v = ((lll)M + pC[p]) / pS[p];
                ans = (v > (lll)hi) ? hi : (ll)v;
                if (ans < plo[p]) ans = plo[p];
            }
        }
        if (ans > L) ans = L;
        char tmp[24]; int tl = 0;
        if (ans == 0) tmp[tl++] = '0';
        else { while (ans > 0) { tmp[tl++] = char('0' + ans % 10); ans /= 10; } }
        while (tl > 0) outb[olen++] = tmp[--tl];
        outb[olen++] = '\n';
    }
    fwrite(outb, 1, olen, stdout);
    return 0;
}
