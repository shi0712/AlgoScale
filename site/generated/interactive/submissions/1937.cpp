#include <bits/stdc++.h>
using namespace std;

using I = long long;

I ask(I l, I r) {
    cout << "xor " << l << ' ' << r << endl;
    I v;
    if (!(cin >> v) || v < 0) exit(0);
    return v;
}

[[noreturn]] void answer(I a, I b, I c) {
    cout << "ans " << a << ' ' << b << ' ' << c << endl;
    exit(0);
}

void small(int n) {
    vector<array<int, 3>> cand;
    for (int a = 1; a <= n; ++a)
        for (int b = a + 1; b <= n; ++b)
            for (int c = b + 1; c <= n; ++c)
                cand.push_back({a, b, c});

    auto value = [](const array<int, 3>& t, int l, int r) {
        int v = 0;
        for (int x : t) if (l <= x && x <= r) v ^= x;
        return v;
    };
    while (cand.size() > 1) {
        int bl = 1, br = 1;
        pair<I,int> best = {LLONG_MAX, INT_MAX};
        for (int l = 1; l <= n; ++l) {
            for (int r = l; r <= n; ++r) {
                int cnt[64]{};
                I cost = 0;
                int worst = 0;
                for (auto &t : cand) {
                    int &f = cnt[value(t, l, r)];
                    cost += 2 * f + 1;
                    worst = max(worst, ++f);
                }
                if (make_pair(cost, worst) < best) {
                    best = {cost, worst};
                    bl = l;
                    br = r;
                }
            }
        }
        I v = ask(bl, br);
        erase_if(cand, [&](auto &t) { return value(t, bl, br) != v; });
    }
    answer(cand[0][0], cand[0][1], cand[0][2]);
}

struct Group {
    int h, k;
    I yl, yr;
};

I n, s;
I lowBand[60], highBand[60];

// In the band opposite s at bit k there is a pair with XOR d.
// Return the inclusive range of possible block indices for the pair.
// A block has length 2^(h+1); one number lies in each half.
pair<I,I> blocks(int k, int h, I d) {
    I w = 1LL << h;
    I first = lowBand[k] / (2*w);
    I last = highBand[k] / (2*w);
    auto ok = [&](I id) {
        I u = id*(2*w) + w;
        I v = min(u + w - 1, highBand[k]);
        return u <= v && (u < v || (u ^ d) != 0);
    };
    if (!ok(first)) ++first;
    if (first <= last && !ok(last)) --last;
    return {first, last};
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    if (!(cin >> n) || n < 3) return 0;
    if (n <= 32) small((int)n);

    s = ask(1, n);

    // The four distinct values a,b,c,s XOR to zero. At their highest
    // differing bit k they split 2:2. Write y for the partner of s.
    // y differs from s first at h<k, and the other pair XORs to s^y.
    vector<int> order;
    for (int i = 0; i < 60; ++i) {
        I w = 1LL << i;
        I base = ((s >> (i+1)) << (i+1)) + ((s & w) ? 0 : w);
        lowBand[i] = max(1LL, base);
        highBand[i] = min(n, base + w - 1);
        if (lowBand[i] <= highBand[i]) order.push_back(i);
    }
    sort(order.begin(), order.end(), [](int i, int j) {
        return lowBand[i] < lowBand[j];
    });

    vector<Group> groups;
    for (int h : order) for (int k : order) if (h < k) {
        // Feasibility is independent of the low bits of d: the only
        // lower-half value to exclude is zero. If band k starts at
        // zero before clipping, the existence of band h implies that
        // band k is not clipped at its upper end.
        auto [a,b] = blocks(k, h, s ^ lowBand[h]);
        if (a <= b) groups.push_back({h,k,lowBand[h],highBand[h]});
    }

    I y;
    for (;;) {
        y = groups.front().yl;
        if (all_of(groups.begin(), groups.end(), [&](const Group &g) {
            return g.yl == y && g.yr == y;
        })) break;

        // After an informative band query there are at most two
        // interpretations of its reply. These two candidate partners
        // cannot both be stolen (that would force a stolen zero).
        bool fixed = all_of(groups.begin(), groups.end(), [](const Group &g) {
            return g.yl == g.yr;
        });
        if (fixed) {
            set<I> choices;
            for (auto &g : groups) choices.insert(g.yl);
            if (choices.size() == 2 && ((*choices.begin() ^ *choices.rbegin()) == s)) {
                if (!ask(y,y)) y = (*choices.begin() ^ *choices.rbegin() ^ y);
                erase_if(groups, [&](const Group &g) { return g.yl != y; });
                break;
            }
        }

        // A union of consecutive bands is queryable in one interaction:
        // any gap contains no possible stolen type. Seek a cut which
        // separates the two occupied bands for many surviving groups.
        I best = LLONG_MAX, bl = 0, br = 0;
        unsigned long long mask = 0;
        for (int i = 0; i < (int)order.size(); ++i) {
            unsigned long long m = 0;
            for (int j = i; j < (int)order.size(); ++j) {
                m |= 1ULL << order[j];
                int neither = 0, both = 0, single = 0, pairOnly = 0;
                for (auto &g : groups) {
                    bool u = (m >> g.h) & 1, v = (m >> g.k) & 1;
                    if (u && v) ++both;
                    else if (u) ++single;
                    else if (v) ++pairOnly;
                    else ++neither;
                }
                if (!s) neither += both, both = 0;
                I score = (1LL*neither*neither + 1LL*both*both) * 4096
                          + (s ? min(single,pairOnly) : 0);
                if (score < best) {
                    best = score;
                    mask = m;
                    bl = lowBand[order[i]];
                    br = highBand[order[j]];
                }
            }
        }
        I r = ask(bl, br);
        vector<Group> next;
        for (Group g : groups) {
            bool u = (mask >> g.h) & 1, v = (mask >> g.k) & 1;
            if (u == v) {
                if (r == (u ? s : 0)) next.push_back(g);
            } else {
                I val = u ? r : (s ^ r);
                if (g.yl <= val && val <= g.yr) {
                    g.yl = g.yr = val;
                    next.push_back(g);
                }
            }
        }
        groups.swap(next);
    }

    I d = s ^ y;
    int h = 63 - __builtin_clzll(d);
    I w = 1LL << h;
    vector<pair<I,I>> runs;
    for (auto &g : groups) {
        auto p = blocks(g.k, h, d);
        if (p.first <= p.second) runs.push_back(p);
    }
    sort(runs.begin(), runs.end());
    runs.erase(unique(runs.begin(), runs.end()), runs.end());

    for (;;) {
        I count = 0;
        for (auto [a,b] : runs) count += b-a+1;
        I rank = count/2, id = 0;
        for (auto [a,b] : runs) {
            if (rank <= b-a) { id = a+rank; break; }
            rank -= b-a+1;
        }
        I mid = id*(2*w) + w;
        if (count == 1) {
            I last = min(n, mid+w-1);
            I num = last-mid+1 - (mid <= d && d <= last);
            if (num == 1) {
                I u = mid + (mid == d);
                answer(y, u, u^d);
            }
        }
        // Cutting the median block in half either solves the pair
        // immediately or eliminates that block and one entire side.
        I p = mid-1;
        I r = ask(1, p) ^ (y <= p ? y : 0);
        if (r != 0 && r != d) answer(y, r, r^d);
        for (auto &[a,b] : runs) {
            if (!r) a = max(a, id+1);
            else b = min(b, id-1);
        }
        erase_if(runs, [](auto p) { return p.first > p.second; });
    }
}
