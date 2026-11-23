#include <bits/stdc++.h>
using namespace std;
typedef long long ll;
typedef unsigned long long ull;
typedef __int128 lll;

static const ull MODP = (1ULL << 61) - 1;
static ull mulm(ull a, ull b) {
    unsigned __int128 r = (unsigned __int128)a * b;
    ull lo = (ull)(r & MODP), hi = (ull)(r >> 61);
    ull s = lo + hi;
    if (s >= MODP) s -= MODP;
    return s;
}
static ull addm(ull a, ull b) { ull s = a + b; if (s >= MODP) s -= MODP; return s; }
static ull powm(ull a, ull e) { ull r = 1; while (e) { if (e & 1) r = mulm(r, a); a = mulm(a, a); e >>= 1; } return r; }
static ull toM(ll v) { // signed to mod
    if (v >= 0) return (ull)v % MODP;
    ull x = (ull)(-v) % MODP;
    return x ? MODP - x : 0;
}
static ull fracM(ll num, ll den) { // num/den mod P
    if (den < 0) { num = -num; den = -den; }
    return mulm(toM(num), powm(toM(den), MODP - 2));
}
static ull strM(const string& s) {
    ull r = 0; bool neg = false;
    for (char c : s) {
        if (c == '-') { neg = true; continue; }
        if (c < '0' || c > '9') continue;
        r = addm(mulm(r, 10), (ull)(c - '0'));
    }
    if (neg && r) r = MODP - r;
    return r;
}

struct Query { ll p, q; ull beta; };
vector<Query> queries;
void addQuery(ll p, ll q, ull beta) {
    ll g = std::gcd(p, q); p /= g; q /= g;
    queries.push_back({p, q, beta});
}

// representable check: x = num/den (den>0) reduced den <= 1000
static bool repr(ll num, ll den, ll& P, ll& Q) {
    ll g = std::gcd(num, den);
    num /= g; den /= g;
    if (den > 1000) return false;
    P = num; Q = den;
    return true;
}

ll X[1005];
ll gLeftZX, gRightZX;
int cntv[1005];

struct UNode { int j; ll W; bool good; };

// Search triple; returns true with queries x1=p1/q1 in (a,b), x2=p2/q2 in (b,c)
static chrono::steady_clock::time_point T0;
static double elapsed() { return chrono::duration<double>(chrono::steady_clock::now() - T0).count(); }

bool tripleSearch(ll a, ll b, ll c, ll W1, ll W2, ll W3, ll& p1, ll& q1, ll& p2, ll& q2) {
    ll d1 = b - a, d2 = c - b;
    long long budget = 4000000;
    if (elapsed() > 6.0) budget = 20000;
    // alternate: from x1 side with denominator q, from x2 side with denominator q
    for (ll q = 1; q <= 1000 && budget > 0; q++) {
        // x1 = p/q
        for (ll p = a * q + 1; p < b * q && budget > 0; p++) {
            budget--;
            if (std::gcd(p, q) != 1) continue;
            ll A = p - a * q, B = b * q - p;
            ll N = W2 * B - W1 * A;
            if (N <= 0) break;  // increasing p increases A, decreases B -> N decreases
            ll M = W3 * B;
            ll g = std::gcd(N, M); N /= g; M /= g;
            ll num = b * (M + N) + d2 * M, den = M + N, P, Q;
            if (repr(num, den, P, Q)) { p1 = p; q1 = q; p2 = P; q2 = Q; return true; }
        }
        // x2 = p/q
        for (ll p = c * q - 1; p > b * q && budget > 0; p--) {
            budget--;
            if (std::gcd(p, q) != 1) continue;
            ll N = c * q - p, M = p - b * q;  // t2 = N/M
            ll A = W2 * M - W3 * N;
            if (A <= 0) break;
            ll B = W1 * M;
            ll g = std::gcd(A, B); A /= g; B /= g;
            ll num = a * (A + B) + d1 * A, den = A + B, P, Q;
            if (repr(num, den, P, Q)) { p1 = P; q1 = Q; p2 = p; q2 = q; return true; }
        }
    }
    return false;
}

void solveChain(const vector<int>& js, bool leftZ, bool rightZ, const vector<ll>& Ws, const vector<bool>& goods) {
    int c = js.size();
    if (c == 0) return;
    // dp
    const int INF = 1e9;
    vector<int> dp(c + 1, INF), choice(c + 1, -1);
    vector<array<ll, 4>> tripInfo(c + 1);
    dp[0] = 0;
    auto pairExact = [&](int i, ll& P, ll& Q) {
        ll a = X[js[i]], b = X[js[i + 1]], d = b - a;
        ll WL = Ws[i], WR = Ws[i + 1];
        return repr(a * (WL + WR) + d * WR, WL + WR, P, Q);
    };
    for (int i = 0; i < c; i++) {
        if (dp[i] >= INF) continue;
        // single
        bool singleOk = goods[i] || (i == 0 && leftZ) || (i == c - 1 && rightZ);
        if (singleOk && dp[i] + 1 < dp[i + 1]) { dp[i + 1] = dp[i] + 1; choice[i + 1] = 1; }
        if (i + 1 < c) {
            ll P, Q;
            int cost = pairExact(i, P, Q) ? 1 : 2;
            if (dp[i] + cost < dp[i + 2]) { dp[i + 2] = dp[i] + cost; choice[i + 2] = (cost == 1 ? 2 : 3); }
        }
    }
    // second pass with triples (lazy)
    for (int i = 0; i < c; i++) dp[i + 1] = INF;
    dp[0] = 0;
    for (int i = 0; i < c; i++) {
        if (dp[i] >= INF) continue;
        bool singleOk = goods[i] || (i == 0 && leftZ) || (i == c - 1 && rightZ);
        if (singleOk && dp[i] + 1 < dp[i + 1]) { dp[i + 1] = dp[i] + 1; choice[i + 1] = 1; }
        if (i + 1 < c) {
            ll P, Q;
            int cost = pairExact(i, P, Q) ? 1 : 2;
            if (dp[i] + cost < dp[i + 2]) { dp[i + 2] = dp[i] + cost; choice[i + 2] = (cost == 1 ? 2 : 3); }
        }
        if (i + 2 < c) {
            // compute lower bound of dp[i+3] via other transitions
            int other = INF;
            {
                // dp[i+3] from dp[i+1]/dp[i+2] needs those finished; approximate: compute best of
                // (single i)+(pair i+1), (pair i)+(single i+2)
                ll P, Q;
                bool s0 = goods[i] || (i == 0 && leftZ);
                bool s2 = goods[i + 2] || (i + 2 == c - 1 && rightZ);
                int p01 = pairExact(i, P, Q) ? 1 : 2, p12 = pairExact(i + 1, P, Q) ? 1 : 2;
                if (s0) other = min(other, 1 + p12);
                if (s2) other = min(other, 1 + p01);
            }
            if (dp[i] + 3 < dp[i + 3]) { dp[i + 3] = dp[i] + 3; choice[i + 3] = 5; }
            if (other > 2 && dp[i] + 2 < dp[i + 3]) {
                ll p1, q1, p2, q2;
                if (tripleSearch(X[js[i]], X[js[i + 1]], X[js[i + 2]], Ws[i], Ws[i + 1], Ws[i + 2], p1, q1, p2, q2)) {
                    dp[i + 3] = dp[i] + 2; choice[i + 3] = 4;
                    tripInfo[i + 3] = {p1, q1, p2, q2};
                }
            }
        }
    }
    // reconstruct
    int i = c;
    while (i > 0) {
        int ch = choice[i];
        if (ch == 1) {
            int k = i - 1;
            if (goods[k]) {
                addQuery(X[js[k]], 1, fracM(Ws[k], 2));
            } else if (k == 0 && leftZ) {
                // interval [X[j]-dl, X[j]]: midpoint; f = v/2 -> beta = W
                // need left zero neighbor x: stored via global prevX
                // handled by caller passing zero coords in Ws? we use helper arrays
                ll zx = gLeftZX, a = X[js[k]];
                addQuery(zx + a, 2, fracM(Ws[k], 1));
            } else {
                ll zx = gRightZX, a = X[js[k]];
                addQuery(zx + a, 2, fracM(Ws[k], 1));
            }
            i -= 1;
        } else if (ch == 2) {
            int k = i - 2;
            ll P, Q; pairExact(k, P, Q);
            addQuery(P, Q, fracM(Ws[k] + Ws[k + 1], 2));
            i -= 2;
        } else if (ch == 3) {
            int k = i - 2;
            ll a = X[js[k]], b = X[js[k + 1]], d = b - a;
            ll WL = Ws[k], WR = Ws[k + 1];
            // f1 at a+d/3: (2/3)L+(1/3)R ; f2 at a+2d/3
            // contrib = f1(2wL-wR) + f2(2wR-wL), w=W/2
            addQuery(3 * a + d, 3, fracM(2 * WL - WR, 2));
            addQuery(3 * a + 2 * d, 3, fracM(2 * WR - WL, 2));
            i -= 2;
        } else if (ch == 4) {
            int k = i - 3;
            ll a = X[js[k]], b = X[js[k + 1]], cc = X[js[k + 2]];
            ll d1 = b - a, d2 = cc - b;
            auto& t = tripInfo[i];
            ll p1 = t[0], q1 = t[1], p2 = t[2], q2 = t[3];
            // beta1 = W1 q1 d1 / (2 (b q1 - p1)), beta2 = W3 q2 d2 / (2 (p2 - b q2))
            ull b1 = mulm(fracM(Ws[k] * d1, 2 * (b * q1 - p1)), toM(q1));
            ull b2 = mulm(fracM(Ws[k + 2] * d2, 2 * (p2 - b * q2)), toM(q2));
            addQuery(p1, q1, b1);
            addQuery(p2, q2, b2);
            i -= 3;
        } else if (ch == 5) {
            int k = i - 3;
            ll a = X[js[k]], b = X[js[k + 1]], cc = X[js[k + 2]];
            addQuery(a + b, 2, fracM(Ws[k], 1));
            addQuery(b + cc, 2, fracM(Ws[k + 2], 1));
            addQuery(b, 1, fracM(Ws[k + 1] - Ws[k] - Ws[k + 2], 2));
            i -= 3;
        } else {
            // should not happen
            i -= 1;
        }
    }
}
static ll cross(pair<ll,ll> o, pair<ll,ll> a, pair<ll,ll> b) {
    return (a.first - o.first) * (b.second - o.second) - (a.second - o.second) * (b.first - o.first);
}

int main() {
    T0 = chrono::steady_clock::now();
    int n;
    if (!(cin >> n)) return 0;
    vector<pair<ll,ll>> pts(n);
    for (auto& p : pts) cin >> p.first >> p.second;

    // convex position check (all points on hull boundary) -> unique polygon
    {
        vector<pair<ll,ll>> s = pts;
        sort(s.begin(), s.end());
        s.erase(unique(s.begin(), s.end()), s.end());
        int m = s.size();
        vector<pair<ll,ll>> h(2 * m + 2);
        int k = 0;
        // hull keeping collinear points: use strict turn removal (cross < 0)
        for (int i = 0; i < m; i++) {
            while (k >= 2 && cross(h[k - 2], h[k - 1], s[i]) < 0) k--;
            h[k++] = s[i];
        }
        for (int i = m - 2, t = k + 1; i >= 0; i--) {
            while (k >= t && cross(h[k - 2], h[k - 1], s[i]) < 0) k--;
            h[k++] = s[i];
        }
        h.resize(k - 1);
        // with collinear points kept, upper/lower chains could double-count when all collinear; ignore
        // count interior points: point strictly inside hull
        // compute strict hull
        vector<pair<ll,ll>> sh(2 * m + 2); k = 0;
        for (int i = 0; i < m; i++) {
            while (k >= 2 && cross(sh[k - 2], sh[k - 1], s[i]) <= 0) k--;
            sh[k++] = s[i];
        }
        for (int i = m - 2, t = k + 1; i >= 0; i--) {
            while (k >= t && cross(sh[k - 2], sh[k - 1], s[i]) <= 0) k--;
            sh[k++] = s[i];
        }
        sh.resize(k - 1);
        int hs = sh.size();
        bool allOn = (hs >= 3) && m == n;
        if (allOn) {
            for (auto& p : s) {
                bool onB = false;
                for (int i = 0; i < hs && !onB; i++) {
                    auto A = sh[i], B = sh[(i + 1) % hs];
                    if (cross(A, B, p) == 0 && min(A.first, B.first) <= p.first && p.first <= max(A.first, B.first) &&
                        min(A.second, B.second) <= p.second && p.second <= max(A.second, B.second)) onB = true;
                }
                if (!onB) { allOn = false; break; }
            }
        }
        if (allOn) {
            ll A2 = 0;
            for (int i = 0; i < hs; i++) {
                auto P = sh[i], Q = sh[(i + 1) % hs];
                A2 += P.first * Q.second - P.second * Q.first;
            }
            A2 = llabs(A2);
            if (A2 % 2 == 0) printf("! %lld 1\n", A2 / 2);
            else printf("! %lld 2\n", A2);
            fflush(stdout);
            return 0;
        }
    }

    map<ll,int> cm;
    for (auto& p : pts) cm[p.first]++;
    int k = 0;
    for (auto& [x, c] : cm) { X[k] = x; cntv[k] = c; k++; }

    // Build node sequence. Node types: 0=Z,1=G,2=L,3=R ; breaks between L and R of same j
    struct Nd { int j; int type; };
    vector<vector<Nd>> chains;
    vector<Nd> cur;
    for (int j = 0; j < k; j++) {
        if (j == 0) {
            cur.push_back({j, cntv[j] == 1 ? 0 : 3});
        } else if (j == k - 1) {
            cur.push_back({j, cntv[j] == 1 ? 0 : 2});
        } else if (cntv[j] == 1) {
            cur.push_back({j, 1});
        } else {
            cur.push_back({j, 2});
            chains.push_back(cur); cur.clear();
            cur.push_back({j, 3});
        }
    }
    chains.push_back(cur);

    for (auto& ch : chains) {
        int m = ch.size();
        vector<ll> W(m, 0);
        for (int i = 0; i + 1 < m; i++) {
            ll d = X[ch[i + 1].j] - X[ch[i].j];
            W[i] += d; W[i + 1] += d;
        }
        bool leftZ = ch[0].type == 0, rightZ = ch[m - 1].type == 0;
        int lo = leftZ ? 1 : 0, hi = rightZ ? m - 1 : m;
        vector<int> js; vector<ll> Ws; vector<bool> goods;
        for (int i = lo; i < hi; i++) { js.push_back(ch[i].j); Ws.push_back(W[i]); goods.push_back(ch[i].type == 1); }
        if (leftZ) gLeftZX = X[ch[0].j];
        if (rightZ) gRightZX = X[ch[m - 1].j];
        solveChain(js, leftZ, rightZ, Ws, goods);
    }

    // merge duplicate queries
    map<pair<ll,ll>, ull> qm;
    for (auto& q : queries) qm[{q.p, q.q}] = addm(qm[{q.p, q.q}], q.beta);

    ull total = 0; // area mod P
    for (auto& [pq, beta] : qm) {
        printf("? %lld %lld\n", pq.first, pq.second);
        fflush(stdout);
        string r, s;
        if (!(cin >> r)) return 0;
        if (r[0] == '-') return 0;
        if (!(cin >> s)) return 0;
        ull rv = strM(r), sv = strM(s);
        ull f = mulm(rv, powm(sv, MODP - 2));
        total = addm(total, mulm(f, beta));
    }
    ull A2 = mulm(total, 2);
    ll v = (ll)A2;
    if (v % 2 == 0) printf("! %lld 1\n", v / 2);
    else printf("! %lld 2\n", v);
    fflush(stdout);
    return 0;
}
