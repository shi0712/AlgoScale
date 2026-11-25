#include <bits/stdc++.h>
using namespace std;

using u32 = uint32_t;
using u64 = uint64_t;

static int ask(u32 x, u32 &subtracted) {
    cout << "- " << x << '\n' << flush;
    int response;
    if (!(cin >> response)) exit(0);
    if (response == -1) exit(0);
    subtracted += x;
    return response;
}

static u32 choose_query(const vector<u32> &candidates, u32 subtracted) {
    const u32 limit = candidates.front() - subtracted;
    if (candidates.size() == 2) return limit;

    vector<u32> tests = {1, limit, max(1u, limit / 2), max(1u, limit / 3)};
    for (int bit = 0; bit < 30; bit++) {
        u32 power = 1u << bit;
        if (power <= limit) tests.push_back(power);
        u32 low = (bit == 29 ? (1u << 30) - 1 : (1u << (bit + 1)) - 1);
        if (low <= limit) tests.push_back(low);
    }

    mt19937 rng(0x9e3779b9u ^ (u32)candidates.size() ^ subtracted);
    for (int i = 0; i < 72; i++) {
        u32 x = uniform_int_distribution<u32>(1, limit)(rng);
        tests.push_back(x);
        if (i < 24) tests.push_back(max(1u, x & limit));
    }

    sort(tests.begin(), tests.end());
    tests.erase(unique(tests.begin(), tests.end()), tests.end());

    const size_t sample_limit = 30000;
    vector<u32> sample;
    if (candidates.size() <= sample_limit) {
        sample = candidates;
    } else {
        sample.reserve(sample_limit);
        for (size_t i = 0; i < sample_limit; i++) {
            size_t index = (u64)i * candidates.size() / sample_limit;
            sample.push_back(candidates[index]);
        }
    }

    u32 best = limit;
    double best_score = -1.0;
    size_t best_largest = numeric_limits<size_t>::max();
    for (u32 x : tests) {
        array<size_t, 31> counts{};
        for (u32 original : sample) {
            int result = __builtin_popcount(original - subtracted - x);
            counts[result]++;
        }
        double score = 0.0;
        size_t largest = 0;
        for (size_t count : counts) {
            if (count) score += count * log((double)count);
            largest = max(largest, count);
        }
        if (largest < best_largest ||
            (largest == best_largest && score > best_score)) {
            best_score = score;
            best_largest = largest;
            best = x;
        }
    }
    return best;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int initial_count;
    if (!(cin >> initial_count)) return 0;
    if (initial_count == -1) return 0;

    u32 subtracted = 0;
    u32 first = 0;
    for (int bit = 0; bit < initial_count; bit += 2) first |= 1u << bit;
    int response = ask(first, subtracted);

    const u32 maximum = 1000000000;
    vector<u32> candidates;
    u64 combination = (1ULL << initial_count) - 1;
    while (combination <= maximum) {
        u32 original = (u32)combination;
        if (__builtin_popcount(original - first) == response) {
            candidates.push_back(original);
        }
        u64 low = combination & -combination;
        u64 ripple = combination + low;
        if (ripple < combination) break;
        combination = ripple | (((combination ^ ripple) >> 2) / low);
    }

    while (candidates.size() > 1) {
        u32 x = choose_query(candidates, subtracted);
        response = ask(x, subtracted);
        size_t kept = 0;
        for (u32 original : candidates) {
            if (__builtin_popcount(original - subtracted) == response) {
                candidates[kept++] = original;
            }
        }
        candidates.resize(kept);
    }

    if (candidates.empty()) return 0;
    cout << "! " << candidates.front() << '\n' << flush;
    return 0;
}
