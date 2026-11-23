#include <bits/stdc++.h>
using namespace std;

typedef long long ll;
ll N, X0, Y0;

bool ask(ll x, ll y) {
    if (x < 1 || x > N || y < 1 || y > N) return false;
    printf("? %lld %lld\n", x, y);
    fflush(stdout);
    char buf[16];
    if (scanf("%15s", buf) != 1) exit(0);
    return buf[0] == 't';
}

vector<pair<int,int>> qs;
vector<unsigned> qm;
unordered_map<unsigned, int> memo;

int f(unsigned mask) {
    if (__builtin_popcount(mask) <= 1) return 0;
    auto it = memo.find(mask);
    if (it != memo.end()) return it->second;
    int best = 100;
    for (size_t q = 0; q < qs.size(); q++) {
        unsigned y = mask & qm[q], n = mask & ~qm[q];
        if (!y || !n) continue;
        best = min(best, 1 + max(f(y), f(n)));
    }
    memo[mask] = best;
    return best;
}

int main() {
    if (scanf("%lld %lld %lld", &N, &X0, &Y0) != 3) return 0;
    // search along x with direction dir
    auto search = [&](int dir) -> ll {
        ll lo = 0, d = 1, hi;
        while (true) {
            if (ask(X0 + dir * d, Y0)) {
                lo = d;
                d *= 2;
            } else {
                hi = d;
                break;
            }
        }
        while (hi - lo > 1) {
            ll mid = (lo + hi) / 2;
            if (ask(X0 + dir * mid, Y0)) lo = mid;
            else hi = mid;
        }
        return lo;
    };
    ll r = search(1);
    ll l = search(-1);
    ll M = r + l + 1;
    ll cx = X0 - l + (M - 1) / 2;
    ll lo = 0, hi = M;
    while (hi - lo > 1) {
        ll mid = (lo + hi) / 2;
        if (ask(X0, Y0 - mid)) lo = mid;
        else hi = mid;
    }
    ll b = lo;
    ll cy = Y0 - b + (M - 1) / 2;

    // candidates
    vector<pair<int,int>> cand;
    unsigned full = 0;
    for (int i = 0; i < 5; i++)
        for (int j = 0; j < 5; j++)
            if ((i + j) % 2 == 0) {
                cand.push_back({i, j});
                full |= 1u << (i * 5 + j);
            }
    for (int a = -4; a <= 4; a++)
        for (int c = -4; c <= 4; c++) {
            if (((a + c) % 2 + 2) % 2 != 0) continue;
            if (a == 0 && c == 0) continue;
            unsigned m = 0;
            for (auto &p : cand) {
                int ni = p.first + a, nj = p.second + c;
                if (ni >= 0 && ni <= 4 && nj >= 0 && nj <= 4) m |= 1u << (p.first * 5 + p.second);
            }
            qs.push_back({a, c});
            qm.push_back(m);
        }
    unsigned mask = full;
    while (__builtin_popcount(mask) > 1) {
        int bestq = -1, bestv = 1000;
        for (size_t q = 0; q < qs.size(); q++) {
            unsigned y = mask & qm[q], n = mask & ~qm[q];
            if (!y || !n) continue;
            int v = 1 + max(f(y), f(n));
            if (v < bestv) {
                bestv = v;
                bestq = q;
            }
        }
        bool ans = ask(cx + qs[bestq].first * M, cy + qs[bestq].second * M);
        if (ans) mask &= qm[bestq];
        else mask &= ~qm[bestq];
    }
    int idx = __builtin_ctz(mask);
    int i = idx / 5, j = idx % 5;
    ll ax = cx + (ll)(2 - i) * M;
    ll ay = cy + (ll)(2 - j) * M;
    printf("! %lld %lld\n", ax, ay);
    fflush(stdout);
    return 0;
}
