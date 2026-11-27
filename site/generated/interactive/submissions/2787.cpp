#include <bits/stdc++.h>
using namespace std;

using int64 = long long;
using u64 = uint64_t;
using i128 = __int128_t;
using u128 = __uint128_t;

static mt19937_64 rng(
    chrono::steady_clock::now().time_since_epoch().count()
);

u64 mulMod(u64 a, u64 b, u64 mod) {
    return (u128)a * b % mod;
}

u64 powMod(u64 a, u64 e, u64 mod) {
    u64 result = 1;
    while (e) {
        if (e & 1) result = mulMod(result, a, mod);
        a = mulMod(a, a, mod);
        e >>= 1;
    }
    return result;
}

bool isPrime(u64 n) {
    if (n < 2) return false;
    for (u64 p : {2ULL, 3ULL, 5ULL, 7ULL, 11ULL, 13ULL,
                  17ULL, 19ULL, 23ULL, 29ULL, 31ULL, 37ULL}) {
        if (n % p == 0) return n == p;
    }

    u64 d = n - 1;
    int s = 0;
    while ((d & 1) == 0) {
        d >>= 1;
        ++s;
    }

    for (u64 a : {2ULL, 325ULL, 9375ULL, 28178ULL,
                  450775ULL, 9780504ULL, 1795265022ULL}) {
        if (a % n == 0) continue;
        u64 x = powMod(a % n, d, n);
        if (x == 1 || x == n - 1) continue;
        bool composite = true;
        for (int r = 1; r < s; ++r) {
            x = mulMod(x, x, n);
            if (x == n - 1) {
                composite = false;
                break;
            }
        }
        if (composite) return false;
    }
    return true;
}

u64 pollardRho(u64 n) {
    if (n % 2 == 0) return 2;
    if (n % 3 == 0) return 3;

    while (true) {
        u64 c = rng() % (n - 1) + 1;
        u64 x = rng() % (n - 2) + 2;
        u64 y = x;
        u64 d = 1;

        auto f = [&](u64 v) -> u64 {
            return (mulMod(v, v, n) + c) % n;
        };

        while (d == 1) {
            x = f(x);
            y = f(f(y));
            u64 diff = x > y ? x - y : y - x;
            d = std::gcd(diff, n);
        }
        if (d != n) return d;
    }
}

void factorize(u64 n, map<u64, int>& factors) {
    if (n == 1) return;
    if (isPrime(n)) {
        ++factors[n];
        return;
    }
    u64 d = pollardRho(n);
    factorize(d, factors);
    factorize(n / d, factors);
}

u64 integerSqrt(u64 x) {
    u64 r = static_cast<u64>(sqrtl(static_cast<long double>(x)));
    while ((u128)(r + 1) * (r + 1) <= x) ++r;
    while ((u128)r * r > x) --r;
    return r;
}

struct Gaussian {
    i128 a = 0;
    i128 b = 0;
};

Gaussian multiplyGaussian(const Gaussian& x, const Gaussian& y) {
    return {
        x.a * y.a - x.b * y.b,
        x.a * y.b + x.b * y.a
    };
}

Gaussian powerGaussian(Gaussian x, int e) {
    Gaussian result{1, 0};
    while (e) {
        if (e & 1) result = multiplyGaussian(result, x);
        x = multiplyGaussian(x, x);
        e >>= 1;
    }
    return result;
}

Gaussian conjugateGaussian(const Gaussian& x) {
    return {x.a, -x.b};
}

Gaussian gaussianPrime(u64 p) {
    u64 nonresidue = 0;
    for (u64 a = 2; a < 1000; ++a) {
        if (powMod(a, (p - 1) / 2, p) == p - 1) {
            nonresidue = a;
            break;
        }
    }
    if (nonresidue == 0) {
        while (true) {
            u64 a = rng() % (p - 2) + 2;
            if (powMod(a, (p - 1) / 2, p) == p - 1) {
                nonresidue = a;
                break;
            }
        }
    }

    u64 root = powMod(nonresidue, (p - 1) / 4, p);
    u64 a = p, b = root;

    while ((u128)b * b > p) {
        u64 remainder = a % b;
        a = b;
        b = remainder;
    }

    u64 d2 = p - b * b;
    u64 d = integerSqrt(d2);
    if (d * d == d2) return {(i128)b, (i128)d};

    throw runtime_error("Cornacchia failure");
}

int64 choose2(int64 x) {
    return static_cast<int64>((i128)x * (x - 1) / 2);
}

int64 inverseChoose2(int64 value) {
    if (value == 0) return 1;

    long double estimate =
        (1.0L + sqrtl(1.0L + 8.0L * static_cast<long double>(value))) / 2.0L;
    int64 x = max<int64>(1, static_cast<int64>(estimate + 0.5L));

    while (choose2(x) > value) --x;
    while (choose2(x + 1) <= value) ++x;
    return x;
}

bool isChoose2(int64 value) {
    return choose2(inverseChoose2(value)) == value;
}

vector<pair<int64, int64>> possibleLengths(int64 total) {
    vector<pair<int64, int64>> answer;
    u64 value = 8ULL * static_cast<u64>(total) + 2;

    map<u64, int> factors;
    factorize(value, factors);

    u64 scalar = 1;
    int exponentOfTwo = 0;
    vector<vector<Gaussian>> options;

    for (auto [p, e] : factors) {
        if (p == 2) {
            exponentOfTwo = e;
        } else if (p % 4 == 3) {
            for (int j = 0; j < e / 2; ++j) scalar *= p;
        } else {
            Gaussian z = gaussianPrime(p);
            vector<Gaussian> current;
            for (int t = 0; t <= e; ++t) {
                Gaussian left = powerGaussian(z, t);
                Gaussian right =
                    powerGaussian(conjugateGaussian(z), e - t);
                current.push_back(multiplyGaussian(left, right));
            }
            options.push_back(move(current));
        }
    }

    Gaussian base = powerGaussian(Gaussian{1, 1}, exponentOfTwo);
    base.a *= static_cast<i128>(scalar);
    base.b *= static_cast<i128>(scalar);

    function<void(int, const Gaussian&)> enumerate =
        [&](int position, const Gaussian& current) {
            if (position == static_cast<int>(options.size())) {
                i128 aa = current.a < 0 ? -current.a : current.a;
                i128 bb = current.b < 0 ? -current.b : current.b;
                u64 u = static_cast<u64>(aa);
                u64 v = static_cast<u64>(bb);

                int64 x = static_cast<int64>((u + 1) / 2);
                int64 y = static_cast<int64>((v + 1) / 2);

                if (x >= 2 && y >= 2) {
                    answer.emplace_back(x, y);
                    if (x != y) answer.emplace_back(y, x);
                }
                return;
            }

            for (const Gaussian& g : options[position]) {
                enumerate(position + 1, multiplyGaussian(current, g));
            }
        };

    enumerate(0, base);
    sort(answer.begin(), answer.end());
    answer.erase(unique(answer.begin(), answer.end()), answer.end());
    return answer;
}

int64 ask(int64 l, int64 r) {
    cout << "? " << l << ' ' << r << '\n' << flush;

    int64 response;
    if (!(cin >> response) || response < 0) exit(0);
    return response;
}

struct LengthState {
    int64 x;
    int64 y;
    vector<int64> possibleI;
};

bool consistentWithSample(
    int64 i, int64 x, int64 y, int64 r, int64 inversions
) {
    int64 firstLength = r - i + 1;
    if (firstLength >= 1 && firstLength <= x &&
        inversions == choose2(firstLength)) {
        return true;
    }

    int64 secondLength = r - i - x + 1;
    if (secondLength >= 1 && secondLength <= y &&
        inversions == choose2(x) + choose2(secondLength)) {
        return true;
    }
    return false;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int64 n;
    if (!(cin >> n)) return 0;

    const int64 total = ask(1, n);
    vector<pair<int64, int64>> globalLengths = possibleLengths(total);

    vector<LengthState> states;
    for (auto [x, y] : globalLengths) {
        if (x + y <= n) states.push_back({x, y, {-1}});
    }

    vector<pair<int64, int64>> samples;
    int64 low = 1, high = n;
    int64 i = -1;
    bool foundI = false;

    while (low < high) {
        int64 r = low + (high - low) / 2;
        int64 inversions = ask(1, r);
        samples.emplace_back(r, inversions);

        if (inversions == 0) {
            low = r + 1;
            continue;
        }

        high = r;
        if (inversions == total) continue;

        vector<LengthState> nextStates;
        set<int64> allPossibleI;

        for (LengthState state : states) {
            vector<int64> current;

            if (isChoose2(inversions)) {
                int64 firstLength = inverseChoose2(inversions);
                if (firstLength >= 2 && firstLength <= state.x) {
                    int64 candidateI = r - firstLength + 1;
                    if (candidateI >= 1) current.push_back(candidateI);
                }
            }

            int64 remainder = inversions - choose2(state.x);
            if (remainder >= 0 && isChoose2(remainder)) {
                int64 secondLength =
                    (remainder == 0 ? 1 : inverseChoose2(remainder));
                if (secondLength >= 1 && secondLength <= state.y) {
                    int64 candidateJ = r - secondLength + 1;
                    int64 candidateI = candidateJ - state.x;
                    if (candidateI >= 1) current.push_back(candidateI);
                }
            }

            sort(current.begin(), current.end());
            current.erase(unique(current.begin(), current.end()), current.end());

            vector<int64> intersection;
            if (state.possibleI.size() == 1 && state.possibleI[0] == -1) {
                intersection = current;
            } else {
                for (int64 a : state.possibleI) {
                    if (binary_search(current.begin(), current.end(), a)) {
                        intersection.push_back(a);
                    }
                }
            }

            if (!intersection.empty()) {
                state.possibleI = move(intersection);
                for (int64 value : state.possibleI) {
                    if (value != -1) allPossibleI.insert(value);
                }
                nextStates.push_back(move(state));
            }
        }

        states = move(nextStates);
        if (allPossibleI.size() == 1) {
            i = *allPossibleI.begin();
            foundI = true;
            break;
        }
    }

    if (!foundI) i = low - 1;

    const int64 span = n - i + 1;
    vector<pair<int64, int64>> candidates;

    for (auto [x, y] : globalLengths) {
        if (x + y > span) continue;

        bool valid = true;
        for (auto [r, inversions] : samples) {
            if (inversions == 0 || inversions == total) continue;
            if (!consistentWithSample(i, x, y, r, inversions)) {
                valid = false;
                break;
            }
        }
        if (valid) candidates.emplace_back(x, y);
    }

    int64 actualX = -1, actualY = -1;

    if (!candidates.empty()) {
        vector<int64> values;
        map<int64, int64> partner;
        for (auto [x, y] : candidates) {
            values.push_back(x);
            partner[x] = y;
        }
        sort(values.begin(), values.end());
        values.erase(unique(values.begin(), values.end()), values.end());

        int64 left = 0, right = static_cast<int64>(values.size());
        while (left < right) {
            int64 mid = left + (right - left) / 2;
            int64 x = values[mid];
            int64 inversions = ask(i, i + x - 1);

            if (inversions == choose2(x)) {
                left = mid + 1;
            } else {
                right = mid;
            }
        }

        if (left > 0) {
            actualX = values[left - 1];
            actualY = inverseChoose2(total - choose2(actualX));
        }
    }

    auto validate = [&](int64 x, int64 y) -> bool {
        int64 j = i + x;
        int64 k = j + y - 1;

        if (i < 1 || j - i < 2 || y < 2 || k > n) return false;

        for (auto [r, inversions] : samples) {
            int64 expected;
            if (r <= i) {
                expected = 0;
            } else if (r < j) {
                expected = choose2(r - i + 1);
            } else if (r <= k) {
                expected = choose2(x) + choose2(r - j + 1);
            } else {
                expected = total;
            }
            if (expected != inversions) return false;
        }
        return true;
    };

    if (actualX < 0 || actualY < 0 || !validate(actualX, actualY)) {
        int64 left = 2, right = span - 2;
        while (left < right) {
            int64 mid = left + (right - left + 1) / 2;
            int64 inversions = ask(i, i + mid - 1);
            if (inversions == choose2(mid)) {
                left = mid;
            } else {
                right = mid - 1;
            }
        }
        actualX = left;
        actualY = inverseChoose2(total - choose2(actualX));
    }

    int64 j = i + actualX;
    int64 k = j + actualY - 1;

    cout << "! " << i << ' ' << j << ' ' << k << '\n' << flush;
    return 0;
}
