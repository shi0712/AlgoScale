// Lakeside reachable region
#include <bits/stdc++.h>
using namespace std;
typedef long double ld;

struct Pt { ld x, y; };
static inline Pt operator+(Pt a, Pt b){ return {a.x+b.x, a.y+b.y}; }
static inline Pt operator-(Pt a, Pt b){ return {a.x-b.x, a.y-b.y}; }
static inline Pt operator*(Pt a, ld t){ return {a.x*t, a.y*t}; }
static inline ld cross(Pt a, Pt b){ return a.x*b.y - a.y*b.x; }
static inline ld dot(Pt a, Pt b){ return a.x*b.x + a.y*b.y; }
static inline ld nrm(Pt a){ return sqrtl(dot(a,a)); }

int n; ld K;
vector<Pt> V;

// ---- point in polygon (even-odd) ----
static bool inPoly(Pt p){
    int cnt = 0;
    Pt farPt{p.x + 1e6L, p.y + 1e-7L};
    for (int i = 0; i < n; i++){
        Pt a = V[i], b = V[(i+1)%n];
        Pt r = farPt - p, s = b - a;
        ld den = cross(r,s);
        if (fabsl(den) < 1e-26L) continue;
        Pt ac = a - p;
        ld t = cross(ac,s)/den, u = cross(ac,r)/den;
        if (t > 0 && u >= 0 && u < 1) cnt++;
    }
    return (cnt & 1) != 0;
}

// ---- does segment [a,b] pass through the open interior of the lake? ----
static bool segFree(Pt a, Pt b){
    Pt r = b - a;
    ld rr = dot(r,r);
    if (rr < 1e-24L) { return !inPoly(a); }
    vector<ld> ts;
    ts.push_back(0); ts.push_back(1);
    for (int i = 0; i < n; i++){
        Pt c = V[i], d = V[(i+1)%n];
        Pt s = d - c; ld ss = dot(s,s);
        ld den = cross(r,s);
        if (fabsl(den) > 1e-24L){
            Pt ac = c - a;
            ld t = cross(ac,s)/den, u = cross(ac,r)/den;
            if (u >= -1e-14L && u <= 1+1e-14L && t >= -1e-14L && t <= 1+1e-14L)
                ts.push_back(max((ld)0, min((ld)1, t)));
        } else {
            // collinear?
            if (ss < 1e-24L){
                ld tt = dot(c-a, r)/rr;
                if (tt >= -1e-14L && tt <= 1+1e-14L && dot(c-a - r*tt, c-a - r*tt) < 1e-20L)
                    ts.push_back(max((ld)0, min((ld)1, tt)));
            } else if (fabsl(cross(c-a, r)) <= 1e-12L * max((ld)1, sqrtl(ss*rr))){
                ld t0 = dot(c-a, r)/rr, t1 = dot(d-a, r)/rr;
                if (min(t0,t1) <= 1+1e-14L && max(t0,t1) >= -1e-14L){
                    ld lo = max((ld)0, min(t0,t1)), hi = min((ld)1, max(t0,t1));
                    if (lo <= hi + 1e-14L){ ts.push_back(lo); ts.push_back(hi); }
                }
            }
        }
    }
    sort(ts.begin(), ts.end());
    for (size_t i = 0; i+1 < ts.size(); i++){
        ld lo = ts[i], hi = ts[i+1];
        if (hi - lo < 1e-13L) continue;
        Pt m = a + r*((lo+hi)*0.5L);
        if (inPoly(m)) return false;
    }
    return true;
}

// ---- sources ----
struct Src { Pt c; ld d, rho; };
vector<Src> S;

// reachability of point q (land point)
static bool reachPt(Pt q){
    for (const Src &s : S){
        if (s.d + nrm(q - s.c) > K + 1e-12L) continue;
        if (segFree(s.c, q)) return true;
    }
    return false;
}

// far extent along direction u
static ld Rof(ld u){
    Pt dir{cosl(u), sinl(u)};
    ld best = 0;
    for (const Src &s : S){
        ld b = dot(s.c, dir);
        ld disc = b*b + s.rho*s.rho - dot(s.c,s.c);
        if (disc < 0) continue;
        ld r2 = b + sqrtl(disc);
        if (r2 <= best) continue;
        if (r2 < 0) continue;
        Pt q = dir * r2;
        if (segFree(s.c, q)) best = r2;
    }
    return best;
}

// lake's radial measure inside [0, R] along direction u
static ld lakeMeasure(ld u, ld Rlim){
    Pt dir{cosl(u), sinl(u)};
    vector<ld> ts;
    for (int i = 0; i < n; i++){
        Pt a = V[i], b = V[(i+1)%n];
        Pt s = b - a;
        ld den = cross(dir, s);
        if (fabsl(den) < 1e-26L) continue;
        Pt ac = a;
        ld t = cross(ac, s)/den;     // param along ray: point = t*dir
        ld w = cross(ac, dir)/den;   // param along edge
        if (t > 0 && w >= 0 && w < 1) ts.push_back(t);
    }
    sort(ts.begin(), ts.end());
    ld res = 0;
    // intervals alternate outside/inside; determine the state just after the first crossing
    for (size_t i = 0; i < ts.size(); i++){
        ld lo = ts[i];
        ld hi = (i+1 < ts.size() ? ts[i+1] : Rlim);
        if (lo >= Rlim) break;
        hi = min(hi, Rlim);
        ld mid = (lo + hi) * 0.5L;
        if (inPoly(dir * mid)) res += hi*hi - lo*lo;
    }
    // interval from 0 to ts[0]
    if (!ts.empty() && ts[0] > 0){
        ld mid = ts[0]*0.5L;
        if (inPoly(dir*mid)) res += ts[0]*ts[0];
    } else if (ts.empty()){
        ld mid = Rlim*0.5L;
        if (Rlim > 0 && inPoly(dir*mid)) res += Rlim*Rlim;
    }
    return res;
}

static ld integrand(ld u){
    ld R = Rof(u);
    ld lm = (R > 0 ? lakeMeasure(u, R) : 0);
    return 0.5L * (R*R - lm);
}

// adaptive Simpson
static ld simpson(ld a, ld b, ld fa, ld fm, ld fb){ return (b-a)/6.0L*(fa + 4*fm + fb); }
static ld adapt(ld a, ld b, ld fa, ld fm, ld fb, ld whole, ld eps, int depth){
    ld m = (a+b)*0.5L;
    ld lm = (a+m)*0.5L, rm = (m+b)*0.5L;
    ld flm = integrand(lm), frm = integrand(rm);
    ld left = simpson(a,m,fa,flm,fm), right = simpson(m,b,fm,frm,fb);
    if (depth <= 0 || fabsl(left+right-whole) <= 15*eps) return left+right+(left+right-whole)/15.0L;
    return adapt(a,m,fa,flm,fm,left,eps*0.5L,depth-1) + adapt(m,b,fm,frm,fb,right,eps*0.5L,depth-1);
}
static ld integrate(ld a, ld b, ld eps){
    ld m = (a+b)*0.5L;
    ld fa = integrand(a), fm = integrand(m), fb = integrand(b);
    return adapt(a,b,fa,fm,fb,simpson(a,b,fa,fm,fb),eps,50);
}

int main(){
    int k;
    if (scanf("%d %d", &n, &k) != 2) return 0;
    K = k;
    V.resize(n);
    for (int i = 0; i < n; i++){
        long long x, y;
        scanf("%lld %lld", &x, &y);
        V[i] = Pt{(ld)x, (ld)y};
    }
    // quick check: lake entirely outside disk?
    {
        bool outside = true;
        for (int i = 0; i < n && outside; i++){
            Pt a = V[i], b = V[(i+1)%n];
            Pt s = b-a; ld ss = dot(s,s);
            ld t = ss > 0 ? -dot(a,s)/ss : 0;
            t = max((ld)0, min((ld)1, t));
            Pt p = a + s*t;
            if (nrm(p) <= K) outside = false;
            if (nrm(a) <= K) outside = false;
        }
        if (outside){
            printf("%.12Lf\n", acosl(-1.0L)*K*K);
            return 0;
        }
    }
    // geodesic distances to vertices: Dijkstra on visibility graph
    int N = n + 1; // nodes 0..n-1 vertices, n = O
    vector<ld> dist(N, 1e30L);
    vector<char> done(N, 0);
    dist[n] = 0;
    for (int it = 0; it < N; it++){
        int u = -1; ld best = 1e30L;
        for (int i = 0; i < N; i++) if (!done[i] && dist[i] < best){ best = dist[i]; u = i; }
        if (u < 0) break;
        done[u] = 1;
        Pt pu = (u == n) ? Pt{0,0} : V[u];
        for (int v = 0; v < N; v++){
            if (done[v]) continue;
            Pt pv = (v == n) ? Pt{0,0} : V[v];
            bool vis;
            if (u == n || v == n) vis = segFree(pu, pv);
            else if ((u+1)%n == v || (v+1)%n == u) vis = true;
            else vis = segFree(pu, pv);
            if (!vis) continue;
            ld nd = dist[u] + nrm(pv - pu);
            if (nd < dist[v]) dist[v] = nd;
        }
    }
    // build sources
    S.clear();
    S.push_back(Src{Pt{0,0}, 0, K});
    for (int i = 0; i < n; i++){
        if (dist[i] < K){
            S.push_back(Src{V[i], dist[i], K - dist[i]});
        }
    }
    // sort sources by distance (promising first)
    sort(S.begin(), S.end(), [](const Src&a, const Src&b){ return a.d < b.d; });

    ld ans = integrate(0, 2*acosl(-1.0L), 1e-11L * max((ld)1, K*K));
    if (ans < 0 && ans > -1e-6L) ans = 0;
    printf("%.12Lf\n", (double)ans);
    return 0;
}
