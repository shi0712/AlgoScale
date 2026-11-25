#include <algorithm>
#include <cstdint>
#include <deque>
#include <iostream>
#include <unordered_map>
#include <utility>

using int64 = long long;

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    int64 n;
    if (!(std::cin >> n)) return 0;

    auto answer = [&](int64 a, int64 b, int64 c) {
        std::cout << "ans " << a << ' ' << b << ' ' << c << '\n' << std::flush;
    };

    if (n == 3) {
        answer(1, 2, 3);
        return 0;
    }

    std::unordered_map<int64, int64> prefix;
    prefix.reserve(256);
    prefix[0] = 0;

    auto query = [&](int64 l, int64 r) -> int64 {
        std::cout << "xor " << l << ' ' << r << '\n' << std::flush;
        int64 value;
        if (!(std::cin >> value) || value == -1) std::exit(0);
        return value;
    };

    const int64 total = query(1, n);
    prefix[n] = total;

    auto getPrefix = [&](int64 r) -> int64 {
        auto it = prefix.find(r);
        if (it != prefix.end()) return it->second;
        int64 value = query(1, r);
        prefix[r] = value;
        return value;
    };

    if (total != 0) {
        int64 low = 0, high = n;
        while (high - low > 1) {
            int64 mid = low + (high - low) / 2;
            if (getPrefix(mid) == 0)
                low = mid;
            else
                high = mid;
        }
        const int64 a = high;

        int64 b = -1, c = -1;
        low = a;
        high = n;
        for (const auto& [r, value] : prefix) {
            if (r < a) continue;
            if (value == a) {
                low = std::max(low, r);
            } else if (value == total) {
                high = std::min(high, r);
            } else {
                b = a ^ value;
                c = total ^ value;
                break;
            }
        }

        while (b == -1 && high - low > 1) {
            int64 mid = low + (high - low) / 2;
            int64 value = getPrefix(mid);
            if (value == a) {
                low = mid;
            } else if (value == total) {
                high = mid;
            } else {
                b = a ^ value;
                c = total ^ value;
            }
        }
        if (b == -1) {
            b = high;
            c = total ^ a ^ b;
        }
        answer(a, b, c);
        return 0;
    }

    int64 known = -1;
    int64 middle = n / 2;
    if (getPrefix(middle) != 0) {
        known = middle;
    } else {
        for (int64 distance = 1; distance <= 8 && known == -1; ++distance) {
            if (getPrefix(distance) != 0)
                known = distance;
            else if (getPrefix(n - distance) != 0)
                known = n - distance;
        }
    }

    std::deque<std::pair<int64, int64>> intervals;
    if (known == -1) {
        intervals.push_back({0, middle});
        intervals.push_back({middle, n});
    }
    while (known == -1) {
        auto [left, right] = intervals.front();
        intervals.pop_front();
        if (right - left < 3) continue;
        int64 mid = left + (right - left) / 2;
        if (getPrefix(mid) != 0) {
            known = mid;
        } else {
            intervals.push_back({left, mid});
            intervals.push_back({mid, right});
        }
    }

    int64 low = 0, high = known;
    for (const auto& [r, value] : prefix) {
        if (r >= known) continue;
        if (value == 0)
            low = std::max(low, r);
        else
            high = std::min(high, r);
    }
    while (high - low > 1) {
        int64 mid = low + (high - low) / 2;
        if (getPrefix(mid) == 0)
            low = mid;
        else
            high = mid;
    }
    const int64 a = high;

    int64 c = -1;
    for (const auto& [r, value] : prefix) {
        if (r >= a && value != 0 && value != a) {
            c = value;
            break;
        }
    }

    if (c == -1) {
        low = known;
        high = n;
        for (const auto& [r, value] : prefix) {
            if (r <= known) continue;
            if (value != 0)
                low = std::max(low, r);
            else
                high = std::min(high, r);
        }
        if (known <= 8) {
            int64 step = 1;
            while (c == -1 && low + step < high) {
                int64 value = getPrefix(low + step);
                if (value == 0)
                    high = low + step;
                else if (value != a)
                    c = value;
                else {
                    low += step;
                    step *= 2;
                }
            }
        }
        while (c == -1 && high - low > 1) {
            int64 mid = low + (high - low) / 2;
            int64 value = getPrefix(mid);
            if (value == 0) {
                high = mid;
            } else if (value != a) {
                c = value;
                break;
            } else {
                low = mid;
            }
        }
        if (c == -1) c = high;
    }

    answer(a, a ^ c, c);
    return 0;
}
