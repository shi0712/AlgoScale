#include <bits/stdc++.h>
using namespace std;

constexpr int MAX_N = 2000;
constexpr int SMALL = 8;
int rankAt[MAX_N];
int tailParity[MAX_N];

// Positions [0,k) are known. Removing their contribution leaves
// the parity of the number of elements in [l,k) above the new element.
int ask(int l, int k) {
    cout << "? " << l + 1 << ' ' << k + 1 << '\n' << flush;
    int answer;
    if (!(cin >> answer) || answer < 0 || answer > 1) exit(0);
    return answer ^ tailParity[l];
}

void applyQuery(int l, int k, vector<int>& candidates) {
    const int m = int(candidates.size());
    if (m < 2) return;

    if (l == k - 1) {
        auto mid = upper_bound(candidates.begin(), candidates.end(), rankAt[l]);
        if (mid == candidates.begin() || mid == candidates.end()) return;
        if (ask(l, k)) candidates.erase(mid, candidates.end());
        else candidates.erase(candidates.begin(), mid);
        return;
    }

    unsigned char prediction[MAX_N];
    int ones = 0;
    if (l == 0) {
        for (int i = 0; i < m; ++i) {
            prediction[i] = (k - candidates[i]) & 1;
            ones += prediction[i];
        }
    } else {
        unsigned char present[MAX_N] = {};
        for (int j = l; j < k; ++j) present[rankAt[j]] = 1;
        int r = k, parity = 0;
        for (int i = m - 1; i >= 0; --i) {
            while (r > candidates[i]) parity ^= present[--r];
            prediction[i] = parity;
            ones += parity;
        }
    }

    if (ones == 0 || ones == m) return;
    int response = ask(l, k), keep = 0;
    for (int i = 0; i < m; ++i)
        if (prediction[i] == response) candidates[keep++] = candidates[i];
    candidates.resize(keep);
}

// Adding an old position of rank r to the queried suffix toggles exactly
// the first cutoff[r] predictions in the ordered candidate list.
void makeCutoffs(int k, const vector<int>& candidates, int* cutoff) {
    const int m = int(candidates.size());
    int j = 0;
    for (int r = 0; r < k; ++r) {
        if (j < m && candidates[j] == r) ++j;
        cutoff[r] = j;
    }
}

constexpr uint64_t lowBits(int t) {
    return t == 64 ? ~uint64_t(0) : (uint64_t(1) << t) - 1;
}

// A tree over 64-bit words. Each count includes its own node's
// complement flag but not any flags of its ancestors.
struct PrefixFlips {
    static constexpr int WORDS = int(bit_ceil(unsigned((MAX_N + 63) / 64)));
    int base;
    uint64_t bits[WORDS] = {};
    int cap[2 * WORDS] = {}, ones[2 * WORDS] = {};
    bool inverted[2 * WORDS] = {};

    explicit PrefixFlips(int m) {
        int words = (m + 63) / 64;
        base = int(bit_ceil(unsigned(words)));
        for (int i = 0; i < words; ++i)
            cap[base + i] = min(64, m - 64 * i);
        for (int v = base - 1; v > 0; --v)
            cap[v] = cap[2 * v] + cap[2 * v + 1];
    }

    void flipNode(int v) {
        inverted[v] = !inverted[v];
        ones[v] = cap[v] - ones[v];
    }

    int flipPrefix(int t) {
        int word = (t - 1) >> 6, b = ((t - 1) & 63) + 1;
        int v = base + word;
        if (b == cap[v]) {
            flipNode(v);
        } else {
            bits[word] ^= lowBits(b);
            int count = popcount(bits[word]);
            ones[v] = inverted[v] ? cap[v] - count : count;
        }
        while (v > 1) {
            if (v & 1) flipNode(v - 1);
            v >>= 1;
            int count = ones[2 * v] + ones[2 * v + 1];
            ones[v] = inverted[v] ? cap[v] - count : count;
        }
        return ones[1];
    }
};

int chooseBalanced(int k, const vector<int>& candidates) {
    int cutoff[MAX_N];
    makeCutoffs(k, candidates, cutoff);

    const int m = int(candidates.size()), goal = m / 2;
    int odd = 0;
    for (int r : candidates) odd += r & 1;
    int best = min(odd, m - odd), bestL = 0;

    // The whole suffix is already an optimal split if rank parity is
    // balanced. Still prefer a nearby equally balanced query if found.
    const int limit = (best == goal ? max(1, k - 48) : 0);
    auto consider = [&](int l, int count) {
        int balance = min(count, m - count);
        if (balance > best || (balance == best && l > bestL)) {
            best = balance;
            bestL = l;
        }
        return balance == goal;
    };

    if (m <= 64) {
        uint64_t mask = 0;
        for (int l = k - 1; l >= limit; --l) {
            int t = cutoff[rankAt[l]];
            if (t == 0 || t == m) continue;
            mask ^= lowBits(t);
            if (consider(l, popcount(mask))) break;
        }
    } else {
        PrefixFlips flips(m);
        for (int l = k - 1; l >= limit; --l) {
            int t = cutoff[rankAt[l]];
            if (t == 0 || t == m) continue;
            if (consider(l, flips.flipPrefix(t))) break;
        }
    }
    return bestL;
}

constexpr auto maskSize = [] {
    array<int, 1 << SMALL> result{};
    for (int s = 1; s < (1 << SMALL); ++s)
        result[s] = result[s >> 1] + (s & 1);
    return result;
}();

struct SmallTree {
    struct Test {
        int mask, l, complement;
    };
    vector<Test> tests;
    int memo[1 << SMALL], choice[1 << SMALL];
    int minDepth[SMALL + 1] = {}, minTotal[SMALL + 1] = {};

    // Minimize total leaf depth, breaking ties by worst-case depth.
    int solve(int state) {
        int size = maskSize[state];
        if (size <= 1) return 0;
        int& best = memo[state];
        if (best != -1) return best;

        best = INT_MAX;
        const int ideal = 16 * minTotal[size] + minDepth[size];
        for (int j = 0; j < int(tests.size()); ++j) {
            int a = state & tests[j].mask;
            if (a == 0 || a == state) continue;
            int b = state ^ a;
            int na = maskSize[a], nb = size - na;
            int lowerBound = 16 * (size + minTotal[na] + minTotal[nb])
                           + 1 + max(minDepth[na], minDepth[nb]);
            if (lowerBound >= best) continue;

            int ca = solve(a), cb = solve(b);
            int cost = 16 * (size + (ca >> 4) + (cb >> 4))
                     + 1 + max(ca & 15, cb & 15);
            if (cost < best) {
                best = cost;
                choice[state] = j;
                if (best == ideal) break;
            }
        }
        return best;
    }

    int identify(int k, const vector<int>& candidates) {
        int m = int(candidates.size());
        int cutoff[MAX_N];
        makeCutoffs(k, candidates, cutoff);
        int full = (1 << m) - 1, mask = 0;
        bool seen[1 << SMALL] = {};
        for (int l = k - 1; l >= 0; --l) {
            mask ^= (1 << cutoff[rankAt[l]]) - 1;
            int canonical = min(mask, mask ^ full);
            if (canonical == 0 || seen[canonical]) continue;
            seen[canonical] = true;
            tests.push_back({canonical, l, int(canonical != mask)});
        }
        sort(tests.begin(), tests.end(), [m](const Test& a, const Test& b) {
            int sa = min(maskSize[a.mask], m - maskSize[a.mask]);
            int sb = min(maskSize[b.mask], m - maskSize[b.mask]);
            if (sa != sb) return sa > sb;
            return a.l > b.l;
        });
        fill(begin(memo), end(memo), -1);
        for (int s = 1; s <= m; ++s) {
            minDepth[s] = int(bit_width(unsigned(s - 1)));
            minTotal[s] = s * (minDepth[s] + 1) - (1 << minDepth[s]);
        }

        // Distinct ranks are separable: the replies at two consecutive
        // left endpoints differ by a single old element's comparison.
        solve(full);
        int state = full;
        while ((state & (state - 1)) != 0) {
            const auto& test = tests[choice[state]];
            int response = ask(test.l, k) ^ test.complement;
            state &= response ? test.mask : full ^ test.mask;
        }
        return candidates[countr_zero(unsigned(state))];
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n) || n < 1 || n > MAX_N) return 0;

    vector<int> candidates;
    candidates.reserve(n);
    int lastExtreme = 0, sameExtreme = 0, alternating = 0;
    int lastStep = 0, adjacentRun = 0;

    for (int k = 1; k < n; ++k) {
        candidates.resize(k + 1);
        iota(candidates.begin(), candidates.end(), 0);
        int previous = rankAt[k - 1];

        // Cheap certificates for strongly suggested runs. Every query
        // still partitions the current candidates, even if the guess fails.
        if (sameExtreme >= 3 || (k >= 2 && sameExtreme == k - 1)) {
            applyQuery(k - 1, k, candidates);
        } else if (alternating >= 4) {
            applyQuery(k - 2, k, candidates);
        } else if (adjacentRun >= 4 && k + 1 > SMALL) {
            int guess = previous + (lastStep > 0);
            applyQuery(k - 1, k, candidates);
            if (int(candidates.size()) > SMALL &&
                binary_search(candidates.begin(), candidates.end(), guess)) {
                int neighborRank = (lastStep > 0 ? guess : guess - 1);
                if (neighborRank >= 0 && neighborRank < k) {
                    int l = 0;
                    while (rankAt[l] != neighborRank) ++l;
                    applyQuery(l + 1, k, candidates);
                    if (int(candidates.size()) > SMALL &&
                        binary_search(candidates.begin(), candidates.end(), guess))
                        applyQuery(l, k, candidates);
                }
            }
        }

        while (int(candidates.size()) > SMALL)
            applyQuery(chooseBalanced(k, candidates), k, candidates);

        int r;
        if (candidates.size() == 1) {
            r = candidates.front();
        } else {
            SmallTree tree;
            r = tree.identify(k, candidates);
        }

        int extreme = (r == 0 ? -1 : (r == k ? 1 : 0));
        sameExtreme = extreme == 0 ? 0 :
                      extreme == lastExtreme ? sameExtreme + 1 : 1;
        alternating = extreme == 0 ? 0 :
                      extreme == -lastExtreme ? alternating + 1 : 1;
        lastExtreme = extreme;

        int step = (r == previous + 1 ? 1 : (r == previous ? -1 : 0));
        adjacentRun = step == 0 ? 0 : step == lastStep ? adjacentRun + 1 : 1;
        lastStep = step;

        int parity = 0;
        for (int l = k - 1; l >= 0; --l) {
            int greater = rankAt[l] >= r;
            parity ^= greater;
            tailParity[l] ^= parity;
            rankAt[l] += greater;
        }
        rankAt[k] = r;
        tailParity[k] = 0;
    }

    cout << '!';
    for (int i = 0; i < n; ++i) cout << ' ' << rankAt[i] + 1;
    cout << '\n' << flush;
    return 0;
}
