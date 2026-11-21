#include <bits/stdc++.h>
using namespace std;

int ask(char op, int a) {
    cout << op << ' ' << a << endl;
    int r;
    if (!(cin >> r) || r < 0) exit(0);
    return r;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n) || n < 1) return 0;

    vector<int> primes;
    vector<bool> composite(n + 1);
    for (int p = 2; p <= n; ++p) {
        if (composite[p]) continue;
        primes.push_back(p);
        for (long long j = 1LL * p * p; j <= n; j += p)
            composite[j] = true;
    }

    // Model the set that would remain without the exception for x.
    vector<int> first(n + 1), hits(n + 1);
    vector<int> candidates;
    int remaining = n;
    const int P = (int)primes.size();

    for (int l = 0; l < P && candidates.empty(); ) {
        int len = max(1, (int)sqrt(2.0 * min(l + 1, P - l)));
        int end = min(P, l + len);

        for (int i = l; i < end; ++i) {
            int p = primes[i], expected = 0;
            for (int v = p; v <= n; v += p) {
                if (!hits[v]) {
                    first[v] = p;
                    ++expected;
                }
                ++hits[v];
            }
            remaining -= expected;
            if (ask('B', p) != expected) {
                // x was already removed in the model. This is the first
                // repeated hit, and its first hit was in this block.
                for (int v = p; v <= n; v += p)
                    if (first[v] >= primes[l] && hits[v] == 2)
                        candidates.push_back(v);
                break;
            }
        }

        if (!candidates.empty()) break;
        if (ask('A', 1) != remaining) {
            // Exactly one processed prime hit x, within this block.
            for (int v = 2; v <= n; ++v)
                if (first[v] >= primes[l] && hits[v] == 1)
                    candidates.push_back(v);
        }
        l = end;
    }

    if (candidates.empty()) {
        cout << "C 1" << endl;
        return 0;
    }
    if (candidates.size() == 1) {
        cout << "C " << candidates[0] << endl;
        return 0;
    }

    // The real set is now precisely the model plus x. Thus any A d
    // is a divisibility test. Choose a most balanced nontrivial split.
    vector<char> possible(n + 1);
    int left = (int)candidates.size(), xr = 0;
    for (int v : candidates) {
        possible[v] = true;
        xr ^= v;
    }

    vector<vector<int>> divisors(n + 1);
    vector<int> count(n + 1), base(n + 1);
    set<pair<int, int>> ranked;
    for (int d = 2; d <= n; ++d) {
        for (int v = d; v <= n; v += d) {
            if (!hits[v]) ++base[d];
            if (possible[v]) {
                ++count[d];
                divisors[v].push_back(d);
            }
        }
        if (count[d]) ranked.emplace(count[d], d);
    }

    auto discard = [&](int v) {
        possible[v] = false;
        --left;
        xr ^= v;
        for (int d : divisors[v]) {
            ranked.erase({count[d], d});
            if (--count[d]) ranked.emplace(count[d], d);
        }
    };

    while (left > 1) {
        auto it = ranked.lower_bound({(left + 1) / 2, 0});
        int d = (it == ranked.end() ? 0 : it->second);
        if (it != ranked.begin()) {
            int c = prev(it)->first;
            if (!d || left - 2 * c <= 2 * count[d] - left)
                d = ranked.lower_bound({c, 0})->second;
        }

        if (ask('A', d) == base[d]) {
            for (int v = d; v <= n; v += d)
                if (possible[v]) discard(v);
        } else {
            for (int v : candidates)
                if (possible[v] && v % d != 0) discard(v);
        }
    }
    cout << "C " << xr << endl;
    return 0;
}
