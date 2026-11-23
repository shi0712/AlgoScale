#include <bits/stdc++.h>
using namespace std;

int N;
int cache_[120][120];

long long ask(int i, int j) {
    if (i == j) return 0;
    if (cache_[i][j] >= 0) return cache_[i][j];
    printf("? %d %d\n", i, j);
    fflush(stdout);
    long long d;
    if (scanf("%lld", &d) != 1) exit(0);
    if (d < 0) exit(0);
    cache_[i][j] = cache_[j][i] = (int)d;
    return d;
}

int main() {
    memset(cache_, -1, sizeof(cache_));
    if (scanf("%d", &N) != 1) return 0;
    int A = 1;
    long long best = -1;
    for (int i = 1; i < N; i++) {
        long long d = ask(0, i);
        if (d > best) {
            best = d;
            A = i;
        }
    }
    long long L = best;
    vector<long long> x(N), h(N);
    x[0] = 0; h[0] = 0;
    x[A] = L; h[A] = 0;
    for (int i = 1; i < N; i++) {
        if (i == A) continue;
        long long d0 = ask(0, i);
        long long dA = ask(A, i);
        x[i] = (d0 + L - dA) / 2;
        h[i] = (d0 + dA - L) / 2;
    }
    set<long long> positions;
    for (int i = 1; i < N; i++) {
        if (i == A) continue;
        positions.insert(x[i]);
    }
    long long R = LLONG_MAX;
    map<long long, long long> rp;
    for (long long p : positions) {
        long long r = 0;
        for (int j = 0; j < N; j++) {
            r = max(r, h[j] + llabs(x[j] - p));
        }
        rp[p] = r;
        R = min(R, r);
    }
    bool anyBalanced = false;
    int half = N / 2;
    for (auto &kv : rp) {
        if (kv.second != R) continue;
        long long p = kv.first;
        int a = 0, b = 0;
        vector<int> grp;
        for (int j = 0; j < N; j++) {
            if (x[j] < p) a++;
            else if (x[j] > p) b++;
            else grp.push_back(j);
        }
        if (a > half || b > half) continue;
        int c = grp.size();
        if (c <= half) {
            anyBalanced = true;
            break;
        }
        // majority detection
        vector<int> G;
        for (int t : grp) {
            if (G.empty()) {
                G.push_back(t);
            } else {
                int rep = G.back();
                long long q = ask(rep, t);
                if (q < h[rep] + h[t]) G.push_back(t);
                else G.pop_back();
            }
        }
        if (G.empty()) {
            anyBalanced = true;
            break;
        }
        int rep = G[0];
        set<int> inG(G.begin(), G.end());
        int cnt = G.size();
        vector<int> others;
        for (int t : grp) if (!inG.count(t)) others.push_back(t);
        bool big = cnt > half;
        int remaining = others.size();
        for (int t : others) {
            if (big) break;
            if (cnt + remaining <= half) break;
            long long q = ask(rep, t);
            if (q < h[rep] + h[t]) cnt++;
            remaining--;
            if (cnt > half) big = true;
        }
        if (!big) {
            anyBalanced = true;
            break;
        }
    }
    printf("! %lld\n", anyBalanced ? R : -R);
    fflush(stdout);
    return 0;
}
