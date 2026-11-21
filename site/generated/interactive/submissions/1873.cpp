#include <bits/stdc++.h>
using namespace std;

using U128 = __uint128_t;

constexpr int LIMIT = 1000000000;
constexpr int BITS = 15;
constexpr int BASE = 1 << BITS;
constexpr int MASK = BASE - 1;
constexpr int LIST_LIMIT = 180000;

int pc[BASE], binom[31][31];
int spent = 0;
uint64_t rngState = 0x91e10da5c79e7b1dULL;

uint64_t randomWord() {
    rngState += 0x9e3779b97f4a7c15ULL;
    uint64_t z = rngState;
    z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
    z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
    return z ^ (z >> 31);
}

int bits(int x) {
    return pc[x & MASK] + pc[x >> BITS];
}

int ask(int newSpent) {
    cout << "- " << newSpent - spent << endl;
    int result;
    if (!(cin >> result) || result == -1) exit(0);
    spent = newSpent;
    return result;
}

void answer(int n) {
    cout << "! " << n << endl;
}

int countWithBits(int limit, int k) {
    int result = 0;
    for (int j = 29; j >= 0; --j) {
        if ((limit >> j) & 1) {
            result += binom[j][k];
            if (--k < 0) return result;
        }
    }
    return result + (k == 0);
}

vector<int> queryOptions(int total, int minimum, const vector<int>& sample,
                         int randomCount, int exhaustiveLimit, bool detailed) {
    int width = minimum - total;
    vector<int> options;
    auto add = [&](long long s) {
        if (s > total && s <= minimum) options.push_back(int(s));
    };
    if (width <= exhaustiveLimit) {
        options.reserve(width);
        for (int s = total + 1; s <= minimum; ++s) options.push_back(s);
        return options;
    }

    for (int d = 0; d < (detailed ? 32 : 8); ++d) {
        add(total + 1 + d);
        add(minimum - d);
    }
    for (int j = 0; (1LL << j) <= width; ++j) {
        long long p = 1LL << j;
        add(total + p - 1);
        add(total + p);
        add(total + p + 1);
        add(minimum - p);
        add(minimum - p + 1);
        add(minimum & ~(p - 1));
        add(total + (width & ~(p - 1)));
        if (detailed) {
            add(minimum - (p >> 1));
            add(total + p + (p >> 1));
        }
    }
    if (detailed && sample.size() <= 96) {
        int take = min<int>(8, sample.size());
        for (int i = 0; i < take; ++i) {
            int n = sample[i * (sample.size() - 1) / max(1, take - 1)];
            for (int j = 0; (1LL << j) <= n - total; ++j) {
                add(n - (1LL << j));
                add(n - (1LL << j) + 1);
            }
        }
    }
    for (int i = 0; i < randomCount; ++i) {
        int range = width;
        if (i % 3 == 1) {
            int shift = int(randomWord() % 5) + 1;
            range = max(1, width >> shift);
        }
        int d = int(randomWord() % range);
        if (i % 3 == 2) add(minimum - d);
        else add(total + 1 + d);
    }

    sort(options.begin(), options.end());
    options.erase(unique(options.begin(), options.end()), options.end());
    return options;
}

struct Bucket {
    int size = 0;
    int first = 0;
    unsigned diff = 0;

    void insert(int x) {
        if (size == 0) first = x;
        else diff |= unsigned(first ^ x);
        ++size;
    }

    int bound() const {
        if (size <= 1) return 0;
        if (size == 2) return 1;
        int lo = countr_zero(diff);
        int hi = bit_width(diff);
        unsigned variableSpan = ((1u << hi) - 1) ^ ((1u << lo) - 1);
        return min(size - 1, bits(first & variableSpan));
    }
};

struct Ranked {
    double cost;
    int query;
    bool operator<(const Ranked& other) const {
        if (cost != other.cost) return cost < other.cost;
        return query < other.query;
    }
};

vector<double> weights(int size, bool exact) {
    vector<double> result(size + 1);
    for (int i = 2; i <= size; ++i)
        result[i] = i * (log2(double(i)) + (exact ? 1.25 : 0.0));
    return result;
}

double sampledCost(const vector<int>& a, int s, const vector<double>& w) {
    int freq[31]{};
    for (int n : a) ++freq[bits(n - s)];
    double result = 0;
    for (int f : freq) result += w[f];
    return result;
}

int separatingQuery(const vector<int>& a, int total) {
    if (a.size() == 2) return a.front();
    if (a.size() > 31) return 0;

    auto options = queryOptions(total, a.front(), a, 125, 320, false);
    for (int s : options) {
        unsigned seen = 0;
        bool good = true;
        for (int n : a) {
            unsigned b = 1u << bits(n - s);
            if (seen & b) {
                good = false;
                break;
            }
            seen |= b;
        }
        if (good) return s;
    }
    return 0;
}

struct Choice {
    int query = 0;
    array<int, 31> next{};
};

Choice chooseExact(const vector<int>& a, const Bucket& state, int budget) {
    int p = countr_zero(state.diff);
    int baseline = spent + (state.first & ((1 << p) - 1)) + (1 << p);

    vector<int> sample;
    constexpr int SAMPLE_SIZE = 2100;
    if (a.size() <= SAMPLE_SIZE) {
        sample = a;
    } else {
        sample.reserve(SAMPLE_SIZE);
        for (int i = 0; i < SAMPLE_SIZE; ++i) {
            int l = int(uint64_t(i) * a.size() / SAMPLE_SIZE);
            int r = int(uint64_t(i + 1) * a.size() / SAMPLE_SIZE);
            sample.push_back(a[l + randomWord() % (r - l)]);
        }
    }

    auto options = queryOptions(spent, a.front(), sample, 950, 1700, true);
    options.push_back(baseline);
    bool fullSample = sample.size() == a.size();
    auto w = weights(int(sample.size()), fullSample);
    vector<Ranked> ranked;
    ranked.reserve(options.size());
    for (int s : options) ranked.push_back({sampledCost(sample, s, w), s});
    sort(ranked.begin(), ranked.end());

    auto exactWeights = fullSample ? w : weights(int(a.size()), true);
    vector<Ranked> feasible;
    auto evaluate = [&](int s) {
        Bucket groups[31];
        for (int n : a) {
            int y = n - s;
            groups[bits(y)].insert(y);
        }
        double cost = 0;
        for (const Bucket& group : groups) {
            if (group.bound() >= budget) return;
            cost += exactWeights[group.size];
        }
        feasible.push_back({cost, s});
    };

    int examine = min<int>(ranked.size(), fullSample ? 150 : 40);
    for (int i = 0; i < examine; ++i) {
        if (i == 0 || ranked[i].query != ranked[i - 1].query)
            evaluate(ranked[i].query);
    }
    evaluate(baseline);
    sort(feasible.begin(), feasible.end());

    Choice best;
    best.query = feasible.front().query;
    if (feasible.front().cost == 0 || a.size() > 96) return best;

    // Estimate the next round exactly for small branches and retain any
    // separating queries found, so they need not be searched for again.
    double bestTwoStep = numeric_limits<double>::infinity();
    int tried = 0, last = -1;
    for (auto [cost, s] : feasible) {
        if (s == last) continue;
        last = s;
        if (++tried > 22) break;
        array<vector<int>, 31> groups;
        for (int n : a) groups[bits(n - s)].push_back(n);
        Choice candidate;
        candidate.query = s;
        double value = 0;
        for (int r = 0; r <= 30; ++r) {
            int size = int(groups[r].size());
            if (size <= 1) continue;
            candidate.next[r] = separatingQuery(groups[r], s);
            if (candidate.next[r]) value += size;
            else value += size * (1.82 + 0.20 * log2(double(size)));
        }
        value += 0.0001 * cost;
        if (value < bestTwoStep) {
            bestTwoStep = value;
            best = candidate;
        }
    }
    return best;
}

void finishList(vector<int> a, int budget, bool strict) {
    while (a.size() > 1) {
        Bucket state;
        for (int n : a) state.insert(n - spent);
        budget = min(budget, state.bound() + (strict ? 0 : 1));
        Choice choice = chooseExact(a, state, budget);
        int response = ask(choice.query);
        --budget;
        a.erase(remove_if(a.begin(), a.end(), [&](int n) {
            return bits(n - spent) != response;
        }), a.end());
        if (a.size() > 1 && choice.next[response]) {
            response = ask(choice.next[response]);
            a.erase(remove_if(a.begin(), a.end(), [&](int n) {
                return bits(n - spent) != response;
            }), a.end());
            --budget;
        }
    }
    answer(a.front());
}

void solveSparse(int initial) {
    int known = 0, p = 0, left = initial;
    while (left) {
        int top = (LIMIT - known) >> p;
        int count = countWithBits(top, left);
        if (count <= LIST_LIMIT) {
            vector<int> candidates;
            candidates.reserve(count);
            unsigned v = (1u << left) - 1;
            while (v <= unsigned(top)) {
                candidates.push_back(known + int(v << p));
                unsigned r = v & -v;
                unsigned next = v + r;
                v = next | (((next ^ v) >> 2) / r);
            }
            finishList(move(candidates), left, true);
            return;
        }
        int r = ask(known + (1 << p));
        int j = p + r - (left - 1);
        known += 1 << j;
        p = j + 1;
        --left;
    }
    answer(known);
}

struct TranscriptCandidates {
    struct Entry {
        U128 key;
        int low;
    };
    struct Block {
        int high;
        int lowIndex;
        int end;
    };

    vector<Entry> lows;
    vector<Block> blocks;
    int count = 0;
    int minimum = LIMIT + 1;

    explicit TranscriptCandidates(const vector<pair<int, int>>& history) : lows(BASE) {
        constexpr int HIGH_MAX = LIMIT >> BITS;
        constexpr int LOW_MAX = LIMIT & MASK;
        vector<int> cuts{0, BASE, LOW_MAX + 1};
        for (auto [s, c] : history) cuts.push_back(s & MASK);
        sort(cuts.begin(), cuts.end());
        cuts.erase(unique(cuts.begin(), cuts.end()), cuts.end());

        for (int low = 0; low < BASE; ++low) {
            U128 key = 0;
            for (auto [s, c] : history)
                key = (key << 4) | pc[(low - (s & MASK)) & MASK];
            lows[low] = {key, low};
        }

        int m = int(history.size());
        array<int, 32> shiftHigh{}, responses{};
        for (int i = 0; i + 1 < int(cuts.size()); ++i) {
            int l = cuts[i], r = cuts[i + 1];
            auto first = lows.begin() + l, last = lows.begin() + r;
            sort(first, last, [](const Entry& a, const Entry& b) {
                return a.key != b.key ? a.key < b.key : a.low < b.low;
            });

            for (int j = 0; j < m; ++j) {
                auto [s, c] = history[j];
                shiftHigh[j] = (s >> BITS) + (l < (s & MASK));
                responses[j] = c;
            }
            int highEnd = HIGH_MAX - (l > LOW_MAX);
            for (int high = shiftHigh[m - 1]; high <= highEnd; ++high) {
                U128 key = 0;
                bool good = true;
                for (int j = 0; j < m; ++j) {
                    int need = responses[j] - pc[high - shiftHigh[j]];
                    if (unsigned(need) > 15u) {
                        good = false;
                        break;
                    }
                    key = (key << 4) | unsigned(need);
                }
                if (!good) continue;

                auto f = lower_bound(first, last, key, [](const Entry& a, U128 b) {
                    return a.key < b;
                });
                if (f == last || f->key != key) continue;
                auto e = f + 1;
                while (e != last && e->key == key) ++e;
                count += int(e - f);
                blocks.push_back({high, int(f - lows.begin()), count});
                minimum = min(minimum, (high << BITS) + f->low);
            }
        }
    }

    vector<int> all() const {
        vector<int> result;
        result.reserve(count);
        int previous = 0;
        for (const auto& b : blocks) {
            int size = b.end - previous;
            for (int j = 0; j < size; ++j)
                result.push_back((b.high << BITS) + lows[b.lowIndex + j].low);
            previous = b.end;
        }
        sort(result.begin(), result.end());
        return result;
    }

    vector<int> sample(int size) const {
        vector<int> result;
        result.reserve(size);
        size_t i = 0;
        for (int j = 0; j < size; ++j) {
            int l = int(uint64_t(j) * count / size);
            int r = int(uint64_t(j + 1) * count / size);
            int rank = l + int(randomWord() % (r - l));
            while (blocks[i].end <= rank) ++i;
            int offset = rank - (i ? blocks[i - 1].end : 0);
            result.push_back((blocks[i].high << BITS) + lows[blocks[i].lowIndex + offset].low);
        }
        sort(result.begin(), result.end());
        return result;
    }
};

void fallback(int left) {
    int base = spent, known = 0, p = 0;
    while (left) {
        int smallest = ((1 << left) - 1) << p;
        int second = smallest + (1 << (p + left - 1));
        if (second > LIMIT - base - known) {
            answer(base + known + smallest);
            return;
        }
        int r = ask(base + known + (1 << p));
        int j = p + r - (left - 1);
        known += 1 << j;
        p = j + 1;
        --left;
    }
    answer(base + known);
}

void solveDense(int initial) {
    vector<pair<int, int>> history{{0, initial}};
    for (;;) {
        TranscriptCandidates candidates(history);
        if (candidates.count <= LIST_LIMIT) {
            finishList(candidates.all(), 31, false);
            return;
        }
        if (history.size() == 32) {
            fallback(history.back().second);
            return;
        }

        auto sample = candidates.sample(1600);
        auto options = queryOptions(spent, candidates.minimum, sample, 1050, 1500, true);
        auto w = weights(int(sample.size()), false);
        vector<Ranked> ranked;
        ranked.reserve(options.size());
        for (int s : options) ranked.push_back({sampledCost(sample, s, w), s});
        int take = min<int>(22, ranked.size());
        partial_sort(ranked.begin(), ranked.begin() + take, ranked.end());

        auto largerSample = candidates.sample(6500);
        auto w2 = weights(int(largerSample.size()), false);
        Ranked best{numeric_limits<double>::infinity(), candidates.minimum};
        for (int i = 0; i < take; ++i) {
            int s = ranked[i].query;
            Ranked current{sampledCost(largerSample, s, w2), s};
            if (current < best) best = current;
        }
        int s = best.query;
        if (best.cost >= w2[largerSample.size()]) s = candidates.minimum;
        int r = ask(s);
        if (r == 0) {
            answer(s);
            return;
        }
        history.push_back({s, r});
    }
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int initial;
    if (!(cin >> initial) || initial == -1) return 0;

    for (int i = 1; i < BASE; ++i) pc[i] = pc[i >> 1] + (i & 1);
    for (int i = 0; i <= 30; ++i) {
        binom[i][0] = binom[i][i] = 1;
        for (int j = 1; j < i; ++j)
            binom[i][j] = binom[i - 1][j - 1] + binom[i - 1][j];
    }

    if (initial <= 9) solveSparse(initial);
    else solveDense(initial);
}
