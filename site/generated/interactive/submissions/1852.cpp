#include <bits/stdc++.h>
using namespace std;

int sentLength = 0;

int ask(const vector<int>& q) {
    cout << "? " << q.size();
    for (int x : q) cout << ' ' << x;
    cout << '\n' << flush;
    sentLength += (int)q.size();
    int r;
    if (!(cin >> r) || r < 0) exit(0);
    return r;
}

[[noreturn]] void finish(int answer) {
    cout << "! " << answer << '\n' << flush;
    exit(0);
}

// A nearly constant (or nearly alternating) path can be solved by
// splitting its edges. One change in a constant path needs only one query.
struct Sparse {
    map<tuple<int, int, bool>, pair<int, int>> memo;

    pair<int, int> bound(int len, int k, bool alternating) {
        if (2 * k > len) {
            k = len - k;
            alternating = !alternating;
        }
        if (k == 0) return {0, 0};
        if (k == 1) {
            if (!alternating) return {1, len + 1};
            return {2, (len + 2) / 2 + 2};
        }
        auto key = make_tuple(len, k, alternating);
        auto it = memo.find(key);
        if (it != memo.end()) return it->second;
        int left = len / 2, right = len - left;
        pair<int, int> best = {0, 0};
        for (int x = max(0, k - right); x <= min(k, left); ++x) {
            auto a = bound(left, x, alternating);
            auto b = bound(right, k - x, alternating);
            best.first = max(best.first, 1 + a.first + b.first);
            best.second = max(best.second, left + 1 + a.second + b.second);
        }
        return memo[key] = best;
    }

    // Index of the last element in the first run, in subsequence coordinates.
    int singleChange(int start, int step, int size) {
        vector<int> q;
        for (int i = 0, j = size - 1; i <= j;) {
            q.push_back(start + step * i++);
            if (i <= j) q.push_back(start + step * j--);
        }
        int r = ask(q);
        if (r == 0) return size - 1;
        int prefix = (r & 1) ? (r + 1) / 2 : size - r / 2;
        return prefix - 1;
    }

    // Count A on [l,r); the right endpoint is not counted here.
    int solve(int l, int r, int changes, int first) {
        int len = r - l;
        bool alt = 2 * changes > len;
        int defects = alt ? len - changes : changes;
        if (defects <= 1) {
            int edge = r;
            if (defects == 1 && !alt) {
                edge = l + singleChange(l, 1, len + 1);
            } else if (defects == 1) {
                int size = len / 2 + 1;
                int i = singleChange(l, 2, size);
                if (i == size - 1) {
                    edge = r - 1;
                } else {
                    int mid = l + 2 * i + 1;
                    edge = mid - (ask({l, mid}) == 0);
                }
            }
            int result = 0;
            for (int i = l; i < r; ++i) {
                int bit = first ^ (alt && ((i - l) & 1)) ^ (i > edge);
                result += bit == 0;
            }
            return result;
        }
        int mid = l + len / 2;
        vector<int> q(mid - l + 1);
        iota(q.begin(), q.end(), l);
        int x = ask(q);
        return solve(l, mid, x, first)
             + solve(mid, r, changes - x, first ^ (x & 1));
    }
};

enum Kind { BASE, SIGNED, BRIDGE, ALL_ONES };

struct Design {
    vector<vector<int>> w;
    const Design* child;
    Kind kind;
    int rows, cols, needBig = 0, needSmall = 0, length = 0;
    bool allOneFirst;

    Design(vector<vector<int>> matrix, const Design* child_, Kind kind_)
        : w(move(matrix)), child(child_), kind(kind_) {
        rows = (int)w.size();
        cols = (int)w[0].size();
        allOneFirst = all_of(w[0].begin(), w[0].end(),
                            [](int x) { return x == 1; });
        for (const auto& row : w) {
            int p = count(row.begin(), row.end(), 1);
            int m = count(row.begin(), row.end(), -1);
            int a = p ? p + 1 : 0, b = m ? m + 1 : 0;
            needBig = max(needBig, max(a, b));
            needSmall = max(needSmall, min(a, b));
            length += p + m + a + b + 1;
        }
    }

    vector<int> decode(const vector<int>& sums) const {
        if (kind == BASE) {
            for (int mask = 0; mask < (1 << cols); ++mask) {
                bool ok = true;
                for (int i = 0; i < rows && ok; ++i) {
                    int s = 0;
                    for (int j = 0; j < cols; ++j)
                        if ((mask >> j) & 1) s += w[i][j];
                    ok = s == sums[i];
                }
                if (ok) {
                    vector<int> answer(cols);
                    for (int j = 0; j < cols; ++j) answer[j] = (mask >> j) & 1;
                    return answer;
                }
            }
            exit(0);
        }

        int r = child->rows, c = child->cols;
        vector<int> u(r), v(r), extra;
        int total = 0;
        if (kind == BRIDGE) total = sums[2 * r];
        if (kind == ALL_ONES) total = sums[0] - sums[r];
        int extraSum = 0;
        for (int i = (kind == ALL_ONES); i < r; ++i) {
            int z = sums[i] + sums[r + i] - total;
            int bit = (z % 2 + 2) % 2;
            int term = bit * w[i][2 * c + i - (kind == ALL_ONES)];
            extra.push_back(bit);
            extraSum += bit;
            u[i] = (z - term) / 2;
            v[i] = sums[i] - u[i] - term;
        }
        if (kind == ALL_ONES) {
            u[0] = sums[r] - extraSum;
            v[0] = total;
        }

        auto a = child->decode(u);
        auto b = child->decode(v);
        a.insert(a.end(), b.begin(), b.end());
        a.insert(a.end(), extra.begin(), extra.end());
        return a;
    }
};

struct Catalog {
    vector<unique_ptr<Design>> designs;

    const Design* add(vector<vector<int>> w, const Design* d, Kind k) {
        designs.push_back(make_unique<Design>(move(w), d, k));
        return designs.back().get();
    }

    void family(vector<vector<int>> base, Kind kind, int limit) {
        if ((int)(base.size() + base[0].size()) > limit) return;
        const Design* d = add(move(base), nullptr, BASE);
        while (true) {
            int r = d->rows, c = d->cols;
            int nr = 2 * r + (kind == BRIDGE);
            int nc = 2 * c + r - (kind == ALL_ONES);
            if (nr + nc > limit) break;
            vector<vector<int>> m(nr, vector<int>(nc));
            for (int i = 0; i < r; ++i) {
                int sum = 0;
                for (int j = 0; j < c; ++j) {
                    int a = d->w[i][j];
                    sum += a;
                    m[i][j] = m[i][c + j] = m[r + i][j] = a;
                    m[r + i][c + j] = kind == SIGNED ? -a : 1 - a;
                }
                if (kind == ALL_ONES && i == 0) {
                    for (int j = 2 * c; j < nc; ++j)
                        m[0][j] = m[r][j] = 1;
                } else {
                    m[i][2 * c + i - (kind == ALL_ONES)] =
                        (kind == SIGNED && sum > 0) ? -1 : 1;
                }
            }
            if (kind == BRIDGE)
                for (int j = c; j < 2 * c; ++j) m[2 * r][j] = 1;
            d = add(move(m), d, kind);
        }
    }

    explicit Catalog(int limit) {
        family({{1}}, SIGNED, limit);
        family({{-1, 1, -1, 1, 0}, {-1, -1, 1, 0, 1},
                {-1, 1, 1, 0, 0}}, SIGNED, limit);
        family({{1}}, BRIDGE, limit);
        family({{1, 1}, {1, 0}}, BRIDGE, limit);
        family({{1, 1, 0, 1}, {1, 0, 1, 1}, {0, 1, 1, 1}}, BRIDGE, limit);
        family({{1}}, ALL_ONES, limit);
        family({{1, 1, 1}, {1, 0, 0}, {0, 1, 0}}, ALL_ONES, limit);
    }
};

int batchBound(int rem, int k, int big) {
    if (rem == 0) return 0;
    if (rem <= max(2, big)) return 1;
    int fixed = 2 * big - k + 1;
    auto prefix = [](long long x) { return x * x / 4; };
    int lo = 1, hi = rem;
    while (lo < hi) {
        int mid = (lo + hi) / 2;
        int flat = min(fixed, mid);
        long long capacity = 1LL * flat * big
                           + prefix(k + mid) - prefix(k + flat);
        if (capacity >= rem) hi = mid;
        else lo = mid + 1;
    }
    return lo;
}

struct Planner {
    // -1: counting batches; -2: basic step; -3: five-mushroom step.
    int cap;
    vector<vector<array<short, 10>>> dp;
    vector<vector<short>> action;
    vector<int> lg;

    Planner(int n, int cap_, const Catalog& catalog)
        : cap(cap_), dp(cap + 1), action(cap + 1), lg(cap + 2) {
        for (int i = 2; i <= cap + 1; ++i) lg[i] = lg[i / 2] + 1;
        for (int k = cap; k >= 2; --k) {
            int mn = (k + 1) / 2, sz = k - mn + 1;
            dp[k].resize(sz);
            action[k].resize(sz);
            for (int big = mn; big <= k; ++big) {
                int pos = big - mn, small = k - big;
                int best = batchBound(n - k, k, big);
                int bestRank = best <= 5 ? 270 : 200;
                int a = -1;
                auto consider = [&](int id, int d, int q, int rank) {
                    int nk = k + d;
                    if (nk > cap || q > best) return;
                    int base = (nk + 1) / 2;
                    int l = max(big, base) - base, r = big + d - base;
                    int h = lg[r - l + 1];
                    int worst = q + max(dp[nk][l][h],
                                        dp[nk][r - (1 << h) + 1][h]);
                    if (worst < best || (worst == best && rank > bestRank)) {
                        best = worst;
                        bestRank = rank;
                        a = id;
                    }
                };
                if (best > 1) {
                    consider(-2, 1, 1, 100);
                    if (big >= 5 && small >= 2)
                        consider(-3, 5, 2, 285);
                    for (int i = 0; i < (int)catalog.designs.size(); ++i) {
                        const auto& d = *catalog.designs[i];
                        if (big < d.needBig || small < d.needSmall) continue;
                        int gain = d.rows + d.cols;
                        int rank = 100 * gain / d.rows
                                 + (d.allOneFirst ? 22 : 0) - d.rows / 3;
                        consider(i, gain, d.rows, rank);
                    }
                }
                action[k][pos] = a;
                dp[k][pos][0] = best;
            }
            for (int h = 1; (1 << h) <= sz; ++h)
                for (int i = 0; i + (1 << h) <= sz; ++i)
                    dp[k][i][h] = max(dp[k][i][h - 1],
                        dp[k][i + (1 << (h - 1))][h - 1]);
        }
    }
};

struct Solver {
    vector<int> species, known[2], todo;
    int at = 0, countedA = 0;

    explicit Solver(int n) : species(n, -1) {}

    void mark(int id, int bit) {
        if (species[id] < 0) {
            species[id] = bit;
            known[bit].push_back(id);
        }
    }

    int majority() const { return known[1].size() > known[0].size(); }
    int remaining() const { return (int)todo.size() - at; }

    void basic() {
        int s = majority();
        int x = todo[at], y = todo[at + 1];
        int r = ask({known[s][0], x, y});
        mark(y, s ^ (r & 1));
        if (r & 1) {
            swap(todo[at], todo[at + 1]);
            ++at;
        } else {
            mark(x, s ^ (r / 2));
            at += 2;
        }
    }

    void learnFive() {
        int s = majority(), t = s ^ 1;
        int endpoint = todo[at++];
        array<int, 4> x;
        vector<int> q = {endpoint, known[s][0]};
        for (int i = 0; i < 4; ++i) {
            x[i] = todo[at++];
            q.push_back(x[i]);
            q.push_back(known[s][i + 1]);
        }
        int r = ask(q);
        mark(endpoint, s ^ (r & 1));
        int diff = r / 2;
        if (diff == 0 || diff == 4) {
            for (int id : x) mark(id, s ^ (diff == 4));
            return;
        }
        int w = ask({x[1], known[t][0], x[0], known[t][1],
                     known[s][0], x[3], known[s][1]}) - 4;
        for (int m = 0; m < 16; ++m) {
            int sum = -2 * (m & 1) - ((m >> 1) & 1) + 2 * ((m >> 3) & 1);
            if (__builtin_popcount((unsigned)m) == diff && sum == w) {
                for (int i = 0; i < 4; ++i) mark(x[i], s ^ ((m >> i) & 1));
                return;
            }
        }
        exit(0);
    }

    void learn(const Design& d) {
        int start = at;
        at += d.cols;
        vector<int> sums(d.rows);
        for (int i = 0; i < d.rows; ++i) {
            vector<int> group[2];
            int fixed = 0;
            for (int j = 0; j < d.cols; ++j) {
                int w = d.w[i][j];
                if (!w) continue;
                int id = todo[start + j];
                if (species[id] >= 0) fixed += w * species[id];
                else group[w < 0].push_back(id);
            }
            if (group[0].empty() && group[1].empty()) {
                sums[i] = fixed;
                continue;
            }
            int need[2];
            for (int s = 0; s < 2; ++s)
                need[s] = group[s].empty() ? 0 : (int)group[s].size() + 1;
            int flip = 0;
            if (need[0] > (int)known[0].size() ||
                need[1] > (int)known[1].size()) flip = 1;

            int endpoint = todo[at++];
            vector<int> q = {endpoint};
            int first = -1, switches = 0;
            for (int g = 0; g < 2; ++g) {
                if (group[g].empty()) continue;
                int s = g ^ flip;
                if (first < 0) first = s;
                else ++switches;
                q.push_back(known[s][0]);
                for (int j = 0; j < (int)group[g].size(); ++j) {
                    q.push_back(group[g][j]);
                    q.push_back(known[s][j + 1]);
                }
            }
            int r = ask(q) - switches;
            mark(endpoint, first ^ (r & 1));
            int diff = r / 2;
            int negative = (int)group[1 ^ flip].size();
            sums[i] = fixed + (flip ? -1 : 1) * (diff - negative);
            int size = (int)(group[0].size() + group[1].size());
            if (diff == 0 || diff == size)
                for (int g = 0; g < 2; ++g)
                    for (int id : group[g]) mark(id, g ^ flip ^ (diff == size));
        }

        auto bits = d.decode(sums);
        for (int j = 0; j < d.cols; ++j) mark(todo[start + j], bits[j]);
    }

    [[noreturn]] void countBatches() {
        while (remaining() > 0) {
            int s = majority(), m = (int)known[s].size(), rem = remaining();
            if (rem == 2) {
                int r = ask({todo[at], known[s][0], todo[at + 1]});
                countedA += s ? r : 2 - r;
                break;
            }
            if (rem == m + 1) {
                vector<int> q;
                for (int j = 0; j < m; ++j) {
                    q.push_back(todo[at + j]);
                    q.push_back(known[s][j]);
                }
                q.push_back(todo[at + m]);
                int r = ask(q);
                int diff = r / 2;
                if ((r & 1) || r == 2 * m) ++diff;
                else if (r != 0) diff += ask({known[s][0], todo[at]});
                countedA += s ? diff : rem - diff;
                break;
            }
            if (m == 1 && rem >= 3) {
                basic();
                continue;
            }
            int len = min(m, rem);
            vector<int> q;
            for (int j = 0; j < len; ++j) {
                q.push_back(todo[at + j]);
                q.push_back(known[s][j]);
            }
            int r = ask(q), diff = r / 2;
            mark(todo[at], s ^ (r & 1));
            if (diff == 0 || diff == len - 1) {
                int bit = s ^ (diff != 0);
                for (int j = 1; j < len; ++j) mark(todo[at + j], bit);
            } else {
                countedA += s ? diff : len - 1 - diff;
            }
            at += len;
        }
        finish((int)known[0].size() + countedA);
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n) || n < 2) return 0;
    if (n == 2) finish(2 - ask({0, 1}));
    if (n == 3) finish(3 - ask({1, 0, 2}));

    vector<int> path(n);
    iota(path.begin(), path.end(), 0);
    int changes = ask(path);
    if (changes == 0) finish(n);
    if (changes == n - 1) finish((n + 1) / 2);

    Solver solver(n);
    solver.mark(0, 0);
    solver.mark(n - 1, changes & 1);
    for (int i = 1; i + 1 < n; ++i) solver.todo.push_back(i);

    int cap = min(n, 520);
    Catalog catalog(cap);
    Planner planner(n, cap, catalog);

    int defects = min(changes, n - 1 - changes);
    if (defects <= 64) {
        Sparse sparse;
        auto b = sparse.bound(n - 1, changes, false);
        int big = (int)solver.known[solver.majority()].size();
        if (b.first <= planner.dp[2][big - 1][0] &&
            sentLength + b.second <= 100000) {
            int ans = sparse.solve(0, n - 1, changes, 0);
            finish(ans + ((changes & 1) == 0));
        }
    }

    while (solver.remaining() > 0) {
        int k = (int)(solver.known[0].size() + solver.known[1].size());
        if (k > cap) break;
        int big = (int)solver.known[solver.majority()].size();
        int a = planner.action[k][big - (k + 1) / 2];
        if (a == -1) break;
        int length = a == -2 ? 3 : a == -3 ? 17 : catalog.designs[a]->length;
        if (sentLength + length + 2 * solver.remaining() + 3 > 100000) break;
        if (a == -2) solver.basic();
        else if (a == -3) solver.learnFive();
        else solver.learn(*catalog.designs[a]);
    }
    solver.countBatches();
}
