#include <bits/stdc++.h>
#include <boost/multiprecision/cpp_int.hpp>

using namespace std;
using boost::multiprecision::int256_t;

struct Pos {
    long long ax, ay;
    int node;
};

struct Arc {
    int c;
    int id;
    int tie;
};

struct Event {
    long long num, den;
    int type; // 0 = end, 1 = start, 2 = tangency
    int a, b;
};

static int N;
static vector<int> X, Y, R, V;
static long long globP, globQ;
static vector<vector<Pos>> circPos;
static vector<pair<int,int>> zeroEdges;
static int posCount = 0;

static set<Arc, struct ArcCmp> *dummySet;
struct ArcCmp {
    bool operator()(const Arc &A, const Arc &B) const;
};

static inline long long llabs_safe(long long x) {
    return x >= 0 ? x : -x;
}

static int sgn256(const int256_t &x) {
    if (x > 0) return 1;
    if (x < 0) return -1;
    return 0;
}

// sign of A + C * sqrt(N)
static int sign_A_C_sqrt(const int256_t &A, const int256_t &C, const int256_t &N) {
    if (C == 0) return sgn256(A);
    if (A == 0) {
        if (N == 0) return 0;
        return C > 0 ? 1 : -1;
    }
    if (A > 0) {
        if (C > 0) return 1;
        int256_t D = A * A - C * C * N;
        return sgn256(D);
    } else {
        if (C < 0) return -1;
        int256_t D = C * C * N - A * A;
        return sgn256(D);
    }
}

// sign of L^2 - M, where L = B + s*sqrt(N), s in {-1,+1}
static int sign_L2_minus_M(long long B, int s, __int128 N128, __int128 M128) {
    int256_t Nn = (int256_t)N128;
    int256_t Mm = (int256_t)M128;
    int256_t A = (int256_t)B * B + Nn - Mm;
    int256_t C = (int256_t)2 * B * s;
    return sign_A_C_sqrt(A, C, Nn);
}

// sign of B + sA*sqrt(N) - sB*sqrt(M)
static int exact_y_sign(long long B, int sA, __int128 N128, int sB, __int128 M128) {
    int ls = sign_A_C_sqrt((int256_t)B, (int256_t)sA, (int256_t)N128);

    if (sB == 0) return ls;

    if (ls > 0) {
        if (sB > 0) return sign_L2_minus_M(B, sA, N128, M128);
        else return 1;
    } else if (ls < 0) {
        if (sB > 0) return -1;
        else {
            int s = sign_L2_minus_M(B, sA, N128, M128);
            if (s < 0) return 1;
            if (s == 0) return 0;
            return -1;
        }
    } else {
        if (M128 == 0) return 0;
        return sB > 0 ? -1 : 1;
    }
}

bool ArcCmp::operator()(const Arc &A, const Arc &B) const {
    if (A.id == B.id) return false;

    long double x = (long double)globP / (long double)globQ;

    auto evalY = [&](const Arc &a) -> long double {
        int c = a.c;
        long double dx = x - (long double)X[c];
        long double inside = (long double)R[c] * R[c] - dx * dx;
        if (inside < 0) inside = 0;
        int s = (a.id & 1) ? 1 : -1;
        return (long double)Y[c] + s * sqrtl(inside);
    };

    long double yA = evalY(A);
    long double yB = evalY(B);
    long double diff = yA - yB;

    // Fast path. Near-equal cases are decided exactly below.
    if (fabsl(diff) > 1e-8L) return diff < 0;

    auto getBaseN = [&](const Arc &a, long long &base, __int128 &NN) {
        int c = a.c;
        base = (long long)Y[c] * globQ;
        long long dxq = globP - (long long)X[c] * globQ;
        NN = (__int128)R[c] * R[c] * globQ * globQ - (__int128)dxq * dxq;
        if (NN < 0) NN = 0;
    };

    long long baseA, baseB;
    __int128 NA, NB;
    getBaseN(A, baseA, NA);
    getBaseN(B, baseB, NB);

    int sA = (A.id & 1) ? 1 : -1;
    int sB = (B.id & 1) ? 1 : -1;

    int sg = exact_y_sign(baseA - baseB, sA, NA, sB, NB);
    if (sg != 0) return sg < 0;

    if (A.tie != B.tie) return A.tie < B.tie;
    return A.id < B.id;
}

static set<Arc, ArcCmp> statusSet;
static vector<set<Arc, ArcCmp>::iterator> posIt;
static vector<char> activeArc;
static unordered_set<unsigned long long> scheduledArc;
static unordered_set<unsigned long long> processedCircle;
static priority_queue<Event, vector<Event>, struct EventCmp> pqSweep;

struct EventCmp {
    bool operator()(const Event &a, const Event &b) const {
        __int128 lhs = (__int128)a.num * b.den;
        __int128 rhs = (__int128)b.num * a.den;
        if (lhs != rhs) return lhs > rhs; // min-heap
        return false;
    }
};

static int addPos(int c, long long ax, long long ay) {
    int id = posCount++;
    circPos[c].push_back({ax, ay, id});
    return id;
}

static bool getTangent(int i, int j, long long &nx, long long &ny, long long &den) {
    long long dx = (long long)X[j] - X[i];
    long long dy = (long long)Y[j] - Y[i];
    __int128 d2 = (__int128)dx * dx + (__int128)dy * dy;

    long long sum = (long long)R[i] + R[j];
    if (d2 == (__int128)sum * sum) {
        den = sum;
        nx = (long long)((__int128)R[j] * X[i] + (__int128)R[i] * X[j]);
        ny = (long long)((__int128)R[j] * Y[i] + (__int128)R[i] * Y[j]);
        return true;
    }

    long long diff = llabs_safe((long long)R[i] - R[j]);
    if (diff > 0 && d2 == (__int128)diff * diff) {
        den = diff;
        if (R[i] > R[j]) {
            nx = (long long)((__int128)R[i] * X[j] - (__int128)R[j] * X[i]);
            ny = (long long)((__int128)R[i] * Y[j] - (__int128)R[j] * Y[i]);
        } else {
            nx = (long long)((__int128)R[j] * X[i] - (__int128)R[i] * X[j]);
            ny = (long long)((__int128)R[j] * Y[i] - (__int128)R[i] * Y[j]);
        }
        return true;
    }

    return false;
}

static unsigned long long arcPairKey(int a, int b) {
    if (a > b) swap(a, b);
    return ((unsigned long long)(unsigned)a << 32) | (unsigned)b;
}

static unsigned long long circlePairKey(int a, int b) {
    if (a > b) swap(a, b);
    return ((unsigned long long)(unsigned)a << 32) | (unsigned)b;
}

static void checkPair(int arcA, int arcB) {
    if (!activeArc[arcA] || !activeArc[arcB]) return;

    int i = arcA >> 1;
    int j = arcB >> 1;
    if (i == j) return;

    long long nx, ny, den;
    if (!getTangent(i, j, nx, ny, den)) return;

    long long g = std::gcd(llabs_safe(nx), den);
    long long pnum = nx / g;
    long long pden = den / g;
    if (pden < 0) {
        pden = -pden;
        pnum = -pnum;
    }

    __int128 lhs = (__int128)pnum * globQ;
    __int128 rhs = (__int128)globP * pden;
    if (lhs <= rhs) return; // only future events

    unsigned long long sak = arcPairKey(arcA, arcB);
    if (scheduledArc.find(sak) != scheduledArc.end()) return;

    scheduledArc.insert(sak);
    pqSweep.push({pnum, pden, 2, arcA, arcB});
}

static void insertArc(int c, int id, int tie) {
    auto it = statusSet.insert(Arc{c, id, tie}).first;
    posIt[id] = it;
    activeArc[id] = 1;
}

static void eraseArc(int id) {
    if (!activeArc[id]) return;

    auto it = posIt[id];
    auto itNext = it;
    ++itNext;

    bool hasPrev = (it != statusSet.begin());
    auto itPrev = it;
    if (hasPrev) --itPrev;

    bool hasNext = (itNext != statusSet.end());

    statusSet.erase(it);
    activeArc[id] = 0;

    if (hasPrev && hasNext) {
        checkPair(itPrev->id, itNext->id);
    }
}

static int halfDir(long long ax, long long ay) {
    if (ay > 0 || (ay == 0 && ax >= 0)) return 0;
    return 1;
}

static __int128 crossPos(const Pos &a, const Pos &b) {
    return (__int128)a.ax * b.ay - (__int128)a.ay * b.ax;
}

static bool sameDirection(const Pos &a, const Pos &b) {
    if (halfDir(a.ax, a.ay) != halfDir(b.ax, b.ay)) return false;
    return crossPos(a, b) == 0;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    long long Sx, Sy, Tx, Ty;
    cin >> N >> Sx >> Sy >> Tx >> Ty;

    X.resize(N);
    Y.resize(N);
    R.resize(N);
    V.resize(N);

    for (int i = 0; i < N; i++) {
        cin >> X[i] >> Y[i] >> R[i] >> V[i];
    }

    if (Sx == Tx && Sy == Ty) {
        cout << "0.00000000000000000\n";
        return 0;
    }

    circPos.assign(N, {});

    vector<int> S_nodes, T_nodes;

    for (int i = 0; i < N; i++) {
        __int128 dx = (__int128)Sx - X[i];
        __int128 dy = (__int128)Sy - Y[i];
        if (dx * dx + dy * dy == (__int128)R[i] * R[i]) {
            int id = addPos(i, Sx - X[i], Sy - Y[i]);
            S_nodes.push_back(id);
        }

        dx = (__int128)Tx - X[i];
        dy = (__int128)Ty - Y[i];
        if (dx * dx + dy * dy == (__int128)R[i] * R[i]) {
            int id = addPos(i, Tx - X[i], Ty - Y[i]);
            T_nodes.push_back(id);
        }
    }

    posIt.resize(2 * N);
    activeArc.assign(2 * N, 0);

    scheduledArc.reserve(2 * N * 2);
    processedCircle.reserve(2 * N * 2);
    scheduledArc.max_load_factor(0.7);
    processedCircle.max_load_factor(0.7);

    for (int i = 0; i < N; i++) {
        pqSweep.push({(long long)X[i] - R[i], 1, 1, i, 0}); // start
        pqSweep.push({(long long)X[i] + R[i], 1, 0, i, 0}); // end
    }

    while (!pqSweep.empty()) {
        Event first = pqSweep.top();
        long long curP = first.num;
        long long curQ = first.den;
        globP = curP;
        globQ = curQ;

        vector<Event> epEvents;
        vector<Event> tanEvents;

        while (!pqSweep.empty()) {
            Event ev = pqSweep.top();
            __int128 lhs = (__int128)ev.num * curQ;
            __int128 rhs = (__int128)curP * ev.den;
            if (lhs != rhs) break;
            pqSweep.pop();

            if (ev.type == 2) tanEvents.push_back(ev);
            else epEvents.push_back(ev);
        }

        if (!epEvents.empty()) {
            sort(epEvents.begin(), epEvents.end(), [&](const Event &a, const Event &b) {
                int ya = Y[a.a], yb = Y[b.a];
                if (ya != yb) return ya < yb;
                return a.type < b.type;
            });

            // Pass 1: create endpoint contact groups and remove ending circles.
            for (int l = 0; l < (int)epEvents.size();) {
                int r = l;
                int yy = Y[epEvents[l].a];
                while (r < (int)epEvents.size() && Y[epEvents[r].a] == yy) r++;

                vector<int> starts, ends;
                for (int k = l; k < r; k++) {
                    if (epEvents[k].type == 1) starts.push_back(epEvents[k].a);
                    else ends.push_back(epEvents[k].a);
                }

                int total = (int)starts.size() + (int)ends.size();
                if (total >= 2) {
                    vector<int> nodes;
                    nodes.reserve(total);

                    for (int c : starts) {
                        long long ax = (long long)((__int128)curP - (__int128)X[c] * curQ);
                        long long ay = (long long)((__int128)yy * curQ - (__int128)Y[c] * curQ);
                        nodes.push_back(addPos(c, ax, ay));
                    }
                    for (int c : ends) {
                        long long ax = (long long)((__int128)curP - (__int128)X[c] * curQ);
                        long long ay = (long long)((__int128)yy * curQ - (__int128)Y[c] * curQ);
                        nodes.push_back(addPos(c, ax, ay));
                    }

                    for (int k = 1; k < (int)nodes.size(); k++) {
                        zeroEdges.push_back({nodes[k - 1], nodes[k]});
                    }
                }

                for (int c : ends) {
                    eraseArc(2 * c);
                    eraseArc(2 * c + 1);
                }

                l = r;
            }

            // Pass 2: insert starting circles in the correct vertical order.
            vector<int> insertedArcs;
            insertedArcs.reserve(epEvents.size() * 2);

            for (int l = 0; l < (int)epEvents.size();) {
                int r = l;
                int yy = Y[epEvents[l].a];
                while (r < (int)epEvents.size() && Y[epEvents[r].a] == yy) r++;

                vector<int> starts;
                for (int k = l; k < r; k++) {
                    if (epEvents[k].type == 1) starts.push_back(epEvents[k].a);
                }

                if (!starts.empty()) {
                    vector<int> desc = starts;
                    sort(desc.begin(), desc.end(), [&](int a, int b) {
                        if (R[a] != R[b]) return R[a] > R[b];
                        return a < b;
                    });

                    vector<int> asc = starts;
                    sort(asc.begin(), asc.end(), [&](int a, int b) {
                        if (R[a] != R[b]) return R[a] < R[b];
                        return a < b;
                    });

                    int rank = 0;

                    // lower arcs: larger radius is lower
                    for (int c : desc) {
                        int id = 2 * c;
                        insertArc(c, id, rank++);
                        insertedArcs.push_back(id);
                    }

                    // upper arcs: smaller radius is lower
                    for (int c : asc) {
                        int id = 2 * c + 1;
                        insertArc(c, id, rank++);
                        insertedArcs.push_back(id);
                    }
                }

                l = r;
            }

            for (int id : insertedArcs) {
                auto it = posIt[id];

                if (it != statusSet.begin()) {
                    auto itPrev = it;
                    --itPrev;
                    checkPair(itPrev->id, id);
                }

                auto itNext = it;
                ++itNext;
                if (itNext != statusSet.end()) {
                    checkPair(id, itNext->id);
                }
            }
        }

        for (const Event &ev : tanEvents) {
            int arcA = ev.a;
            int arcB = ev.b;

            int i = arcA >> 1;
            int j = arcB >> 1;
            if (i == j) continue;

            unsigned long long ckey = circlePairKey(i, j);
            unsigned long long akey = arcPairKey(arcA, arcB);

            if (processedCircle.find(ckey) != processedCircle.end()) {
                scheduledArc.erase(akey);
                continue;
            }

            if (!activeArc[arcA] || !activeArc[arcB]) {
                scheduledArc.erase(akey);
                continue;
            }

            auto itA = posIt[arcA];
            auto itB = posIt[arcB];

            bool adjacent = false;

            auto itNext = itA;
            ++itNext;
            if (itNext == itB) adjacent = true;

            itNext = itB;
            ++itNext;
            if (itNext == itA) adjacent = true;

            if (!adjacent) {
                scheduledArc.erase(akey);
                continue;
            }

            long long nx, ny, den;
            if (!getTangent(i, j, nx, ny, den)) {
                scheduledArc.erase(akey);
                continue;
            }

            long long ax1 = (long long)((__int128)nx - (__int128)X[i] * den);
            long long ay1 = (long long)((__int128)ny - (__int128)Y[i] * den);
            long long ax2 = (long long)((__int128)nx - (__int128)X[j] * den);
            long long ay2 = (long long)((__int128)ny - (__int128)Y[j] * den);

            int id1 = addPos(i, ax1, ay1);
            int id2 = addPos(j, ax2, ay2);

            zeroEdges.push_back({id1, id2});

            processedCircle.insert(ckey);
            scheduledArc.erase(akey);
        }
    }

    int M = posCount;

    for (int c = 0; c < N; c++) {
        auto &vec = circPos[c];
        sort(vec.begin(), vec.end(), [](const Pos &a, const Pos &b) {
            int ha = halfDir(a.ax, a.ay);
            int hb = halfDir(b.ax, b.ay);
            if (ha != hb) return ha < hb;

            __int128 cr = crossPos(a, b);
            if (cr != 0) return cr > 0;
            return a.node < b.node;
        });
    }

    vector<int> deg(M, 0);

    auto countEdge = [&](int u, int v) {
        if (u == v) return;
        deg[u]++;
        deg[v]++;
    };

    for (auto [u, v] : zeroEdges) {
        countEdge(u, v);
    }

    for (int c = 0; c < N; c++) {
        auto &vec = circPos[c];
        int k = (int)vec.size();
        if (k <= 1) continue;

        for (int i = 0; i < k; i++) {
            int j = (i + 1) % k;
            countEdge(vec[i].node, vec[j].node);
        }
    }

    vector<int> head(M + 1, 0);
    for (int i = 0; i < M; i++) {
        head[i + 1] = head[i] + deg[i];
    }

    int D = head[M];
    vector<int> to(D);
    vector<double> wt(D);
    vector<int> cur = head;

    auto addGraphEdge = [&](int u, int v, double w) {
        if (u == v) return;
        int p = cur[u]++;
        to[p] = v;
        wt[p] = w;

        p = cur[v]++;
        to[p] = u;
        wt[p] = w;
    };

    for (auto [u, v] : zeroEdges) {
        addGraphEdge(u, v, 0.0);
    }

    const long double PI = acosl(-1.0L);
    const long double TWO_PI = 2.0L * PI;

    for (int c = 0; c < N; c++) {
        auto &vec = circPos[c];
        int k = (int)vec.size();
        if (k <= 1) continue;

        vector<long double> ang(k);
        for (int i = 0; i < k; i++) {
            ang[i] = atan2l((long double)vec[i].ay, (long double)vec[i].ax);
            if (ang[i] < 0) ang[i] += TWO_PI;
        }

        for (int i = 0; i < k; i++) {
            int j = (i + 1) % k;

            long double delta;

            if (sameDirection(vec[i], vec[j])) {
                delta = 0;
            } else if (i + 1 < k) {
                delta = ang[j] - ang[i];
                if (delta < 0) delta += TWO_PI;
            } else {
                delta = ang[0] + TWO_PI - ang[i];
                if (delta < 0) delta += TWO_PI;
                if (delta > TWO_PI) delta -= TWO_PI;
            }

            double w = (double)(delta * (long double)R[c] * (long double)V[c]);
            addGraphEdge(vec[i].node, vec[j].node, w);
        }
    }

    // Free memory no longer needed.
    vector<vector<Pos>>().swap(circPos);
    vector<pair<int,int>>().swap(zeroEdges);
    vector<int>().swap(deg);
    vector<int>().swap(cur);

    const double INF = 1e100;
    vector<double> dist(M, INF);

    priority_queue<pair<double,int>, vector<pair<double,int>>, greater<pair<double,int>>> pq;

    for (int id : S_nodes) {
        if (dist[id] > 0.0) {
            dist[id] = 0.0;
            pq.push({0.0, id});
        }
    }

    while (!pq.empty()) {
        auto [d, u] = pq.top();
        pq.pop();

        if (d != dist[u]) continue;

        for (int e = head[u]; e < head[u + 1]; e++) {
            int v = to[e];
            double nd = d + wt[e];
            if (nd < dist[v]) {
                dist[v] = nd;
                pq.push({nd, v});
            }
        }
    }

    double ans = INF;
    for (int id : T_nodes) {
        if (dist[id] < ans) ans = dist[id];
    }

    if (ans >= INF / 2) {
        cout << -1 << '\n';
    } else {
        cout.setf(ios::fixed);
        cout << setprecision(17) << ans << '\n';
    }

    return 0;
}
