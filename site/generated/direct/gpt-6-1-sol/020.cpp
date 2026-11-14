#include <bits/stdc++.h>
using namespace std;

static constexpr int MOD = 1000000007;

int modPow(int a, long long e, int mod = MOD) {
    int r = 1;
    while (e > 0) {
        if (e & 1) r = (long long)r * a % mod;
        a = (long long)a * a % mod;
        e >>= 1;
    }
    return r;
}

template<int M, int ROOT>
struct NTT {
    static vector<int>& roots() {
        static vector<int> r{0, 1};
        return r;
    }

    static int power(int a, int e) {
        int r = 1;
        while (e) {
            if (e & 1) r = (long long)r * a % M;
            a = (long long)a * a % M;
            e >>= 1;
        }
        return r;
    }

    static void transform(vector<int>& a, bool inverse) {
        int n = (int)a.size();
        if (inverse && n > 1) reverse(a.begin() + 1, a.end());

        auto& rt = roots();
        if ((int)rt.size() < n) {
            int k = __builtin_ctz((unsigned)rt.size());
            rt.resize(n);
            while ((1 << k) < n) {
                int z = power(ROOT, (M - 1) >> (k + 1));
                for (int i = 1 << (k - 1); i < (1 << k); ++i) {
                    rt[2 * i] = rt[i];
                    rt[2 * i + 1] = (long long)rt[i] * z % M;
                }
                ++k;
            }
        }

        for (int i = 1, j = 0; i < n; ++i) {
            int bit = n >> 1;
            for (; j & bit; bit >>= 1) j ^= bit;
            j ^= bit;
            if (i < j) swap(a[i], a[j]);
        }

        for (int len = 1; len < n; len <<= 1) {
            for (int start = 0; start < n; start += 2 * len) {
                for (int j = 0; j < len; ++j) {
                    int u = a[start + j];
                    int v = (long long)a[start + j + len] * rt[len + j] % M;
                    int x = u + v;
                    if (x >= M) x -= M;
                    int y = u - v;
                    if (y < 0) y += M;
                    a[start + j] = x;
                    a[start + j + len] = y;
                }
            }
        }

        if (inverse) {
            int invN = power(n, M - 2);
            for (int& x : a) x = (long long)x * invN % M;
        }
    }
};

// A pair of bivariate polynomials:
// P(t,x), Q(t,x), stored using index t * (width + 1) + x.
struct PolyPair {
    int degree = 0;
    int width = 0;
    vector<int> p, q;
};

struct Term {
    int t, x, value;
};

vector<Term> getTerms(const vector<int>& a, int degree, int width) {
    vector<Term> result;
    int stride = width + 1;
    for (int t = 0; t <= degree; ++t) {
        for (int x = 0; x <= width; ++x) {
            int value = a[t * stride + x];
            if (value) result.push_back({t, x, value});
        }
    }
    return result;
}

template<int M, int ROOT>
pair<vector<int>, vector<int>> multiplyUnderPrime(
    const PolyPair& a, const PolyPair& b, bool needP
) {
    int degree = a.degree + b.degree;
    int width = a.width + b.width;
    int stride = width + 1;
    int outputSize = (degree + 1) * stride;

    int length = 1;
    while (length < outputSize) length <<= 1;

    vector<int> ap(length), aq(length), bp(length), bq(length);

    auto load = [&](const vector<int>& source, int d, int w,
                    vector<int>& target) {
        int oldStride = w + 1;
        for (int t = 0; t <= d; ++t) {
            for (int x = 0; x <= w; ++x) {
                int value = source[t * oldStride + x];
                target[t * stride + x] = value % M;
            }
        }
    };

    load(a.p, a.degree, a.width, ap);
    load(a.q, a.degree, a.width, aq);
    load(b.p, b.degree, b.width, bp);
    load(b.q, b.degree, b.width, bq);

    NTT<M, ROOT>::transform(ap, false);
    NTT<M, ROOT>::transform(aq, false);
    NTT<M, ROOT>::transform(bp, false);
    NTT<M, ROOT>::transform(bq, false);

    for (int i = 0; i < length; ++i) {
        int left = (long long)aq[i] * bp[i] % M;
        int right = (long long)ap[i] * bq[i] % M;
        int sum = left + right;
        if (sum >= M) sum -= M;
        aq[i] = sum;
        if (needP) ap[i] = (long long)ap[i] * bp[i] % M;
    }

    vector<int>().swap(bp);
    vector<int>().swap(bq);

    NTT<M, ROOT>::transform(aq, true);
    aq.resize(outputSize);

    if (needP) {
        NTT<M, ROOT>::transform(ap, true);
        ap.resize(outputSize);
    } else {
        vector<int>().swap(ap);
    }

    return {move(ap), move(aq)};
}

vector<int> reconstructCRT(
    const vector<int>& a, const vector<int>& b, const vector<int>& c
) {
    static constexpr long long P1 = 998244353;
    static constexpr long long P2 = 1004535809;
    static constexpr long long P3 = 469762049;

    static const long long invP1P2 = modPow(P1 % P2, P2 - 2, P2);
    static const long long p12modP3 = P1 % P3 * (P2 % P3) % P3;
    static const long long invP12P3 = modPow(p12modP3, P3 - 2, P3);
    static const long long p12modMOD = P1 % MOD * (P2 % MOD) % MOD;

    vector<int> result(a.size());

    for (int i = 0; i < (int)a.size(); ++i) {
        long long t = (b[i] - (long long)a[i]) % P2;
        if (t < 0) t += P2;
        t = t * invP1P2 % P2;

        long long xmodP3 = (a[i] + (P1 % P3) * t) % P3;
        long long u = (c[i] - xmodP3 + P3) % P3;
        u = u * invP12P3 % P3;

        result[i] = (
            a[i] + (P1 % MOD) * (t % MOD) % MOD
            + p12modMOD * (u % MOD)
        ) % MOD;
    }

    return result;
}

// (P1,Q1) * (P2,Q2) = (P1*P2, Q1*P2 + P1*Q2).
PolyPair combine(const PolyPair& a, const PolyPair& b, bool needP) {
    PolyPair result;
    result.degree = a.degree + b.degree;
    result.width = a.width + b.width;

    int stride = result.width + 1;
    int outputSize = (result.degree + 1) * stride;

    auto ap = getTerms(a.p, a.degree, a.width);
    auto aq = getTerms(a.q, a.degree, a.width);
    auto bp = getTerms(b.p, b.degree, b.width);
    auto bq = getTerms(b.q, b.degree, b.width);

    long long work =
        (long long)aq.size() * bp.size()
        + (long long)ap.size() * bq.size();

    if (needP) work += (long long)ap.size() * bp.size();

    if (work <= 1000000) {
        if (needP) result.p.assign(outputSize, 0);
        result.q.assign(outputSize, 0);

        auto multiplyAdd = [&](const vector<Term>& x,
                               const vector<Term>& y,
                               vector<int>& out) {
            for (const auto& u : x) {
                for (const auto& v : y) {
                    int index = (u.t + v.t) * stride + u.x + v.x;
                    int add = (long long)u.value * v.value % MOD;
                    int value = out[index] + add;
                    if (value >= MOD) value -= MOD;
                    out[index] = value;
                }
            }
        };

        if (needP) multiplyAdd(ap, bp, result.p);
        multiplyAdd(aq, bp, result.q);
        multiplyAdd(ap, bq, result.q);
        return result;
    }

    // Release sparse representations before the larger NTT allocations.
    vector<Term>().swap(ap);
    vector<Term>().swap(aq);
    vector<Term>().swap(bp);
    vector<Term>().swap(bq);

    auto r1 = multiplyUnderPrime<998244353, 3>(a, b, needP);
    auto r2 = multiplyUnderPrime<1004535809, 3>(a, b, needP);
    auto r3 = multiplyUnderPrime<469762049, 3>(a, b, needP);

    if (needP) {
        result.p = reconstructCRT(r1.first, r2.first, r3.first);
    }
    result.q = reconstructCRT(r1.second, r2.second, r3.second);
    return result;
}

int n;
vector<vector<int>> graph;
vector<int> subtreeSize;
vector<vector<int>> dp;
vector<int> fact, invFact;

// Construct the group polynomials for a subtree with s vertices.
// At most limit vertices may be used in the root-level prefix.
PolyPair makeGroup(int s, const vector<int>& ways, int limit) {
    limit = min(limit, s);

    PolyPair result;
    result.degree = s;
    result.width = limit;

    int stride = limit + 1;
    int totalSize = (s + 1) * stride;
    result.p.assign(totalSize, 0);
    result.q.assign(totalSize, 0);

    result.p[0] = 1;

    // P[t^a x^b] =
    // (s)_a * (-1)^(a-b) * C(a-1,b-1) / b!.
    for (int a = 1; a <= limit; ++a) {
        int falling = (long long)fact[s] * invFact[s - a] % MOD;
        for (int b = 1; b <= a; ++b) {
            int value = (long long)falling * fact[a - 1] % MOD;
            value = (long long)value * invFact[b - 1] % MOD;
            value = (long long)value * invFact[a - b] % MOD;
            value = (long long)value * invFact[b] % MOD;
            if ((a - b) & 1) value = value ? MOD - value : 0;
            result.p[a * stride + b] = value;
        }
    }

    // q[k] = (-1)^k * ways[k] * (s-k)!.
    vector<int> weighted(s + 1, 0);
    for (int k = 1; k <= s; ++k) {
        weighted[k] = (long long)ways[k] * fact[s - k] % MOD;
        if (k & 1) {
            if (weighted[k]) weighted[k] = MOD - weighted[k];
        }
    }

    vector<int> moment(limit + 1, 0);
    vector<int> fallingLimit(limit + 1, 1);
    for (int b = 1; b <= limit; ++b) {
        fallingLimit[b] =
            (long long)fallingLimit[b - 1] * (limit + 2 - b) % MOD;
    }

    // moment[b] = sum weighted[k] * (m-k)_b,
    // with 0 <= m-k <= limit.
    for (int m = 1; m <= s; ++m) {
        for (int b = limit; b >= 1; --b) {
            moment[b] =
                (moment[b] + (long long)b * moment[b - 1]) % MOD;
        }

        moment[0] += weighted[m];
        if (moment[0] >= MOD) moment[0] -= MOD;

        int removed = m - limit - 1;
        if (removed >= 1) {
            for (int b = 0; b <= limit; ++b) {
                int sub =
                    (long long)weighted[removed] * fallingLimit[b] % MOD;
                moment[b] -= sub;
                if (moment[b] < 0) moment[b] += MOD;
            }
        }

        int maxB = min(limit, m - 1);
        for (int b = 0; b <= maxB; ++b) {
            int value = (long long)moment[b] * invFact[b] % MOD;
            value = (long long)value * invFact[b] % MOD;
            value = (long long)value * invFact[s - m] % MOD;
            if ((m - b) & 1) value = value ? MOD - value : 0;
            result.q[m * stride + b] = value;
        }
    }

    return result;
}

void solve(int u, int parent) {
    vector<int> children;
    subtreeSize[u] = 1;

    for (int v : graph[u]) {
        if (v == parent) continue;
        solve(v, u);
        children.push_back(v);
        subtreeSize[u] += subtreeSize[v];
    }

    int size = subtreeSize[u];
    dp[u].assign(size + 1, 0);

    if (children.empty()) {
        dp[u][1] = 1;
        return;
    }

    // Important special case: avoid creating quadratic polynomials
    // along long chains.
    if (children.size() == 1) {
        int v = children[0];
        int s = subtreeSize[v];

        auto get = [&](int k) -> int {
            if (k == 0) return 1;
            if (k < 0 || k > s) return 0;
            return dp[v][k];
        };

        for (int k = 1; k <= size; ++k) {
            long long answer = get(k);
            answer += get(k - 1);
            if (k >= 2) {
                answer += (long long)(s - k + 2) * get(k - 2);
            }
            dp[u][k] = answer % MOD;
        }
        return;
    }

    int heavy = children[0];
    for (int v : children) {
        if (subtreeSize[v] > subtreeSize[heavy]) heavy = v;
    }

    // Combine the light groups, including the vertex u itself.
    vector<unique_ptr<PolyPair>> pool;
    priority_queue<pair<int, int>,
                   vector<pair<int, int>>,
                   greater<pair<int, int>>> queue;

    auto insert = [&](PolyPair value) {
        int id = (int)pool.size();
        int degree = value.degree;
        pool.push_back(make_unique<PolyPair>(move(value)));
        queue.push({degree, id});
    };

    vector<int> singletonWays{0, 1};
    insert(makeGroup(1, singletonWays, 1));

    for (int v : children) {
        if (v != heavy) {
            insert(makeGroup(subtreeSize[v], dp[v], subtreeSize[v]));
        }
    }

    while (queue.size() > 1) {
        int first = queue.top().second;
        queue.pop();
        int second = queue.top().second;
        queue.pop();

        PolyPair merged = combine(*pool[first], *pool[second], true);
        pool[first].reset();
        pool[second].reset();
        insert(move(merged));
    }

    int lightId = queue.top().second;
    int lightSize = size - subtreeSize[heavy];

    PolyPair heavyGroup = makeGroup(
        subtreeSize[heavy],
        dp[heavy],
        min(subtreeSize[heavy], lightSize + 1)
    );

    PolyPair all = combine(heavyGroup, *pool[lightId], false);

    int stride = all.width + 1;
    for (int k = 1; k <= size; ++k) {
        long long answer = 0;
        for (int b = 0; b <= min(k, all.width); ++b) {
            answer += (long long)all.q[k * stride + b] * fact[b] % MOD;
            if (answer >= (long long)MOD * MOD) answer %= MOD;
        }
        dp[u][k] = answer % MOD;
    }
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    cin >> n;
    graph.resize(n);
    subtreeSize.resize(n);
    dp.resize(n);

    for (int i = 0; i < n - 1; ++i) {
        int u, v;
        cin >> u >> v;
        --u;
        --v;
        graph[u].push_back(v);
        graph[v].push_back(u);
    }

    fact.resize(n + 3);
    invFact.resize(n + 3);

    fact[0] = 1;
    for (int i = 1; i < (int)fact.size(); ++i) {
        fact[i] = (long long)fact[i - 1] * i % MOD;
    }

    invFact.back() = modPow(fact.back(), MOD - 2);
    for (int i = (int)invFact.size() - 1; i >= 1; --i) {
        invFact[i - 1] = (long long)invFact[i] * i % MOD;
    }

    solve(0, -1);
    cout << dp[0][n] << '\n';
    return 0;
}
