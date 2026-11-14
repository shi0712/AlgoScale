#include <algorithm>
#include <iostream>
#include <vector>

using int64 = long long;

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    int n, q;
    std::cin >> n >> q;

    std::vector<int> frequency(1'000'001, 0);
    int64 total = 0;
    int maximum = 0;
    for (int i = 0; i < n; ++i) {
        int a;
        std::cin >> a;
        ++frequency[a];
        total += a;
        maximum = std::max(maximum, a);
    }

    std::vector<int64> at_least(maximum + 2, 0);
    for (int value = maximum; value >= 1; --value) {
        at_least[value] = at_least[value + 1] + frequency[value];
    }

    std::vector<int64> full_cakes(maximum + 1, 0);
    for (int t = 1; t <= maximum; ++t) {
        for (int multiple = t; multiple <= maximum; multiple += t) {
            full_cakes[t] += at_least[multiple];
        }
    }

    while (q--) {
        int64 k;
        std::cin >> k;

        if (k > total) {
            std::cout << "NO\n";
            continue;
        }

        int64 lower = std::max((maximum + k - 1) / k,
                               (total + k * n - 1) / (k * n));
        int64 upper = total / k;

        bool possible = lower <= upper && lower <= maximum &&
                        full_cakes[lower] >= k;
        std::cout << (possible ? "YES\n" : "NO\n");
    }

    return 0;
}
