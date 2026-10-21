#include <bits/stdc++.h>
using namespace std;

class Solver {
    using U64 = unsigned long long;
    int n, m = 1;
    vector<int> rk, order, inv, cached, cost;
    function<int(int, int)> oracle;

    // G(l) = inv(l, m+1) XOR inv(l, m)
    //      = XOR_{j=l..m} [p[j] > p[m+1]].
    int getG(int l) {
        if (l == m + 1) return 0;
        if (cached[l] != -1) return cached[l];
        int answer = oracle(l, m + 1);
        return cached[l] = answer ^ inv[l];
    }

    // Predict G(l) for each possible insertion rank r in cand.
    vector<int> prediction(int l, const vector<int>& cand) const {
        vector<int> bit(cand.size());
        int value = m, parity = 0;
        for (int t = (int)cand.size() - 1; t >= 0; --t) {
            while (value > cand[t]) {
                parity ^= (order[value - 1] >= l);
                --value;
            }
            bit[t] = parity;
        }
        return bit;
    }

    void filter(int l, vector<int>& cand) {
        vector<int> bit = prediction(l, cand);
        int answer = getG(l);
        int out = 0;
        for (int t = 0; t < (int)cand.size(); ++t)
            if (bit[t] == answer) cand[out++] = cand[t];
        cand.resize(out);
    }

    // Find the single interval query with the most balanced partition.
    // Moving l left by one flips a prefix of the candidate bit vector.
    pair<int, int> bestQuery(const vector<int>& cand) const {
        int k = (int)cand.size();
        vector<int> less(m + 1);
        int ptr = 0;
        for (int value = 1; value <= m; ++value) {
            while (ptr < k && cand[ptr] < value) ++ptr;
            less[value] = ptr;
        }

        int words = (k + 63) / 64;
        vector<U64> bits(words, 0);
        vector<int> ones(words, 0);
        int total = 0, best = k, where = m;

        for (int l = m; l >= 1; --l) {
            int t = less[rk[l]], full = t / 64, rem = t % 64;
            for (int w = 0; w < full; ++w) {
                total += 64 - 2 * ones[w];
                ones[w] = 64 - ones[w];
                bits[w] = ~bits[w];
            }
            if (rem) {
                U64 mask = (1ULL << rem) - 1;
                int before = __builtin_popcountll(bits[full] & mask);
                int delta = rem - 2 * before;
                bits[full] ^= mask;
                ones[full] += delta;
                total += delta;
            }
            int bigger = max(total, k - total);
            if (bigger < best) {
                best = bigger;
                where = l;
                if (best == (k + 1) / 2) break;
            }
        }
        return {where, best};
    }

    // At most two queries reduce k candidates to at most ceil(k/2).
    // When k <= 3, use just one nonconstant query instead.
    void medianStep(vector<int>& cand) {
        int k = (int)cand.size();
        int pivot = cand[k / 2];
        int l = order[pivot - 1];
        vector<int> a = prediction(l, cand);
        int countA = 0, countB = 0;
        for (int t = 0; t < k; ++t) {
            countA += a[t];
            countB += a[t] ^ (pivot > cand[t]);
        }
        int first = l, second = l + 1;
        if (max(countB, k - countB) < max(countA, k - countA))
            swap(first, second);

        filter(first, cand);
        if (k > 3 && (int)cand.size() > (k + 1) / 2)
            filter(second, cand);
    }

public:
    Solver(int n_, function<int(int, int)> oracle_)
        : n(n_), rk(n + 2), inv(n + 2), cached(n + 2),
          cost(n + 2), oracle(std::move(oracle_)) {
        for (int k = 2; k <= n; ++k)
            cost[k] = (k <= 3 ? k - 1 : 2 + cost[(k + 1) / 2]);
        rk[1] = 1;
        order.push_back(1);
    }

    vector<int> run() {
        for (int i = 2; i <= n; ++i) {
            m = i - 1;
            fill(cached.begin(), cached.end(), -1);

            // The insertion rank r is the number of old elements < p[i].
            int parity = (m & 1) ^ getG(1);
            vector<int> cand;
            for (int r = parity; r <= m; r += 2) cand.push_back(r);

            while (cand.size() > 1) {
                int k = (int)cand.size();
                if (k <= 3) {
                    medianStep(cand);
                    continue;
                }
                auto [l, bigger] = bestQuery(cand);
                if (1 + cost[bigger] <= cost[k]) {
                    filter(l, cand);
                } else {
                    medianStep(cand);
                }
            }

            int r = cand[0];
            // Update all inversion parities inv(l, i), with no queries.
            int paritySuffix = 0;
            for (int j = m; j >= 1; --j) {
                paritySuffix ^= (rk[j] > r);
                inv[j] ^= paritySuffix;
            }
            inv[i] = 0;

            for (int j = 1; j <= m; ++j)
                if (rk[j] > r) ++rk[j];
            rk[i] = r + 1;
            order.insert(order.begin() + r, i);
        }
        return vector<int>(rk.begin() + 1, rk.begin() + n + 1);
    }
};

#ifndef LOCAL_TEST
int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n) || n < 1 || n > 2000) return 0;
    auto ask = [](int l, int r) -> int {
        cout << "? " << l << ' ' << r << endl;
        int answer;
        if (!(cin >> answer) || (answer != 0 && answer != 1)) exit(0);
        return answer;
    };

    Solver solver(n, ask);
    vector<int> answer = solver.run();
    cout << '!';
    for (int x : answer) cout << ' ' << x;
    cout << endl;
    return 0;
}
#endif
