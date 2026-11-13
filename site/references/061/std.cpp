#include <algorithm>
#include <cstdint>
#include <iostream>
#include <vector>

using i64 = std::int64_t;
constexpr i64 MOD = 998244353;

static i64 power(i64 base, i64 exponent) {
    i64 result = 1;
    for (; exponent; exponent >>= 1, base = base * base % MOD) {
        if (exponent & 1) result = result * base % MOD;
    }
    return result;
}

struct TestCase {
    int pairs;
    std::vector<int> sizes;
};

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    int test_count;
    if (!(std::cin >> test_count)) return 0;
    std::vector<TestCase> cases(test_count);
    int maximum_size = 0;
    for (auto& test : cases) {
        int set_count;
        std::cin >> set_count >> test.pairs;
        test.sizes.resize(set_count);
        for (int& size : test.sizes) std::cin >> size;
        maximum_size = std::max(maximum_size, 2 * test.pairs);
    }

    std::vector<i64> factorial(maximum_size + 1, 1);
    std::vector<i64> inverse_factorial(maximum_size + 1, 1);
    std::vector<i64> powers_of_two(maximum_size + 1, 1);
    for (int i = 1; i <= maximum_size; ++i) {
        factorial[i] = factorial[i - 1] * i % MOD;
        powers_of_two[i] = powers_of_two[i - 1] * 2 % MOD;
    }
    inverse_factorial[maximum_size] = power(factorial[maximum_size], MOD - 2);
    for (int i = maximum_size; i > 0; --i) {
        inverse_factorial[i - 1] = inverse_factorial[i] * i % MOD;
    }

    for (const auto& test : cases) {
        const int m = test.pairs;
        const int total = 2 * m;
        if (test.sizes.size() == 1) {
            std::cout << 0 << '\n';
            continue;
        }

        // arrival[r]: expected entries into r ancestors from larger states.
        // occupation[r]: expected total time spent at r ancestors, including
        // repeated rounds with no merging. Initially all total ancestors exist.
        std::vector<i64> arrival(total + 1, 0), occupation(total + 1, 0);
        arrival[total] = 1;

        for (int r = total; r >= 2; --r) {
            i64 stay_probability = 0;
            // More than m selected endpoints force at least one selected pair.
            // Check this BEFORE indexing inverse_factorial[m - r].
            if (r <= m) {
                stay_probability = factorial[total - r] * factorial[m] % MOD;
                stay_probability = stay_probability * powers_of_two[r] % MOD;
                stay_probability = stay_probability * inverse_factorial[m - r] % MOD;
                stay_probability = stay_probability * inverse_factorial[total] % MOD;
            }
            const i64 leave_probability = (1 - stay_probability + MOD) % MOD;
            occupation[r] = arrival[r] * power(leave_probability, MOD - 2) % MOD;

            i64 common = factorial[r] * factorial[total - r] % MOD;
            common = common * factorial[m] % MOD * inverse_factorial[total] % MOD;
            for (int merged = std::max(1, r - m); merged <= r / 2; ++merged) {
                i64 probability = common * powers_of_two[r - 2 * merged] % MOD;
                probability = probability * inverse_factorial[r - 2 * merged] % MOD;
                probability = probability * inverse_factorial[merged] % MOD;
                probability = probability * inverse_factorial[m - r + merged] % MOD;
                arrival[r - merged] =
                    (arrival[r - merged] + occupation[r] * probability) % MOD;
            }
        }

        i64 answer = 0;
        for (int r = 2; r <= total; ++r) {
            i64 same_initial_set = 0;
            for (int size : test.sizes) {
                if (size < r) continue;
                // C(size, r) / C(total, r), with r! cancelled.
                i64 probability = factorial[size] * factorial[total - r] % MOD;
                probability = probability * inverse_factorial[size - r] % MOD;
                probability = probability * inverse_factorial[total] % MOD;
                same_initial_set = (same_initial_set + probability) % MOD;
            }
            answer = (answer + occupation[r] * (1 - same_initial_set + MOD)) % MOD;
        }
        std::cout << answer << '\n';
    }
    return 0;
}
