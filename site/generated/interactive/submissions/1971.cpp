#include <bits/stdc++.h>
using namespace std;

// 16 * (area)^2 of the triangle with sides a,b,c, or 0 if non-degenerate triangle impossible.
static inline int fval(int a, int b, int c) {
    long long s = (long long)a + b + c;
    long long x = s - 2LL * a, y = s - 2LL * b, z = s - 2LL * c;
    if (x <= 0 || y <= 0 || z <= 0) return 0;
    return (int)(s * x * y * z);
}

// A pair of known values (u,v) is "good" if f(u,v,.) is injective on {1,2,3,4}.
static inline bool goodpair(int u, int v) {
    if (u < 2 || v < 2) return false;
    if ((u == 2 && v == 4) || (u == 4 && v == 2)) return false;
    return true;
}

static inline int decodeThird(int u, int v, int r) {
    for (int w = 1; w <= 4; w++)
        if (fval(u, v, w) == r) return w;
    return -1;
}

// If one queried side is a known 1, a nonzero answer r means the other two are equal,
// and fval(1,x,x)==r identifies x.
static inline int decodeEqual(int r) {
    for (int x = 1; x <= 4; x++)
        if (fval(1, x, x) == r) return x;
    return -1;
}

template <class Ask>
vector<int> solve(int n, Ask askRaw) {
    auto Q = [&](int i, int j, int k) {
        int a[3] = {i, j, k};
        sort(a, a + 3);
        return askRaw(a[0], a[1], a[2]);
    };

    if (n == 3) {
        int r = Q(0, 1, 2);
        int cnt = 0;
        vector<int> best;
        for (int a = 1; a <= 4; a++)
            for (int b = 1; b <= 4; b++)
                for (int c = 1; c <= 4; c++)
                    if (fval(a, b, c) == r) { cnt++; best = {a, b, c}; }
        if (cnt == 1) return best;
        return {};
    }
    if (n == 4) {
        vector<tuple<int, int, int, int>> qs;
        for (int i = 0; i < 4; i++)
            for (int j = i + 1; j < 4; j++)
                for (int k = j + 1; k < 4; k++)
                    qs.emplace_back(i, j, k, Q(i, j, k));
        int cnt = 0;
        vector<int> best;
        for (int code = 0; code < 256; code++) {
            int v[4], t = code;
            for (int i = 0; i < 4; i++) { v[i] = 1 + t % 4; t /= 4; }
            bool ok = true;
            for (auto &q : qs)
                if (fval(v[get<0>(q)], v[get<1>(q)], v[get<2>(q)]) != get<3>(q)) { ok = false; break; }
            if (ok) { cnt++; best = {v[0], v[1], v[2], v[3]}; }
        }
        if (cnt == 1) return best;
        return {};
    }

    // n >= 5
    auto getValsets = [&](int m, const vector<tuple<int, int, int, int>> &qs) {
        vector<set<int>> vs(m);
        int tot = 1;
        for (int i = 0; i < m; i++) tot *= 4;
        int v[5];
        for (int code = 0; code < tot; code++) {
            int t = code;
            for (int i = 0; i < m; i++) { v[i] = 1 + t % 4; t /= 4; }
            bool ok = true;
            for (auto &q : qs)
                if (fval(v[get<0>(q)], v[get<1>(q)], v[get<2>(q)]) != get<3>(q)) { ok = false; break; }
            if (ok) for (int i = 0; i < m; i++) vs[i].insert(v[i]);
        }
        return vs;
    };

    // For n >= 5: the answers to all triples determine the array uniquely iff
    // (all values are 1) or (some value among 2,3,4 appears at least twice).
    auto finish = [&](vector<int> val) -> vector<int> {
        int cnt[5] = {0, 0, 0, 0, 0};
        for (int x : val) cnt[x]++;
        bool amb = (cnt[2] <= 1 && cnt[3] <= 1 && cnt[4] <= 1 && (cnt[2] + cnt[3] + cnt[4] >= 1));
        if (amb) return {};
        return val;
    };

    // Decode a list of unknown positions.  When a known 1 is available, a query
    // with it decides two indices at once whenever they are equal; wasteful cases
    // fall back to one exact query per index.
    auto adaptiveBatch = [&](vector<int> unknown, int anchor, int p, int q, int u, int v,
                             vector<int> &val) {
        long long zeros = 0, processed = 0;
        size_t i = 0;
        while (i < unknown.size()) {
            if (i + 1 < unknown.size()) {
                int a = unknown[i], b = unknown[i + 1];
                int r = Q(anchor, a, b);
                processed++;
                if (r == 3) {
                    val[a] = 1;
                    val[b] = 1;
                } else if (r > 0) {
                    int x = decodeEqual(r);
                    val[a] = x;
                    val[b] = x;
                } else {
                    zeros++;
                    val[a] = decodeThird(u, v, Q(p, q, a));
                    val[b] = decodeThird(u, v, Q(p, q, b));
                }
                i += 2;
                if (2 * zeros - processed > 8) break;
            } else {
                int k = unknown[i];
                val[k] = decodeThird(u, v, Q(p, q, k));
                i++;
            }
        }
        for (; i < unknown.size(); i++) {
            int k = unknown[i];
            val[k] = decodeThird(u, v, Q(p, q, k));
        }
    };

    // We know a good pair at positions p,q with values u,v.
    auto finishWithPair = [&](int m, const vector<set<int>> &vs, int p, int q, int u, int v) -> vector<int> {
        vector<int> val(n, -1);
        for (int pos = 0; pos < m; pos++)
            if (vs[pos].size() == 1) val[pos] = *vs[pos].begin();
        val[p] = u;
        val[q] = v;

        vector<int> unknown;
        for (int k = 0; k < n; k++)
            if (val[k] == -1) unknown.push_back(k);

        if (n < 64 || unknown.empty()) {
            for (int k : unknown) val[k] = decodeThird(u, v, Q(p, q, k));
            return finish(val);
        }

        int r1 = -1;
        for (int pos = 0; pos < m; pos++)
            if (vs[pos].size() == 1 && *vs[pos].begin() == 1) { r1 = pos; break; }

        if (r1 == -1) {
            // These per-index queries are needed anyway; stop as soon as a 1 is met.
            size_t i = 0;
            for (; i < unknown.size(); i++) {
                int k = unknown[i];
                val[k] = decodeThird(u, v, Q(p, q, k));
                if (val[k] == 1) { r1 = k; i++; break; }
            }
            unknown.erase(unknown.begin(), unknown.begin() + i);
            if (r1 == -1) return finish(val);
        }
        adaptiveBatch(unknown, r1, p, q, u, v, val);
        return finish(val);
    };

    // No fixed good pair in the prefix; here the prefix class always yields at
    // least two positions that are certainly 1.
    auto runNoGood = [&](int m, const vector<set<int>> &vs) -> vector<int> {
        vector<int> ones;
        for (int pos = 0; pos < m; pos++)
            if (vs[pos].size() == 1 && *vs[pos].begin() == 1) ones.push_back(pos);
        int r1 = ones[0], r2 = ones[1];

        vector<int> val(n, -1);
        for (int pos = 0; pos < m; pos++)
            if (vs[pos].size() == 1) val[pos] = *vs[pos].begin();

        vector<int> S; // indices certainly not equal to 1
        for (int pos = 0; pos < m; pos++) {
            if (val[pos] != -1) continue;
            if (!vs[pos].count(1)) S.push_back(pos);
            else {
                int r = Q(r1, r2, pos);
                if (r == 3) val[pos] = 1;
                else S.push_back(pos);
            }
        }

        auto findRepeatAmong4 = [&](int &p, int &q, int &pv) {
            for (int i = 0; i < 4 && p == -1; i++)
                for (int j = i + 1; j < 4 && p == -1; j++) {
                    int r = Q(r1, S[i], S[j]);
                    if (r > 0) { p = S[i]; q = S[j]; pv = decodeEqual(r); }
                }
        };

        int p = -1, q = -1, pv = 0;
        auto maybeFind = [&]() {
            if (p == -1 && (int)S.size() >= 4) findRepeatAmong4(p, q, pv);
        };
        if ((int)S.size() >= 4) findRepeatAmong4(p, q, pv);

        int idx = m;
        while (idx < n && p == -1) {
            if (idx + 1 < n) {
                int a = idx, b = idx + 1;
                int r = Q(r1, a, b);
                if (r == 3) {
                    val[a] = 1;
                    val[b] = 1;
                } else if (r > 0) {
                    p = a;
                    q = b;
                    pv = decodeEqual(r);
                } else {
                    int ra = Q(r1, r2, a);
                    if (ra == 3) {
                        val[a] = 1;
                        S.push_back(b);
                        maybeFind();
                    } else {
                        S.push_back(a);
                        maybeFind();
                        if (p == -1) {
                            int rb = Q(r1, r2, b);
                            if (rb == 3) val[b] = 1;
                            else {
                                S.push_back(b);
                                maybeFind();
                            }
                        }
                    }
                }
                idx += 2;
            } else {
                int k = idx;
                int r = Q(r1, r2, k);
                if (r == 3) val[k] = 1;
                else {
                    S.push_back(k);
                    maybeFind();
                }
                idx++;
            }
        }

        if (p != -1) {
            val[p] = pv;
            val[q] = pv;
            vector<int> unknown;
            for (int k = 0; k < n; k++)
                if (val[k] == -1) unknown.push_back(k);
            if (n >= 64) adaptiveBatch(unknown, r1, p, q, pv, pv, val);
            else for (int k : unknown) val[k] = decodeThird(pv, pv, Q(p, q, k));
            return finish(val);
        }

        // Here |S| <= 3; look for a repeated value among them.
        int pp = -1, qq = -1, pval = 0;
        for (int i = 0; i < (int)S.size() && pp == -1; i++)
            for (int j = i + 1; j < (int)S.size() && pp == -1; j++) {
                int r = Q(r1, S[i], S[j]);
                if (r > 0) { pp = S[i]; qq = S[j]; pval = decodeEqual(r); }
            }

        if (pp != -1) {
            val[pp] = pval;
            val[qq] = pval;
            for (int s : S) {
                if (val[s] != -1) continue;
                int r = Q(pp, qq, s);
                val[s] = decodeThird(pval, pval, r);
            }
            return finish(val);
        }

        if (S.empty()) return vector<int>(n, 1);
        return {};
    };

    vector<tuple<int, int, int, int>> qs;
    for (int i = 0; i < 4; i++)
        for (int j = i + 1; j < 4; j++)
            for (int k = j + 1; k < 4; k++)
                qs.emplace_back(i, j, k, Q(i, j, k));
    vector<set<int>> vs4 = getValsets(4, qs);

    int gp = -1, gq = -1, gu = 0, gv = 0;
    for (int i = 0; i < 4 && gp == -1; i++)
        for (int j = i + 1; j < 4 && gp == -1; j++)
            if (vs4[i].size() == 1 && vs4[j].size() == 1) {
                int u = *vs4[i].begin(), v = *vs4[j].begin();
                if (goodpair(u, v)) { gp = i; gq = j; gu = u; gv = v; }
            }
    if (gp != -1) return finishWithPair(4, vs4, gp, gq, gu, gv);

    int cert1_4 = 0;
    for (int pos = 0; pos < 4; pos++)
        if (vs4[pos].size() == 1 && *vs4[pos].begin() == 1) cert1_4++;
    if (cert1_4 >= 2 && n > 5) return runNoGood(4, vs4);

    // Extend the probe to a prefix of length 5.
    for (int i = 0; i < 4; i++)
        for (int j = i + 1; j < 4; j++)
            qs.emplace_back(i, j, 4, Q(i, j, 4));
    vector<set<int>> vs5 = getValsets(5, qs);

    if (n == 5) {
        // The whole array's triples are known: this is exactly the equivalence class.
        bool allSingleton = true;
        for (int pos = 0; pos < 5; pos++)
            if (vs5[pos].size() > 1) allSingleton = false;
        if (!allSingleton) return {};
        vector<int> val(5);
        for (int pos = 0; pos < 5; pos++) val[pos] = *vs5[pos].begin();
        return val;
    }

    gp = -1; gq = -1; gu = 0; gv = 0;
    for (int i = 0; i < 5 && gp == -1; i++)
        for (int j = i + 1; j < 5 && gp == -1; j++)
            if (vs5[i].size() == 1 && vs5[j].size() == 1) {
                int u = *vs5[i].begin(), v = *vs5[j].begin();
                if (goodpair(u, v)) { gp = i; gq = j; gu = u; gv = v; }
            }
    if (gp != -1) return finishWithPair(5, vs5, gp, gq, gu, gv);

    return runNoGood(5, vs5);
}

#ifndef LOCAL_TEST
int main() {
    int n;
    if (scanf("%d", &n) != 1) return 0;
    auto ask = [](int i, int j, int k) {
        printf("? %d %d %d\n", i + 1, j + 1, k + 1);
        fflush(stdout);
        int r;
        if (scanf("%d", &r) != 1) exit(0);
        return r;
    };
    vector<int> res = solve(n, ask);
    if (res.empty()) {
        printf("! -1\n");
    } else {
        printf("!");
        for (int x : res) printf(" %d", x);
        printf("\n");
    }
    fflush(stdout);
    return 0;
}
#endif
