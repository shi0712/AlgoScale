#include <bits/stdc++.h>
using namespace std;

int main() {
    int n;
    if (!(cin >> n)) return 0;
    if (n == 1) {
        printf("C 1\n");
        fflush(stdout);
        return 0;
    }
    vector<int> primes;
    {
        vector<bool> comp(n + 1, false);
        for (int i = 2; i <= n; i++) {
            if (!comp[i]) {
                primes.push_back(i);
                for (long long j = (long long)i * i; j <= n; j += i) comp[j] = true;
            }
        }
    }
    reverse(primes.begin(), primes.end());
    vector<char> del(n + 1, 0);
    int sizeS = n;
    auto ask = [&](char t, long long a) -> int {
        printf("%c %lld\n", t, a);
        fflush(stdout);
        int r;
        if (!(cin >> r)) exit(0);
        if (r < 0) exit(0);
        return r;
    };
    size_t idx = 0;
    int P = primes.size();
    while (idx < (size_t)P) {
        bool large = (long long)primes[idx] * primes[idx] > n;
        size_t gsz = large ? 100 : 8;
        vector<int> group;
        while (idx < (size_t)P && group.size() < gsz) {
            bool l2 = (long long)primes[idx] * primes[idx] > n;
            if (l2 != large) break;
            group.push_back(primes[idx]);
            idx++;
        }
        for (int p : group) {
            ask('B', p);
            for (int m = p; m <= n; m += p) {
                if (!del[m]) {
                    del[m] = 1;
                    sizeS--;
                }
            }
        }
        if (idx >= (size_t)P) {
            // last group: still need check
        }
        int r = ask('A', 1);
        if (r > sizeS) {
            vector<int> found;
            long long cur = 1;
            for (int p : group) {
                int c = ask('A', p);
                if (c == 1) {
                    found.push_back(p);
                    cur *= p;
                    if ((long long)p * p > n) break;
                }
            }
            int gmin = group.back();
            vector<int> cands = found;
            for (int q : primes) {
                if (q < gmin) cands.push_back(q);
            }
            sort(cands.begin(), cands.end(), greater<int>());
            for (int q : cands) {
                while (cur * q <= n) {
                    int c = ask('A', cur * q);
                    if (c == 1) cur *= q;
                    else break;
                }
            }
            printf("C %lld\n", cur);
            fflush(stdout);
            return 0;
        }
    }
    printf("C 1\n");
    fflush(stdout);
    return 0;
}
