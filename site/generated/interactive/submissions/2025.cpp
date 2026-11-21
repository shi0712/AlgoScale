#include <algorithm>
#include <array>
#include <bit>
#include <bitset>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

using namespace std;

class Endgame {
    using Set = bitset<384>;
    struct Cost {
        int total;
        int worst;
        int query;
    };

    array<array<Set, 7>, 64> branches{};
    unordered_map<Set, Cost> memo;
    Set possible;

    Cost search(const Set& states, int count) {
        if (count < 2) return {0, 0, -1};
        if (auto it = memo.find(states); it != memo.end()) return it->second;
        Cost best{1000000, 1000000, -1};
        for (int q = 0; q < (1 << size); ++q) {
            Cost current{count, 1, q};
            for (int r = 0; r <= size; ++r) {
                Set child = states & branches[q][r];
                int childCount = child.count();
                if (childCount == count) {
                    current.total = 1000000;
                    break;
                }
                Cost cost = search(child, childCount);
                current.total += cost.total;
                current.worst = max(current.worst, cost.worst + 1);
                if (current.total > best.total) break;
            }
            if (current.total < best.total ||
                (current.total == best.total && current.worst < best.worst)) best = current;
        }
        return memo.emplace(states, best).first->second;
    }

public:
    struct Layout {
        array<int, 6> controller{};
        int signs;
    };

    int size;
    array<int, 6> switches{}, doors{};
    vector<Layout> layouts;
    int asked = 0;

    explicit Endgame(int count) : size(count) {}

    void add(const Layout& layout) {
        int index = layouts.size();
        possible.set(index);
        layouts.push_back(layout);
        for (int q = 0; q < (1 << size); ++q) {
            int r = 0;
            while (r < size && !((q ^ layout.signs) & (1 << layout.controller[r]))) ++r;
            branches[q][r].set(index);
        }
    }

    int next() {
        asked = search(possible, possible.count()).query;
        return asked;
    }

    const Layout& result() const {
        for (size_t i = 0; i < layouts.size(); ++i) {
            if (possible[i]) return layouts[i];
        }
        return layouts.front();
    }

    bool observe(int reply, int n) {
        int r = 0;
        if (reply == n) r = size;
        else {
            while (r < size && doors[r] != reply) ++r;
            if (r == size) return false;
        }
        possible &= branches[asked][r];
        return possible.any();
    }
};

class CaveSolver {
    using Word = uint64_t;
    struct Pool {
        int door;
        int choices;
        int fresh;
        bool nested;
        bool tight;
    };

    int n, words;
    int depth = -1;
    int remaining, queries = 0, ones = 0, lastGuess = 0;
    int mappingProbe = -1;
    int oldest = -1, queriesAtOldest = 0;
    vector<Word> possible, pools;
    vector<Word> baseline, trial, unused, covered, reserved, changed;
    vector<int> active;
    vector<int> mappingCounts;
    vector<Pool> groups;
    vector<int> flipsForGroup;
    vector<float> reciprocal;
    float entropy[4097];
    vector<unsigned char> hit;
    uint64_t randomState;
    unique_ptr<Endgame> endgame;

    Word randomWord() {
        randomState += 0x9e3779b97f4a7c15ULL;
        Word z = randomState;
        z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
        z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
        return z ^ (z >> 31);
    }

    unsigned randomBelow(unsigned limit) {
        return (uint64_t(uint32_t(randomWord())) * limit) >> 32;
    }

    Word* candidates(int door) { return possible.data() + size_t(door) * words; }
    Word* poolBits(int door) { return pools.data() + size_t(door) * words; }

    int firstBit(const Word* bits, bool filterUnused = false) const {
        for (int j = 0; j < words; ++j) {
            Word word = filterUnused ? bits[j] & unused[j] : bits[j];
            if (word) return 64 * j + countr_zero(word);
        }
        return -1;
    }

    void assign(int door, int sw) {
        Word bit = Word(1) << (sw % 64);
        bool value = bool(baseline[sw / 64] & bit) ^ (door == depth);
        positions[sw] = char('0' + value);
        wiring[sw] = door;
        ones += value;
        if (value) baseline[sw / 64] |= bit;
        else baseline[sw / 64] &= ~bit;
        unused[sw / 64] &= ~bit;
        if (!mappingCounts.empty()) {
            for (int d : active) mappingCounts[d] -= bool(candidates(d)[sw / 64] & bit);
        }
        --remaining;
    }

    // A door that has been closed below the deepest reply has a disjoint
    // candidate set from all earlier doors. Its closing trial disagrees with
    // the baseline there, and agrees on all earlier doors. Every other set
    // contains the earlier sets because it has only agreement constraints.
    void findPools() {
        fill(covered.begin(), covered.end(), 0);
        fill(reserved.begin(), reserved.end(), 0);
        groups.clear();
        int pending = 0;
        size_t keep = 0;
        for (int door : active) {
            Word* candidatesHere = candidates(door);
            Word* newHere = poolBits(door);
            int count = 0;
            int fresh = 0;
            int example = -1;
            for (int j = 0; j < words; ++j) {
                Word eligible = candidatesHere[j] & unused[j] & ~reserved[j];
                Word newBits = eligible & ~covered[j];
                newHere[j] = newBits;
                count += popcount(eligible);
                fresh += popcount(newBits);
                if (eligible) example = 64 * j + countr_zero(eligible);
            }
            if (count == 1) {
                assign(door, example);
                continue;
            }
            bool nested = !hit[door] || door == depth;
            int choices = count - (nested ? pending : 0);
            bool tight = nested && choices == 1;
            // A tight set will be used up by this door and the preceding
            // doors it contains. Its only fresh switch, if any, must be here.
            if (tight && fresh == 1) {
                assign(door, firstBit(newHere));
                for (int j = 0; j < words; ++j) reserved[j] |= candidatesHere[j] & unused[j];
                groups.back().tight = true;
                pending = 0;
                continue;
            }
            if (tight) {
                for (int j = 0; j < words; ++j) {
                    reserved[j] |= candidatesHere[j] & unused[j];
                }
                pending = 0;
            } else {
                for (int j = 0; j < words; ++j) covered[j] |= newHere[j];
                ++pending;
            }
            groups.push_back({door, choices, fresh, nested, tight});
            active[keep++] = door;
        }
        active.resize(keep);
    }

    bool smallEndgame() const {
        if (remaining <= 4) return true;
        if (remaining > 6) return false;
        int count = 1;
        for (int k = 1; k <= n - depth - 1; ++k) count *= 2 * k;
        for (const Pool& group : groups) count *= group.choices;
        return count <= 128;
    }

    void startEndgame() {
        endgame = make_unique<Endgame>(remaining);
        int index = 0;
        for (int sw = 0; sw < n; ++sw) {
            if (wiring[sw] == -1) endgame->switches[index++] = sw;
        }
        index = 0;
        for (int d : active) endgame->doors[index++] = d;
        for (int d = depth + 1; d < n; ++d) endgame->doors[index++] = d;

        array<int, 6> order{0, 1, 2, 3, 4, 5};
        do {
            int fixed = 0, values = 0;
            bool valid = true;
            for (int i = 0; i < remaining && endgame->doors[i] <= depth; ++i) {
                int d = endgame->doors[i], sw = endgame->switches[order[i]];
                Word bit = Word(1) << (sw % 64);
                if (!(candidates(d)[sw / 64] & bit)) {
                    valid = false;
                    break;
                }
                fixed |= 1 << order[i];
                bool openBit = bool(baseline[sw / 64] & bit) ^ (d == depth);
                values |= int(openBit) << order[i];
            }
            if (valid) {
                for (int signs = 0; signs < (1 << remaining); ++signs) {
                    if ((signs & fixed) == values) endgame->add({order, signs});
                }
            }
        } while (next_permutation(order.begin(), order.begin() + remaining));
    }

    bool endgameTrial() {
        int choice = endgame->next();
        if (choice < 0) {
            const auto& layout = endgame->result();
            for (int i = 0; i < remaining; ++i) {
                int sw = endgame->switches[layout.controller[i]];
                positions[sw] = char('0' + ((layout.signs >> layout.controller[i]) & 1));
                wiring[sw] = endgame->doors[i];
            }
            return false;
        }
        trial = baseline;
        for (int i = 0; i < remaining; ++i) {
            int sw = endgame->switches[i];
            Word bit = Word(1) << (sw % 64);
            if (choice & (1 << i)) trial[sw / 64] |= bit;
            else trial[sw / 64] &= ~bit;
        }
        return true;
    }

    void setFlips(const Pool& group, int flipCount) {
        const Word* bits = poolBits(group.door);
        int left = group.fresh;
        for (int j = 0; j < words && left; ++j) {
            Word take = 0;
            Word undecided = bits[j];
            while (undecided && flipCount && flipCount < left) {
                Word bit = undecided & -undecided;
                undecided ^= bit;
                if (randomBelow(left) < unsigned(flipCount)) {
                    take |= bit;
                    --flipCount;
                }
                --left;
            }
            int rest = popcount(undecided);
            if (flipCount == left) {
                take |= undecided;
                flipCount -= rest;
            }
            left -= rest;
            changed[j] = (changed[j] & ~bits[j]) | take;
        }
    }

    // Earlier open doors consume unflipped candidates. A nested set contains
    // every such door in its block, so its number of available flips stays the same.
    float information(int first, int inherited, float tail, bool frontier) const {
        float survive = 1, benefit = 0;
        for (int i = first; i < int(groups.size()); ++i) {
            const Pool& group = groups[i];
            int turned = flipsForGroup[i] + (group.nested ? inherited : 0);
            inherited = group.tight ? 0 : inherited + flipsForGroup[i];
            if (frontier && group.door == depth) turned = group.choices - turned;
            if (turned <= 0) continue;
            if (turned >= group.choices) return benefit;
            float p = turned * reciprocal[group.choices];
            benefit += survive * entropy[int(4096 * p + 0.5f)];
            survive *= 1 - p;
        }
        return benefit + survive * tail;
    }

    void improve(bool frontier) {
        int count = groups.size();
        if (!count) return;
        int unknown = n - depth - 1;
        float tail = unknown > 20 ? 2 : unknown > 0 ? 2 - ldexpf(2, -unknown) : 0;
        // Moving the number of flips between pools changes the distribution
        // of the first closed door. Optimize it against the whole suffix.
        for (int sweep = 0; sweep < 2; ++sweep) {
            for (int offset = 0; offset < count; ++offset) {
                int i = sweep % 2 ? offset : count - 1 - offset;
                const Pool& group = groups[i];
                if (!group.fresh) continue;
                int inherited = 0;
                for (int j = i; j-- > 0 && !groups[j].tight;) inherited += flipsForGroup[j];
                int& choice = flipsForGroup[i];
                int best = choice;
                float bestScore = information(i, inherited, tail, frontier);
                auto evaluate = [&](int amount) {
                    choice = amount;
                    float score = information(i, inherited, tail, frontier);
                    if (score > bestScore + 0.000001f) {
                        bestScore = score;
                        best = amount;
                    }
                    return score;
                };
                int lo = 0, hi = group.fresh;
                while (hi - lo > 5) {
                    int m1 = (2 * lo + hi) / 3;
                    int m2 = (lo + 2 * hi) / 3;
                    float score1 = evaluate(m1);
                    float score2 = evaluate(m2);
                    if (score1 < score2) lo = m1 + 1;
                    else hi = m2 - 1;
                }
                for (int j = lo; j <= hi; ++j) evaluate(j);
                choice = best;
            }
        }
    }

    int majorityGuess() const {
        int fixed = n - remaining;
        int imbalance = 2 * ones - fixed;
        if ((imbalance >= 12 || imbalance <= -12) && imbalance * imbalance >= 10 * fixed) {
            return int(imbalance > 0);
        }
        return -1;
    }

    bool guessTrial(int guess, bool frontier) {
        if (guess < 0 || n - remaining - lastGuess < 12) return false;
        lastGuess = n - remaining;
        flipsForGroup.clear();
        for (int j = 0; j < words; ++j) {
            changed[j] = (baseline[j] ^ (guess ? ~Word(0) : 0)) & unused[j];
        }
        for (const Pool& group : groups) {
            int flips = 0;
            const Word* bits = poolBits(group.door);
            for (int j = 0; j < words; ++j) flips += popcount(changed[j] & bits[j]);
            flipsForGroup.push_back(flips);
        }
        if (information(0, 0, depth < n - 1 ? 1 : 0, frontier) == 0) return false;
        for (int j = 0; j < words; ++j) trial[j] = baseline[j] ^ changed[j];
        return true;
    }

    void searchTrial() {
        bool frontier = depth >= 0 && !active.empty() && active.back() == depth;
        if (queries == 0 || (queries == 1 && depth == 0)) {
            for (int j = 0; j < words; ++j) trial[j] = queries ? unused[j] : 0;
            return;
        }

        int guess = majorityGuess();
        if (guessTrial(guess, frontier)) return;
        bool splitBudget = queries >= 6 * n + 100;
        if (groups.size() > 128 && !splitBudget) {
            const Pool& first = groups.front();
            int ordinal = randomBelow(first.fresh);
            const Word* bits = poolBits(first.door);
            trial = baseline;
            for (int j = 0; j < words; ++j) {
                Word word = bits[j];
                int count = popcount(word);
                if (ordinal >= count) {
                    ordinal -= count;
                    continue;
                }
                for (; ordinal > 0; --ordinal) word &= word - 1;
                trial[j] ^= word & -word;
                break;
            }
            return;
        }
        for (int j = 0; j < words; ++j) {
            Word target = guess < 0 ? randomWord() : guess ? ~Word(0) : 0;
            changed[j] = (baseline[j] ^ target) & unused[j];
        }

        flipsForGroup.clear();
        int inherited = 0;
        int early = int(groups.size()) - int(frontier);
        for (const Pool& group : groups) {
            int previous = group.nested ? inherited : 0;
            int desired = 0;
            if (frontier && group.door == depth) {
                desired = max(1, (3 * group.choices + 2) / 4);
            } else {
                --early;
                desired = (group.choices + (early + 3) / 2) / (early + 3);
            }
            int flips = clamp(desired - previous, 0, group.fresh);
            flipsForGroup.push_back(flips);
            inherited = group.tight ? 0 : inherited + flips;
        }

        int first = active.empty() ? -1 : active.front();
        if (first != oldest) {
            oldest = first;
            queriesAtOldest = 0;
        }
        ++queriesAtOldest;
        bool forceSplit = first >= 0 && (queriesAtOldest >= 40 || splitBudget);
        if (!forceSplit && groups.size() <= 80) improve(frontier);
        if (forceSplit || information(0, 0, depth < n - 1 ? 1 : 0, frontier) == 0) {
            flipsForGroup.front() = groups.front().choices / 2;
        }

        for (size_t i = 0; i < groups.size(); ++i) setFlips(groups[i], flipsForGroup[i]);
        for (int j = 0; j < words; ++j) trial[j] = baseline[j] ^ changed[j];
    }

    int largeMappingPool() {
        if (mappingCounts.empty()) {
            mappingCounts.resize(n);
            for (int door : active) {
                const Word* bits = candidates(door);
                for (int j = 0; j < words; ++j) mappingCounts[door] += popcount(bits[j] & unused[j]);
            }
        }
        while (active.size() > 128) {
            auto it = min_element(active.begin(), active.end(), [&](int a, int b) {
                return mappingCounts[a] < mappingCounts[b];
            });
            int door = *it;
            int sw = firstBit(candidates(door), true);
            if (mappingCounts[door] > 1) return sw;
            assign(door, sw);
            active.erase(it);
        }
        return -1;
    }

    void mappingTrial(int sw) {
        mappingProbe = sw;
        trial = baseline;
        trial[sw / 64] ^= Word(1) << (sw % 64);
    }

public:
    string positions;
    vector<int> wiring;

    CaveSolver(int size, uint64_t seed)
        : n(size), words((n + 63) / 64), remaining(n), possible(size_t(n) * words),
          pools(size_t(n) * words), baseline(words), trial(words), unused(words, ~Word(0)),
          covered(words), reserved(words), changed(words), reciprocal(n + 1), hit(n), randomState(seed),
          positions(n, '0'), wiring(n, -1) {
        if (n % 64) unused.back() = (Word(1) << (n % 64)) - 1;
        active.reserve(n);
        groups.reserve(n);
        flipsForGroup.reserve(n);
        for (int i = 1; i <= n; ++i) reciprocal[i] = 1.0f / i;
        for (int i = 1; i < 4096; ++i) {
            float p = i / 4096.0f;
            entropy[i] = -p * log2f(p) - (1 - p) * log2f(1 - p);
        }
        entropy[0] = entropy[4096] = 0;
    }

    bool next(string& query) {
        mappingProbe = -1;
        if (depth >= n - 1 && active.size() > 128) {
            int sw = largeMappingPool();
            if (sw >= 0) mappingTrial(sw);
        }
        if (mappingProbe < 0) {
            if (!endgame) {
                findPools();
                if (!remaining) return false;
                if (smallEndgame()) startEndgame();
            }
            if (endgame) {
                if (!endgameTrial()) return false;
            } else if (depth >= n - 1) {
                const Pool* best = nullptr;
                for (const Pool& group : groups) {
                    if (group.fresh && (!best || group.choices < best->choices)) best = &group;
                }
                mappingTrial(firstBit(poolBits(best->door)));
            } else {
                searchTrial();
            }
        }
        ++queries;
        for (int i = 0; i < n; ++i) query[i] = char('0' + ((trial[i / 64] >> (i % 64)) & 1));
        return true;
    }

    bool observe(int reply) {
        if (reply == -1) reply = n;
        else if (reply < 0 || reply >= n) return false;
        if (endgame) return endgame->observe(reply, n);
        if (mappingProbe >= 0) {
            // Once every door has been reached, a single flip identifies
            // its controller's door, including the last door if it opens.
            if (reply >= depth) {
                if (reply != n || depth != n - 1) return false;
                reply = depth;
            }
            if (wiringAlreadyKnown(reply)) return false;
            assign(reply, mappingProbe);
            active.erase(lower_bound(active.begin(), active.end(), reply));
            return true;
        }
        if (reply < n && wiringAlreadyKnown(reply)) return false;
        for (int j = 0; j < words; ++j) changed[j] = baseline[j] ^ trial[j];
        // Candidates for earlier doors agree with the deepest trial. The
        // deepest closed door has the opposite position on its candidates.
        for (int door : active) {
            if (door > reply) break;
            bool requireFlip = (door == reply) != (door == depth);
            Word* bits = candidates(door);
            for (int j = 0; j < words; ++j) bits[j] &= requireFlip ? changed[j] : ~changed[j];
        }
        if (reply < n) hit[reply] = true;
        if (reply > depth) {
            for (int door = depth + 1; door <= reply && door < n; ++door) {
                copy(unused.begin(), unused.end(), candidates(door));
                active.push_back(door);
            }
            baseline = trial;
            depth = reply;
        }
        return true;
    }

private:
    bool wiringAlreadyKnown(int door) const {
        return door <= depth && !binary_search(active.begin(), active.end(), door);
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    int n;
    if (!(cin >> n) || n < 1 || n > 5000) return 0;
    CaveSolver solver(n, chrono::steady_clock::now().time_since_epoch().count());
    string query(n, '0');
    while (solver.next(query)) {
        cout << "? " << query << '\n' << flush;
        int reply;
        if (!(cin >> reply) || !solver.observe(reply)) return 0;
    }
    cout << "! " << solver.positions;
    for (int door : solver.wiring) cout << ' ' << door;
    cout << '\n' << flush;
    return 0;
}
