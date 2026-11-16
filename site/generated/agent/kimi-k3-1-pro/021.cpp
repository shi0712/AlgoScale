#include <bits/stdc++.h>
using namespace std;

static constexpr int MOD = 1000000007;

struct Query {
    int a, b, c;
};

int N;
vector<int> mu, lp, largest_prime, tau, D, mertens;

struct Evaluator {
    vector<uint64_t> keys;
    vector<int> values;
    size_t mask;
    size_t used = 0;
    int shift;

    explicit Evaluator(long long total_limit) {
        size_t size = 1024;
        while (size < size_t(total_limit * 8)) size <<= 1;
        keys.assign(size, 0);
        values.resize(size);
        mask = size - 1;
        shift = 64 - __builtin_ctzll(size);
    }

    int base(int x, int y) const {
        long long result = 0;
        for (int l = 1; l <= y;) {
            int qx = x / l;
            int qy = y / l;
            int r = min(y, x / qx);
            r = min(r, y / qy);
            long long coefficient = mertens[r] - mertens[l - 1];
            result += coefficient * D[qx] % MOD * D[qy];
            if (result >= (1LL << 62) || result <= -(1LL << 62)) result %= MOD;
            l = r + 1;
        }
        result %= MOD;
        if (result < 0) result += MOD;
        return int(result);
    }

    int get(int d, int x, int y) {
        if (x == 0 || y == 0) return 0;
        if (x < y) swap(x, y);
        uint64_t key = (uint64_t(d) << 40) | (uint64_t(x) << 20) | uint32_t(y);
        size_t index = (key * 11400714819323198485ULL) >> shift;
        while (keys[index]) {
            if (keys[index] == key) return values[index];
            index = (index + 1) & mask;
        }
        keys[index] = key;
        ++used;

        int result;
        if (d == 1) {
            result = base(x, y);
        } else {
            int p = largest_prime[d];
            int e = d / p;
            long long sum = 0;
            for (int xp = x, yp = y; xp >= p || yp >= p; xp /= p, yp /= p) {
                sum += get(e, xp / p, yp);
                sum += get(e, xp, yp / p);
                sum -= 2LL * get(e, xp / p, yp / p);
            }
            sum %= MOD;
            if (sum < 0) sum += MOD;
            result = int(sum);
        }
        values[index] = result;
        return result;
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int T;
    cin >> T;
    vector<Query> queries(T);
    N = 1;
    long long total_limit = 0;
    for (auto &q : queries) {
        cin >> q.a >> q.b >> q.c;
        int limit = max({q.a, q.b, q.c});
        N = max(N, limit);
        total_limit += limit;
    }

    mu.assign(N + 1, 0);
    lp.assign(N + 1, 0);
    largest_prime.assign(N + 1, 0);
    tau.assign(N + 1, 0);
    D.assign(N + 1, 0);
    mertens.assign(N + 1, 0);

    mu[1] = 1;
    tau[1] = 1;
    vector<int> primes;
    for (int i = 2; i <= N; ++i) {
        if (!lp[i]) {
            lp[i] = i;
            largest_prime[i] = i;
            primes.push_back(i);
            mu[i] = -1;
            tau[i] = 2;
        }
        for (int p : primes) {
            long long xx = 1LL * i * p;
            if (xx > N) break;
            int x = int(xx);
            lp[x] = p;
            largest_prime[x] = max(largest_prime[i], p);
            if (i % p == 0) {
                mu[x] = 0;
                int exponent = 0;
                for (int y = i; y % p == 0; y /= p) ++exponent;
                tau[x] = tau[i] / (exponent + 1) * (exponent + 2);
                break;
            }
            mu[x] = -mu[i];
            tau[x] = tau[i] * 2;
        }
    }
    for (int i = 1; i <= N; ++i) {
        D[i] = D[i - 1] + tau[i];
        if (D[i] >= MOD) D[i] -= MOD;
        mertens[i] = mertens[i - 1] + mu[i];
    }

    Evaluator evaluator(total_limit);
    for (const auto &query : queries) {
        long long answer = 0;
        for (int d = 1; d <= query.a; ++d) {
            if (!mu[d]) continue;
            long long term = 1LL * D[query.a / d] * evaluator.get(d, query.b, query.c) % MOD;
            answer += mu[d] > 0 ? term : -term;
            if (answer >= (1LL << 62) || answer <= -(1LL << 62)) answer %= MOD;
        }
        answer %= MOD;
        if (answer < 0) answer += MOD;
        cout << answer << '\n';
    }
#ifdef LOCAL
    cerr << "states=" << evaluator.used << " slots=" << evaluator.keys.size() << '\n';
#endif
}
