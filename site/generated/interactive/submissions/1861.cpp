#include <bits/stdc++.h>
using namespace std;

using U64 = uint64_t;

struct Bits {
    U64 w[8]{};
};

struct Diagonal {
    vector<int> value, column;
    vector<Bits> prefix;
    vector<uint16_t> bucket;
    int leftWord, rightWord;

    int rankLE(int v) const {
        int k = bucket[v >> 8];
        while (k < (int)value.size() && value[k] <= v) ++k;
        return k;
    }
};

struct Snake {
    int len, revision = 1;
    vector<Bits> cells;
    vector<uint8_t> leftWord, rightWord;
    vector<uint16_t> minRank, maxRank;
    vector<int> diagLow, diagHigh, fixedMax, low, high, checked;

    Snake(int len, int d) : len(len), cells(d + 1),
        leftWord(d + 1), rightWord(d + 1),
        minRank(d + 1), maxRank(d + 1),
        diagLow(d + 1), diagHigh(d + 1), fixedMax(d + 1),
        low(d + 1), high(d + 1), checked(d + 1) {}
};

struct Candidate {
    int low, high, id, time;

    bool operator<(const Candidate& b) const {
        if (low != b.low) return low > b.low;
        if (high != b.high) return high > b.high;
        return id < b.id;
    }
};

class Solver {
    int n, m, d, all, words;
    int cutoff, atMost;
    long long work = 55000000;
    vector<Diagonal> diag;
    vector<Snake> snakes;
    vector<int> positionTime, positionColumn, histogram;
    vector<int> minQueue, maxQueue;
    priority_queue<Candidate> candidates;

    static bool contains(const Bits& b, int c) {
        return (b.w[c >> 6] >> (c & 63)) & 1;
    }

    static void shrink(Snake& s, int t) {
        const Bits& b = s.cells[t];
        int l = s.leftWord[t], r = s.rightWord[t];
        while (l < r && !b.w[l]) ++l;
        while (r > l && !b.w[r]) --r;
        s.leftWord[t] = l;
        s.rightWord[t] = r;
    }

    void lowerCutoff() {
        while (cutoff > 1 && atMost - histogram[cutoff] >= m) {
            atMost -= histogram[cutoff];
            --cutoff;
        }
    }

    void refresh(Snake& s, int first, int last) {
        int al = 0, ar = 0, bl = 0, br = 0;
        int begin = max(1, first - s.len + 1);
        for (int t = begin; t <= last; ++t) {
            int start = t - s.len + 1;
            while (al < ar && minQueue[al] < start) ++al;
            while (bl < br && maxQueue[bl] < start) ++bl;
            while (al < ar && s.diagLow[minQueue[ar - 1]] <= s.diagLow[t]) --ar;
            while (bl < br && s.diagHigh[maxQueue[br - 1]] <= s.diagHigh[t]) --br;
            minQueue[ar++] = t;
            maxQueue[br++] = t;
            if (t < first) continue;

            int fixed = s.fixedMax[t];
            s.low[t] = max(s.low[t], max(fixed, s.diagLow[minQueue[al]]));
            int upper = max(fixed, s.diagHigh[maxQueue[bl]]);
            int old = s.high[t];
            if (old != upper) {
                if (old) {
                    --histogram[old];
                    if (old > cutoff && upper <= cutoff) ++atMost;
                }
                ++histogram[upper];
                s.high[t] = upper;
            }
        }
    }

    void enqueue(int id) {
        const Snake& s = snakes[id];
        int best = 0, low = cutoff, high = all + 1;
        for (int t = 1; t <= d; ++t) {
            int a = s.low[t], b = s.high[t];
            if (a >= b || a >= cutoff) continue;
            if (a < low || (a == low && b < high)) {
                low = a;
                high = b;
                best = t;
            }
        }
        if (best) candidates.push({low, high, id, best});
    }

    template<bool Forward>
    bool trimStep(Snake& s, int t) const {
        Bits& cur = s.cells[t];
        const Bits& prev = s.cells[t + (Forward ? -1 : 1)];
        int l = s.leftWord[t], r = s.rightWord[t];
        if (l == r && !(cur.w[l] & (cur.w[l] - 1))) return false;
        U64 changes = 0;
        for (int k = l; k <= r; ++k) {
            U64 p = prev.w[k], allowed;
            if constexpr (Forward) {
                allowed = p | (p << 1);
                if (k) allowed |= prev.w[k - 1] >> 63;
            } else {
                allowed = p | (p >> 1);
                if (k + 1 < words) allowed |= prev.w[k + 1] << 63;
            }
            U64 before = cur.w[k];
            cur.w[k] = before & allowed;
            changes |= before ^ cur.w[k];
        }
        if (changes) shrink(s, t);
        return changes != 0;
    }

    void incorporate(Snake& s, int time, int value) {
        int first = d + 1, last = 0;
        int anchor = value > s.fixedMax[time] ? positionTime[value] : 0;
        for (int t = max(1, time - s.len + 1); t <= time; ++t) {
            if (t == anchor) {
                if (s.minRank[t] == s.maxRank[t]) continue;
                int c = positionColumn[value];
                s.cells[t] = Bits{};
                s.cells[t].w[c >> 6] = U64(1) << (c & 63);
                s.leftWord[t] = s.rightWord[t] = c >> 6;
            } else {
                if (s.diagHigh[t] <= value) continue;
                const auto& g = diag[t];
                const Bits& allow = g.prefix[g.rankLE(value)];
                for (int k = s.leftWord[t]; k <= s.rightWord[t]; ++k)
                    s.cells[t].w[k] &= allow.w[k];
                shrink(s, t);
            }
            first = min(first, t);
            last = t;
        }

        if (last == 0) return;

        for (int t = max(2, first); t <= d; ++t) {
            if (trimStep<true>(s, t)) last = max(last, t);
            else if (t > last) break;
        }
        for (int t = min(d - 1, last); t >= 1; --t) {
            if (trimStep<false>(s, t)) first = min(first, t);
            else if (t < first) break;
        }

        int loChanged = d + 1, hiChanged = 0;
        for (int t = first; t <= last; ++t) {
            const auto& g = diag[t];
            int l = s.minRank[t], r = s.maxRank[t];
            while (!contains(s.cells[t], g.column[l])) ++l;
            while (!contains(s.cells[t], g.column[r])) --r;
            if (l == s.minRank[t] && r == s.maxRank[t]) continue;
            s.minRank[t] = l;
            s.maxRank[t] = r;
            s.diagLow[t] = g.value[l];
            s.diagHigh[t] = g.value[r];
            loChanged = min(loChanged, t);
            hiChanged = t;
        }
        ++s.revision;
        if (hiChanged) {
            refresh(s, loChanged, min(d, hiChanged + s.len - 1));
            lowerCutoff();
        }
    }

    // Return the first diagonal where the threshold-constrained traversal dies.
    // A complete traversal can be extended at both ends because cells is trimmed.
    template<bool Forward>
    int failure(const Snake& s, int a, int b, int v) {
        constexpr int step = Forward ? 1 : -1;
        int start = Forward ? a : b;
        int end = (Forward ? b : a) + step;
        Bits reachable{};
        bool full = true;
        int cost = 0;

        for (int t = start; t != end; t += step) {
            ++cost;
            bool restricted = s.diagHigh[t] > v;
            if (full && !restricted) continue;
            if (s.diagLow[t] > v) {
                work -= cost;
                return t;
            }

            const Bits& allowed = s.cells[t];
            const Bits* limit = nullptr;
            if (restricted) {
                const auto& g = diag[t];
                limit = &g.prefix[g.rankLE(v)];
            }
            Bits next{};
            U64 any = 0, missing = 0;
            int l = s.leftWord[t], r = s.rightWord[t];
            cost += r - l + 1;
            for (int k = l; k <= r; ++k) {
                U64 mask = allowed.w[k];
                if (limit) mask &= limit->w[k];
                if (!full) {
                    U64 p = reachable.w[k], movement;
                    if constexpr (Forward) {
                        movement = p | (p << 1);
                        if (k) movement |= reachable.w[k - 1] >> 63;
                    } else {
                        movement = p | (p >> 1);
                        if (k + 1 < words) movement |= reachable.w[k + 1] << 63;
                    }
                    mask &= movement;
                }
                next.w[k] = mask;
                any |= mask;
                missing |= allowed.w[k] ^ mask;
            }
            if (!any) {
                work -= cost;
                return t;
            }
            full = !missing;
            if (!full) reachable = next;
        }
        work -= cost;
        return 0;
    }

    bool improve(Snake& s, int t) {
        if (work <= 0 || s.checked[t] == s.revision || s.len == 1 ||
            s.low[t] + 1 >= s.high[t]) return false;

        int a = max(1, t - s.len + 1);
        if (a == t) return false;
        int bad = failure<true>(s, a, t, s.low[t]);
        if (!bad) return false;

        int lower = s.low[t] + 1;
        // If the upper bound is not forced, some path avoids its unique
        // maximizing cell, and therefore has a strictly smaller maximum.
        int upper = min(cutoff, s.high[t] - 1);
        while (lower < upper && work > 0) {
            int mid = (lower + upper) / 2;
            int f = failure<true>(s, a, t, mid);
            if (f) {
                lower = mid + 1;
                bad = f;
            } else {
                upper = mid;
            }
        }

        s.low[t] = lower;
        if (lower == upper) s.checked[t] = s.revision;
        if (work > 0) {
            int left = failure<false>(s, a, bad, lower - 1);
            if (left) {
                int rightmost = min(d, left + s.len - 1);
                for (int end = bad; end <= rightmost; ++end)
                    s.low[end] = max(s.low[end], lower);
            }
        }
        return true;
    }

public:
    Solver(int n, int m) : n(n), m(m), d(2 * n - 1), all(n * n),
        words((n + 63) / 64), cutoff(all), atMost(n * d), diag(d + 1),
        positionTime(all + 1), positionColumn(all + 1), histogram(all + 1),
        minQueue(d + 1), maxQueue(d + 1) {}

    bool initialize() {
        int stride = n + 1;
        const int inf = all + 1;
        vector<int> grid(stride * stride, inf), rowMax(n + 1);
        vector<vector<pair<int, int>>> entries(d + 1);
        for (int r = 1; r <= n; ++r) {
            for (int c = 1; c <= n; ++c) {
                int v;
                if (!(cin >> v) || v == -1) return false;
                int t = r + c - 1;
                positionTime[v] = t;
                positionColumn[v] = c - 1;
                if (r != 1 || c == 1) {
                    grid[r * stride + c] = v;
                    entries[t].emplace_back(v, c - 1);
                }
                if (r == 1) rowMax[c] = max(rowMax[c - 1], v);
            }
        }

        vector<vector<int>> initial(n + 1, vector<int>(d + 1, inf));
        for (int t = 1; t <= d; ++t) {
            auto& e = entries[t];
            auto& g = diag[t];
            sort(e.begin(), e.end());
            int size = e.size();
            g.value.resize(size);
            g.column.resize(size);
            g.prefix.resize(size + 1);
            g.leftWord = words - 1;
            g.rightWord = 0;
            for (int i = 0; i < size; ++i) {
                auto [v, c] = e[i];
                g.value[i] = v;
                g.column[i] = c;
                g.prefix[i + 1] = g.prefix[i];
                g.prefix[i + 1].w[c >> 6] |= U64(1) << (c & 63);
                g.leftWord = min(g.leftWord, c >> 6);
                g.rightWord = max(g.rightWord, c >> 6);
            }
            g.bucket.resize((all >> 8) + 1);
            int rank = 0;
            for (int i = 0; i < (int)g.bucket.size(); ++i) {
                while (rank < size && g.value[rank] <= (i << 8)) ++rank;
                g.bucket[i] = rank;
            }
            initial[1][t] = g.value.front();
        }

        // Minimum bottleneck of a path of each length ending at each cell.
        vector<int> prev = grid, cur(grid.size(), inf);
        for (int len = 2; len <= n; ++len) {
            cur[stride + 1] = inf;
            auto& best = initial[len];
            for (int r = 2; r <= n; ++r) {
                int c0 = max(1, len - r + 1);
                int p = r * stride + c0;
                int t = r + c0 - 1;
                for (int c = c0; c <= n; ++c, ++p, ++t) {
                    int v = max(grid[p], min(prev[p - stride], prev[p - 1]));
                    cur[p] = v;
                    best[t] = min(best[t], v);
                }
            }
            prev.swap(cur);
        }

        snakes.reserve(n);
        for (int len = 1; len <= n; ++len) {
            snakes.emplace_back(len, d);
            Snake& s = snakes.back();
            for (int t = 1; t <= d; ++t) {
                const auto& g = diag[t];
                s.cells[t] = g.prefix.back();
                s.leftWord[t] = g.leftWord;
                s.rightWord[t] = g.rightWord;
                s.maxRank[t] = (int)g.value.size() - 1;
                s.diagLow[t] = g.value.front();
                s.diagHigh[t] = g.value.back();
                if (t <= len) s.fixedMax[t] = rowMax[len - t + 1];
                s.low[t] = initial[min(t, len)][t];
            }
            refresh(s, 1, d);
        }
        lowerCutoff();
        for (int id = 0; id < n; ++id) enqueue(id);
        return true;
    }

    void run() {
        while (!candidates.empty() && candidates.top().low < cutoff) {
            Candidate c = candidates.top();
            candidates.pop();
            Snake& s = snakes[c.id];
            if (!improve(s, c.time)) {
                cout << "? " << s.len << ' ' << c.time << '\n' << flush;
                int value;
                if (!(cin >> value) || value < 1 || value > all) return;
                incorporate(s, c.time, value);
            }
            enqueue(c.id);
        }

        cout << '!';
        int remaining = m;
        for (int v = 1; v <= cutoff && remaining; ++v) {
            int copies = min(remaining, histogram[v]);
            remaining -= copies;
            while (copies--) cout << ' ' << v;
        }
        cout << '\n' << flush;
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, m;
    if (!(cin >> n >> m) || n == -1 || m == -1) return 0;
    Solver solver(n, m);
    if (solver.initialize()) solver.run();
    return 0;
}
