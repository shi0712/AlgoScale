#include <bits/stdc++.h>
using namespace std;

namespace {

struct FastScanner {
    static const int SZ = 1 << 20;
    int idx = 0, size = 0;
    char buf[SZ];
    inline char read() {
        if (idx >= size) {
            size = (int)fread(buf, 1, SZ, stdin);
            idx = 0;
            if (size == 0) return 0;
        }
        return buf[idx++];
    }
    int nextInt() {
        char c;
        do { c = read(); } while (c && (c < '0' || c > '9'));
        int x = 0;
        while (c >= '0' && c <= '9') {
            x = x * 10 + (c - '0');
            c = read();
        }
        return x;
    }
};

struct Evaluator {
    int maxH, D, SMAX;
    const vector<int>* hvals;
    const vector<int>* freq;
    const vector<int>* isqrtv;
    vector<int> c0arr;      // C_T(h)
    vector<uint8_t> q0arr;  // qgt(h,T)
    vector<uint8_t> qendarr; // qgt at the last chosen positive sqrt
    vector<int> bucket;
    vector<int> csmall;
    vector<uint8_t> qsmall;
    int T;

    inline void calcLarge(int x, int& c, int& qg) const {
        if (x == 0) { c = 0; qg = 0; return; }
        int den = 2 * T + 1;
        int e;
        if (x < den) {
            qg = 0;
        } else {
            int bx = 31 - __builtin_clz((unsigned)x);
            int bd = 31 - __builtin_clz((unsigned)den);
            e = bx - bd;
            if (((long long)den << e) > x) --e;
            qg = e + 1;
        }
        c = (x >> qg) + T * qg;
    }

    void prepareSmall() {
        int den = 2 * T + 1;
        for (int x = 0; x <= SMAX; ++x) {
            int qg;
            if (x == 0 || x < den) {
                qg = 0;
            } else {
                int bx = 31 - __builtin_clz((unsigned)x);
                int bd = 31 - __builtin_clz((unsigned)den);
                int e = bx - bd;
                if (((long long)den << e) > x) --e;
                qg = e + 1;
            }
            qsmall[x] = (uint8_t)qg;
            csmall[x] = (x >> qg) + T * qg;
        }
    }

    // Computes D(T) and one valid q-count of a minimizer at this price.
    void eval(int t, long long& D, long long& qcount) {
        T = t;
        prepareSmall();

        const vector<int>& hv = *hvals;
        const vector<int>& fq = *freq;
        const vector<int>& sq = *isqrtv;
        int nD = (int)hv.size();

        fill(bucket.begin(), bucket.end(), 0);

        long long base = 0, savSum = 0;
        long long savCnt = 0;
        int maxS = 0;

        for (int id = 0; id < nD; ++id) {
            int h = hv[id];
            int f = fq[h];
            int c0, q0;
            calcLarge(h, c0, q0);
            c0arr[id] = c0;
            q0arr[id] = (uint8_t)q0;
            base += (long long)f * c0;

            int x = h;
            int cprev = c0, qprev = q0;
            while (x > 1) {
                int xn = sq[x];
                int cn = csmall[xn];
                int s = cprev - cn;
                if (s <= 0) break;
                bucket[s] += f;
                savCnt += f;
                savSum += (long long)s * f;
                if (s > maxS) maxS = s;
                cprev = cn;
                qprev = qsmall[xn];
                x = xn;
            }
            qendarr[id] = (uint8_t)qprev;
        }

        long long selected;
        int cutoff;
        long long remCut = 0;
        if ((long long)P >= savCnt) {
            selected = savSum;
            cutoff = 0;
        } else {
            long long left = P;
            long long got = 0;
            int v = maxS;
            for (; v >= 1; --v) {
                int cnt = bucket[v];
                if (cnt == 0) continue;
                if (cnt < left) {
                    got += (long long)cnt * v;
                    left -= cnt;
                } else if (cnt == left) {
                    got += (long long)cnt * v;
                    left = 0;
                    // all items with saving >= v are selected; no partial bucket
                    cutoff = (v > 1 ? v - 1 : 0);
                    remCut = 0;
                    break;
                } else {
                    got += left * v;
                    remCut = left;
                    cutoff = v;
                    left = 0;
                    break;
                }
            }
            selected = got;
            if (v < 1) cutoff = 0;
        }

        D = base - selected - (long long)T * Q;

        // Compute q-count for the selected p allocation.
        // Reconstruct the number of selected lines from the bucket cutoff.
        long long qc = 0;
        if (cutoff == 0) {
            // all positive marginals selected
            for (int id = 0; id < nD; ++id) {
                int h = hv[id];
                qc += (long long)fq[h] * qendarr[id];
            }
        } else {
            long long rem = remCut;
            // First skip all items strictly above cutoff by using second pass.
            for (int id = 0; id < nD; ++id) {
                int h = hv[id];
                int f = fq[h];
                int cs[8];
                int qs[8];
                int len = 1;
                cs[0] = c0arr[id];
                qs[0] = q0arr[id];
                int x = h;
                while (x > 1) {
                    int xn = sq[x];
                    int cn = csmall[xn];
                    int s = cs[len - 1] - cn;
                    if (s <= 0) break;
                    cs[len] = cn;
                    qs[len] = qsmall[xn];
                    ++len;
                    x = xn;
                }
                int m = len - 1;
                int basep = 0;
                while (basep < m && cs[basep] - cs[basep + 1] > cutoff) ++basep;
                int r = 0;
                while (basep + r < m && cs[basep + r] - cs[basep + r + 1] == cutoff) ++r;

                long long capacity = (long long)f * r;
                if (capacity > 0 && rem > 0) {
                    long long take = min(rem, capacity);
                    long long full = take / r;
                    int part = (int)(take % r);
                    qc += (long long)(f - full - (part > 0)) * qs[basep];
                    if (full > 0) qc += full * qs[basep + r];
                    if (part > 0) qc += qs[basep + part];
                    rem -= take;
                } else {
                    qc += (long long)f * qs[basep];
                }
            }
        }
        qcount = qc;
    }

    // P is needed inside eval; store a copy.
    int P, Q;
};

} // namespace

int main() {
    FastScanner fs;
    int n = fs.nextInt();
    int P = fs.nextInt();
    int Q = fs.nextInt();

    const int MAXV = 2000000;
    vector<int> freq(MAXV + 1, 0);
    int maxH = 0;
    for (int i = 0; i < n; ++i) {
        int h = fs.nextInt();
        ++freq[h];
        if (h > maxH) maxH = h;
    }

    if (maxH == 0) {
        puts("0");
        return 0;
    }

    vector<int> isqrtv(maxH + 1);
    for (int x = 0; x <= maxH; ++x) {
        int r = (int)std::sqrt((double)x);
        while ((long long)(r + 1) * (r + 1) <= x) ++r;
        while ((long long)r * r > x) --r;
        isqrtv[x] = r;
    }

    vector<int> hvals;
    hvals.reserve(maxH + 1);
    for (int h = 0; h <= maxH; ++h)
        if (freq[h]) hvals.push_back(h);

    // Zeroability check: minimal number of halvings needed to make all heights 0,
    // after spending at most P square-root operations.
    {
        long long baseZ = 0;
        long long cntZ[32] = {};
        for (int h : hvals) {
            int f = freq[h];
            int z0 = (h == 0 ? 0 : 32 - __builtin_clz((unsigned)h));
            baseZ += (long long)f * z0;
            int x = h;
            int zp = z0;
            while (x > 1) {
                int xn = isqrtv[x];
                int zn = 32 - __builtin_clz((unsigned)xn);
                int d = zp - zn;
                if (d > 0) cntZ[d] += f;
                x = xn;
                zp = zn;
            }
        }
        long long left = P;
        long long reduced = 0;
        for (int d = 31; d >= 1 && left > 0; --d) {
            if (!cntZ[d]) continue;
            long long take = min(left, cntZ[d]);
            reduced += take * d;
            left -= take;
        }
        if (baseZ - reduced <= Q) {
            puts("0");
            return 0;
        }
    }

    int SMAX = isqrtv[maxH];
    Evaluator ev;
    ev.maxH = maxH;
    ev.D = (int)hvals.size();
    ev.SMAX = SMAX;
    ev.hvals = &hvals;
    ev.freq = &freq;
    ev.isqrtv = &isqrtv;
    ev.P = P;
    ev.Q = Q;
    ev.c0arr.resize(ev.D);
    ev.q0arr.resize(ev.D);
    ev.qendarr.resize(ev.D);
    ev.bucket.assign(maxH + 2, 0);
    ev.csmall.assign(SMAX + 1, 0);
    ev.qsmall.assign(SMAX + 1, 0);

    int lo = 1, hi = (maxH + 1) / 2;
    long long best = LLONG_MIN;
    while (lo <= hi) {
        int mid = lo + (hi - lo) / 2;
        long long D, qc;
        ev.eval(mid, D, qc);
        if (D > best) best = D;
        if (qc == Q) {
            break;
        } else if (qc > Q) {
            lo = mid + 1;
        } else {
            hi = mid - 1;
        }
    }

    printf("%lld\n", best);
    return 0;
}
