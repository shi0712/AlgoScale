#include <bits/stdc++.h>
using namespace std;

static constexpr int MOD = 998244353;
static constexpr int ROOT = 3;

int modpow(int a, int e) {
    int r = 1;
    while (e) {
        if (e & 1) r = (long long)r * a % MOD;
        a = (long long)a * a % MOD;
        e >>= 1;
    }
    return r;
}

class Solver {
    using Poly = vector<int>;

    int n;
    uint64_t k;
    vector<uint64_t> values;
    vector<int> prefixLeft;
    vector<int> fact, ifact;

    int maxLog, maxNTT;
    vector<int> roots, rev;
    vector<int> inverseLength;

    void ntt(Poly &a, bool inverse) {
        int sz = (int)a.size();
        int lg = __builtin_ctz((unsigned)sz);
        int shift = maxLog - lg;

        if (inverse) reverse(a.begin() + 1, a.end());

        for (int i = 0; i < sz; ++i) {
            int j = rev[i] >> shift;
            if (i < j) swap(a[i], a[j]);
        }

        for (int len = 1; len < sz; len <<= 1) {
            for (int start = 0; start < sz; start += len << 1) {
                int *p = a.data() + start;
                for (int j = 0; j < len; ++j) {
                    int x = p[j];
                    int y = (long long)p[j + len] * roots[len + j] % MOD;
                    int sum = x + y;
                    if (sum >= MOD) sum -= MOD;
                    int difference = x - y;
                    if (difference < 0) difference += MOD;
                    p[j] = sum;
                    p[j + len] = difference;
                }
            }
        }

        if (inverse) {
            int inv = inverseLength[lg];
            for (int &x : a) x = (long long)x * inv % MOD;
        }
    }

    Poly multiply(Poly a, Poly b, int limit = INT_MAX) {
        int need = min<long long>(
            limit, (long long)a.size() + b.size() - 1
        );
        if (need <= 0) return {};

        if ((int)a.size() > need) a.resize(need);
        if ((int)b.size() > need) b.resize(need);

        if (a.size() > b.size()) swap(a, b);

        if (a.size() == 1) {
            b.resize(need);
            if (a[0] != 1) {
                for (int &x : b) x = (long long)x * a[0] % MOD;
            }
            return b;
        }

        if (a.size() <= 32 ||
            (long long)a.size() * b.size() <= 4096) {
            Poly result(need);
            int as = (int)a.size();
            int bs = (int)b.size();

            // At most 16 products fit in an unsigned 64-bit accumulator.
            for (int t = 0; t < need; ++t) {
                int lo = max(0, t - bs + 1);
                int hi = min(as - 1, t);
                int answer = 0;

                for (int first = lo; first <= hi; first += 16) {
                    int last = min(hi, first + 15);
                    uint64_t sum = 0;
                    for (int i = first; i <= last; ++i) {
                        sum += (uint64_t)a[i] * b[t - i];
                    }
                    int part = sum % MOD;
                    answer += part;
                    if (answer >= MOD) answer -= MOD;
                }
                result[t] = answer;
            }
            return result;
        }

        int size = 1;
        while (size < (int)a.size() + (int)b.size() - 1) size <<= 1;
        a.resize(size);
        b.resize(size);

        ntt(a, false);
        ntt(b, false);
        for (int i = 0; i < size; ++i) {
            a[i] = (long long)a[i] * b[i] % MOD;
        }
        ntt(a, true);
        a.resize(need);
        return a;
    }

    Poly completeGraph(int left, int right) const {
        int degree = min(left, right);
        Poly result(degree + 1);
        int base = (long long)fact[left] * fact[right] % MOD;

        for (int i = 0; i <= degree; ++i) {
            int x = (long long)base * ifact[left - i] % MOD;
            x = (long long)x * ifact[right - i] % MOD;
            result[i] = (long long)x * ifact[i] % MOD;
        }
        return result;
    }

    // Matching polynomial of the bipartite complement.
    Poly complement(Poly f, int left, int right) {
        if (f.size() == 1) return completeGraph(left, right);

        int degree = min(left, right);
        for (int i = 0; i < (int)f.size(); ++i) {
            int x = (long long)f[i] * fact[left - i] % MOD;
            x = (long long)x * fact[right - i] % MOD;
            if (i & 1) x = x ? MOD - x : 0;
            f[i] = x;
        }

        Poly exponential(degree + 1);
        copy(ifact.begin(), ifact.begin() + degree + 1,
             exponential.begin());

        Poly result = multiply(move(f), move(exponential), degree + 1);
        for (int i = 0; i <= degree; ++i) {
            int x = (long long)result[i] * ifact[left - i] % MOD;
            result[i] = (long long)x * ifact[right - i] % MOD;
        }
        return result;
    }

    // Add count vertices, all adjacent to every vertex on the opposite side.
    // 'opposite' is the number of vertices on that opposite side.
    Poly addUniversal(Poly f, int count, int opposite) {
        if (count == 0 || opposite == 0) return f;

        if (count <= 8) {
            for (int step = 0; step < count; ++step) {
                int degree = min(opposite, (int)f.size());
                f.resize(degree + 1, 0);
                for (int i = degree; i >= 1; --i) {
                    int extra =
                        (long long)f[i - 1] * (opposite - i + 1) % MOD;
                    f[i] += extra;
                    if (f[i] >= MOD) f[i] -= MOD;
                }
            }
            return f;
        }

        int degree = min(opposite, (int)f.size() - 1 + count);
        for (int i = 0; i < (int)f.size(); ++i) {
            f[i] = (long long)f[i] * fact[opposite - i] % MOD;
        }

        int bound = min(count, degree);
        Poly choose(bound + 1);
        for (int i = 0; i <= bound; ++i) {
            int x = (long long)fact[count] * ifact[i] % MOD;
            choose[i] = (long long)x * ifact[count - i] % MOD;
        }

        Poly result = multiply(move(f), move(choose), degree + 1);
        for (int i = 0; i <= degree; ++i) {
            result[i] =
                (long long)result[i] * ifact[opposite - i] % MOD;
        }
        return result;
    }

    // Join a large graph to a small graph.
    // missing[j] counts j-matchings in the small graph's complement.
    Poly joinSmall(const Poly &large, const Poly &missing,
                   int largeLeft, int largeRight,
                   int smallLeft, int smallRight) {
        int degree = min(largeLeft + smallLeft, largeRight + smallRight);
        degree = min(degree,
                     (int)large.size() - 1 + smallLeft + smallRight);
        Poly result(degree + 1);

        for (int j = 0; j < (int)missing.size(); ++j) {
            if (missing[j] == 0) continue;

            Poly current = addUniversal(large, smallLeft - j, largeRight);
            current = addUniversal(
                move(current), smallRight - j,
                largeLeft + smallLeft - j
            );

            int coefficient = missing[j];
            if (j & 1) coefficient = MOD - coefficient;

            for (int i = 0;
                 i < (int)current.size() && i + j <= degree; ++i) {
                int value = (long long)current[i] * coefficient % MOD;
                int &target = result[i + j];
                target += value;
                if (target >= MOD) target -= MOD;
            }
        }

        return result;
    }

    int leftCount(int l, int r) const {
        return prefixLeft[r] - prefixLeft[l];
    }

    // flipped = true asks for the bipartite complement of this subgraph.
    Poly solve(int l, int r, bool flipped) {
        int left = leftCount(l, r);
        int right = r - l - left;
        if (left == 0 || right == 0) return {1};

        uint64_t difference = (values[l] ^ values[r - 1]) >> 1;
        if (difference == 0) {
            // a == (b xor k), hence a xor b == k.
            return flipped ? Poly{1} : completeGraph(left, right);
        }

        int bit = 63 - __builtin_clzll(difference);
        uint64_t mask = 1ULL << (bit + 1);

        int low = l, high = r;
        while (low < high) {
            int mid = (low + high) >> 1;
            if (values[mid] & mask) high = mid;
            else low = mid + 1;
        }
        int middle = low;

        int left0 = leftCount(l, middle);
        int right0 = middle - l - left0;
        int left1 = left - left0;
        int right1 = right - right0;

        bool joined = (((k >> bit) & 1ULL) == 0) ^ flipped;
        bool pure0 = left0 == 0 || right0 == 0;
        bool pure1 = left1 == 0 || right1 == 0;

        if (!joined) {
            if (pure0) return solve(middle, r, flipped);
            if (pure1) return solve(l, middle, flipped);

            Poly first = solve(l, middle, flipped);
            Poly second = solve(middle, r, flipped);
            return multiply(move(first), move(second));
        }

        // A one-sided child simply adds universal vertices.
        if (pure0) {
            Poly other = solve(middle, r, flipped);
            if (left0 != 0) {
                return addUniversal(move(other), left0, right1);
            }
            return addUniversal(move(other), right0, left1);
        }
        if (pure1) {
            Poly other = solve(l, middle, flipped);
            if (left1 != 0) {
                return addUniversal(move(other), left1, right0);
            }
            return addUniversal(move(other), right1, left0);
        }

        // Avoid large transforms along highly unbalanced trie paths.
        int size0 = middle - l;
        int size1 = r - middle;
        if (min(size0, size1) <= 8) {
            if (size0 <= size1) {
                Poly large = solve(middle, r, flipped);
                Poly missing = solve(l, middle, !flipped);
                return joinSmall(large, missing,
                                 left1, right1, left0, right0);
            } else {
                Poly large = solve(l, middle, flipped);
                Poly missing = solve(middle, r, !flipped);
                return joinSmall(large, missing,
                                 left0, right0, left1, right1);
            }
        }

        // Complementing a join turns it into a disjoint union.
        Poly first = solve(l, middle, !flipped);
        Poly second = solve(middle, r, !flipped);
        Poly united = multiply(move(first), move(second));
        return complement(move(united), left, right);
    }

public:
    Solver(int n_, uint64_t k_) : n(n_), k(k_) {
        fact.resize(n + 1);
        ifact.resize(n + 1);
        fact[0] = 1;
        for (int i = 1; i <= n; ++i) {
            fact[i] = (long long)fact[i - 1] * i % MOD;
        }
        ifact[n] = modpow(fact[n], MOD - 2);
        for (int i = n; i >= 1; --i) {
            ifact[i - 1] = (long long)ifact[i] * i % MOD;
        }

        maxNTT = 1;
        maxLog = 0;
        while (maxNTT < 2 * n + 1) {
            maxNTT <<= 1;
            ++maxLog;
        }

        roots.resize(maxNTT);
        roots[1] = 1;
        for (int len = 1; len < maxNTT / 2; len <<= 1) {
            int z = modpow(ROOT, (MOD - 1) / (len << 2));
            for (int i = len; i < (len << 1); ++i) {
                roots[i << 1] = roots[i];
                roots[i << 1 | 1] = (long long)roots[i] * z % MOD;
            }
        }

        rev.resize(maxNTT);
        for (int i = 1; i < maxNTT; ++i) {
            rev[i] = (rev[i >> 1] >> 1)
                   | ((i & 1) << (maxLog - 1));
        }

        inverseLength.resize(maxLog + 1);
        inverseLength[0] = 1;
        constexpr int INV2 = (MOD + 1) / 2;
        for (int i = 1; i <= maxLog; ++i) {
            inverseLength[i] =
                (long long)inverseLength[i - 1] * INV2 % MOD;
        }
    }

    void run() {
        values.reserve(2 * n);

        for (int i = 0; i < n; ++i) {
            uint64_t x;
            cin >> x;
            values.push_back(x << 1); // Left vertex.
        }
        for (int i = 0; i < n; ++i) {
            uint64_t x;
            cin >> x;
            values.push_back(((x ^ k) << 1) | 1ULL); // Right vertex.
        }

        sort(values.begin(), values.end());

        prefixLeft.resize(2 * n + 1);
        for (int i = 0; i < 2 * n; ++i) {
            prefixLeft[i + 1] =
                prefixLeft[i] + ((values[i] & 1ULL) == 0);
        }

        Poly answer = solve(0, 2 * n, false);
        answer.resize(n + 1, 0);

        for (int i = 1; i <= n; ++i) {
            if (i > 1) cout << ' ';
            cout << answer[i];
        }
        cout << '\n';
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    uint64_t k;
    cin >> n >> k;

    Solver solver(n, k);
    solver.run();
    return 0;
}
