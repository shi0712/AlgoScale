#include <bits/stdc++.h>
using namespace std;

static int n;
static unsigned long long fullmask;
static long long action_count = 0;

static int readK() {
    int x;
    if (scanf("%d", &x) != 1) exit(0);
    return x;
}

static void doAction(int wheel, long long shift) {
    // wheel is 0-based
    printf("%d %lld\n", wheel + 1, shift);
    fflush(stdout);
    ++action_count;
}

static bool isPeriodic(unsigned long long S) {
    for (int h = 1; h < n; ++h) {
        unsigned long long r = ((S << h) | (S >> (n - h))) & fullmask;
        if (r == S) return true;
    }
    return false;
}

static int popcnt(unsigned long long x) {
    return __builtin_popcountll(x);
}

static int firstEmpty(unsigned long long S) {
    for (int i = 0; i < n; ++i)
        if (((S >> i) & 1ULL) == 0) return i;
    return -1;
}

struct Hyp {
    int q;
    int f;          // 1 if wheel is unique at its current position
    int base;       // number of occupied positions among the other wheels
    unsigned long long emptyMask; // positions empty among the other wheels
};

int main() {
    if (scanf("%d", &n) != 1) return 0;
    int k = readK();
    if (k == 1) return 0;

    fullmask = (n == 64 ? ~0ULL : ((1ULL << n) - 1ULL));

    // ------------------------------------------------------------
    // Initial full scan of wheel 0.  Its original position is our
    // coordinate origin 0.
    // ------------------------------------------------------------
    vector<int> kval(n);
    kval[0] = k;
    for (int t = 1; t <= n; ++t) {
        doAction(0, 1);
        k = readK();
        if (k == 1) return 0;
        if (t < n) kval[t] = k;
    }
    // Wheel 0 is back at position 0.
    int minK = *min_element(kval.begin(), kval.end());
    unsigned long long Scur = 1ULL << 0; // wheel 0 itself
    for (int t = 0; t < n; ++t)
        if (kval[t] == minK) Scur |= (1ULL << t);

    // Also keep the currently known distinct count.
    int curK = popcnt(Scur); // equals k of the current real configuration
    // ------------------------------------------------------------
    // Make the occupied set aperiodic and leave at least one empty
    // position.  Wheel 0 is moved if necessary; T is its position.
    // ------------------------------------------------------------
    int T = 0;                 // current position of wheel 0
    while (isPeriodic(Scur)) {
        if (popcnt(Scur) == n) {
            // Full set: move wheel 0 by one step to an occupied value.
            int old = T;
            int np = (old + 1) % n;
            doAction(0, 1);
            k = readK();
            if (k == 1) return 0;
            Scur &= ~(1ULL << old);
            // np is occupied by another wheel, so no new bit is needed.
            T = np;
            curK = k;
        } else {
            int e = firstEmpty(Scur);
            int old = T;
            int shift = (e - old + n) % n;
            doAction(0, shift);
            k = readK();
            if (k == 1) return 0;
            if (k == curK) {
                // Wheel 0 was unique at its old position.
                Scur &= ~(1ULL << old);
            }
            Scur |= (1ULL << e);
            T = e;
            curK = k;
        }
    }

    vector<pair<int,int>> displaced; // (wheel, its current position)

    // ------------------------------------------------------------
    // Process wheels 1..n-1 (0-based).
    // ------------------------------------------------------------
    for (int i = 1; i < n; ++i) {
        // Keep Scur aperiodic.  If it became periodic again, temporarily
        // move the previously processed wheel away from T.
        if (isPeriodic(Scur)) {
            int e = firstEmpty(Scur);
            if (e != -1) {
                int w = i - 1;
                if (w >= 1) {
                    int shift = (e - T + n) % n;
                    doAction(w, shift);
                    k = readK();
                    if (k == 1) return 0;
                    // T is still occupied by wheel 0, so only e is added.
                    Scur |= (1ULL << e);
                    displaced.push_back({w, e});
                } else {
                    // Should not happen: before processing the second wheel
                    // Scur is aperiodic.  Handle it safely anyway.
                    int old = T;
                    int sh = (e - old + n) % n;
                    doAction(0, sh);
                    k = readK();
                    if (k == 1) return 0;
                    if (k == curK) Scur &= ~(1ULL << old);
                    Scur |= (1ULL << e);
                    T = e;
                    curK = k;
                    // If it is still periodic, fall through; the setup loop
                    // above is not repeated here, but this situation cannot
                    // occur with a correct state.
                }
            }
        }

        int Dcur = popcnt(Scur);

        vector<Hyp> hyps;
        hyps.reserve(2 * n);
        for (int q = 0; q < n; ++q) {
            if (((Scur >> q) & 1ULL) == 0) continue;
            // f = 0: another wheel occupies q as well
            {
                Hyp h;
                h.q = q; h.f = 0;
                h.base = Dcur;
                h.emptyMask = fullmask ^ Scur;
                hyps.push_back(h);
            }
            // f = 1: this wheel is the only one at q
            {
                Hyp h;
                h.q = q; h.f = 1;
                h.base = Dcur - 1;
                h.emptyMask = (fullmask ^ Scur) | (1ULL << q);
                hyps.push_back(h);
            }
        }

        int t = 0; // total shift applied so far, modulo n
        int guard = 0;
        while (hyps.size() > 1 && guard < 4 * n + 100) {
            ++guard;
            int bestR = 1;
            int bestMax = INT_MAX;
            long long bestSq = LLONG_MAX;

            for (int r = 1; r < n; ++r) {
                int cnt[3] = {0, 0, 0};
                for (const Hyp &h : hyps) {
                    int y = (h.q + t + r) % n;
                    int pred = h.base + (((h.emptyMask >> y) & 1ULL) ? 1 : 0);
                    ++cnt[pred - (Dcur - 1)];
                }
                int mx = 0;
                long long sq = 0;
                for (int z = 0; z < 3; ++z) {
                    mx = max(mx, cnt[z]);
                    sq += 1LL * cnt[z] * cnt[z];
                }
                if (mx < bestMax || (mx == bestMax && sq < bestSq)) {
                    bestMax = mx;
                    bestSq = sq;
                    bestR = r;
                }
            }

            // With an aperiodic Scur, some move always splits the set.
            if (bestMax == (int)hyps.size()) {
                // Defensive fallback; should be unreachable.
                bestR = 1;
            }

            doAction(i, bestR);
            k = readK();
            if (k == 1) return 0;

            t = (t + bestR) % n;

            vector<Hyp> nh;
            nh.reserve(hyps.size());
            for (const Hyp &h : hyps) {
                int y = (h.q + t) % n;
                int pred = h.base + (((h.emptyMask >> y) & 1ULL) ? 1 : 0);
                if (pred == k) nh.push_back(h);
            }
            hyps.swap(nh);
            if (hyps.empty()) {
                // Should never happen.  Avoid an infinite loop / invalid access.
                // Restore a single plausible hypothesis.
                int q = -1;
                for (int z = 0; z < n; ++z) if ((Scur >> z) & 1ULL) { q = z; break; }
                if (q == -1) q = 0;
                Hyp h; h.q = q; h.f = 0; h.base = Dcur;
                h.emptyMask = fullmask ^ Scur;
                hyps.push_back(h);
                break;
            }
        }

        int q = hyps[0].q;
        int f = hyps[0].f;
        int cur = (q + t) % n;
        if (cur != T) {
            int shift = (T - cur + n) % n;
            doAction(i, shift);
            k = readK();
            if (k == 1) return 0;
        }

        if (f == 1 && q != T) {
            Scur &= ~(1ULL << q);
        }
        Scur |= (1ULL << T);
    }

    // ------------------------------------------------------------
    // Put all temporarily displaced wheels back onto T.
    // ------------------------------------------------------------
    for (auto &pr : displaced) {
        int w = pr.first;
        int pos = pr.second;
        if (pos != T) {
            int shift = (T - pos + n) % n;
            doAction(w, shift);
            k = readK();
            if (k == 1) return 0;
        }
    }

    return 0;
}
