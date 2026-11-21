#include <bits/stdc++.h>
using namespace std;

namespace {
constexpr int cap = 15;

// A set of deleted edges preserves connectivity exactly when it contains
// no bridge and takes at most one edge from each cycle. Each row stores
// such a set and the query IDs of all its prefixes.
struct Row {
    vector<int> edges;
    vector<int> prefix{0};

    int size() const { return (int)edges.size(); }
    void append(int e, int id) {
        edges.push_back(e);
        prefix.push_back(id);
    }
};

Row rows[cap + 1];
int m, calls;
vector<int> single, answer, before, upperBound, knownRep, fixedLength;
unordered_map<unsigned long long, int> cache;

// Return the new query ID on success and zero on disconnection.
int ask(int parent, int edge) {
    if (single[edge] == 0) return 0;
    if (parent == 0 && single[edge] >= 0) return single[edge];
    auto key = (static_cast<unsigned long long>(parent) << 14) | edge;
    auto it = cache.find(key);
    if (it != cache.end()) return it->second;
    cout << "? " << parent << ' ' << edge << endl;
    int result;
    if (!(cin >> result) || (result != 0 && result != 1)) exit(0);
    ++calls;
    int id = result ? calls : 0;
    cache.emplace(key, id);
    if (parent == 0) single[edge] = id;
    return id;
}

// An alphabetic decision tree tests thresholds in an ordered distribution.
pair<double, int> decisionTree(const vector<double>& w, int lo, int hi) {
    if (hi <= lo) return {0, lo};
    if (hi == lo + 1) return {w[lo] + w[hi], lo};
    double dp[17][17]{};
    int split = lo;
    for (int l = hi; l >= lo; --l) {
        double sum = w[l];
        for (int r = l + 1; r <= hi; ++r) {
            sum += w[r];
            double best = 1e100;
            for (int k = l; k < r; ++k) {
                double cost = dp[l][k] + dp[k + 1][r];
                if (cost < best) {
                    best = cost;
                    if (l == lo && r == hi) split = k;
                }
            }
            dp[l][r] = sum + best;
        }
    }
    return {dp[lo][hi], split};
}

vector<int> firstRow() {
    Row& r = rows[1];
    vector<int> rest;
    double bridgeRate = 0.12, newRate = 0.65;
    int failures = 0, lastBridge = -100;
    for (int e = 1; e <= m; ++e) {
        before[e] = upperBound[e] = r.size();
        bool trySingle = r.size() && bridgeRate > newRate + 0.04;
        int id = 0;
        if (!trySingle || ask(0, e))
            id = ask(r.prefix.back(), e);

        bridgeRate *= 0.88;
        newRate *= 0.88;
        if (id) {
            knownRep[e] = r.size() + 1;
            r.append(e, id);
            newRate += 0.12;
            continue;
        }

        ++failures;
        // Occasionally identify bridges early to choose the first query in
        // bridge-heavy portions of the input.
        if (failures <= 2 || e - lastBridge <= 3 || failures % 73 == 0)
            ask(0, e);
        if (single[e] == 0) {
            answer[e] = -1;
            lastBridge = e;
            bridgeRate += 0.12;
        } else {
            rest.push_back(e);
        }
    }
    return rest;
}

// Query the prefix immediately before the newest possible representative.
// A positive response identifies the cycle as well as ruling out a bridge.
bool testNewest(int e) {
    int hi = upperBound[e];
    if (!hi) return false;
    if ((hi == 1 && single[e] > 0) || ask(rows[1].prefix[hi - 1], e)) {
        knownRep[e] = hi;
        return true;
    }
    upperBound[e] = hi - 1;
    return false;
}

bool useDirect(const vector<int>& rest) {
    if (rest.size() <= 2 || rows[1].size() <= 3) return true;
    int samples = min(18, max(1, (int)sqrt(rest.size()) / 3));
    int hits = 0;
    for (int j = 0; j < samples; ++j) {
        int pos = (int)((2LL * j + 1) * rest.size() / (2 * samples));
        hits += testNewest(rest[pos]);
    }
    return hits == samples || (samples >= 8 && hits >= 0.9 * samples);
}

void solveDirect(const vector<int>& rest) {
    int r = rows[1].size();
    vector<int> count(r + 1, 1);
    set<int> small;
    for (int i = 1; i <= r; ++i) small.insert(i);
    int filterSize = -1, complement = 0;
    double local = 0.8;
    for (int e : rest) {
        int lo = single[e] > 0 ? 1 : 0, hi = upperBound[e];
        if (knownRep[e]) lo = hi = knownRep[e];
        if (lo < hi && r <= 3 && !small.empty() && (int)small.size() < r) {
            if (filterSize != (int)small.size()) {
                complement = 0;
                for (int rep : small)
                    complement = ask(complement, rows[1].edges[rep - 1]);
                filterSize = (int)small.size();
            }
            if (ask(complement, e)) {
                answer[e] = 0;
                continue;
            }
            vector<int> options;
            if (lo == 0) options.push_back(0);
            for (int rep : small) {
                if (lo <= rep && rep <= hi) options.push_back(rep);
            }
            int left = 0, right = (int)options.size() - 1;
            while (left < right) {
                int k = (left + right) / 2;
                if (ask(rows[1].prefix[options[k]], e)) left = k + 1;
                else right = k;
            }
            lo = hi = options[left];
        }
        while (lo < hi) {
            auto it = small.lower_bound(max(1, lo));
            if (it == small.end() || *it > hi) {
                answer[e] = (lo || ask(0, e)) ? 0 : -1;
                break;
            }
            int mid = lo + (hi - lo) / 2;
            if (hi == before[e] && local * log2(hi - lo + 1.0) > 1.0)
                mid = hi - 1;
            if (ask(rows[1].prefix[mid], e)) lo = mid + 1;
            else hi = mid;
        }
        if (lo != hi) continue;
        knownRep[e] = lo;
        local = 0.93 * local + 0.07 * (lo == before[e]);
        if (!lo) answer[e] = -1;
        else if (++count[lo] == cap) small.erase(lo);
    }
    for (int e = 1; e <= m; ++e) {
        if (knownRep[e]) {
            int len = count[knownRep[e]];
            answer[e] = len >= cap ? 0 : len;
        }
    }
}

// If a few large cycles dominate the remaining input, a single successful
// deletion against all the other representatives identifies any of them.
struct LargeFilter {
    int prefix = 0, covered = 0;

    void build() {
        const auto& base = rows[1];
        const auto& big = rows[cap];
        int n = base.size();
        covered = big.size();
        vector<bool> large(n + 1, false);
        if (covered * log2(n + 1.0) < n) {
            for (int e : big.edges) {
                int lo = 1, hi = upperBound[e];
                if (knownRep[e]) lo = hi = knownRep[e];
                while (lo < hi) {
                    int mid = (lo + hi) / 2;
                    if (ask(base.prefix[mid], e)) lo = mid + 1;
                    else hi = mid;
                }
                large[lo] = true;
            }
        } else {
            int need = covered;
            for (int t = 1; t <= n && need; ++t) {
                if (need == n - t + 1 || !ask(big.prefix.back(), base.edges[t - 1])) {
                    large[t] = true;
                    --need;
                }
            }
        }
        for (int t = 1; t <= n; ++t) {
            if (large[t]) fixedLength[t] = cap;
            else prefix = ask(prefix, base.edges[t - 1]);
        }
    }
};

// Row j receives the j-th edge encountered on each cycle. Its colors form
// a subset of row j-1's colors. The first row accepting an edge is therefore
// a threshold, and adjacent equal cardinalities rule out an entire outcome.
void buildRows(const vector<int>& rest) {
    int top = 1;
    // Outcomes 2..15 are positions, 16 is overflow, 17 is a bridge.
    array<double, 18> recent{};
    LargeFilter filter;
    int processed = 0, lastGrowth = 0;
    for (int e : rest) {
        ++processed;
        int rank = rows[1].size(), big = rows[cap].size();
        if (big && before[e] == big) {
            answer[e] = (knownRep[e] || ask(0, e)) ? 0 : -1;
            continue;
        }
        int wait = big * 2 < rank ? 16 : clamp(2 * rank, 32, 250);
        if (!filter.covered && big && recent[16] > 0.66 &&
            processed - lastGrowth > wait) {
            double setup = rank - big + min<double>(rank, big * log2(rank + 1.0));
            double saving = ((int)rest.size() - processed) * (2 * recent[16] - 1);
            if (saving > 1.45 * setup + 24)
                filter.build();
        }
        if (!upperBound[e]) {
            answer[e] = -1;
            continue;
        }
        if (filter.covered && ask(filter.prefix, e)) {
            answer[e] = 0;
            continue;
        }
        vector<int> value, gap;
        int last = before[e];
        for (int j = 2; j <= min(cap, top + 1); ++j) {
            int now = rows[j].size();
            if (last > now) {
                value.push_back(j);
                gap.push_back(last - now);
            }
            last = now;
        }
        if (top == cap && last > filter.covered) {
            value.push_back(16);
            gap.push_back(last - filter.covered);
        }

        bool nonbridge = single[e] > 0 || knownRep[e] > 0;
        if (!nonbridge) {
            value.push_back(17);
            gap.push_back(0);
        }
        vector<double> w;
        for (int i = 0; i < (int)value.size(); ++i) {
            double prior = (value[i] == 17) ? 0.07 :
                           (double)gap[i] / max(1, before[e]);
            w.push_back(0.23 * prior + recent[value[i]] + 0.0001);
        }
        int lo = 0, hi = (int)value.size() - 1;
        while (lo < hi) {
            int k = decisionTree(w, lo, hi).second, j = value[k];
            int p = j == 16 ? 0 : rows[j].prefix.back();
            if (ask(p, e)) hi = k;
            else lo = k + 1;
        }
        int j = value[lo];
        if (j <= cap) {
            int id = ask(rows[j].prefix.back(), e);
            rows[j].append(e, id);
            if (j == cap) lastGrowth = processed;
            top = max(top, j);
        } else {
            answer[e] = j == 16 ? 0 : -1;
        }
        for (double& a : recent) a *= 0.94;
        recent[j] += 0.06;
    }
}

// Fenwick tree of unmatched positions for inverse lookup in row prefixes.
struct Positions {
    vector<int> bit;
    explicit Positions(int n) : bit(n + 1) {
        for (int i = 1; i <= n; ++i) bit[i] = i & -i;
    }
    void remove(int pos) {
        for (int i = pos; i < (int)bit.size(); i += i & -i) --bit[i];
    }
    int sum(int pos) const {
        int n = 0;
        for (int i = pos; i; i -= i & -i) n += bit[i];
        return n;
    }
    int at(int rank) const {
        int pos = 0;
        for (int step = bit_floor((unsigned)bit.size()); step; step >>= 1) {
            int p = pos + step;
            if (p < (int)bit.size() && bit[p] < rank) {
                pos = p;
                rank -= bit[p];
            }
        }
        return pos + 1;
    }
};

// Internal labels use 15 for all large cycles until the rows are decoded.
void decode(int i) {
    const Row& a = rows[i];
    const Row& b = rows[i + 1];
    array<int, cap + 1> freq{};
    freq[i] = a.size() - b.size();
    for (int e : b.edges) ++freq[answer[e]];

    // For rare lengths, looking up their representatives in a prefix tree
    // can save a membership test on every member of a much larger row.
    vector<int> groups;
    for (int j = i + 1; j <= cap; ++j)
        if (freq[j]) groups.push_back(j);
    sort(groups.begin(), groups.end(), [&](int x, int y) {
        return freq[x] < freq[y];
    });
    array<bool, cap + 1> inverse{};
    double best = 1e100, inverseCost = 0;
    int cut = 0, left = a.size();
    for (int n = 0; n <= (int)groups.size(); ++n) {
        vector<double> weights;
        for (int j = i; j <= cap; ++j) {
            if (freq[j] && !inverse[j]) weights.push_back(freq[j]);
        }
        double cost = inverseCost + 0.77 * decisionTree(weights, 0, (int)weights.size() - 1).first;
        if (cost < best) {
            best = cost;
            cut = n;
        }
        if (n < (int)groups.size()) {
            int g = groups[n];
            inverse[g] = true;
            inverseCost += freq[g] * max(1.0, log2(max(2, left)) - 0.7);
            left -= freq[g];
        }
    }
    inverse.fill(false);
    for (int n = 0; n < cut; ++n) inverse[groups[n]] = true;

    if (cut) {
        Positions remaining(a.size());
        double local = 0.23;
        for (int e : b.edges) {
            int g = answer[e];
            if (!inverse[g]) continue;
            int end = (int)(lower_bound(a.edges.begin(), a.edges.end(), e) - a.edges.begin());
            if (i == 1) end = min(end, upperBound[e]);
            int hi = remaining.sum(end), lo = 1, maxRank = hi;
            if (i == 1 && knownRep[e]) lo = hi = remaining.sum(knownRep[e]);
            while (lo < hi) {
                int mid = lo + (hi - lo) / 2;
                if (hi == maxRank && local * log2(hi - lo + 1.0) > 1)
                    mid = hi - 1;
                if (ask(a.prefix[remaining.at(mid)], e)) lo = mid + 1;
                else hi = mid;
            }
            int pos = remaining.at(lo);
            answer[a.edges[pos - 1]] = g;
            remaining.remove(pos);
            local = 0.93 * local + 0.07 * (lo == maxRank);
        }
    }

    // An upper-row representative always follows its lower-row partner.
    // Going backwards bounds each length by the supply still available.
    array<int, cap + 1> available{};
    available[i] = freq[i];
    int t = b.size() - 1;
    for (int pos = a.size() - 1; pos >= 0; --pos) {
        int e = a.edges[pos];
        while (t >= 0 && b.edges[t] > e) {
            int g = answer[b.edges[t--]];
            if (!inverse[g]) ++available[g];
        }
        if (answer[e] > 0) continue;
        if (i == 1 && fixedLength[pos + 1]) {
            int g = fixedLength[pos + 1];
            answer[e] = g;
            --available[g];
            continue;
        }
        vector<int> value;
        vector<double> w;
        for (int j = i; j <= cap; ++j) {
            if (available[j]) {
                value.push_back(j);
                w.push_back(available[j]);
            }
        }
        int lo = 0, hi = (int)value.size() - 1;
        while (lo < hi) {
            int k = decisionTree(w, lo, hi).second;
            int p = rows[value[k + 1]].prefix.back();
            if (ask(p, e)) hi = k;
            else lo = k + 1;
        }
        int g = value[lo];
        answer[e] = g;
        --available[g];
    }
}

void solve() {
    vector<int> rest = firstRow();
    if (useDirect(rest)) {
        solveDirect(rest);
        return;
    }
    fixedLength.assign(rows[1].size() + 1, 0);
    buildRows(rest);
    for (int e : rows[cap].edges) answer[e] = cap;
    for (int i = cap - 1; i >= 2; --i) decode(i);
    for (int e : rest) {
        if (knownRep[e])
            fixedLength[knownRep[e]] = answer[e] == 0 ? cap : answer[e];
    }
    decode(1);
    for (int& a : answer) if (a == cap) a = 0;
}
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    if (!(cin >> m) || m < 1 || m > 10000) return 0;
    single.assign(m + 1, -1);
    answer.assign(m + 1, -2);
    before.resize(m + 1);
    upperBound.resize(m + 1);
    knownRep.resize(m + 1);
    cache.reserve(8 * m);
    solve();
    cout << '!';
    for (int e = 1; e <= m; ++e) cout << ' ' << answer[e];
    cout << endl;
    int confirmation;
    if (!(cin >> confirmation) || confirmation != 1) return 0;
    return 0;
}
