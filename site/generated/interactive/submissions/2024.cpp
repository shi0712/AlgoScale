#include <algorithm>
#include <array>
#include <bit>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <unordered_map>
#include <utility>
#include <vector>

using Number = std::uint64_t;
using Triple = std::array<Number, 3>;

struct Range {
    Number lo, hi;

    bool contains(Number value) const {
        return lo <= value && value <= hi;
    }
};

Number ask(Range range) {
    std::cout << "xor " << range.lo << ' ' << range.hi << std::endl;
    std::int64_t reply;
    if (!(std::cin >> reply) || reply == -1) {
        std::exit(0);
    }
    return static_cast<Number>(reply);
}

int highest_bit(Number value) {
    return std::bit_width(value) - 1;
}

Number power(int bit) {
    return Number{1} << bit;
}

Range block_at(Number value, int bits) {
    Number mask = power(bits) - 1;
    return {value & ~mask, value | mask};
}

// Find the two numbers with XOR difference, in one of the allowed intervals.
// The intervals are aligned to whole blocks, apart from the bounds 1 and n.
Triple find_pair(Number n, Number single, Number difference, std::vector<Range> intervals) {
    Number half = power(highest_bit(difference));
    Number size = 2 * half;
    std::sort(intervals.begin(), intervals.end(), [](Range a, Range b) {
        return a.lo < b.lo;
    });

    std::vector<Range> blocks;
    for (Range interval : intervals) {
        if (interval.hi < half) {
            continue;
        }
        Number first = interval.lo / size;
        Number last = (interval.hi - half) / size;
        if (first == 0 && difference == half && std::min(interval.hi, size - 1) == half) {
            ++first; // The only pair in this block would use zero.
        }
        if (first <= last) {
            blocks.push_back({first, last});
        }
    }

    auto midpoint = [&](Number block) {
        return block * size + half;
    };

    while (blocks.size() > 1 || blocks.front().lo != blocks.front().hi) {
        Number count = 0;
        Number nearest = 0, nearest_rank = 0, distance = n;
        for (Range run : blocks) {
            Number block = std::clamp(single / size, run.lo, run.hi);
            Number mid = midpoint(block);
            Number gap = mid > single ? mid - single : single - mid;
            if (gap < distance) {
                nearest = block;
                nearest_rank = count + block - run.lo;
                distance = gap;
            }
            count += run.hi - run.lo + 1;
        }
        Number rank = (nearest_rank + count / 2) % count;
        Number opposite = 0;
        for (Range run : blocks) {
            Number length = run.hi - run.lo + 1;
            if (rank < length) {
                opposite = run.lo + rank;
                break;
            }
            rank -= length;
        }

        // Each endpoint cuts a different block in half. A pair in either
        // endpoint block gives one member directly. The closest block to
        // the singleton is one pivot, and the other balances the outcomes.
        Number lo = std::min(nearest, opposite), hi = std::max(nearest, opposite);
        Range query{midpoint(lo), midpoint(hi) - 1};
        Number value = ask(query);
        if (query.contains(single)) {
            value ^= single;
        }
        if (value != 0 && value != difference) {
            return {single, value, value ^ difference};
        }

        std::vector<Range> next;
        for (Range run : blocks) {
            if (value == difference) {
                Range inside{std::max(run.lo, lo + 1), std::min(run.hi, hi - 1)};
                if (inside.lo <= inside.hi) {
                    next.push_back(inside);
                }
            } else {
                if (run.lo < lo) {
                    next.push_back({run.lo, std::min(run.hi, lo - 1)});
                }
                if (run.hi > hi) {
                    next.push_back({std::max(run.lo, hi + 1), run.hi});
                }
            }
        }
        blocks = std::move(next);
    }

    Number mid = midpoint(blocks.front().lo);
    Range upper{mid, std::min(n, mid + half - 1)};
    Number count = upper.hi - upper.lo + 1 - upper.contains(difference);
    Number value = count == 1 ? upper.lo + (upper.lo == difference) : ask(upper);
    return {single, value, value ^ difference};
}

struct Branch {
    int bit;
    Range single;
    Range pair;
};

Triple solve_nonzero(Number n, Number total) {
    std::vector<Branch> branches;
    for (int bit = 1; bit <= highest_bit(n); ++bit) {
        Range child = block_at(total, bit);
        Number other = child.lo ^ power(bit);
        Range pair{std::max(Number{1}, other), std::min(n, other + power(bit) - 1)};
        if (pair.lo >= pair.hi) {
            continue;
        }

        // The possible differences within the paired child are all nonzero
        // numbers below the next power of two at or above its length.
        int bits = std::bit_width(std::bit_ceil(pair.hi - pair.lo + 1)) - 1;
        child = block_at(total, bits);
        Range single{std::max(Number{1}, child.lo), std::min(n, child.hi)};
        if (single.lo <= single.hi) {
            branches.push_back({bit, single, pair});
        }
    }

    int min_difference_bit = 0;
    std::size_t step = 1;
    bool bisect = false;
    Number single;

    while (true) {
        Range forbidden = block_at(total, min_difference_bit);
        std::erase_if(branches, [&](const Branch& branch) {
            return forbidden.contains(branch.single.lo) && forbidden.contains(branch.single.hi);
        });

        Number smallest = n, largest = 0;
        for (const Branch& branch : branches) {
            Range r = branch.single;
            smallest = std::min(smallest, forbidden.contains(r.lo) ? forbidden.hi + 1 : r.lo);
            largest = std::max(largest, forbidden.contains(r.hi) ? forbidden.lo - 1 : r.hi);
        }
        if (smallest == largest) {
            single = smallest;
            break;
        }

        // The block on the path of total contains all three above the
        // split, then the singleton, and finally none below its divergence.
        std::size_t i = bisect ? branches.size() / 2
                               : branches.size() - std::min(step, branches.size());
        if (step == 1 && i > 0) {
            Range top_pair = branches[i].pair;
            Number length = top_pair.hi - top_pair.lo + 1;
            if (length > 8 && length <= power(branches[i - 1].bit - 1)) {
                --i; // A short paired child can be tested by a smaller block.
            }
        } else if (i == 0 && branches.size() > 1 && branches.front().bit == 1) {
            // If all three fit in four values, their XOR determines them.
            ++i;
        }
        int bit = branches[i].bit;
        Range query = block_at(total, bit);
        Number value = ask({std::max(Number{1}, query.lo), std::min(n, query.hi)});
        if (value == total) {
            branches.resize(i);
            step = std::min<std::size_t>(2 * step, 64);
        } else if (value == 0) {
            branches.erase(branches.begin(), branches.begin() + i + 1);
            min_difference_bit = bit;
            bisect = true;
        } else {
            branches.erase(branches.begin(), branches.begin() + i);
            single = value;
            break;
        }
    }

    std::vector<Range> intervals;
    for (const Branch& branch : branches) {
        if (branch.single.contains(single)) {
            intervals.push_back(branch.pair);
        }
    }
    return find_pair(n, single, total ^ single, std::move(intervals));
}

Triple solve_zero(Number n) {
    struct Exponents {
        int lo, hi;
    };
    std::vector<Exponents> groups{{0, highest_bit(n - 1)}};
    int probes = 0, middle = 0;

    while (true) {
        if (groups.size() == 1 && groups[0].lo == 0 && groups[0].hi == 1) {
            return {1, 2, 3};
        }

        // The singleton's highest bit and the pair's common highest bit
        // stay in the same group after every zero reply. With two endpoints
        // we can split two different groups at once.
        std::array<int, 2> cuts{};
        for (std::size_t i = 0; i < groups.size(); ++i) {
            Exponents group = groups[i];
            if (probes == 0) {
                cuts[i] = group.hi;
            } else if (probes == 2 && i == 0 && group.hi >= 8) {
                middle = cuts[i] = (group.lo + group.hi + 1) / 2;
            } else if (probes > 2 && middle != 0) {
                cuts[i] = group.hi < middle ? group.lo + 1 : group.hi;
            } else {
                cuts[i] = std::max(group.lo + 1, group.hi - 1);
            }
        }
        Range query{power(cuts[0]), groups.size() == 1
                                  ? std::min(n, power(groups[0].hi + 1) - 1)
                                  : power(cuts[1]) - 1};
        Number single = ask(query);
        ++probes;
        if (single != 0) {
            int bit = highest_bit(single);
            for (std::size_t i = 0; i < groups.size(); ++i) {
                if (groups[i].lo <= bit && bit < cuts[i]) {
                    Range pair{power(cuts[i]), std::min(n, power(groups[i].hi + 1) - 1)};
                    return find_pair(n, single, single, {pair});
                }
            }
        }

        std::vector<Exponents> next;
        for (std::size_t i = 0; i < groups.size(); ++i) {
            if (groups[i].lo + 1 < cuts[i]) {
                next.push_back({groups[i].lo, cuts[i] - 1});
            }
            if (cuts[i] < groups[i].hi) {
                next.push_back({cuts[i], groups[i].hi});
            }
        }
        groups = std::move(next);
    }
}

// There are at most 56 triples for n <= 8. For n <= 20, fixing the
// total XOR leaves at most C(20, 2) / 3 triples, since two values fix the third.
Triple solve_small(Number n) {
    Number known_total = n > 8 ? ask({1, n}) : 0;
    std::vector<Triple> triples;
    for (Number a = 1; a <= n; ++a) {
        for (Number b = a + 1; b <= n; ++b) {
            for (Number c = b + 1; c <= n; ++c) {
                if (n <= 8 || (a ^ b ^ c) == known_total) {
                    triples.push_back({a, b, c});
                }
            }
        }
    }

    struct Probe {
        Range interval;
        std::array<Number, 32> groups{};
    };
    std::vector<Probe> probes;
    for (Number lo = 1; lo <= n; ++lo) {
        for (Number hi = lo; hi <= n; ++hi) {
            Probe probe{{lo, hi}};
            for (std::size_t i = 0; i < triples.size(); ++i) {
                Number reply = 0;
                for (Number value : triples[i]) {
                    if (probe.interval.contains(value)) {
                        reply ^= value;
                    }
                }
                probe.groups[reply] |= power(i);
            }
            probes.push_back(probe);
        }
    }

    struct Choice {
        int total, worst;
        std::size_t probe;
    };
    std::unordered_map<Number, Choice> memo;
    auto best = [&](auto&& self, Number mask) -> Choice {
        if (std::has_single_bit(mask)) {
            return {0, 0, 0};
        }
        if (auto it = memo.find(mask); it != memo.end()) {
            return it->second;
        }
        int remaining = std::popcount(mask);
        std::vector<std::pair<int, std::size_t>> order;
        for (std::size_t i = 0; i < probes.size(); ++i) {
            int bound = remaining;
            for (Number group : probes[i].groups) {
                int size = std::popcount(mask & group);
                if (size == remaining) {
                    bound = 0;
                    break;
                }
                if (size > 1) {
                    bound += size;
                }
            }
            if (bound != 0) {
                order.emplace_back(bound, i);
            }
        }
        std::sort(order.begin(), order.end());

        Choice result{1000000, 1000000, 0};
        for (auto [bound, i] : order) {
            if (bound > result.total || (bound == result.total && result.worst <= 2)) {
                break;
            }
            int total = bound;
            int worst = 1;
            for (Number group : probes[i].groups) {
                Number child = mask & group;
                if (child != 0 && !std::has_single_bit(child)) {
                    Choice next = self(self, child);
                    total += next.total - std::popcount(child);
                    worst = std::max(worst, 1 + next.worst);
                    if (total > result.total || (total == result.total && worst >= result.worst)) {
                        break;
                    }
                }
            }
            if (total < result.total || (total == result.total && worst < result.worst)) {
                result = {total, worst, i};
                if (worst == 1) {
                    break;
                }
            }
        }
        memo.emplace(mask, result);
        return result;
    };

    Number mask = power(triples.size()) - 1;
    while (!std::has_single_bit(mask)) {
        const Probe& probe = probes[best(best, mask).probe];
        mask &= probe.groups[ask(probe.interval)];
    }
    return triples[std::countr_zero(mask)];
}

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    std::int64_t input;
    if (!(std::cin >> input) || input == -1) {
        return 0;
    }
    Number n = input;
    Triple answer;
    if (n <= 20) {
        answer = solve_small(n);
    } else {
        Number total = ask({1, n});
        answer = total == 0 ? solve_zero(n) : solve_nonzero(n, total);
    }
    std::cout << "ans " << answer[0] << ' ' << answer[1] << ' ' << answer[2] << std::endl;
    return 0;
}
