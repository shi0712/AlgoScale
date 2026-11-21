#include <bits/stdc++.h>
using namespace std;

#if defined(__x86_64__)
#pragma GCC target("popcnt")
#endif

using U = uint64_t;
constexpr int LIM = 1000000000;
constexpr int WORDS = LIM / 64 + 1;
constexpr int GROUPS = (WORDS + 63) / 64;

static U bits[WORDS], active[GROUPS];
static int pref[GROUPS + 1];
static mt19937 rng(7620193);

static int pc(U x) { return __builtin_popcountll(x); }
static int lowbit(U x) { return __builtin_ctzll(x); }

static void initialize(int k) {
    U masks[32]{};
    for (int i = 0; i < 64; ++i) masks[pc(i)] |= U(1) << i;
    for (int g = 0; g < GROUPS; ++g) {
        int sum = 0;
        for (int j = 0; j < 64 && 64*g+j < WORDS; ++j) {
            int w = 64*g+j, need = k-pc(w);
            U v = (need >= 0 && need <= 6) ? masks[need] : 0;
            if (w == WORDS-1) v &= (U(2) << (LIM%64))-1;
            bits[w] = v;
            if (v) active[g] |= U(1) << j;
            sum += pc(v);
        }
        pref[g+1] = pref[g]+sum;
    }
}

// Keep original numbers y satisfying popcount(y-t) == response.
static void restrict_to(int t, int response) {
    U mask[2][32]{};
    int a = t >> 6, b = t & 63;
    for (int i = 0; i < 64; ++i) {
        int need = response-pc((i-b+64)&63);
        if (need >= 0) mask[i<b][need] |= U(1) << i;
    }
    for (int g = 0; g < GROUPS; ++g) {
        U rem = active[g], next = 0;
        int sum = 0;
        while (rem) {
            int j = lowbit(rem), w = 64*g+j;
            U flag = rem & -rem;
            rem -= flag;
            int d = w-a;
            U m = 0;
            if (d >= 0) m = mask[0][pc(d)];
            if (d > 0) m |= mask[1][pc(d-1)];
            U v = bits[w] &= m;
            if (v) next |= flag;
            sum += pc(v);
        }
        active[g] = next;
        pref[g+1] = pref[g]+sum;
    }
}

static int at_rank(int r) {
    int g = int(upper_bound(pref, pref+GROUPS+1, r)-pref)-1;
    r -= pref[g];
    U rem = active[g];
    for (;;) {
        int w = 64*g+lowbit(rem);
        U v = bits[w];
        int c = pc(v);
        if (r < c) {
            while (r--) v &= v-1;
            return 64*w+lowbit(v);
        }
        r -= c;
        rem &= rem-1;
    }
}

static int choose_query(int last, int total) {
    int lo = last+1, hi = at_rank(0);
    if (lo == hi || total == 2) return hi;

    int N = min(1536, total);
    vector<int> sample(N);
    for (int i = 0; i < N; ++i) {
        int l = (long long)i*total/N, r = (long long)(i+1)*total/N;
        int rank = l + int(rng() % (r-l));
        if (i == 0) rank = 0;
        if (i == N-1) rank = total-1;
        sample[i] = at_rank(rank);
    }

    // Estimate the remaining work after each possible response.
    vector<double> cost(N+1);
    for (int i = 2; i <= N; ++i) {
        double h = log2(double(i));
        cost[i] = i * (N <= 96 ? max(1.0, 0.46*h) + 0.018*h : h);
    }

    unordered_set<int> seen;
    seen.reserve(16000);
    using Entry = pair<double,int>;
    vector<Entry> elite;
    int best = hi;
    double value = 1e100;
    auto test = [&](long long z) {
        if (z < lo || z > hi) return;
        int t = int(z);
        if (!seen.insert(t).second) return;
        int hist[32]{};
        for (int y : sample) ++hist[__builtin_popcount(unsigned(y-t))];
        double s = 0;
        for (int c : hist) s += cost[c];
        if (s < value || (s == value && t < best)) {
            value = s;
            best = t;
        }
        Entry e{s,t};
        if (elite.size() < 10 || e < elite.back()) {
            elite.insert(lower_bound(elite.begin(), elite.end(), e), e);
            if (elite.size() > 10) elite.pop_back();
        }
    };
    test(lo);
    test(hi);
    if (hi-lo <= 2500) {
        for (int t = lo+1; t < hi && value != 0; ++t) test(t);
        return best;
    }

    // Try boundaries where a borrow can propagate through many bits,
    // as well as queries giving several independent carry chains.
    for (int j = 0; j <= 30; ++j) {
        long long p = 1LL<<j, m = p-1;
        test(p); test(p-1);
        test(lo+p); test(hi-p);
        for (int k = 0; k < 18; ++k) {
            long long y = k == 0 ? hi : sample[rng()%N];
            long long r = (k == 1 ? 0 : (y + (k%4 != 2)) & m);
            test(lo + ((r-lo)&m));
            test(hi - ((hi-r)&m));
        }
    }
    int span = hi-lo+1;
    int trials = N <= 100 ? 7200 : 1600;
    for (int i = 0; i < trials && value != 0; ++i) {
        int t = lo + int(rng()%span);
        int j = rng()%30;
        int m = (1<<j)-1;
        switch (i%7) {
            case 0: t &= ~m; break;
            case 1: t = (t&~m) | ((sample[rng()%N]+1)&m); break;
            case 2: t = lo+int(rng()%min(span,m+1)); break;
            case 3: t = hi-int(rng()%min(span,m+1)); break;
            default: break;
        }
        test(t);
    }
    for (int round = 0; round < 6 && value != 0; ++round) {
        auto old = elite;
        for (auto [s,t] : old) {
            for (int j = 0; j < 30; ++j) {
                int p = 1<<j;
                test(t^p);
                test(t+p);
                test(t-p);
                test((t & -p) | ((sample[rng()%N]+1)&(p-1)));
            }
        }
        if (old == elite && round >= 2) break;
    }
    return best;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int k;
    if (!(cin >> k) || k < 1 || k > 30) return 0;
    initialize(k);
    int last = 0;
    while (pref[GROUPS] > 1) {
        int t = choose_query(last, pref[GROUPS]);
        cout << "- " << t-last << endl;
        int response;
        if (!(cin >> response) || response < 0 || response > 30) return 0;
        if (response == 0) {
            cout << "! " << t << endl;
            return 0;
        }
        restrict_to(t, response);
        last = t;
    }
    if (pref[GROUPS] == 1) cout << "! " << at_rank(0) << endl;
    return 0;
}
