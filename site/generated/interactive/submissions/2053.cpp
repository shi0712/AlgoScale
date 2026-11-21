#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <limits>
#include <numeric>
#include <random>
#include <tuple>
#include <utility>
#include <vector>

using namespace std;

namespace {
constexpr int limit = 1'000'000'000;
constexpr int bits = 15;
constexpr int base = 1 << bits;
constexpr int mask = base - 1;
constexpr auto halfWeight = [] {
    array<unsigned char, base> table{};
    for (int i = 1; i < base; ++i) table[i] = table[i / 2] + (i & 1);
    return table;
}();
int ones(int value) { return halfWeight[value & mask] + halfWeight[value >> bits]; }

// A group is the Cartesian product of two sorted lists of 15-bit halves.
// Distinct groups represent disjoint sets of original secrets.
struct Group {
    vector<int> high, low;
};

class Candidates {
    vector<Group> groups;
    vector<int> values;
    vector<int> ends;

    void summarize() {
        ends.clear();
        count = 0;
        minimum = limit;
        maximum = 0;
        for (const auto& g : groups) {
            count += int(g.high.size() * g.low.size());
            ends.push_back(count);
            minimum = min(minimum, g.high.front() * base + g.low.front());
            maximum = max(maximum, g.high.back() * base + g.low.back());
        }
        if (count <= 12000) {
            values.reserve(count);
            for (const auto& g : groups)
                for (int h : g.high)
                    for (int l : g.low)
                        values.push_back(h * base + l);
            groups.clear();
        }
    }

public:
    int count = 0, minimum = limit, maximum = 0;

    explicit Candidates(int weight) {
        // Within each rectangle every high half can be paired with every low half.
        for (int side = 0; side < 2; ++side) {
            array<vector<int>, bits + 1> highs, lows;
            for (int h = 0; h <= (limit >> bits) - side; ++h)
                highs[ones(h)].push_back(h);
            for (int l = 0; l < base; ++l)
                if ((l > (limit & mask)) == side)
                    lows[ones(l)].push_back(l);
            for (int i = max(0, weight - bits); i <= min(bits, weight); ++i)
                if (!highs[i].empty() && !lows[weight - i].empty())
                    groups.push_back({move(highs[i]), move(lows[weight-i])});
        }
        summarize();
    }

    void observe(int total, int response) {
        if (!values.empty()) {
            erase_if(values, [&](int v) { return ones(v - total) != response; });
            count = int(values.size());
            if (count) {
                auto bounds = minmax_element(values.begin(), values.end());
                minimum = *bounds.first;
                maximum = *bounds.second;
            }
            return;
        }
        // For n = h*base + l, subtracting total gives a low half of
        // (l-lo) mod base and a high half of h-hi-(l<lo). Fixing the
        // borrow and the low weight makes the two halves independent again.
        vector<Group> next;
        int hi = total >> bits, lo = total & mask;
        for (const auto& g : groups) {
            array<array<vector<int>, bits + 1>, 2> lows;
            for (int l : g.low)
                lows[l < lo][ones((l - lo) & mask)].push_back(l);
            for (int borrow = 0; borrow <= 1; ++borrow) {
                if (borrow ? g.low.front() >= lo : g.low.back() < lo) continue;
                array<vector<int>, bits + 1> highs;
                for (int h : g.high) {
                    int lowWeight = response - ones(h - hi - borrow);
                    if (0 <= lowWeight && lowWeight <= bits && !lows[borrow][lowWeight].empty())
                        highs[lowWeight].push_back(h);
                }
                for (int w = 0; w <= bits; ++w)
                    if (!highs[w].empty())
                        next.push_back({move(highs[w]), move(lows[borrow][w])});
            }
        }
        groups = move(next);
        summarize();
    }

    struct Outcomes {
        array<int, 31> size{};
        array<int, 31> smallest, common;
        Outcomes() { smallest.fill(limit); common.fill((1 << 30)-1); }
    };

    Outcomes outcomes(int total) const {
        Outcomes out;
        if (!values.empty()) {
            for (int n : values) {
                int w = ones(n - total);
                ++out.size[w];
                out.common[w] &= n - total;
                out.smallest[w] = min(out.smallest[w], n);
            }
            return out;
        }
        int hi = total >> bits, lo = total & mask;
        for (const auto& g : groups) {
            array<array<int, bits+1>, 2> numLow{}, minLow, commonLow;
            for (auto& row : commonLow) row.fill(mask);
            for (auto& row : minLow) row.fill(base);
            for (int l : g.low) {
                int w = ones((l - lo) & mask), b = l < lo;
                ++numLow[b][w];
                commonLow[b][w] &= (l - lo) & mask;
                minLow[b][w] = min(minLow[b][w], l);
            }
            for (int b = 0; b <= 1; ++b) {
                if (b ? g.low.front() >= lo : g.low.back() < lo) continue;
                array<int, bits+1> numHigh{}, minHigh, commonHigh;
                commonHigh.fill(mask);
                minHigh.fill(base);
                for (int h : g.high) {
                    int w = ones(h - hi - b);
                    ++numHigh[w];
                    commonHigh[w] &= h - hi - b;
                    minHigh[w] = min(minHigh[w], h);
                }
                for (int i = 0; i <= bits; ++i) if (numHigh[i]) {
                    for (int j = 0; j <= bits; ++j) if (numLow[b][j]) {
                        out.size[i+j] += numHigh[i] * numLow[b][j];
                        out.common[i+j] &= commonHigh[i]*base + commonLow[b][j];
                        out.smallest[i+j] = min(out.smallest[i+j], minHigh[i]*base + minLow[b][j]);
                    }
                }
            }
        }
        return out;
    }

    // Sample both by binary length and by distance between the outermost
    // set bits. This also represents narrow integers and shifted bit patterns.
    vector<int> sampleWidths(mt19937& rng) const {
        struct Block {
            const int* high;
            const int* low;
            int lowCount, size;
        };
        using Classes = array<vector<Block>,31>;
        array<Classes,2> classes;
        array<array<vector<int>,31>,2> singles;
        auto single = [&](int n) {
            int len = 32-__builtin_clz(n);
            singles[0][len].push_back(n);
            singles[1][len-__builtin_ctz(n)].push_back(n);
        };
        for (int n : values) single(n);
        vector<array<vector<int>,bits>> lowByZeros(groups.size());
        for (size_t i = 0; i < groups.size(); ++i) {
            const auto& g = groups[i];
            for (int l : g.low) {
                if (l) {
                    lowByZeros[i][__builtin_ctz(l)].push_back(l);
                    if (!g.high.front()) single(l);
                } else {
                    for (int h : g.high) if (h) single(h*base);
                }
            }
            auto begin = g.high.begin() + (g.high.front()==0);
            while (begin != g.high.end()) {
                int highLen = 32-__builtin_clz(*begin);
                auto end = lower_bound(begin, g.high.end(), 1 << highLen);
                int len = bits + highLen;
                for (int z = 0; z < bits; ++z) {
                    const auto& lows = lowByZeros[i][z];
                    if (lows.empty()) continue;
                    int nl = int(lows.size());
                    Block block{&*begin, lows.data(), nl, int(end-begin)*nl};
                    classes[0][len].push_back(block);
                    classes[1][len-z].push_back(block);
                }
                begin = end;
            }
        }
        vector<int> result;
        constexpr int perWidth = 20;
        for (int kind = 0; kind < 2; ++kind) {
            for (int width = 1; width <= 30; ++width) {
                const auto& blocks = classes[kind][width];
                const auto& direct = singles[kind][width];
                int count = int(direct.size());
                for (const auto& b : blocks) count += b.size;
                if (!count) continue;
                int offset = int(direct.size());
                size_t pos = 0;
                for (int j = 0; j < perWidth; ++j) {
                    int rank = int((int64_t(j)*count+rng()%count)/perWidth);
                    if (rank < int(direct.size())) {
                        result.push_back(direct[rank]);
                        continue;
                    }
                    while (rank >= offset + blocks[pos].size) offset += blocks[pos++].size;
                    int idx = rank-offset;
                    const auto& b = blocks[pos];
                    result.push_back(b.high[idx/b.lowCount]*base+b.low[idx%b.lowCount]);
                }
            }
        }
        return result;
    }

    vector<int> sample(int size, mt19937& rng) const {
        if (!values.empty() && count <= size) return values;
        size = min(size, count);
        vector<int> items;
        items.reserve(size);
        for (int i = 0; i < size; ++i) {
            // Stratify over the disjoint rectangles and their contents.
            int start = int(int64_t(i) * count / size);
            int end = int(int64_t(i + 1) * count / size);
            int idx = start + int(rng() % (end - start));
            if (!values.empty()) {
                items.push_back(values[idx]);
            } else {
                int j = int(upper_bound(ends.begin(), ends.end(), idx) - ends.begin());
                int pos = idx - (j ? ends[j-1] : 0);
                const auto& g = groups[j];
                items.push_back(g.high[pos / g.low.size()] * base + g.low[pos % g.low.size()]);
            }
        }
        return items;
    }
};

// Estimate the remaining number of queries for a reply class of this size.
double estimate(double n) {
    if (n <= 1.000001) return 0;
    double lg = log2(n);
    return max(1.0 + .015 * (lg-1), (lg+1.2) / 3.9);
}

struct Option {
    double cost;
    int q;
    bool operator<(const Option& other) const {
        return tie(cost, q) < tie(other.cost, other.q);
    }
};

vector<Option> options(const vector<int>& sample, int count, int total, int minimum,
                       mt19937& rng, bool deep, int keep, int hint = 0,
                       const vector<int>* byWidth = nullptr, double widthWeight = .28) {
    int avail = minimum - total;
    if (count == 2) return {{0.0, minimum}};
    vector<int> xs;
    auto add = [&](int x) {
        if (x >= 1 && x <= avail) xs.push_back(x);
    };
    int trials = deep ? 1400 : 100;
    if (avail <= 2*trials) {
        xs.resize(avail);
        iota(xs.begin(), xs.end(), 1);
    } else {
        for (int x = 1; x <= 8; ++x) add(x);
        add(avail);
        add(hint-total);
        for (int b = 0; (1 << b) <= avail; ++b) {
            int p = 1 << b;
            add(p);
            add(p-1);
            add(avail / p * p);
            add(avail-p);
        }
        for (int i = 0; i < trials; ++i) {
            int randomX = 1 + int(rng() % avail);
            add(randomX);
            // A low half equal to a candidate's residue, or one greater,
            // ends a run of borrows or starts one at the chosen bit.
            const auto& targets = byWidth && i%3 == 0 ? *byWidth : sample;
            int v = targets[rng() % targets.size()] - total;
            int b = 1 + int(rng() % 30);
            int p = 1 << b;
            int r = (v & (p-1)) + int(rng() & 1);
            if (r <= avail) {
                add(r);
                add(r + int(rng() % (1 + (avail-r)/p)) * p);
            }
        }
    }
    sort(xs.begin(), xs.end());
    xs.erase(unique(xs.begin(), xs.end()), xs.end());

    vector<double> cost(sample.size()+1);
    double scale = double(count) / sample.size();
    for (int i = 1; i < int(cost.size()); ++i)
        cost[i] = i * estimate(i*scale);

    vector<Option> ranked;
    array<Option,31> diverse;
    diverse.fill({numeric_limits<double>::infinity(), 0});
    double unseen = estimate(max(1.0,scale*0.15));
    auto evaluate = [&](int x) {
        array<int, 31> buckets{};
        int q = total+x;
        for (int n : sample) ++buckets[ones(n-q)];
        double loss = 0;
        for (int c : buckets) loss += cost[c];
        loss /= sample.size();
        if (byWidth) {
            array<int,31> hits{};
            for (int n : *byWidth) ++hits[ones(n-q)];
            double extra = 0;
            for (int r = 1; r <= 30; ++r) if (hits[r]) {
                int c = buckets[r];
                extra += hits[r] * (c ? cost[c]/c : unseen);
            }
            loss += widthWeight*(extra/byWidth->size()-loss);
        }
        ranked.push_back({loss, q});
        if (deep) {
            auto& opt = diverse[ones(minimum-q)];
            opt = min(opt, ranked.back());
        }
    };
    for (int x : xs) evaluate(x);
    auto sortBest = [&](int k) {
        k = min(k, int(ranked.size()));
        partial_sort(ranked.begin(), ranked.begin()+k, ranked.end());
        ranked.resize(k);
    };
    if (deep && avail > 2*trials) {
        sortBest(20);
        for (int iter = 0; iter < 3 && ranked.front().cost > 0; ++iter) {
            xs.clear();
            for (Option opt : ranked) {
                int x = opt.q - total;
                add(x);
                for (int b = 0; (1 << b) <= avail; ++b) {
                    int p = 1 << b;
                    add(x ^ p);
                    add(x-p);
                    add(x+p);
                    int partner = ranked[rng()%ranked.size()].q-total;
                    add((x & -p) | (partner & (p-1)));
                }
            }
            sort(xs.begin(), xs.end());
            xs.erase(unique(xs.begin(),xs.end()),xs.end());
            ranked.clear();
            for (int x : xs) evaluate(x);
            sortBest(20);
        }
    }
    sortBest(keep);
    if (deep && ranked.front().cost) {
        sort(diverse.begin(), diverse.end());
        for (int i = 0; i < 12 && diverse[i].q; ++i) ranked.push_back(diverse[i]);
    }
    return ranked;
}

int choose(const Candidates& candidates, int total, mt19937& rng,
           int hint = 0, array<int,31>* nextHints = nullptr) {
    int avail = candidates.minimum - total;
    if (avail <= 1 || candidates.count == 2) return candidates.minimum;
    auto state = candidates.outcomes(total);
    int r0 = ones(avail);
    // Remove the common ones and subtract one. The response reveals
    // the position of the lowest of the remaining, unknown ones.
    int common = state.common[r0];
    int peel = total + common + 1;
    if (r0 - ones(common) == 1) return peel;
    auto sample = candidates.sample(1536, rng);
    auto byWidth = candidates.sampleWidths(rng);
    // Include the feasible endpoints when comparing splits.
    int copies = int(byWidth.size()/6);
    byWidth.insert(byWidth.end(), copies, candidates.minimum);
    byWidth.insert(byWidth.end(), copies, candidates.maximum);
    auto ranked = options(sample, candidates.count, total, candidates.minimum, rng, true, 16, hint, &byWidth);
    if (!ranked.front().cost) return ranked.front().q;
    // Look one reply ahead using the exact legal range on each branch.
    vector<int> choices;
    for (Option opt : ranked) choices.push_back(opt.q);
    choices.push_back(candidates.minimum);
    choices.push_back(total+1);
    choices.push_back(peel);
    if (hint > total && hint <= candidates.minimum) choices.push_back(hint);
    sort(choices.begin(), choices.end());
    choices.erase(unique(choices.begin(), choices.end()), choices.end());
    auto pool = candidates.sample(8192, rng);
    double best = numeric_limits<double>::infinity();
    // The minimum is always legal and gives zero for exactly one secret.
    // Every other query considered for use must also split the exact set.
    int winner = candidates.minimum;
    for (int q : choices) {
        auto result = candidates.outcomes(q);
        if (*max_element(result.size.begin(),result.size.end()) == candidates.count) continue;
        array<vector<int>,31> children;
        for (int n : pool) children[ones(n-q)].push_back(n);
        array<vector<int>,31> widthChildren;
        for (int n : byWidth) widthChildren[ones(n-q)].push_back(n);
        double loss = 0;
        array<int,31> hints{};
        for (int r = 0; r <= 30; ++r) {
            int size = result.size[r];
            if (size <= 1) continue;
            const auto& widthSample = widthChildren[r];
            double widthMass = .28*widthSample.size()/byWidth.size();
            double mass = .72*size/candidates.count + widthMass;
            double childCost = estimate(size);
            auto& sample2 = children[r];
            int unknown = r - ones(result.common[r]);
            if (unknown == 1) {
                childCost = 1;
                hints[r] = q + result.common[r] + 1;
            } else if (size == 2) {
                childCost = 1;
                hints[r] = result.smallest[r];
            } else if (!sample2.empty()) {
                vector<int> training;
                if (sample2.size() > 192) {
                    shuffle(sample2.begin(), sample2.end(), rng);
                    training.assign(sample2.begin(),sample2.begin()+192);
                }
                auto child = options(training.empty() ? sample2 : training, size, q,
                                     result.smallest[r], rng, false, 1, q+result.common[r]+1,
                                     widthSample.empty() ? nullptr : &widthSample, widthMass/mass).front();
                hints[r] = child.q;
                array<int,31> frequency{}, widthFreq{}, commonBits;
                commonBits.fill((1<<30)-1);
                for (int n : sample2) {
                    int v = n-child.q, w = ones(v);
                    ++frequency[w];
                    commonBits[w] &= v;
                }
                for (int n : widthSample) ++widthFreq[ones(n-child.q)];
                childCost = 1;
                double scale = double(size)/sample2.size();
                for (int w = 1; w <= 30; ++w) {
                    int f = frequency[w];
                    if (!f && !widthFreq[w]) continue;
                    double remaining = estimate(scale * (f ? f : .15));
                    if (f && (size == int(sample2.size()) || f >= 6))
                        remaining = min(remaining, double(w-ones(commonBits[w])));
                    double fraction = (mass-widthMass)/mass * f/sample2.size();
                    if (widthMass)
                        fraction += widthMass/mass * widthFreq[w]/widthSample.size();
                    childCost += fraction * remaining;
                }
            }
            loss += mass * min(childCost, double(unknown));
        }
        if (loss < best - 1e-8) {
            best = loss;
            winner = q;
            if (nextHints) *nextHints = hints;
        }
    }
    return winner;
}
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    int weight;
    if (!(cin >> weight) || weight < 1 || weight > 30) return 0;
    Candidates candidates(weight);
    mt19937 rng(7128241);
    int total = 0, hint = 0;
    while (candidates.count > 1) {
        array<int,31> hints{};
        int next = choose(candidates, total, rng, hint, &hints);
        cout << "- " << next - total << endl;
        int reply;
        if (!(cin >> reply) || reply < 0 || reply > 30) return 0;
        candidates.observe(next, reply);
        total = next;
        hint = hints[reply];
    }
    if (candidates.count == 1) cout << "! " << candidates.minimum << endl;
    return 0;
}
