#include <bits/stdc++.h>
using namespace std;

struct Bounds {
    int lo, hi;
};
static Bounds combine(Bounds a, Bounds b) {
    return {max(a.lo, b.lo), max(a.hi, b.hi)};
}

class Solver {
    static constexpr int W = 8;
    using Mask = array<uint64_t, W>;

    int n, m, D, stride, base, words, L;
    int cut, belowOrEqual;
    vector<int> val, pos, tim, pref, hist;
    vector<vector<int>> diag, upper;
    vector<Mask> publicBits, smallBits, bits;
    vector<unsigned char> initial, alive, dirty;
    vector<int> leftPtr, rightPtr, changed, removed;
    vector<Bounds> tree;
    vector<int> run, work;
    int runCut = -1;

    int prefixValue(int t) const {
        return t <= L ? pref[L-t+1] : 0;
    }

    void eraseThresholdBit(int v) {
        int p = pos[v], x = p / stride - 1;
        smallBits[tim[p]][x >> 6] &= ~(1ULL << (x & 63));
    }

    void tighten(int t, int v) {
        int &u = upper[L][t];
        if (v >= u) return;
        --hist[u];
        ++hist[v];
        if (u > cut && v <= cut) ++belowOrEqual;
        u = v;
        while (belowOrEqual - hist[cut] >= m) {
            belowOrEqual -= hist[cut--];
            eraseThresholdBit(cut);
        }
    }

    void markRemoved(int p) {
        if (!alive[p]) return;
        alive[p] = 0;
        removed.push_back(p);
        int t = tim[p], x = p / stride - 1;
        bits[t][x >> 6] &= ~(1ULL << (x & 63));
        if (!dirty[t]) {
            dirty[t] = 1;
            changed.push_back(t);
        }
    }

    void apply(int t, int v) {
        int a = max(1, t-L+1);
        for (int i = a; i <= t; ++i) {
            auto &d = diag[i];
            int &r = rightPtr[i];
            while (r >= 0 && val[d[r]] > v)
                markRemoved(d[r--]);
        }
        int p = pos[v], h = tim[p];
        if (p / stride != 1 && leftPtr[h] != rightPtr[h]) {
            for (int j = leftPtr[h]; j <= rightPtr[h]; ++j)
                if (diag[h][j] != p) markRemoved(diag[h][j]);
        }

        // Delete vertices with no predecessor or no successor.
        for (size_t j = 0; j < removed.size(); ++j) {
            int b = removed[j], q;
            q = b+1;
            if (alive[q] && !alive[q-stride]) markRemoved(q);
            q = b+stride;
            if (alive[q] && !alive[q-1]) markRemoved(q);
            q = b-1;
            if (alive[q] && !alive[q+stride]) markRemoved(q);
            q = b-stride;
            if (alive[q] && !alive[q+1]) markRemoved(q);
        }
        removed.clear();

        for (int i : changed) {
            dirty[i] = 0;
            auto &d = diag[i];
            while (!alive[d[leftPtr[i]]]) ++leftPtr[i];
            while (!alive[d[rightPtr[i]]]) --rightPtr[i];
            int k = base+i;
            Bounds b{val[d[leftPtr[i]]], val[d[rightPtr[i]]]};
            while (tree[k].lo != b.lo || tree[k].hi != b.hi) {
                tree[k] = b;
                if (k == 1) break;
                k >>= 1;
                b = combine(tree[k*2], tree[k*2+1]);
            }
        }
        changed.clear();
    }

    Bounds get(int t) const {
        int l = base+max(1, t-L+1), r = base+t+1;
        Bounds b{prefixValue(t), prefixValue(t)};
        for (; l < r; l >>= 1, r >>= 1) {
            if (l & 1) b = combine(b, tree[l++]);
            if (r & 1) b = combine(b, tree[--r]);
        }
        return b;
    }

    // Maximum possible number of consecutive small head values up to each
    // second, without any information specific to this snake.
    void publicRuns() {
        if (runCut == cut) return;
        runCut = cut;
        fill(run.begin(), run.end(), 0);
        for (int x = 1; x <= n; ++x) {
            int last = x == 1 ? 1 : n;
            for (int y = 1; y <= last; ++y) {
                int p = x*stride+y;
                int z = val[p] < cut ? 1+max(work[p-1],work[p-stride]) : 0;
                work[p] = z;
                run[x+y-1] = max(run[x+y-1], z);
            }
        }
    }

    // Exact feasibility of a value strictly below cut for this window.
    bool canBeSmall(int t) const {
        int a = max(1, t-L+1), b = t;
        while (a <= b && tree[base+a].hi < cut) ++a;
        while (b >= a && tree[base+b].hi < cut) --b;
        if (a >= b) return true; // The single-layer lower bound was checked.
        Mask s{};
        for (int j = 0; j < words; ++j)
            s[j] = bits[a][j] & smallBits[a][j];
        for (int i = a+1; i <= b; ++i) {
            uint64_t carry = 0, any = 0;
            for (int j = 0; j < words; ++j) {
                uint64_t old = s[j];
                s[j] = (old | (old << 1) | carry) & bits[i][j] & smallBits[i][j];
                carry = old >> 63;
                any |= s[j];
            }
            if (!any) return false;
        }
        return true;
    }

    void solveRange(int l, int r, int depth) {
        int first = -1, last = -1, mid = (l+r)/2, best = -1;
        int low = -1;
        bool same = true;
        for (int i = l; i <= r; ++i) {
            if (run[i] < min(i,L) || prefixValue(i) >= cut) continue;
            Bounds b = get(i);
            tighten(i,b.hi);
            if (b.lo >= cut || b.lo == b.hi) continue;
            if (first < 0) first = i, low = b.lo;
            if (low != b.lo) same = false;
            last = i;
            if (best < 0 || abs(mid-i) < abs(mid-best)) best = i;
        }
        if (best < 0) return;
        // On a likely constant plateau, testing its extremities can certify
        // its interior without querying there.
        if ((depth & 1) == 0 && same && last-first < L) {
            best = (depth & 2) ? first : last;
        }
        Bounds b = get(best);
        if (b.lo < cut && (b.hi <= cut || canBeSmall(best))) {
            cout << "? " << L << ' ' << best << endl;
            int v;
            if (!(cin >> v) || v < 1 || v > n*n) exit(0);
            tighten(best,v);
            apply(best,v);
        }
        if (first < best) solveRange(first,best-1,depth+1);
        if (best < last) solveRange(best+1,last,depth+1);
    }

public:
    void solve() {
        if (!(cin >> n >> m)) return;
        D = 2*n-1;
        stride = n+2;
        words = (n+63)/64;
        base = 1;
        while (base <= D) base *= 2;
        int size = stride*stride;
        val.resize(size);
        tim.resize(size);
        initial.resize(size);
        pos.resize(n*n+1);
        pref.resize(n+1);
        diag.resize(D+1);
        publicBits.resize(D+1, Mask{});
        for (int x = 1; x <= n; ++x) {
            for (int y = 1; y <= n; ++y) {
                int p = x*stride+y, t = x+y-1;
                if (!(cin >> val[p])) return;
                pos[val[p]] = p;
                tim[p] = t;
                if (x == 1) pref[y] = max(pref[y-1], val[p]);
                if (x == 1 && y > 1) continue;
                initial[p] = 1;
                diag[t].push_back(p);
                publicBits[t][(x-1)/64] |= 1ULL << ((x-1)%64);
            }
        }
        vector<Bounds> original(base*2, {0,0});
        for (int i = 1; i <= D; ++i) {
            auto &d = diag[i];
            sort(d.begin(),d.end(),[&](int a,int b){ return val[a]<val[b]; });
            original[base+i] = {val[d.front()], val[d.back()]};
        }
        for (int i = base-1; i; --i)
            original[i] = combine(original[2*i],original[2*i+1]);

        hist.assign(n*n+1,0);
        upper.assign(n+1, vector<int>(D+1));
        tree = original;
        for (L = 1; L <= n; ++L)
            for (int t = 1; t <= D; ++t)
                ++hist[upper[L][t] = get(t).hi];

        belowOrEqual = 0;
        for (cut = 1; cut <= n*n; ++cut) {
            belowOrEqual += hist[cut];
            if (belowOrEqual >= m) break;
        }
        smallBits = publicBits;
        for (int v = cut; v <= n*n; ++v) eraseThresholdBit(v);
        run.resize(D+1);
        work.assign(size,0);
        leftPtr.resize(D+1);
        rightPtr.resize(D+1);
        dirty.assign(D+1,0);
        for (L = 1; L <= n && cut > 1; ++L) {
            publicRuns();
            bool needed = false;
            for (int t = 1; t <= D; ++t)
                if (run[t] >= min(L,t) && prefixValue(t) < cut) {
                    needed = true;
                    break;
                }
            if (!needed) continue;
            alive = initial;
            bits = publicBits;
            tree = original;
            fill(leftPtr.begin(),leftPtr.end(),0);
            for (int i = 1; i <= D; ++i) rightPtr[i] = int(diag[i].size())-1;
            solveRange(1,D,0);
        }
        cout << '!';
        int count = 0;
        for (int v = 1; count < m; ++v) {
            int k = min(hist[v], m-count);
            count += k;
            while (k--) cout << ' ' << v;
        }
        cout << endl;
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    Solver().solve();
    return 0;
}
