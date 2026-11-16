#include <bits/stdc++.h>
using namespace std;
using ll = long long;

int main(){
    int n, m;
    if (scanf("%d %d", &n, &m) != 2) return 0;
    vector<int> t(n);
    for (int i = 0; i < n; ++i) scanf("%d", &t[i]);
    vector<ll> a((size_t)n * m), d((size_t)n * m);
    for (size_t i = 0; i < a.size(); ++i) scanf("%lld", &a[i]);
    for (size_t i = 0; i < d.size(); ++i) scanf("%lld", &d[i]);

    const int full = 1 << m;
    vector<ll> A(m, 0), at(m, 0), dt(m, 0);
    for (int i = 0; i < n; ++i){
        const ll* as = a.data() + (size_t)i * m;
        const ll* ds = d.data() + (size_t)i * m;
        for (int j = 0; j < m; ++j){
            A[j] += as[j];
            if (t[i]){ at[j] += as[j]; dt[j] += ds[j]; }
        }
    }
    vector<ll> TA(full, 0), DT(full, 0), M(full, 0), need(full, 0);
    vector<unsigned char> bad(full, 0);
    for (int R = 1; R < full; ++R){
        int b = __builtin_ctz((unsigned)R);
        int rest = R & (R - 1);
        TA[R] = TA[rest] + at[b];
        DT[R] = DT[rest] + dt[b];
    }
    ll* nd = need.data();
    ll* mp = M.data();
    unsigned char* bp = bad.data();
    for (int i = 0; i < n; ++i){
        const ll* as = a.data() + (size_t)i * m;
        const ll* ds = d.data() + (size_t)i * m;
        bool has = false;
        for (int j = 0; j < m; ++j) if (as[j] > 0){ has = true; break; }
        if (!has) continue;                 // need_s(R) == 0 always
        nd[0] = 0;
        if (t[i]){
            for (int q = 0; q < m; ++q){
                const ll aq = as[q], delta = as[q] - ds[q];
                const int half = 1 << q;
                for (int k = 0; k < half; ++k){
                    ll v = nd[k] + delta;
                    if (v < aq) v = aq;
                    nd[half + k] = v;
                    if (v) bp[half + k] = 1;   // trump condition violated
                }
            }
        } else {
            for (int q = 0; q < m; ++q){
                const ll aq = as[q], delta = as[q] - ds[q];
                const int half = 1 << q;
                for (int k = 0; k < half; ++k){
                    ll v = nd[k] + delta;
                    if (v < aq) v = aq;
                    nd[half + k] = v;
                    mp[half + k] += v;         // accumulate deficits
                }
            }
        }
    }
    unsigned good = 0;
    for (int R = 1; R < full; ++R)
        if (!bad[R] && TA[R] + M[R] <= DT[R]) good |= (unsigned)R;
    ll ans = 0;
    for (int j = 0; j < m; ++j)
        if (A[j] > 0 && !((good >> j) & 1u)) ans += A[j];
    printf("%lld\n", ans);
    return 0;
}
