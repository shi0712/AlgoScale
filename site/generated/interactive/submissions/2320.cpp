#include <bits/stdc++.h>
using namespace std;

int N;
map<pair<int,int>, long long> cache_;

long long ask(int i, int j) {
    if (i == j) return 0;
    if (i > j) swap(i, j);
    auto it = cache_.find({i, j});
    if (it != cache_.end()) return it->second;
    printf("? %d %d\n", i, j);
    fflush(stdout);
    long long d;
    if (scanf("%lld", &d) != 1) exit(0);
    if (d < 0) exit(0);
    cache_[{i, j}] = d;
    return d;
}

int main() {
    if (scanf("%d", &N) != 1) return 0;
    vector<long long> d0(N, 0), du(N, 0);
    for (int i = 1; i < N; i++) d0[i] = ask(0, i);
    int u = 1;
    for (int i = 1; i < N; i++) if (d0[i] > d0[u]) u = i;
    for (int i = 0; i < N; i++) {
        if (i == u) du[i] = 0;
        else du[i] = ask(u, i);
    }
    long long D = 0;
    for (int i = 0; i < N; i++) D = max(D, du[i]);
    long long du0 = du[0];
    vector<long long> P(N);
    for (int i = 0; i < N; i++) P[i] = du[i] + du0 - d0[i]; // doubled position
    // candidates: i != u, i != 0
    long long best = LLONG_MAX;
    for (int i = 0; i < N; i++) {
        if (i == u || i == 0) continue;
        long long val = max(P[i], 2 * D - P[i]);
        best = min(best, val);
    }
    long long R = best / 2;
    set<long long> hubs;
    for (int i = 0; i < N; i++) {
        if (i == u || i == 0) continue;
        long long val = max(P[i], 2 * D - P[i]);
        if (val == best) hubs.insert(P[i]);
    }
    int T = N / 2;
    bool balanced = false;
    for (long long hp : hubs) {
        int L = 0, Rt = 0;
        vector<int> at;
        for (int i = 0; i < N; i++) {
            if (P[i] < hp) L++;
            else if (P[i] > hp) Rt++;
            else at.push_back(i);
        }
        if (L > T || Rt > T) continue;
        if ((int)at.size() <= T) { balanced = true; break; }
        // majority check
        auto same = [&](int a, int b) -> bool {
            long long d = ask(a, b);
            return d < du[a] + du[b] - hp;
        };
        vector<int> Ls, B;
        for (int x : at) {
            if (Ls.empty()) { Ls.push_back(x); continue; }
            if (same(x, Ls.back())) B.push_back(x);
            else {
                Ls.push_back(x);
                if (!B.empty()) { Ls.push_back(B.back()); B.pop_back(); }
            }
        }
        int c = Ls.back();
        int cnt = (int)B.size() + 1;
        int i = (int)Ls.size() - 2;
        bool maj = false;
        while (true) {
            if (cnt > T) { maj = true; break; }
            if (i < 0) break;
            int possible = (i + 2) / 2; // ceil((i+1)/2)
            if (cnt + possible <= T) break;
            if (same(Ls[i], c)) { cnt++; i -= 2; }
            else i -= 1;
        }
        if (!maj) { balanced = true; break; }
    }
    printf("! %lld\n", balanced ? R : -R);
    fflush(stdout);
    return 0;
}
