#include <bits/stdc++.h>
using namespace std;

static int n, N;
static int cr = 0, cc = 0;
static int R1, R2, C1, C2;
static int cnt = 0, testId = 0;
static bool testing = false;
static vector<int> stampv;

static inline bool inR(int r, int c) { return r >= R1 && r <= R2 && c >= C1 && c <= C2; }
static bool neededCell(int r, int c) {
    if (inR(r, c)) {
        if (r == R1 && R1 > -n) return true;
        if (r == R2 && R2 < n) return true;
        if (c == C1 && C1 > -n) return true;
        if (c == C2 && C2 < n) return true;
        return false;
    }
    if (r >= R1 && r <= R2 && (c == C1 - 1 || c == C2 + 1)) return true;
    if (c >= C1 && c <= C2 && (r == R1 - 1 || r == R2 + 1)) return true;
    return false;
}
static void visit(int r, int c, char resp) {
    if (!testing || resp == '.') return;
    if (!neededCell(r, c)) return;
    int id = (r + n) * N + (c + n);
    if (stampv[id] == testId) return;
    stampv[id] = testId;
    int nr = r, nc = c;
    if (resp == '^') nr--; else if (resp == 'v') nr++;
    else if (resp == '<') nc--; else if (resp == '>') nc++; else return;
    if (inR(nr, nc) != inR(r, c)) cnt++;
}
static char step(char d) {
    if (d == '^') cr--; else if (d == 'v') cr++; else if (d == '<') cc--; else cc++;
    cout << d << '\n'; cout.flush();
    string s;
    if (!(cin >> s)) exit(0);
    if (s == "G") exit(0);
    if (s == "-1") exit(0);
    char resp = s[0]; visit(cr, cc, resp); return resp;
}
static void walkTo(int r, int c) {
    while (cr != r) step(cr < r ? 'v' : '^');
    while (cc != c) step(cc < c ? '>' : '<');
}
struct Rect { int r1, r2, c1, c2; };
struct Plan { Rect rc; int mode, bi, bj, sr, sc; double cost; };
static double futureCost(int r1, int r2, int c1, int c2) {
    int h = r2 - r1 + 1, w = c2 - c1 + 1;
    bool t = (r1 == -n), b = (r2 == n), l = (c1 == -n), r = (c2 == n);
    int nb = (int)t + (int)b + (int)l + (int)r;
    bool cor = (t && l) || (t && r) || (b && l) || (b && r);
    double f = (nb >= 3 || cor) ? 5.0 : ((nb >= 1) ? 7.0 : 10.0);
    return f * (double)(h + w);
}
static void makePlans(const Rect& rc, vector<Plan>& out) {
    int r1 = rc.r1, r2 = rc.r2, c1 = rc.c1, c2 = rc.c2, h = r2 - r1 + 1, w = c2 - c1 + 1;
    bool f[4] = {r1 > -n, c2 < n, r2 < n, c1 > -n};
    int len[4] = {w, h, w, h}, sum = 0;
    for (int k = 0; k < 4; k++) if (f[k]) sum += len[k];
    if (sum == 0) { Plan p; p.rc = rc; p.mode = 6; p.bi = p.bj = 0; p.sr = cr; p.sc = cc; p.cost = 0; out.push_back(p); return; }
    int isR[4] = {r1, r1, r2, r2}, isC[4] = {c1, c2, c2, c1};
    int ieR[4] = {r1, r2, r2, r1}, ieC[4] = {c2, c2, c1, c1};
    if (f[0] && f[2] && !f[1] && !f[3]) {
        int base = 2 * sum + h, st[4][2] = {{r1-1,c1},{r2+1,c1},{r1-1,c2},{r2+1,c2}}, md[4] = {2,3,7,8};
        for (int i = 0; i < 4; i++) { Plan p; p.rc = rc; p.mode = md[i]; p.bi = p.bj = 0; p.sr = st[i][0]; p.sc = st[i][1]; p.cost = base + abs(cr-p.sr) + abs(cc-p.sc); out.push_back(p); }
        return;
    }
    if (f[1] && f[3] && !f[0] && !f[2]) {
        int base = 2 * sum + w, st[4][2] = {{r1,c1-1},{r1,c2+1},{r2,c1-1},{r2,c2+1}}, md[4] = {4,5,9,10};
        for (int i = 0; i < 4; i++) { Plan p; p.rc = rc; p.mode = md[i]; p.bi = p.bj = 0; p.sr = st[i][0]; p.sc = st[i][1]; p.cost = base + abs(cr-p.sr) + abs(cc-p.sc); out.push_back(p); }
        return;
    }
    int bi = 0, bj = 0, bl = 5;
    for (int i = 0; i < 4; i++) for (int l = 1; l <= 4; l++) {
        bool ok = true;
        for (int k = 0; k < 4; k++) if (f[k] && ((k - i + 4) % 4) >= l) { ok = false; break; }
        if (ok && l < bl) { bl = l; bi = i; bj = (i + l - 1) % 4; }
    }
    int base = 2 * sum + 2 * bl; Plan p; p.rc = rc; p.bi = bi; p.bj = bj;
    p.mode = 0; p.sr = isR[bi]; p.sc = isC[bi]; p.cost = base + abs(cr-p.sr) + abs(cc-p.sc); out.push_back(p);
    p.mode = 1; p.sr = ieR[bj]; p.sc = ieC[bj]; p.cost = base + abs(cr-p.sr) + abs(cc-p.sc); out.push_back(p);
}
static bool execPlan(const Plan& p) {
    R1 = p.rc.r1; R2 = p.rc.r2; C1 = p.rc.c1; C2 = p.rc.c2;
    if (p.mode == 6) return true;
    cnt = 0; testId++; testing = true;
    int r1 = R1, r2 = R2, c1 = C1, c2 = C2;
    if (p.mode <= 1) {
        int isR[4]={r1,r1,r2,r2}, isC[4]={c1,c2,c2,c1};
        int ieR[4]={r1,r2,r2,r1}, ieC[4]={c2,c2,c1,c1};
        int osR[4]={r1-1,r1,r2+1,r2}, osC[4]={c1,c2+1,c2,c1-1};
        int oeR[4]={r1-1,r2,r2+1,r1}, oeC[4]={c2,c2+1,c1,c1-1};
        const char od[4]={'^','>','v','<'};
        if (p.mode == 0) {
            walkTo(isR[p.bi], isC[p.bi]);
            for (int k = p.bi;; k = (k+1)&3) { walkTo(ieR[k], ieC[k]); if (k == p.bj) break; }
            step(od[p.bj]);
            for (int k = p.bj;; k = (k+3)&3) { walkTo(osR[k], osC[k]); if (k == p.bi) break; int kp=(k+3)&3; walkTo(oeR[kp], oeC[kp]); }
        } else {
            walkTo(ieR[p.bj], ieC[p.bj]);
            for (int k = p.bj;; k = (k+3)&3) { walkTo(isR[k], isC[k]); if (k == p.bi) break; }
            step(od[p.bi]);
            for (int k = p.bi;; k = (k+1)&3) { walkTo(oeR[k], oeC[k]); if (k == p.bj) break; int kn=(k+1)&3; walkTo(osR[kn], osC[kn]); }
        }
    } else {
        int wp[8][2];
        switch (p.mode) {
            case 2: { int t[8][2]={{r1-1,c1},{r1-1,c2},{r1,c2},{r1,c1},{r2,c1},{r2,c2},{r2+1,c2},{r2+1,c1}}; memcpy(wp,t,sizeof(t)); break; }
            case 3: { int t[8][2]={{r2+1,c1},{r2+1,c2},{r2,c2},{r2,c1},{r1,c1},{r1,c2},{r1-1,c2},{r1-1,c1}}; memcpy(wp,t,sizeof(t)); break; }
            case 7: { int t[8][2]={{r1-1,c2},{r1-1,c1},{r1,c1},{r1,c2},{r2,c2},{r2,c1},{r2+1,c1},{r2+1,c2}}; memcpy(wp,t,sizeof(t)); break; }
            case 8: { int t[8][2]={{r2+1,c2},{r2+1,c1},{r2,c1},{r2,c2},{r1,c2},{r1,c1},{r1-1,c1},{r1-1,c2}}; memcpy(wp,t,sizeof(t)); break; }
            case 4: { int t[8][2]={{r1,c1-1},{r2,c1-1},{r2,c1},{r1,c1},{r1,c2},{r2,c2},{r2,c2+1},{r1,c2+1}}; memcpy(wp,t,sizeof(t)); break; }
            case 5: { int t[8][2]={{r1,c2+1},{r2,c2+1},{r2,c2},{r1,c2},{r1,c1},{r2,c1},{r2,c1-1},{r1,c1-1}}; memcpy(wp,t,sizeof(t)); break; }
            case 9: { int t[8][2]={{r2,c1-1},{r1,c1-1},{r1,c1},{r2,c1},{r2,c2},{r1,c2},{r1,c2+1},{r2,c2+1}}; memcpy(wp,t,sizeof(t)); break; }
            default:{ int t[8][2]={{r2,c2+1},{r1,c2+1},{r1,c2},{r2,c2},{r2,c1},{r1,c1},{r1,c1-1},{r2,c1-1}}; memcpy(wp,t,sizeof(t)); break; }
        }
        for (int i = 0; i < 8; i++) walkTo(wp[i][0], wp[i][1]);
    }
    testing = false;
    return ((cnt & 1) ^ (inR(0,0) ? 1 : 0)) != 0;
}
int main() {
    ios::sync_with_stdio(false); cin.tie(nullptr);
    if (!(cin >> n)) return 0;
    N = 2*n+1;
    stampv.assign((size_t)N*(size_t)N, 0);
    { int mnR=0,mxR=0,mnC=0,mxC=0; char dir='^'; long long steps=0;
      while (true) {
        char resp = step(dir); steps++;
        mnR=min(mnR,cr); mxR=max(mxR,cr); mnC=min(mnC,cc); mxC=max(mxC,cc);
        if (resp=='.') break;
        dir = resp;
        int h=mxR-mnR+1, w=mxC-mnC+1;
        if (steps > 4LL*(h+w) && 2LL*steps < (long long)N*N) break;
      } }
    int br1=-n, br2=n, bc1=-n, bc2=n;
    int lim = N/32;
    if (lim >= 1 && abs(cr)+abs(cc) <= N/4) {
        int D=1; while (2*D <= lim) D*=2;
        for (int d=1; d<=D; d*=2) {
            Rect rc{-d,d,-d,d}; vector<Plan> ps; makePlans(rc,ps);
            int best=0; for (int i=1;i<(int)ps.size();i++) if (ps[i].cost<ps[best].cost) best=i;
            if (execPlan(ps[best])) { br1=-d;br2=d;bc1=-d;bc2=d; break; }
        }
    }
    while (true) {
        int h=br2-br1+1, w=bc2-bc1+1; long long area=(long long)h*w;
        vector<Rect> cands;
        if (h>=2) { int mid=br1+h/2-1; cands.push_back({br1,mid,bc1,bc2}); cands.push_back({mid+1,br2,bc1,bc2}); }
        if (w>=2) { int mid=bc1+w/2-1; cands.push_back({br1,br2,bc1,mid}); cands.push_back({br1,br2,mid+1,bc2}); }
        double bestVal=1e18; Plan bestPlan{}; bool have=false;
        for (auto& rc : cands) {
            Rect comp;
            if (rc.r1==br1 && rc.r2<br2) comp={rc.r2+1,br2,bc1,bc2};
            else if (rc.r2==br2 && rc.r1>br1) comp={br1,rc.r1-1,bc1,bc2};
            else if (rc.c1==bc1 && rc.c2<bc2) comp={br1,br2,rc.c2+1,bc2};
            else comp={br1,br2,bc1,rc.c1-1};
            double fut=0.5*(futureCost(rc.r1,rc.r2,rc.c1,rc.c2)+futureCost(comp.r1,comp.r2,comp.c1,comp.c2));
            vector<Plan> ps; makePlans(rc,ps);
            for (auto& p : ps) { double v=p.cost+fut; if (v<bestVal) { bestVal=v; bestPlan=p; have=true; } }
        }
        int cs[4][2]={{br1,bc1},{br1,bc2},{br2,bc1},{br2,bc2}};
        int bi=0, bd=INT_MAX;
        for (int i=0;i<4;i++){ int dd=abs(cr-cs[i][0])+abs(cc-cs[i][1]); if (dd<bd){bd=dd;bi=i;} }
        double directCost=(double)area+bd;
        if (!have || area<=16 || directCost<=bestVal) {
            int sr=cs[bi][0], sc=cs[bi][1];
            walkTo(sr,sc);
            int dR=(sr==br1?1:-1), dC=(sc==bc1?1:-1), r=sr;
            for (int i=0;i<h;i++){
                int tc=(i%2==0)?(dC>0?bc2:bc1):(dC>0?bc1:bc2);
                walkTo(r,tc);
                if (i+1<h){ step(dR>0?'v':'^'); r+=dR; }
            }
            walkTo(-n,-n);
            for (int rr=-n;rr<=n;rr++){ if (rr>-n) step('v'); walkTo(rr, ((rr+n)%2==0)?n:-n); }
            return 0;
        }
        bool in = execPlan(bestPlan);
        Rect rc = bestPlan.rc;
        if (in) { br1=rc.r1; br2=rc.r2; bc1=rc.c1; bc2=rc.c2; }
        else {
            if (rc.r1==br1 && rc.r2<br2 && rc.c1==bc1 && rc.c2==bc2) br1=rc.r2+1;
            else if (rc.r2==br2 && rc.r1>br1 && rc.c1==bc1 && rc.c2==bc2) br2=rc.r1-1;
            else if (rc.c1==bc1 && rc.c2<bc2 && rc.r1==br1 && rc.r2==br2) bc1=rc.c2+1;
            else bc2=rc.c1-1;
        }
    }
}
