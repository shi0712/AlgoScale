#include <bits/stdc++.h>
using namespace std;

static long long query(const vector<int>& s) {
    printf("? %d\n", (int)s.size());
    for (size_t i = 0; i < s.size(); i++) printf("%d ", s[i]);
    printf("\n");
    fflush(stdout);
    long long r;
    if (scanf("%lld", &r) != 1) exit(0);
    if (r < 0) exit(0);
    return r;
}

int n;
vector<int> par, dep;
vector<int> U, ordv, fv_found;
vector<long long> fv;
int curU;

long long getf(int j) {
    if (fv[j] >= 0) return fv[j];
    vector<int> S(ordv.begin(), ordv.begin() + j);
    long long q2 = (j >= 2) ? query(S) : 0;
    S.push_back(curU);
    long long q1 = query(S);
    fv[j] = q1 - q2;
    return fv[j];
}

void rec(int lo, int hi) {
    long long cnt = fv[hi] - fv[lo];
    if (cnt == 0) return;
    if (hi - lo == 1 || cnt == hi - lo) {
        for (int i = lo; i < hi; i++) fv_found.push_back(ordv[i]);
        return;
    }
    int mid = (lo + hi) / 2;
    getf(mid);
    rec(lo, mid);
    rec(mid, hi);
}

int main() {
    if (scanf("%d", &n) != 1) return 0;
    if (n == 1) {
        printf("Y 1\n1\n");
        fflush(stdout);
        return 0;
    }
    par.assign(n + 1, 0);
    dep.assign(n + 1, 0);
    for (int i = 2; i <= n; i++) U.push_back(i);
    long long eU = (U.size() >= 2) ? query(U) : 0;
    deque<int> dq;
    dq.push_back(1);
    while (!dq.empty() && !U.empty()) {
        int u = dq.front();
        dq.pop_front();
        vector<int> S = U;
        S.push_back(u);
        long long q = query(S);
        long long d = q - eU;
        if (d <= 0) continue;
        ordv = U;
        curU = u;
        int sz = ordv.size();
        fv.assign(sz + 1, -1);
        fv[0] = 0;
        fv[sz] = d;
        fv_found.clear();
        rec(0, sz);
        set<int> fs(fv_found.begin(), fv_found.end());
        vector<int> nu;
        for (int x : ordv) {
            if (fs.count(x)) {
                par[x] = u;
                dep[x] = dep[u] + 1;
                dq.push_back(x);
            } else
                nu.push_back(x);
        }
        U = nu;
        if (U.size() >= 2 && !U.empty()) eU = query(U);
        else eU = 0;
    }
    vector<int> side[2];
    for (int i = 1; i <= n; i++) side[dep[i] & 1].push_back(i);
    for (int c = 0; c < 2; c++) {
        vector<int>& A = side[c];
        if (A.size() < 2) continue;
        long long e = query(A);
        if (e > 0) {
            // smallest prefix k with e>0
            int lo = 2, hi = A.size();
            while (lo < hi) {
                int mid = (lo + hi) / 2;
                vector<int> S(A.begin(), A.begin() + mid);
                if (query(S) > 0) hi = mid;
                else lo = mid + 1;
            }
            int k = lo;
            int v = A[k - 1];
            // neighbors of v in A[0..k-1)
            int l2 = 1, h2 = k - 1;
            while (l2 < h2) {
                int mid = (l2 + h2) / 2;
                vector<int> S(A.begin(), A.begin() + mid);
                S.push_back(v);
                if (query(S) > 0) h2 = mid;
                else l2 = mid + 1;
            }
            int w = A[l2 - 1];
            // path
            vector<int> pv, pw;
            int a = v, b = w;
            while (dep[a] > dep[b]) { pv.push_back(a); a = par[a]; }
            while (dep[b] > dep[a]) { pw.push_back(b); b = par[b]; }
            while (a != b) {
                pv.push_back(a); a = par[a];
                pw.push_back(b); b = par[b];
            }
            pv.push_back(a);
            reverse(pw.begin(), pw.end());
            vector<int> cyc = pv;
            for (int x : pw) cyc.push_back(x);
            printf("N %d\n", (int)cyc.size());
            for (size_t i = 0; i < cyc.size(); i++) printf("%d ", cyc[i]);
            printf("\n");
            fflush(stdout);
            return 0;
        }
    }
    printf("Y %d\n", (int)side[0].size());
    for (size_t i = 0; i < side[0].size(); i++) printf("%d ", side[0][i]);
    printf("\n");
    fflush(stdout);
    return 0;
}
