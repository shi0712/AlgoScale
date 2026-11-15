#include <bits/stdc++.h>
using namespace std;
typedef long long ll;
typedef unsigned long long ull;

static const ll MAXW = 1000000000LL;
static const int M = 1 << 20;

static inline ull mk(ll value, int idx) {
    return ((ull)value << 20) | (ull)(M - 1 - idx);
}

int main() {
    int T;
    if (scanf("%d", &T) != 1) return 0;
    while (T--) {
        int n;
        ll W;
        scanf("%d %lld", &n, &W);
        vector<ll> w(n + 1), r(n + 1);
        for (int i = 1; i <= n; i++) scanf("%lld", &w[i]);
        for (int i = 1; i <= n; i++) scanf("%lld", &r[i]);

        vector<int> kn;
        for (int i = 1; i <= n; i++) if (w[i] > 0) kn.push_back(i);
        sort(kn.begin(), kn.end(), [&](int a, int b) {
            if (w[a] != w[b]) return w[a] < w[b];
            return a < b;
        });
        int K = kn.size();
        vector<int> rk(n + 1, 0);
        vector<ll> ws(K), P(K + 1, 0);
        vector<int> is(K);
        for (int k = 0; k < K; k++) {
            rk[kn[k]] = k + 1;
            ws[k] = w[kn[k]];
            is[k] = kn[k];
            P[k + 1] = P[k] + ws[k];
        }
        // AC list
        vector<pair<ull, int>> tmp;
        vector<int> Bidx;
        const int INF = INT_MAX;
        int minC = INF;
        for (int i = 1; i <= n; i++) {
            if (r[i] > 0) tmp.push_back({mk(r[i], i), i});
            if (w[i] > 0 && r[i] == 0) Bidx.push_back(i);
            if (w[i] == 0) minC = min(minC, i);
        }
        sort(tmp.begin(), tmp.end(), [](const pair<ull, int>& a, const pair<ull, int>& b) {
            return a.first > b.first;
        });
        int na = tmp.size();
        vector<int> AC(na);
        vector<ull> keyAC(na);
        for (int i = 0; i < na; i++) { AC[i] = tmp[i].second; keyAC[i] = tmp[i].first; }
        int nb = Bidx.size();

        // prefix/suffix min idx of B by rank
        vector<int> pminB(K + 2, INF), sminB(K + 2, INF);
        for (int p = 1; p <= K; p++) {
            pminB[p] = pminB[p - 1];
            int it = is[p - 1];
            if (r[it] == 0) pminB[p] = min(pminB[p], it);
        }
        // sminB[p] = min idx among B with rank > p
        sminB[K] = INF;
        for (int p = K - 1; p >= 0; p--) {
            sminB[p] = sminB[p + 1];
            int it = is[p]; // rank p+1
            if (r[it] == 0) sminB[p] = min(sminB[p], it);
        }

        auto check2 = [&](int p) -> bool {
            bool allCap = (p == K || ws[p] == MAXW);
            ll m = allCap ? 0 : ws[p];
            int q = allCap ? 0 : is[p];
            int pS = 0, pN = 0;
            auto advS = [&]() { while (pS < nb && rk[Bidx[pS]] > p) pS++; };
            auto advN = [&]() { while (pN < nb && rk[Bidx[pN]] <= p) pN++; };
            advS(); advN();
            int a = 0;
            ll Ksum = 0, capSum = 0;
            while (true) {
                ull aK = a < na ? keyAC[a] : 0;
                ull sK = pS < nb ? mk(MAXW, Bidx[pS]) : 0;
                ull nK = pN < nb ? mk(1, Bidx[pN]) : 0;
                if (aK == 0 && sK == 0 && nK == 0) break;
                if (aK > sK && aK > nK) {
                    int x = AC[a++];
                    if (w[x] == 0) {
                        ll cap;
                        if (allCap) cap = MAXW - 1;
                        else cap = (x < q) ? m : m - 1;
                        capSum += cap;
                    } else {
                        if (rk[x] <= p) Ksum += w[x];
                        else {
                            ll L = W - w[x] + 1;
                            if (L > 0 && Ksum + capSum < L) return false;
                        }
                    }
                } else if (sK > nK) {
                    Ksum += w[Bidx[pS]];
                    pS++; advS();
                } else {
                    int x = Bidx[pN];
                    ll L = W - w[x] + 1;
                    if (L > 0 && Ksum + capSum < L) return false;
                    pN++; advN();
                }
            }
            return true;
        };

        int lo = 0, hi = K;
        while (lo < hi) {
            int mid = (lo + hi) / 2;
            if (check2(mid)) hi = mid; else lo = mid + 1;
        }
        int E = lo;

        // condition (1) bad intervals
        vector<int> diff(K + 3, 0);
        {
            vector<ull> ent;
            ent.reserve(2 * K);
            for (int k = 0; k < K; k++) {
                int it = is[k];
                if (r[it] > 0) {
                    ull key = mk(r[it], it);
                    ent.push_back(key << 1);
                    ent.push_back((key << 1) | 1);
                } else {
                    ull kS = mk(MAXW, it), kN = mk(1, it);
                    ent.push_back((kS << 1) | 1);
                    ent.push_back(kN << 1);
                }
            }
            sort(ent.begin(), ent.end());
            vector<ll> tree(K + 1, 0);
            int hb = 1;
            while ((hb << 1) <= K) hb <<= 1;
            for (ull e : ent) {
                int type = e & 1;
                ull key = e >> 1;
                int idx = M - 1 - (int)(key & (M - 1));
                if (type == 1) {
                    for (int x = rk[idx]; x <= K; x += x & -x) tree[x] += w[idx];
                } else {
                    ll rem = w[idx];
                    int pos = 0;
                    for (int step = hb; step; step >>= 1) {
                        if (pos + step <= K && tree[pos + step] < rem) {
                            pos += step;
                            rem -= tree[pos];
                        }
                    }
                    int p0 = pos + 1;
                    if (p0 <= K && p0 <= rk[idx] - 1) {
                        diff[p0]++;
                        diff[rk[idx]]--;
                    }
                }
            }
        }

        auto dcheck = [&](int p) -> bool {
            ull kA = na ? keyAC[0] : 0;
            ull kS = (p >= 1 && pminB[p] < INF) ? mk(MAXW, pminB[p]) : 0;
            ull kN = (sminB[p] < INF) ? mk(1, sminB[p]) : 0;
            if (kA > kS && kA > kN) {
                int item = AC[0];
                if (w[item] == 0) {
                    if (P[p] > W - 1) return false;
                    if (!(p == K || ws[p] == MAXW)) {
                        ll cap = (item < is[p]) ? ws[p] : ws[p] - 1;
                        if (cap < 1) return false;
                    }
                }
            }
            return true;
        };

        int chosen = -1;
        int cur = 0;
        for (int p = 0; p <= K; p++) {
            cur += diff[p];
            if (p < E) continue;
            if (P[p] > W) break;
            if (cur > 0) continue;
            if (W == MAXW && !dcheck(p)) continue;
            if (p >= 1 && ws[p - 1] == MAXW) {
                if (minC != INF && minC < is[p - 1]) continue;
            }
            chosen = p;
            break;
        }
        if (chosen < 0) {
            puts("No");
            continue;
        }
        int p = chosen;
        vector<ll> fw = w, fr = r;
        for (int i = 1; i <= n; i++) {
            if (w[i] > 0 && r[i] == 0) fr[i] = (rk[i] <= p) ? MAXW : 1;
        }
        vector<pair<ull, int>> ord(n);
        for (int i = 1; i <= n; i++) ord[i - 1] = {mk(fr[i], i), i};
        sort(ord.begin(), ord.end(), [](const pair<ull, int>& a, const pair<ull, int>& b) {
            return a.first > b.first;
        });
        bool allCap = (p == K || ws[p] == MAXW);
        ll m = allCap ? 0 : ws[p];
        int q = allCap ? 0 : is[p];
        ll rem = W - P[p];
        for (auto& pr : ord) {
            int i = pr.second;
            if (w[i] != 0) continue;
            ll cap = allCap ? MAXW - 1 : ((i < q) ? m : m - 1);
            ll x = min(cap, rem);
            if (x >= 1) { fw[i] = x; rem -= x; }
            else fw[i] = MAXW;
        }
        string out = "Yes\n";
        for (int i = 1; i <= n; i++) { out += to_string(fw[i]); out += (i == n ? '\n' : ' '); }
        for (int i = 1; i <= n; i++) { out += to_string(fr[i]); out += (i == n ? '\n' : ' '); }
        fputs(out.c_str(), stdout);
    }
    return 0;
}
