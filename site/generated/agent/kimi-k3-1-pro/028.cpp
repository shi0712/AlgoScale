#include <bits/stdc++.h>
using namespace std;

static constexpr int MOD = 998244353;
static constexpr int A = 6000;

int addmod(int a, int b) {
    int s = a + b;
    return s >= MOD ? s - MOD : s;
}

int mulmod(int a, int b) {
    return int((long long)a * b % MOD);
}

int power(int a, long long e) {
    int r = 1;
    while (e) {
        if (e & 1) r = mulmod(r, a);
        a = mulmod(a, a);
        e >>= 1;
    }
    return r;
}

struct Factor {
    int scope;
    vector<int> value;
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    cin >> n;
    vector<int> count(A + 1, 0);
    for (int i = 0, a; i < n; ++i) {
        cin >> a;
        ++count[a];
    }

    vector<int> spf(A + 1);
    iota(spf.begin(), spf.end(), 0);
    for (int p = 2; p * p <= A; ++p) {
        if (spf[p] != p) continue;
        for (int x = p * p; x <= A; x += p) {
            if (spf[x] == x) spf[x] = p;
        }
    }

    vector<int> primes;
    for (int p = 2; p * p <= A; ++p) {
        if (spf[p] == p) primes.push_back(p);
    }
    int smallCount = (int)primes.size();
    vector<int> primeIndex(A + 1, -1);
    for (int i = 0; i < smallCount; ++i) primeIndex[primes[i]] = i;

    struct Entry {
        int mask;
        int weight;
    };
    vector<vector<Entry>> groups(A + 1);
    vector<int> smallWeight(1 << smallCount, 1);
    int constant = 1;

    for (int a = 1; a <= A; ++a) {
        if (!count[a]) continue;
        int x = a, mask = 0, large = 1;
        while (x > 1) {
            int p = spf[x];
            x /= p;
            if (primeIndex[p] >= 0) mask |= 1 << primeIndex[p];
            else large = p;
        }
        int w = power(a + 1, count[a]);
        if (large == 1) {
            if (!mask) constant = mulmod(constant, w);
            else smallWeight[mask] = mulmod(smallWeight[mask], w);
        } else {
            groups[large].push_back({mask, w});
        }
    }

    vector<Factor> factors;
    for (int mask = 1; mask < (1 << smallCount); ++mask) {
        if (smallWeight[mask] == 1) continue;
        Factor f{mask, vector<int>(1 << __builtin_popcount(mask), 1)};
        f.value.back() = smallWeight[mask];
        factors.push_back(std::move(f));
    }

    for (int p = 2; p <= A; ++p) {
        if (groups[p].empty()) continue;
        int scope = 0;
        for (const Entry &e : groups[p]) scope |= e.mask;
        int k = __builtin_popcount(scope);
        vector<int> localBit(smallCount, -1);
        for (int i = 0, j = 0; i < smallCount; ++i) {
            if (scope >> i & 1) localBit[i] = j++;
        }
        vector<int> product(1 << k, 1);
        for (const Entry &e : groups[p]) {
            int localMask = 0;
            for (int i = 0; i < smallCount; ++i) {
                if (e.mask >> i & 1) localMask |= 1 << localBit[i];
            }
            product[localMask] = mulmod(product[localMask], e.weight);
        }
        for (int b = 0; b < k; ++b) {
            int bit = 1 << b;
            for (int mask = 0; mask < (1 << k); ++mask) {
                if (mask & bit) {
                    product[mask] = mulmod(product[mask], product[mask ^ bit]);
                }
            }
        }
        int inverse = power(p, MOD - 2), multiplier = p - 1;
        for (int &x : product) {
            x = mulmod(inverse, addmod(1, mulmod(multiplier, x)));
        }
        if (!scope) constant = mulmod(constant, product[0]);
        else factors.push_back({scope, std::move(product)});
    }

    for (int v = smallCount - 1; v >= 0; --v) {
        int newScope = 0;
        for (const Factor &f : factors) {
            if (f.scope >> v & 1) newScope |= f.scope;
        }
        newScope &= ~(1 << v);
        int k = __builtin_popcount(newScope);
        int newSize = 1 << k;
        vector<int> position(smallCount, -1);
        for (int i = 0, j = 0; i < smallCount; ++i) {
            if (newScope >> i & 1) position[i] = j++;
        }
        vector<int> product0(newSize, 1), product1(newSize, 1);

        vector<int> index0(newSize), index1(newSize);
        for (const Factor &f : factors) {
            if (!(f.scope >> v & 1)) continue;
            int localV = __builtin_popcount(f.scope & ((1 << v) - 1));
            vector<int> mappedBit(k, -1);
            for (int i = 0; i < smallCount; ++i) {
                if (i == v || !(f.scope >> i & 1)) continue;
                int b = __builtin_popcount(f.scope & ((1 << i) - 1));
                mappedBit[position[i]] = b;
            }
            for (int mask = 1; mask < newSize; ++mask) {
                int b = __builtin_ctz(mask);
                index0[mask] = index0[mask & (mask - 1)];
                if (mappedBit[b] >= 0) index0[mask] |= 1 << mappedBit[b];
            }
            for (int mask = 0; mask < newSize; ++mask) {
                index1[mask] = index0[mask] | (1 << localV);
                product0[mask] = mulmod(product0[mask], f.value[index0[mask]]);
                product1[mask] = mulmod(product1[mask], f.value[index1[mask]]);
            }
        }

        vector<Factor> nextFactors;
        nextFactors.reserve(factors.size());
        for (Factor &f : factors) {
            if (!(f.scope >> v & 1)) nextFactors.push_back(std::move(f));
        }
        factors = std::move(nextFactors);

        vector<int> value(newSize);
        for (int mask = 0; mask < newSize; ++mask) {
            value[mask] = addmod(product0[mask],
                mulmod(primes[v] - 1, product1[mask]));
        }
        if (!newScope) constant = mulmod(constant, value[0]);
        else factors.push_back({newScope, std::move(value)});
    }

    for (int p : primes) constant = mulmod(constant, power(p, MOD - 2));
    cout << constant << '\n';
    return 0;
}
