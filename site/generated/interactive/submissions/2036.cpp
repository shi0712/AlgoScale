#include <bits/stdc++.h>
using namespace std;

constexpr int MAX_N = 800;
using Bits = bitset<MAX_N>;

struct Equation {
    Bits coefficients;
    int rhs = 0;
};

class LinearSystem {
    vector<Equation> rows;

    size_t reduce(Equation& equation) const {
        size_t pivot = equation.coefficients._Find_first();
        while (pivot < rows.size() && rows[pivot].coefficients.any()) {
            equation.coefficients ^= rows[pivot].coefficients;
            equation.rhs ^= rows[pivot].rhs;
            pivot = equation.coefficients._Find_next(pivot);
        }
        return pivot;
    }

public:
    explicit LinearSystem(int n) : rows(n) {}

    int known(Equation equation) const {
        return reduce(equation) == MAX_N ? equation.rhs : -1;
    }

    void add(Equation equation) {
        size_t pivot = reduce(equation);
        if (pivot < rows.size()) rows[pivot] = equation;
    }

    vector<int> solve() const {
        Bits values;
        vector<int> result(rows.size());
        for (int i = int(rows.size()) - 1; i >= 0; --i) {
            if (rows[i].coefficients.none()) exit(0);
            result[i] = rows[i].rhs ^ int((values & rows[i].coefficients).count() & 1);
            values[i] = result[i];
        }
        return result;
    }
};

Equation equationFor(initializer_list<int> positions, int rhs = 0) {
    Equation equation;
    for (int position : positions) equation.coefficients.flip(position);
    equation.rhs = rhs;
    return equation;
}

int readReply() {
    int reply;
    if (!(cin >> reply) || (reply != 0 && reply != 1)) exit(0);
    return reply;
}

int askPair(int x, int y) {
    cout << "? 2 " << x + 1 << ' ' << y + 1 << '\n' << flush;
    return readReply();
}

int askTriple(int x, int y, int z) {
    cout << "? 3 " << x + 1 << ' ' << y + 1 << ' ' << z + 1 << '\n' << flush;
    return readReply();
}

int askReference(const vector<int>& reference, int slot1 = -1, int in1 = -1,
                 int slot2 = -1, int in2 = -1) {
    cout << "? " << reference.size();
    for (int j = 0; j < int(reference.size()); ++j) {
        int position = reference[j];
        if (j == slot1) position = in1;
        if (j == slot2) position = in2;
        cout << ' ' << position + 1;
    }
    cout << '\n' << flush;
    return readReply();
}

class SmallDecoder {
    using Candidates = bitset<20160>;  // 8! / 2

    struct Test {
        unsigned mask;
        vector<int> positions;
        Candidates yes;
    };

    vector<vector<int>> permutations;
    vector<Test> tests;
    Candidates all;

public:
    explicit SmallDecoder(int n) {
        unsigned end = 1u << n;
        for (unsigned mask = 1; mask + 1 < end; ++mask) {
            if (popcount(mask) < 2) continue;
            Test test{mask, {}, {}};
            for (int i = 0; i < n; ++i) {
                if ((mask >> i) & 1) test.positions.push_back(i);
            }
            tests.push_back(move(test));
        }
        sort(tests.begin(), tests.end(), [](const Test& a, const Test& b) {
            if (a.positions.size() != b.positions.size()) return a.positions.size() < b.positions.size();
            return a.positions < b.positions;
        });

        vector<int> p(n);
        iota(p.begin(), p.end(), 1);
        array<int, 256> sum{};
        do {
            if (p[0] > n / 2) break;
            size_t id = permutations.size();
            permutations.push_back(p);
            all.set(id);
            for (unsigned mask = 1; mask < end; ++mask) {
                sum[mask] = sum[mask & (mask - 1)] + p[countr_zero(mask)];
            }
            for (Test& test : tests) {
                if (sum[test.mask] % int(test.positions.size()) == 0) test.yes.set(id);
            }
        } while (next_permutation(p.begin(), p.end()));
    }

    vector<int> decode() const {
        Candidates active = all;
        int remaining = int(permutations.size());
        while (remaining > 1) {
            const Test* best = nullptr;
            int split = 0, bestYes = 0;
            for (const Test& test : tests) {
                int yes = int((active & test.yes).count());
                int score = min(yes, remaining - yes);
                if (score <= split) continue;
                best = &test;
                split = score;
                bestYes = yes;
                if (split == remaining / 2) break;
            }
            if (!best) exit(0);
            int reply = askReference(best->positions);
            active &= reply ? best->yes : ~best->yes;
            remaining = reply ? bestYes : remaining - bestYes;
        }
        return permutations[active._Find_first()];
    }
};

using Groups = vector<vector<int>>;

Groups groupByResidue(const vector<int>& residues, int modulus) {
    Groups groups(modulus);
    for (int i = 0; i < int(residues.size()); ++i) groups[residues[i]].push_back(i);
    return groups;
}

vector<int> parityResidues(int n) {
    vector<int> residues(n);
    int count[2] = {1, 0};
    for (int i = 1; i < n; ++i) {
        if (count[0] == n / 2) residues[i] = 1;
        else if (count[1] != n / 2) residues[i] = !askPair(0, i);
        ++count[residues[i]];
    }
    return residues;
}

// A nonconstant histogram has a consecutive block of larger classes. Its
// affine symmetries are exactly identity and reflection: a shift of one
// overlaps the smaller of the block and its complement in all but one point.
void orientFromHistogram(vector<int>& residues, int modulus) {
    int n = int(residues.size());
    if (n % modulus == 0) return;

    vector<int> count(modulus), larger(modulus);
    for (int r : residues) ++count[r];
    int mask = modulus - 1;
    for (int a = 1; a < modulus; a += 2) {
        for (int r = 0; r < modulus; ++r) larger[(a * r) & mask] = count[r] > n / modulus;
        int start = -1;
        for (int r = 0; r < modulus; ++r) {
            if (!larger[r] || larger[(r - 1) & mask]) continue;
            if (start != -1) {
                start = -1;
                break;
            }
            start = r;
        }
        if (start == -1) continue;
        for (int& r : residues) r = (a * r + 1 - start) & mask;
        return;
    }
    exit(0);
}

struct Affine {
    int multiplier;
    int shift;
};

// The final answer is invariant under the choice of representative of a
// reflection pair. The actual orientation is restored using position one.
vector<Affine> affineMaps(int h) {
    if (h == 2) return {{1, 0}};
    vector<Affine> maps;
    for (int a = 1; a < h / 2; a += 2) {
        for (int b = 0; b < h; ++b) maps.push_back({a, b});
    }
    return maps;
}

struct Swap {
    int from1, from2, to1, to2;
};

// For small classes, track every balanced split relative to the first
// member. A query can replace either reference member or both of them.
class PartitionPlan {
    struct Test {
        Swap swap;
        vector<uint64_t> ones;
    };

    vector<unsigned> partitions;
    vector<uint64_t> all;
    vector<Test> tests;

public:
    explicit PartitionPlan(int size) {
        for (unsigned bits = 0; bits < (1u << size); bits += 2) {
            int count = popcount(bits);
            if (count == size / 2 || count == (size + 1) / 2) partitions.push_back(bits);
        }
        int words = (int(partitions.size()) + 63) / 64;
        all.assign(words, 0);
        for (size_t i = 0; i < partitions.size(); ++i) all[i / 64] |= uint64_t(1) << (i % 64);

        auto add = [&](Swap swap, unsigned mask) {
            Test test{swap, vector<uint64_t>(words)};
            for (size_t i = 0; i < partitions.size(); ++i) {
                if (popcount(partitions[i] & mask) & 1) test.ones[i / 64] |= uint64_t(1) << (i % 64);
            }
            tests.push_back(move(test));
        };
        for (int i = 2; i < size; ++i) add({0, -1, i, -1}, 1u | (1u << i));
        for (int i = 2; i < size; ++i) add({1, -1, i, -1}, 2u | (1u << i));
        for (int i = 2; i < size; ++i) {
            for (int j = i + 1; j < size; ++j) add({0, 1, i, j}, 3u | (1u << i) | (1u << j));
        }
    }

    unsigned decode(const vector<int>& group, const vector<int>& reference, int r,
                    int baseline, LinearSystem& system) const {
        vector<uint64_t> active = all;
        int remaining = int(partitions.size());
        while (remaining > 1) {
            int split = 0, bestOnes = 0;
            const Test* best = nullptr;
            for (const Test& test : tests) {
                int ones = 0;
                for (size_t w = 0; w < active.size(); ++w) ones += popcount(active[w] & test.ones[w]);
                int score = min(ones, remaining - ones);
                if (score <= split) continue;
                best = &test;
                bestOnes = ones;
                split = score;
                if (split == remaining / 2) break;
            }
            if (!best) exit(0);

            const auto& [from1, from2, to1, to2] = best->swap;
            Equation equation = equationFor({group[from1], group[to1]});
            if (from2 != -1) {
                equation.coefficients.flip(group[from2]);
                equation.coefficients.flip(group[to2]);
            }
            int bit = system.known(equation);
            if (bit == -1) {
                int slot2 = from2 == -1 ? -1 : 2 * r + from2;
                int in2 = to2 == -1 ? -1 : group[to2];
                bit = askReference(reference, 2 * r + from1, group[to1], slot2, in2) ^ baseline;
                equation.rhs = bit;
                system.add(equation);
            }
            for (size_t w = 0; w < active.size(); ++w) {
                active[w] &= bit ? best->ones[w] : ~best->ones[w];
            }
            remaining = bit ? bestOnes : remaining - bestOnes;
        }
        for (size_t w = 0; w < active.size(); ++w) {
            if (active[w]) return partitions[64 * w + countr_zero(active[w])];
        }
        exit(0);
    }
};

// A uniform odd class size means that the larger half of each lifted
// class belongs to the same interval of h residues. Once the within-class
// splits are known, the unknown phases are determined by the affine map.
Affine identifyOddLift(const Groups& groups, const vector<int>& relative,
                       const vector<int>& majority, const vector<int>& reference,
                       int baseline) {
    int h = int(groups.size());
    int mask = h - 1;
    vector<Affine> maps = affineMaps(h);
    int remaining = int(maps.size());
    int words = (remaining + 63) / 64;
    vector<uint64_t> active(words);
    vector<vector<uint64_t>> phase(h, vector<uint64_t>(words));
    for (int i = 0; i < remaining; ++i) {
        uint64_t bit = uint64_t(1) << (i % 64);
        active[i / 64] |= bit;
        for (int r = 0; r < h; ++r) {
            int v = (maps[i].multiplier * r + maps[i].shift) & (2 * h - 1);
            if (v == 0 || v > h) phase[r][i / 64] |= bit;
        }
    }

    mt19937 random(19021999);
    while (remaining > 1) {
        Swap best{};
        int bestSplit = 0, bestOnes = 0;
        auto consider = [&](Swap candidate) {
            const auto& [r, s, t, u] = candidate;
            int ones = 0;
            for (int w = 0; w < words; ++w) {
                uint64_t prediction = phase[r][w] ^ phase[s][w] ^ phase[t][w] ^ phase[u][w];
                ones += popcount(prediction & active[w]);
            }
            int split = min(ones, remaining - ones);
            if (split > bestSplit) {
                best = candidate;
                bestSplit = split;
                bestOnes = ones;
            }
        };
        for (int i = 0; i < 256 && bestSplit * 2 < remaining - 1; ++i) {
            int r = int(random() & mask);
            int s = int(random() & mask);
            int t = int(random() & mask);
            int u = (r + s - t) & mask;
            if (t != u) consider({r, s, t, u});
        }
        for (int r = 1; r + 1 < h && bestSplit * 2 < remaining - 1; ++r) {
            consider({r, 1, r + 1, 0});
        }
        if (!bestSplit) exit(0);

        const auto& [r, s, t, u] = best;
        int secondSlot = 2 * s + int(r == s);
        int secondOut = reference[secondSlot];
        int reply = askReference(reference, 2 * r, groups[t][2], secondSlot, groups[u][2]);
        int carry = (t + u - r - s) / h;
        int bit = reply ^ baseline ^ (carry & 1) ^ relative[secondOut]
                  ^ relative[groups[t][2]] ^ relative[groups[u][2]]
                  ^ majority[r] ^ majority[s] ^ majority[t] ^ majority[u];
        for (int w = 0; w < words; ++w) {
            uint64_t prediction = phase[r][w] ^ phase[s][w] ^ phase[t][w] ^ phase[u][w];
            active[w] &= bit ? prediction : ~prediction;
        }
        remaining = bit ? bestOnes : remaining - bestOnes;
    }
    for (int w = 0; w < words; ++w) {
        if (active[w]) return maps[64 * w + countr_zero(active[w])];
    }
    exit(0);
}

void lift(vector<int>& residues, int h) {
    int n = int(residues.size());
    const Groups groups = groupByResidue(residues, h);
    bool exact = h == 2 || n % h != 0;
    bool oddUniform = !exact && ((n / h) & 1);
    LinearSystem system(n);
    vector<int> highCount(h);
    for (int value = 1; value <= n; ++value) {
        if (value & h) ++highCount[value & (h - 1)];
    }

    // The kernel of all query rows consists of a constant flip on each
    // parity of residues. An odd class with a known split fixes that flip.
    bool fixed[2] = {};
    vector<int> reference;
    for (int r = 0; r < h; ++r) {
        const auto& group = groups[r];
        int size = int(group.size());
        reference.push_back(group[0]);
        reference.push_back(group[1]);
        if (!exact && (size & 1)) continue;
        Equation equation;
        for (int i : group) equation.coefficients.set(i);
        equation.rhs = (exact ? highCount[r] : size / 2) & 1;
        system.add(equation);
        if (size & 1) fixed[r & 1] = true;
    }
    for (int parity = 0; parity < 2; ++parity) {
        if (!fixed[parity]) system.add(equationFor({groups[parity][0]}));
    }

    // The sum of the low residues is h(h-1), so this query's reply is
    // the XOR of the high bits of its selected positions.
    Equation base;
    for (int i : reference) base.coefficients.set(i);
    int baseline = system.known(base);
    if (baseline == -1) {
        baseline = askReference(reference);
        base.rhs = baseline;
        system.add(base);
    }

    constexpr int MAX_PLANNED_SIZE = 16;
    array<unique_ptr<PartitionPlan>, MAX_PLANNED_SIZE + 1> plans;
    vector<int> relative(n), majority(h);
    for (int r = 0; r < h; ++r) {
        const auto& group = groups[r];
        int size = int(group.size());
        if (5 <= size && size <= MAX_PLANNED_SIZE) {
            auto& plan = plans[size];
            if (!plan) plan = make_unique<PartitionPlan>(size);
            unsigned partition = plan->decode(group, reference, r, baseline, system);
            for (int j = 1; j < size; ++j) {
                int bit = (partition >> j) & 1;
                relative[group[j]] = bit;
                system.add(equationFor({group[0], group[j]}, bit));
            }
            majority[r] = 2 * popcount(partition) > size;
            continue;
        }
        int limit = (size + 1) / 2;
        int count[2] = {1, 0};
        for (int step = 2; step <= size; ++step) {
            int j = step == size ? 1 : step;
            int i = group[j];
            Equation comparison = equationFor({group[0], i});
            int bit;
            if (count[0] == limit) bit = 1;
            else if (count[1] == limit) bit = 0;
            else bit = system.known(comparison);

            if (bit == -1) {
                if (j == 1) {
                    bit = askReference(reference, 2 * r + 1, group[2]) ^ baseline ^ relative[group[2]];
                } else {
                    bit = askReference(reference, 2 * r, i) ^ baseline;
                }
            }
            relative[i] = bit;
            comparison.rhs = bit;
            system.add(comparison);
            ++count[bit];
        }
        majority[r] = count[1] > count[0];
    }

    if (oddUniform) {
        Affine map = identifyOddLift(groups, relative, majority, reference, baseline);
        for (int r = 0; r < h; ++r) {
            int low = (map.multiplier * r + map.shift) & (h - 1);
            int largerResidue = low == 0 ? h : low;
            for (int i : groups[r]) {
                residues[i] = largerResidue ^ ((relative[i] ^ majority[r]) * h);
            }
        }
        return;
    }

    // Equal-sum two-position swaps link phases at r, 1, r+1 and 0.
    // Together with the within-class comparisons these span all but the
    // two kernel dimensions accounted for above.
    for (int r = 1; r + 1 < h; ++r) {
        int secondSlot = 2 + int(r == 1);
        Equation equation = equationFor({groups[r][0], reference[secondSlot],
                                         groups[r + 1][2], groups[0][2]});
        if (system.known(equation) != -1) continue;
        equation.rhs = askReference(reference, 2 * r, groups[r + 1][2],
                                    secondSlot, groups[0][2]) ^ baseline;
        system.add(equation);
    }
    vector<int> high = system.solve();
    for (int i = 0; i < n; ++i) residues[i] += h * high[i];
    orientFromHistogram(residues, 2 * h);
}

int firstValue(int residue, int h) {
    return residue == 0 ? h : residue;
}

int missingColor(int residue, int h) {
    return (firstValue(residue, h) + 2 * h) % 3;
}

struct ColoredMap {
    Affine map;
    array<int, 3> label;
};

// In the uniform final case all classes are pairs. Their missing colors
// identify the affine map and color names together. Two maps with the same
// missing colors would give an affine symmetry of [1, n] modulo 3h.
ColoredMap identifyUniformFinal(const Groups& groups, int anchorResidue,
                               int thirdResidue, const array<int, 3>& anchors) {
    int h = int(groups.size());
    vector<ColoredMap> candidates;
    for (Affine map : affineMaps(h)) {
        array<int, 3> label = {0, 1, 2};
        do {
            int r0 = (map.multiplier * anchorResidue + map.shift) & (h - 1);
            if (label[missingColor(r0, h)] == 2) candidates.push_back({map, label});
        } while (next_permutation(label.begin(), label.end()));
    }

    vector<array<int, 3>> counts(h);
    while (candidates.size() > 1) {
        int total = int(candidates.size());
        fill(counts.begin(), counts.end(), array<int, 3>{});
        for (const ColoredMap& candidate : candidates) {
            int mapped = candidate.map.shift;
            for (int r = 0; r < h; ++r) {
                ++counts[r][candidate.label[missingColor(mapped, h)]];
                mapped = (mapped + candidate.map.multiplier) & (h - 1);
            }
        }
        int bestSplit = 0, residue = -1, label = -1;
        for (int r = 0; r < h; ++r) {
            if (r == anchorResidue) continue;
            for (int c = 0; c < 3; ++c) {
                if (r == thirdResidue && c == 2) continue;
                int split = min(counts[r][c], total - counts[r][c]);
                if (split > bestSplit) {
                    bestSplit = split;
                    residue = r;
                    label = c;
                }
            }
        }
        if (residue == -1) exit(0);
        int reply = askTriple(groups[residue][0], groups[residue][1], anchors[label]);
        erase_if(candidates, [&](const ColoredMap& candidate) {
            int mapped = (candidate.map.multiplier * residue + candidate.map.shift) & (h - 1);
            return (candidate.label[missingColor(mapped, h)] == label) != bool(reply);
        });
    }
    if (candidates.empty()) exit(0);
    return candidates.front();
}

class FinalDecoder {
    int n, h;
    Groups groups;
    vector<int> answer;
    array<int, 3> anchors = {-1, -1, -1};

    void assign(int position, int value) {
        answer[position] = value;
        if (anchors[value % 3] == -1) anchors[value % 3] = position;
    }

    void assignColor(int position, int residue, int color) {
        for (int value = firstValue(residue, h); value <= n; value += h) {
            if (value % 3 != color) continue;
            assign(position, value);
            return;
        }
        exit(0);
    }

    void assignPair(int residue, int firstColor, int secondColor) {
        assignColor(groups[residue][0], residue, firstColor);
        assignColor(groups[residue][1], residue, secondColor);
    }

    bool hasColor(int position, int color) const {
        return askTriple(anchors[(color + 1) % 3], anchors[(color + 2) % 3], position);
    }

    void prepareUniform() {
        int r = 0;
        int s = h / 2;
        int x = groups[r][0], y = groups[r][1];
        int z = groups[s][0];
        // A half-period shift has distinct missing color under every odd
        // multiplier, so exactly one member of the second pair works.
        if (!askTriple(x, y, z)) z = groups[s][1];
        ColoredMap candidate = identifyUniformFinal(groups, r, s, {x, y, z});
        Groups mapped(h);
        for (int residue = 0; residue < h; ++residue) {
            int target = (candidate.map.multiplier * residue + candidate.map.shift) & (h - 1);
            mapped[target] = move(groups[residue]);
        }
        groups = move(mapped);

        array<int, 3> color;
        for (int c = 0; c < 3; ++c) color[candidate.label[c]] = c;
        int targetR = candidate.map.shift;
        int targetS = (candidate.map.multiplier * s + candidate.map.shift) & (h - 1);
        assignColor(x, targetR, color[0]);
        assignColor(y, targetR, color[1]);
        assignColor(z, targetS, color[2]);
    }

    void prepareNonuniform() {
        if (n < 2 * h) {
            for (int r = 0; r < h; ++r) {
                if (groups[r].size() == 1) assign(groups[r][0], firstValue(r, h));
            }
            for (int c = 0; c < 3; ++c) {
                if (anchors[c] != -1) continue;
                for (int r = 0; r < h; ++r) {
                    if (groups[r].size() != 2 || missingColor(r, h) == c) continue;
                    int position = groups[r][hasColor(groups[r][0], c) ? 0 : 1];
                    assignColor(position, r, c);
                    break;
                }
            }
            return;
        }

        int r = -1, s = -1;
        for (int i = 0; i < h; ++i) {
            if (groups[i].size() != 2) continue;
            if (r == -1) r = i;
            else if (missingColor(r, h) != missingColor(i, h)) {
                s = i;
                break;
            }
        }
        if (s == -1) exit(0);
        int c0 = missingColor(r, h), c1 = missingColor(s, h);
        int shared = 3 - c0 - c1;
        if (askTriple(groups[r][0], groups[r][1], groups[s][0])) assignPair(s, c0, shared);
        else assignPair(s, shared, c0);
        if (askTriple(groups[s][0], groups[s][1], groups[r][0])) assignPair(r, c1, shared);
        else assignPair(r, shared, c1);
    }

public:
    FinalDecoder(int size, int modulus, const vector<int>& residues)
        : n(size), h(modulus), groups(groupByResidue(residues, h)), answer(n) {}

    vector<int> decode() {
        if (n == 2 * h) prepareUniform();
        else prepareNonuniform();

        for (int r = 0; r < h; ++r) {
            vector<int> available;
            for (int value = firstValue(r, h); value <= n; value += h) available.push_back(value);
            for (int i : groups[r]) {
                if (answer[i]) erase(available, answer[i]);
            }
            for (int i : groups[r]) {
                if (answer[i]) continue;
                int j = 0;
                while (j + 1 < int(available.size()) && !hasColor(i, available[j] % 3)) ++j;
                assign(i, available[j]);
                available.erase(available.begin() + j);
            }
        }
        return answer;
    }
};

vector<int> reconstruct(int n) {
    if (n == 2) return {1, 2};
    if (n == 6) {
        static const SmallDecoder decoder(6);
        return decoder.decode();
    }
    if (n == 8) {
        static const SmallDecoder decoder(8);
        return decoder.decode();
    }
    vector<int> residues = parityResidues(n);
    int modulus = 2;
    while (3 * modulus <= n) {
        lift(residues, modulus);
        modulus *= 2;
    }
    return FinalDecoder(n, modulus, residues).decode();
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n) || n < 2) return 0;
    vector<int> answer = reconstruct(n);
    if (answer[0] > n / 2) {
        for (int& value : answer) value = n + 1 - value;
    }
    cout << '!';
    for (int value : answer) cout << ' ' << value;
    cout << '\n' << flush;
    return 0;
}
