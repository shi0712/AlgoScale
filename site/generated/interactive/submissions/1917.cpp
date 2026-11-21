#include <bits/stdc++.h>
using namespace std;

using U = uint64_t;

struct Constraint {
    U state;
    int cells, count;
};

int n, m, allCells, visible;
int L[55], R[55], cover[55];
U incident[10], parityMask, known, initialOn;
unordered_set<U> impossible[10];
vector<Constraint> pending;

U bit(int i) { return U(1) << i; }

void fixBits(U mask, U values) {
    initialOn = (initialOn & ~mask) | (values & mask);
    known |= mask;
}

// Status of a cell at an earlier (or current) flip parity.
int status(int c, U state) {
    U s = state & incident[c];
    if ((s ^ initialOn) & known & incident[c]) return 0;
    if (!(incident[c] & ~known)) return 1;
    if (impossible[c].count(s)) return 0;
    return -1;
}

void record(U state, int cells, int count) {
    if (count != 0 && count != __builtin_popcount((unsigned)cells)) {
        pending.push_back({state, cells, count});
        return;
    }
    while (cells) {
        int c = __builtin_ctz((unsigned)cells);
        cells &= cells - 1;
        if (count) fixBits(incident[c], state);
        else impossible[c].insert(state & incident[c]);
    }
}

void propagate() {
    bool changed;
    do {
        changed = false;
        size_t w = 0;
        for (auto q : pending) {
            int undecided = 0, rem = q.count;
            for (int t = q.cells; t; t &= t - 1) {
                int c = __builtin_ctz((unsigned)t);
                int s = status(c, q.state);
                if (s < 0) undecided |= 1 << c;
                else rem -= s;
            }
            if (!undecided) continue;
            if (!rem || rem == __builtin_popcount((unsigned)undecided)) {
                record(q.state, undecided, rem);
                changed = true;
            } else {
                pending[w++] = {q.state, undecided, rem};
            }
        }
        pending.resize(w);
    } while (changed);
}

int flip(int b) {
    cout << L[b]+1 << ' ' << R[b]+1 << endl;
    int v;
    if (!(cin >> v) || v < 0 || v == n) exit(0);
    U old = parityMask;
    parityMask ^= bit(b);
    int d = v - visible;
    if (d != 0)
        fixBits(bit(b), d > 0 ? parityMask : old);
    // On the active side of the flip the whole interval is dark.
    record(old, cover[b], max(-d, 0));
    record(parityMask, cover[b], max(d, 0));
    record(parityMask, allCells ^ cover[b], v - max(d, 0));
    visible = v;
    propagate();
    return d;
}

vector<U> route;
int ptrRoute, target;
U planKnown;

// Plan a Gray-code search over just the barriers at one cell.
// Discard signatures already ruled out, shortcutting the discarded steps.
void plan() {
    double best = 1e100;
    int minK = 60;
    for (int c = 0; c < n; c++)
        if (incident[c] & ~known)
            minK = min(minK, __builtin_popcountll(incident[c] & ~known));

    for (int c = 0; c < n; c++) {
        U freeBits = incident[c] & ~known;
        int k = __builtin_popcountll(freeBits);
        if (!k || k > minK + 1) continue;
        vector<int> ids;
        for (U x = freeBits; x; x &= x-1)
            ids.push_back(__builtin_ctzll(x));
        sort(ids.begin(), ids.end(), [](int a, int b) {
            return R[a]-L[a] > R[b]-L[b];
        });

        U start = parityMask & incident[c];
        U s = start, prev = start, bothOne = freeBits, anyOne = 0;
        vector<U> p;
        double total = 0;
        int dist = 0;
        for (int t = 1; t < (1<<k); t++) {
            s ^= bit(ids[__builtin_ctz((unsigned)t)]);
            if (impossible[c].count(s)) continue;
            bothOne &= s;
            anyOne |= s;
            dist += __builtin_popcountll(s ^ prev);
            prev = s;
            total += dist;
            p.push_back(s);
        }
        if (p.empty()) continue;
        U forced = freeBits & (bothOne | ~anyOne);
        if (forced) {
            fixBits(forced, bothOne);
            propagate();
            route.clear();
            return;
        }
        double cost = total / p.size();
        // Prefer first moves which also test signatures at other cells.
        int first = __builtin_ctzll(p[0] ^ start);
        if (cost - 0.015*(R[first]-L[first]) < best) {
            best = cost - 0.015*(R[first]-L[first]);
            route = move(p);
            target = c;
        }
    }
    ptrRoute = 0;
    planKnown = known;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    if (!(cin >> n >> visible) || visible < 0 || visible == n) return 0;
    allCells = (1<<n)-1;
    for (int i=0; i<n; i++) for (int j=i; j<n; j++) {
        L[m]=i; R[m]=j;
        for (int c=i; c<=j; c++) {
            incident[c] |= bit(m);
            cover[m] |= 1<<c;
        }
        m++;
    }
    record(0, allCells, visible);
    propagate();

    for (;;) {
        U active = (parityMask ^ initialOn) & known;
        if (active) {
            int b = __builtin_ctzll(active);
            flip(b);
            route.clear();
            continue;
        }
        int lit=0, unsure=0;
        for (int c=0; c<n; c++) {
            int s=status(c, parityMask);
            if (s==1) lit |= 1<<c;
            if (s<0) unsure |= 1<<c;
        }
        if (visible > __builtin_popcount((unsigned)lit)) {
            // Localize a visible cell without disturbing certified visible
            // cells. A singleton is always available; an unknown longer
            // interval can additionally uncover cells if it was active.
            double best=-1e20;
            int choice=-1;
            for (int b=0; b<m; b++) {
                if ((cover[b]&lit) ||
                    __builtin_popcount((unsigned)(cover[b]&unsure))!=1)
                    continue;
                double score = (known & bit(b)) ? -20 : 0;
                score += 0.1*(R[b]-L[b]);
                int c=__builtin_ctz((unsigned)(cover[b]&unsure));
                score += 0.01*__builtin_popcountll(incident[c]&~known);
                if (score>best) best=score, choice=b;
            }
            // If the one uncertain cell disappears, it was visible before
            // the flip (all other covered cells were known dark).
            U old=parityMask;
            int c=__builtin_ctz((unsigned)(cover[choice]&unsure));
            if (flip(choice)<0) {
                fixBits(incident[c], old);
                propagate();
            }
            route.clear();
            continue;
        }

        // Every uncertified cell is currently dark.
        record(parityMask, allCells ^ lit, 0);
        if (route.empty() || known!=planKnown) {
            route.clear();
            plan();
            if (route.empty()) continue;
        }
        while (ptrRoute<(int)route.size() &&
               impossible[target].count(route[ptrRoute])) ++ptrRoute;
        if (ptrRoute==(int)route.size()) {
            route.clear();
            continue;
        }
        U diff=(parityMask ^ route[ptrRoute]) & incident[target];
        if (!diff) {
            ++ptrRoute;
            continue;
        }
        int b=__builtin_ctzll(diff);
        for (U x=diff; x; x&=x-1) {
            int a=__builtin_ctzll(x);
            if (R[a]-L[a]>R[b]-L[b]) b=a;
        }
        if (flip(b)!=0) route.clear();
    }
}
