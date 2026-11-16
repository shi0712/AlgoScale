#include <iostream>
#include <string>
#include <vector>

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    constexpr int LIMIT = 1000000;

    int n, q;
    std::cin >> n >> q;

    std::vector<int> count(LIMIT + 1, 0);
    long long total = 0;
    int maximum = 0;
    for (int i = 0; i < n; ++i) {
        int a;
        std::cin >> a;
        ++count[a];
        total += a;
        if (a > maximum) maximum = a;
    }

    std::vector<int> at_least(LIMIT + 2, 0);
    for (int a = LIMIT; a >= 1; --a) {
        at_least[a] = at_least[a + 1] + count[a];
    }

    std::vector<long long> full_chunks(LIMIT + 1, 0);
    for (int t = 1; t <= LIMIT; ++t) {
        for (int m = t; m <= LIMIT; m += t) {
            full_chunks[t] += at_least[m];
        }
    }

    std::string output;
    output.reserve(static_cast<std::size_t>(q) * 4);
    while (q--) {
        long long k;
        std::cin >> k;
        long long t = maximum / k + (maximum % k != 0);
        output += (k <= total / t && full_chunks[t] >= k ? "YES\n" : "NO\n");
    }
    std::cout << output;
    return 0;
}
