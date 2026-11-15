#include <bits/stdc++.h>
using namespace std;

using ll = long long;
using ld = long double;

const double PI = acos(-1.0);
const double TWO_PI = 2.0 * PI;

struct Circle {
    ll x, y, r;
    int v;
};

vector<Circle> cir;
ll sweepX;

struct Arc {
    int id;
    int type; // -1: lower arc, +1: upper arc
};

struct Cmp {
    bool operator()(const Arc& a, const Arc& b) const {
        const Circle& A = cir[a.id];
        const Circle& B = cir[b.id];

        ld dxA = (ld)sweepX - A.x;
        ld valA = (ld)A.r * A.r - dxA * dxA;
        if (valA < 0) valA = 0;
        ld ya = (ld)A.y + (ld)a.type * sqrtl(valA);

        ld dxB = (ld)sweepX - B.x;
        ld valB = (ld)B.r * B.r - dxB * dxB;
        if (valB < 0) valB = 0;
        ld yb = (ld)B.y + (ld)b.type * sqrtl(valB);

        if (ya < yb) return true;
        if (ya > yb) return false;

        // Tie breaker used mainly when several circles start at the same point.
        ld ca = (ld)a.type * sqrtl((ld)A.r);
        ld cb = (ld)b.type * sqrtl((ld)B.r);
        if (ca < cb) return true;
        if (ca > cb) return false;
        if (a.id != b.id) return a.id < b.id;
        return a.type < b.type;
    }
};

struct RawContact {
    ll xn, yn;
    int den;
    int a, b;
};

struct Incidence {
    double angle;
    int node;
};

ll absll(ll x) {
    return x >= 0 ? x : -x;
}

void reduceKey(ll& xn, ll& yn, ll& den) {
    ll g1 = std::gcd(absll(xn), absll(yn));
    ll g = std::gcd(g1, den);
    if (g == 0) g = 1;
    xn /= g;
    yn /= g;
    den /= g;
}

bool getTangencyKey(int i, int j, ll& xn, ll& yn, int& denOut) {
    const Circle& A = cir[i];
    const Circle& B = cir[j];

    __int128 dx = (__int128)A.x - B.x;
    __int128 dy = (__int128)A.y - B.y;
    __int128 D = dx * dx + dy * dy;

    ll sum = A.r + B.r;
    ll diff = A.r >= B.r ? A.r - B.r : B.r - A.r;

    bool external = false;

    if (D == (__int128)sum * sum) {
        external = true;
    } else if (diff > 0 && D == (__int128)diff * diff) {
        external = false;
    } else {
        return false;
    }

    __int128 den128, xn128, yn128;

    if (external) {
        den128 = sum;
        xn128 = (__int128)A.x * den128 + (__int128)A.r * (B.x - A.x);
        yn128 = (__int128)A.y * den128 + (__int128)A.r * (B.y - A.y);
    } else {
        den128 = diff;

        if (A.r > B.r) {
            xn128 = (__int128)A.x * den128 + (__int128)A.r * (B.x - A.x);
            yn128 = (__int128)A.y * den128 + (__int128)A.r * (B.y - A.y);
        } else {
            xn128 = (__int128)A.x * den128 - (__int128)A.r * (B.x - A.x);
            yn128 = (__int128)A.y * den128 - (__int128)A.r * (B.y - A.y);
        }
    }

    ll den = (ll)den128;
    ll xnl = (ll)xn128;
    ll ynl = (ll)yn128;

    reduceKey(xnl, ynl, den);

    xn = xnl;
    yn = ynl;
    denOut = (int)den;
    return true;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    ll Sx, Sy, Tx, Ty;
    cin >> n >> Sx >> Sy >> Tx >> Ty;

    if (Sx == Tx && Sy == Ty) {
        cout << 0 << '\n';
        return 0;
    }

    cir.resize(n);
    for (int i = 0; i < n; i++) {
        cin >> cir[i].x >> cir[i].y >> cir[i].r >> cir[i].v;
    }

    vector<vector<Incidence>> incVec(n);

    vector<RawContact> raw;
    raw.reserve(2LL * n + 10);

    vector<int> ordL(n), ordR(n);
    iota(ordL.begin(), ordL.end(), 0);
    iota(ordR.begin(), ordR.end(), 0);

    sort(ordL.begin(), ordL.end(), [&](int i, int j) {
        ll li = cir[i].x - cir[i].r;
        ll lj = cir[j].x - cir[j].r;
        if (li != lj) return li < lj;
        return i < j;
    });

    sort(ordR.begin(), ordR.end(), [&](int i, int j) {
        ll ri = cir[i].x + cir[i].r;
        ll rj = cir[j].x + cir[j].r;
        if (ri != rj) return ri < rj;
        return i < j;
    });

    set<Arc, Cmp> active;
    vector<set<Arc, Cmp>::iterator> arcIt(2 * n);

    auto checkPair = [&](const Arc& a, const Arc& b) {
        if (a.id == b.id) return;

        ll xn, yn;
        int den;
        if (getTangencyKey(a.id, b.id, xn, yn, den)) {
            raw.push_back({xn, yn, den, a.id, b.id});
        }
    };

    auto insertCircle = [&](int id) {
        Arc lower{id, -1};
        Arc upper{id, +1};

        auto res = active.insert(lower);
        auto it = res.first;
        arcIt[2 * id] = it;

        if (it != active.begin()) {
            auto p = prev(it);
            checkPair(*p, *it);
        }
        auto nx = next(it);
        if (nx != active.end()) {
            checkPair(*it, *nx);
        }

        res = active.insert(upper);
        it = res.first;
        arcIt[2 * id + 1] = it;

        if (it != active.begin()) {
            auto p = prev(it);
            checkPair(*p, *it);
        }
        nx = next(it);
        if (nx != active.end()) {
            checkPair(*it, *nx);
        }
    };

    auto removeCircle = [&](int id) {
        int lo = 2 * id;
        int up = 2 * id + 1;

        auto it = arcIt[lo];
        auto nx = active.erase(it);

        if (nx != active.end()) {
            auto p = (nx == active.begin() ? active.end() : prev(nx));
            if (p != active.end()) checkPair(*p, *nx);
        }

        it = arcIt[up];
        nx = active.erase(it);

        if (nx != active.end()) {
            auto p = (nx == active.begin() ? active.end() : prev(nx));
            if (p != active.end()) checkPair(*p, *nx);
        }
    };

    size_t pl = 0, pr = 0;

    vector<int> ins, rem, all;
    ins.reserve(n);
    rem.reserve(n);
    all.reserve(n);

    while (pl < n || pr < n) {
        ll lx = (pl < n ? cir[ordL[pl]].x - cir[ordL[pl]].r : LLONG_MAX);
        ll rx = (pr < n ? cir[ordR[pr]].x + cir[ordR[pr]].r : LLONG_MAX);
        ll cur = min(lx, rx);

        sweepX = cur;

        ins.clear();
        rem.clear();

        while (pl < n && cir[ordL[pl]].x - cir[ordL[pl]].r == cur) {
            ins.push_back(ordL[pl++]);
        }

        while (pr < n && cir[ordR[pr]].x + cir[ordR[pr]].r == cur) {
            rem.push_back(ordR[pr++]);
        }

        all.clear();
        all.insert(all.end(), ins.begin(), ins.end());
        all.insert(all.end(), rem.begin(), rem.end());

        sort(all.begin(), all.end(), [&](int i, int j) {
            if (cir[i].y != cir[j].y) return cir[i].y < cir[j].y;
            return i < j;
        });

        for (size_t p = 0; p < all.size();) {
            size_t q = p + 1;
            while (q < all.size() && cir[all[q]].y == cir[all[p]].y) q++;

            if (q - p >= 2) {
                int first = all[p];
                ll ykey = cir[first].y;
                for (size_t t = p + 1; t < q; t++) {
                    raw.push_back({cur, ykey, 1, first, all[t]});
                }
            }

            p = q;
        }

        for (int id : rem) removeCircle(id);
        for (int id : ins) insertCircle(id);
    }

    active.clear();
    vector<set<Arc, Cmp>::iterator>().swap(arcIt);
    vector<int>().swap(ordL);
    vector<int>().swap(ordR);
    vector<int>().swap(ins);
    vector<int>().swap(rem);
    vector<int>().swap(all);

    sort(raw.begin(), raw.end(), [](const RawContact& a, const RawContact& b) {
        if (a.xn != b.xn) return a.xn < b.xn;
        if (a.yn != b.yn) return a.yn < b.yn;
        return a.den < b.den;
    });

    int nextNode = 2; // 0 = S, 1 = T
    vector<int> mark(n, -1);
    int stamp = 0;

    for (size_t l = 0; l < raw.size();) {
        size_t r = l + 1;
        while (r < raw.size() &&
               raw[r].xn == raw[l].xn &&
               raw[r].yn == raw[l].yn &&
               raw[r].den == raw[l].den) {
            r++;
        }

        bool isS = (raw[l].den == 1 && raw[l].xn == Sx && raw[l].yn == Sy);
        bool isT = (raw[l].den == 1 && raw[l].xn == Tx && raw[l].yn == Ty);

        if (!isS && !isT) {
            int node = nextNode++;
            ++stamp;

            ld px = (ld)raw[l].xn / (ld)raw[l].den;
            ld py = (ld)raw[l].yn / (ld)raw[l].den;

            for (size_t e = l; e < r; e++) {
                int ids[2] = {raw[e].a, raw[e].b};

                for (int id : ids) {
                    if (mark[id] == stamp) continue;
                    mark[id] = stamp;

                    ld dx = px - cir[id].x;
                    ld dy = py - cir[id].y;

                    double ang = atan2((double)dy, (double)dx);
                    if (ang < 0) ang += TWO_PI;

                    incVec[id].push_back({ang, node});
                }
            }
        }

        l = r;
    }

    vector<RawContact>().swap(raw);
    vector<int>().swap(mark);

    // Add S and T incidences.
    for (int i = 0; i < n; i++) {
        {
            __int128 dx = (__int128)Sx - cir[i].x;
            __int128 dy = (__int128)Sy - cir[i].y;
            if (dx * dx + dy * dy == (__int128)cir[i].r * cir[i].r) {
                double ang = atan2((double)(Sy - cir[i].y), (double)(Sx - cir[i].x));
                if (ang < 0) ang += TWO_PI;
                incVec[i].push_back({ang, 0});
            }
        }

        {
            __int128 dx = (__int128)Tx - cir[i].x;
            __int128 dy = (__int128)Ty - cir[i].y;
            if (dx * dx + dy * dy == (__int128)cir[i].r * cir[i].r) {
                double ang = atan2((double)(Ty - cir[i].y), (double)(Tx - cir[i].x));
                if (ang < 0) ang += TWO_PI;
                incVec[i].push_back({ang, 1});
            }
        }
    }

    for (int i = 0; i < n; i++) {
        sort(incVec[i].begin(), incVec[i].end(), [](const Incidence& a, const Incidence& b) {
            return a.angle < b.angle;
        });
    }

    int N = nextNode;

    struct ContactIncidence {
        int circle;
        int pos;
        int next;
    };

    vector<int> head(N, -1);

    size_t totalIncidences = 0;
    for (const auto& v : incVec) totalIncidences += v.size();

    vector<ContactIncidence> ci;
    ci.reserve(totalIncidences);

    for (int c = 0; c < n; c++) {
        auto& v = incVec[c];

        for (int pos = 0; pos < (int)v.size(); pos++) {
            int node = v[pos].node;

            ci.push_back({c, pos, head[node]});
            head[node] = (int)ci.size() - 1;
        }
    }

    const double INF = 1e100;

    vector<double> dist(N, INF);
    priority_queue<pair<double, int>,
                   vector<pair<double, int>>,
                   greater<pair<double, int>>> pq;

    dist[0] = 0.0;
    pq.push({0.0, 0});

    while (!pq.empty()) {
        auto [d, v] = pq.top();
        pq.pop();

        if (d != dist[v]) continue;
        if (v == 1) break;

        for (int idx = head[v]; idx != -1; idx = ci[idx].next) {
            int c = ci[idx].circle;
            int pos = ci[idx].pos;

            auto& vec = incVec[c];
            int k = (int)vec.size();
            if (k <= 1) continue;

            double coeff = (double)cir[c].r * (double)cir[c].v;
            double curAng = vec[pos].angle;

            int nxt = (pos + 1 == k ? 0 : pos + 1);
            double deltaNext = (nxt > pos)
                ? (vec[nxt].angle - curAng)
                : (vec[nxt].angle + TWO_PI - curAng);

            double nd = d + deltaNext * coeff;
            int to = vec[nxt].node;

            if (nd < dist[to]) {
                dist[to] = nd;
                pq.push({nd, to});
            }

            int prv = (pos == 0 ? k - 1 : pos - 1);
            double deltaPrev = (pos > prv)
                ? (curAng - vec[prv].angle)
                : (curAng + TWO_PI - vec[prv].angle);

            nd = d + deltaPrev * coeff;
            to = vec[prv].node;

            if (nd < dist[to]) {
                dist[to] = nd;
                pq.push({nd, to});
            }
        }
    }

    double ans = dist[1];

    if (ans >= INF / 2) {
        cout << -1 << '\n';
    } else {
        cout << setprecision(15) << ans << '\n';
    }

    return 0;
}
