#include <bits/stdc++.h>
using namespace std;

#if defined(__GNUC__) && defined(__x86_64__)
#pragma GCC target("popcnt")
#endif

using int64 = long long;
using u64 = unsigned long long;
using u128 = __uint128_t;

static constexpr int MOD = 1000000007;

class Solver {
    int n;

    vector<int> spf;
    vector<int8_t> mu;
    vector<unsigned char> omegaWeight;
    vector<int> divisorPrefix;

    // mask[x] contains precisely the square-free positive integers
    // coprime to x, up to floor(n / x).
    vector<int> offset;
    vector<u64> masks;
    vector<u64> negativeMu;

    int A, B, C;
    bool equalDimensions;

    void preprocess() {
        spf.assign(n + 1, 0);
        mu.assign(n + 1, 0);
        omegaWeight.assign(n + 1, 0);
        divisorPrefix.assign(n + 1, 0);

        vector<int> primes;
        for (int i = 2; i <= n; ++i) {
            if (spf[i] == 0) {
                spf[i] = i;
                primes.push_back(i);
            }
            for (int p : primes) {
                if (p > spf[i] || 1LL * i * p > n) break;
                spf[i * p] = p;
            }
        }

        vector<unsigned char> exponent(n + 1, 0);
        mu[1] = 1;
        omegaWeight[1] = 1;
        divisorPrefix[1] = 1;

        for (int x = 2; x <= n; ++x) {
            int p = spf[x];
            int y = x / p;

            if (y % p == 0) {
                mu[x] = 0;
                exponent[x] = exponent[y] + 1;
                divisorPrefix[x] =
                    divisorPrefix[y] / (exponent[y] + 1)
                    * (exponent[x] + 1);
            } else {
                mu[x] = -mu[y];
                omegaWeight[x] = omegaWeight[y] * 2;
                exponent[x] = 1;
                divisorPrefix[x] = divisorPrefix[y] * 2;
            }
        }

        for (int i = 1; i <= n; ++i) {
            divisorPrefix[i] += divisorPrefix[i - 1];
        }

        negativeMu.assign(n / 64 + 1, 0);
        for (int i = 1; i <= n; ++i) {
            if (mu[i] == -1) {
                negativeMu[i >> 6] |= 1ULL << (i & 63);
            }
        }

        // Multiples-of-p exclusion masks are needed only for p <= sqrt(n).
        int root = (int)sqrt(n);
        while (1LL * (root + 1) * (root + 1) <= n) ++root;
        while (1LL * root * root > n) --root;

        vector<vector<u64>> notDivisible(root + 1);
        for (int p : primes) {
            if (p > root) break;

            int limit = n / p;
            auto &v = notDivisible[p];
            v.assign(limit / 64 + 1, ~0ULL);

            for (int j = 0; j <= limit; j += p) {
                v[j >> 6] &= ~(1ULL << (j & 63));
            }
        }

        offset.assign(n + 1, -1);
        size_t totalWords = 0;
        for (int x = 1; x <= n; ++x) {
            if (mu[x] != 0) {
                offset[x] = (int)totalWords;
                totalWords += (n / x) / 64 + 1;
            }
        }
        masks.assign(totalWords, 0);

        u64 *first = masks.data() + offset[1];
        for (int i = 1; i <= n; ++i) {
            if (mu[i] != 0) {
                first[i >> 6] |= 1ULL << (i & 63);
            }
        }

        for (int x = 2; x <= n; ++x) {
            if (mu[x] == 0) continue;

            int p = spf[x];
            int parent = x / p;
            int limit = n / x;
            int words = limit / 64 + 1;

            const u64 *src = masks.data() + offset[parent];
            u64 *dst = masks.data() + offset[x];

            if (p > limit) {
                copy_n(src, words, dst);
            } else {
                const u64 *exclude = notDivisible[p].data();
                for (int j = 0; j < words; ++j) {
                    dst[j] = src[j] & exclude[j];
                }
            }
        }
    }

    // Sum of mu(f) over L <= f <= R, with gcd(f, g*d*e) = 1.
    inline int signedCount(
        const u64 *mg,
        const u64 *md,
        const u64 *me,
        int L,
        int R
    ) const {
        int leftWord = L >> 6;
        int rightWord = R >> 6;

        u64 leftMask = ~0ULL << (L & 63);
        u64 rightMask = ~0ULL >> (63 - (R & 63));

        int result = 0;

        if (leftWord == rightWord) {
            u64 bits = mg[leftWord] & md[leftWord] & me[leftWord];
            bits &= leftMask & rightMask;
            return __builtin_popcountll(bits)
                 - 2 * __builtin_popcountll(bits & negativeMu[leftWord]);
        }

        {
            u64 bits = mg[leftWord] & md[leftWord] & me[leftWord] & leftMask;
            result += __builtin_popcountll(bits)
                    - 2 * __builtin_popcountll(bits & negativeMu[leftWord]);
        }

        for (int w = leftWord + 1; w < rightWord; ++w) {
            u64 bits = mg[w] & md[w] & me[w];
            result += __builtin_popcountll(bits)
                    - 2 * __builtin_popcountll(bits & negativeMu[w]);
        }

        {
            u64 bits = mg[rightWord] & md[rightWord] & me[rightWord] & rightMask;
            result += __builtin_popcountll(bits)
                    - 2 * __builtin_popcountll(bits & negativeMu[rightWord]);
        }

        return result;
    }

    // Sum over the last edge f. The other two edges are d <= e.
    // repeated means d = e = 1, so only three distinct permutations exist.
    int64 sumLastEdge(int g, int d, int e, int L, int R, bool repeated) const {
        int gd = g * d;
        int ge = g * e;
        int p = gd * e;

        u64 px = divisorPrefix[A / p];
        u64 py = divisorPrefix[B / p];
        u64 pz = divisorPrefix[C / p];

        // All other divisor-prefix values are at most pz.
        bool safe64 = pz <= 1000000;

        int raw[6] = {
            A / gd, B / gd, C / gd,
            A / ge, B / ge, C / ge
        };

        int bases[6], mapping[6], baseCount = 0;
        for (int i = 0; i < 6; ++i) {
            int j = 0;
            while (j < baseCount && bases[j] != raw[i]) ++j;
            if (j == baseCount) bases[baseCount++] = raw[i];
            mapping[i] = j;
        }

        const u64 *mg = masks.data() + offset[g];
        const u64 *md = masks.data() + offset[d];
        const u64 *me = masks.data() + offset[e];

        int64 result = 0;

        while (L <= R) {
            int finish = R;
            int quotient[6];

            for (int j = 0; j < baseCount; ++j) {
                int q = bases[j] / L;
                quotient[j] = q;
                if (q != 0) {
                    finish = min(finish, bases[j] / q);
                }
            }

            int count = signedCount(mg, md, me, L, finish);

            if (count != 0) {
                u64 qx = divisorPrefix[quotient[mapping[0]]];
                u64 qy = divisorPrefix[quotient[mapping[1]]];
                u64 qz = divisorPrefix[quotient[mapping[2]]];

                u64 rx = divisorPrefix[quotient[mapping[3]]];
                u64 ry = divisorPrefix[quotient[mapping[4]]];
                u64 rz = divisorPrefix[quotient[mapping[5]]];

                u64 value;

                if (equalDimensions) {
                    unsigned multiplicity = repeated ? 3 : 6;
                    if (safe64) {
                        value = (px * qx * rx * multiplicity) % MOD;
                    } else {
                        value = (u128(px) * qx * rx * multiplicity) % MOD;
                    }
                } else if (repeated) {
                    if (safe64) {
                        value = (
                            px * qy * qz +
                            py * qx * qz +
                            pz * qx * qy
                        ) % MOD;
                    } else {
                        value = (
                            u128(px) * qy * qz +
                            u128(py) * qx * qz +
                            u128(pz) * qx * qy
                        ) % MOD;
                    }
                } else {
                    if (safe64) {
                        value = (
                            px * (qy * rz + qz * ry) +
                            py * (qx * rz + qz * rx) +
                            pz * (qx * ry + qy * rx)
                        ) % MOD;
                    } else {
                        value = (
                            u128(px) * (qy * rz + qz * ry) +
                            u128(py) * (qx * rz + qz * rx) +
                            u128(pz) * (qx * ry + qy * rx)
                        ) % MOD;
                    }
                }

                result += (int64)value * count;
            }

            L = finish + 1;
        }

        return result % MOD;
    }

public:
    explicit Solver(int maximum) : n(maximum) {
        preprocess();
    }

    int solve(int a, int b, int c) {
        array<int, 3> dimensions{a, b, c};
        sort(dimensions.begin(), dimensions.end());

        A = dimensions[0];
        B = dimensions[1];
        C = dimensions[2];
        equalDimensions = (A == C);

        int64 answer = 0;

        auto add = [&](int64 value, int coefficient) {
            answer += value * coefficient;
            constexpr int64 LIMIT = 1LL << 60;
            if (answer >= LIMIT || answer <= -LIMIT) {
                answer %= MOD;
            }
        };

        for (int g = 1; g <= A; ++g) {
            if (mu[g] == 0) continue;

            int weight = omegaWeight[g];

            // d = e = f = 1.
            int64 allOne = 1LL * divisorPrefix[A / g]
                                   * divisorPrefix[B / g] % MOD;
            allOne = allOne * divisorPrefix[C / g] % MOD;
            add(allOne, weight);

            // d = e = 1, f > 1.
            int last = B / g;
            if (last >= 2) {
                add(sumLastEdge(g, 1, 1, 2, last, true), weight);
            }

            // The remaining cases have d < e < f.
            int cOverG = C / g;
            for (int e = 2;
                 1LL * e * (e + 1) <= cOverG && 1LL * g * e <= A;
                 ++e) {
                if (mu[e] == 0 || gcd(g, e) != 1) continue;

                int ge = g * e;
                int maxD = min(e - 1, A / ge);
                int maxFromC = C / ge;

                for (int d = 1; d <= maxD; ++d) {
                    if (mu[d] == 0 || gcd(d, ge) != 1) continue;

                    int finish = min(maxFromC, B / (g * d));
                    if (finish <= e) continue;

                    int coefficient = weight * int(mu[d]) * int(mu[e]);
                    add(
                        sumLastEdge(g, d, e, e + 1, finish, false),
                        coefficient
                    );
                }
            }
        }

        answer %= MOD;
        if (answer < 0) answer += MOD;
        return (int)answer;
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int T;
    cin >> T;

    vector<array<int, 3>> tests(T);
    int maximum = 1;

    for (auto &[a, b, c] : tests) {
        cin >> a >> b >> c;
        maximum = max({maximum, a, b, c});
    }

    Solver solver(maximum);

    for (auto [a, b, c] : tests) {
        cout << solver.solve(a, b, c) << '\n';
    }

    return 0;
}
