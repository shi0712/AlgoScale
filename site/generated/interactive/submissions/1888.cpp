#include <bits/stdc++.h>
using namespace std;

class Solver {
    static constexpr int MAXM = 200;
    using Mask = bitset<MAXM>;

    int m, n = 0, largest = 0;
    vector<int> len, parent, owner, level;
    vector<vector<int>> vertex;
    vector<int> singles, groupVertex, groupOf, remaining;
    vector<Mask> possible;
    mt19937 rng{712367821};

    int response() {
        int x;
        if (!(cin >> x) || x == -1) exit(0);
        return x;
    }

    bool insertBall(int u, int x) {
        largest = max(largest, x);
        cout << "? 1 " << u << ' ' << x << '\n' << flush;
        return response() == 1;
    }

    vector<int> collect() {
        cout << "? 2\n" << flush;
        int s = response();
        vector<int> a(s);
        for (int &x : a) x = response();
        return a;
    }

    void answer() {
        cout << "! " << n;
        for (int u = 0; u < n - 1; ++u) cout << ' ' << parent[u];
        cout << '\n' << flush;
    }

    void numberVertices() {
        parent.assign(n, -1);
        owner.resize(n);
        level.resize(n);
        vertex.resize(m);
        int next = m;
        for (int i = 0; i < m; ++i) {
            vertex[i].resize(len[i]);
            for (int j = 0; j < len[i]; ++j) {
                int u;
                if (j + 1 == len[i]) u = i;
                else if (i == 0 && j == 0) u = n - 1;
                else u = next++;
                vertex[i][j] = u;
                owner[u] = i;
                level[u] = j;
                if (j) parent[u] = vertex[i][j - 1];
            }
        }
    }

    struct Codec {
        int heads = 0, radix = 1;
        vector<vector<int>> word;
    };

    Codec makeCodec(const vector<int>& lengths, int colors, bool words) {
        int mx = 1, paths = 0;
        for (int i = 1; i < m; ++i)
            if (lengths[i]) {
                ++paths;
                mx = max(mx, lengths[i]);
            }
        vector<int> freq(mx + 1);
        for (int i = 1; i < m; ++i)
            if (lengths[i]) ++freq[lengths[i]];

        Codec c;
        c.heads = paths;
        for (int q = 2; q <= paths; ++q) {
            int h = 0;
            for (int j = mx; j >= 2; --j)
                h = (h + freq[j] + q - 1) / q;
            h += freq[1];
            if (q + h < c.radix + c.heads) {
                c.radix = q;
                c.heads = h;
            }
        }
        if (!words) return c;

        c.word.resize(m);
        const int cut = colors + c.heads;
        c.word[0].assign(lengths[0], cut);
        c.word[0][0] = 0;

        vector<int> order;
        for (int i = 1; i < m; ++i)
            if (lengths[i]) order.push_back(i);
        stable_sort(order.begin(), order.end(), [&](int a, int b) {
            return lengths[a] < lengths[b];
        });

        // Canonical prefix code: the first digit uses the head alphabet,
        // and every subsequent digit uses the continuation alphabet.
        vector<int> digits;
        for (int i : order) {
            if (!digits.empty()) {
                int j = (int)digits.size() - 1;
                while (j > 0 && ++digits[j] == c.radix) {
                    digits[j] = 0;
                    --j;
                }
                if (j == 0) ++digits[0];
            }
            digits.resize(lengths[i], 0);
            c.word[i] = digits;
            c.word[i][0] += colors;
            for (int j = 1; j < lengths[i]; ++j) c.word[i][j] += cut;
        }
        return c;
    }

    vector<vector<int>> decode(const vector<int>& a, const Codec& c,
                               int rootLength, int colors) {
        struct Node {
            map<int, int> next;
            int leaf = -1;
        };
        vector<Node> trie(1);
        for (int i = 1; i < m; ++i) {
            if (c.word[i].empty()) continue;
            int t = 0;
            for (int x : c.word[i]) {
                auto it = trie[t].next.find(x);
                if (it == trie[t].next.end()) {
                    int v = (int)trie.size();
                    trie[t].next.emplace(x, v);
                    trie.emplace_back();
                    t = v;
                } else {
                    t = it->second;
                }
            }
            trie[t].leaf = i;
        }

        vector<vector<int>> count(n, vector<int>(colors));
        const int cut = colors + c.heads;
        int at = 0;

        // A continuation is visited after all other occupied children.
        // The prefix code tells us when we have reached a chain's last
        // vertex, where there cannot be any occupied children.
        auto parse = [&](auto&& self, bool root) -> int {
            int t = root ? 0 : trie[0].next.at(a[at]);
            ++at;
            vector<array<int, 3>> events;
            int j = 0;
            while (root ? j + 1 < rootLength : trie[t].leaf < 0) {
                while (a[at] < cut) {
                    if (a[at] < colors) {
                        events.push_back({j, a[at++], 0});
                    } else {
                        int child = self(self, false);
                        events.push_back({j, child, 1});
                    }
                }
                if (!root) t = trie[t].next.at(a[at]);
                ++at;
                ++j;
            }
            int i = root ? 0 : trie[t].leaf;
            for (auto [depth, x, child] : events) {
                int u = vertex[i][depth];
                if (child) parent[x] = u;
                else ++count[u][x];
            }
            return vertex[i][0];
        };
        parse(parse, true);
        return count;
    }

    vector<vector<int>> measure(const vector<int>& lengths, int colors,
                                const vector<int>& chosen) {
        Codec c = makeCodec(lengths, colors, true);
        for (int i = 0; i < m; ++i)
            for (int x : c.word[i])
                if (!insertBall(i, x)) exit(0);

        for (int i : singles)
            if (chosen[i] >= 0 && !insertBall(i, chosen[i])) exit(0);

        return decode(collect(), c, lengths[0], colors);
    }

    vector<int> activeLeaves() const {
        vector<int> a;
        for (int i : singles)
            if (parent[i] < 0) a.push_back(i);
        return a;
    }

    // Only fill chain prefixes necessary to reach a possible attachment.
    // Keep one continuation below each attachment, including attachments
    // made by the retained prefixes of other chains.
    vector<int> projection(const vector<int>& leaves) const {
        Mask need;
        for (int i : leaves) need |= possible[i];
        vector<int> lengths(m);
        for (int g = 0; g < (int)groupVertex.size(); ++g) {
            if (!need[g]) continue;
            int u = groupVertex[g];
            lengths[owner[u]] = max(lengths[owner[u]], level[u] + 2);
        }
        for (int i = m - 1; i > 0; --i) {
            if (!lengths[i]) continue;
            int u = parent[vertex[i][0]];
            lengths[owner[u]] = max(lengths[owner[u]], level[u] + 2);
        }
        return lengths;
    }

    // Keep only edges that occur in some capacitated matching. Given
    // one matching, all alternatives are cycles of transfers between
    // the attachment vertices.
    void refine(const vector<int>& leaves, const vector<int>& cap) {
        if (leaves.empty()) return;
        int gsz = (int)groupVertex.size();
        Mask usable;
        vector<int> groups;
        for (int g = 0; g < gsz; ++g) {
            if (cap[g]) {
                usable.set(g);
                groups.push_back(g);
            }
        }

        vector<vector<int>> assigned(gsz);
        vector<int> sorted = leaves;
        for (int i : sorted) possible[i] &= usable;
        stable_sort(sorted.begin(), sorted.end(), [&](int a, int b) {
            return possible[a].count() < possible[b].count();
        });

        Mask seen;
        auto augment = [&](auto&& self, int i) -> bool {
            for (int g : groups) {
                if (!possible[i][g] || seen[g]) continue;
                seen.set(g);
                if ((int)assigned[g].size() < cap[g]) {
                    assigned[g].push_back(i);
                    return true;
                }
                for (int& j : assigned[g]) {
                    if (self(self, j)) {
                        j = i;
                        return true;
                    }
                }
            }
            return false;
        };
        for (int i : sorted) {
            seen.reset();
            if (!augment(augment, i)) exit(0);
        }

        vector<Mask> reach(gsz), same(gsz);
        for (int g : groups)
            for (int i : assigned[g]) reach[g] |= possible[i];
        for (int k : groups)
            for (int g : groups)
                if (reach[g][k]) reach[g] |= reach[k];
        for (int g : groups)
            for (int h : groups)
                if (reach[g][h] && reach[h][g]) same[g].set(h);

        for (int g : groups) {
            for (int i : assigned[g]) {
                possible[i] &= same[g];
                if (possible[i].count() == 1) {
                    parent[i] = groupVertex[g];
                    --remaining[g];
                }
            }
        }
    }

    void colorQuery(const vector<int>& active, const vector<int>& chosen,
                    int colors) {
        vector<int> selected;
        for (int i : active)
            if (chosen[i] >= 0) selected.push_back(i);
        if (selected.empty()) return;

        auto count = measure(projection(selected), colors, chosen);
        const int gsz = (int)groupVertex.size();
        vector<vector<int>> cap(colors + 1, vector<int>(gsz));
        vector<vector<int>> buckets(colors + 1);
        for (int g = 0; g < gsz; ++g) {
            int left = remaining[g];
            for (int c = 0; c < colors; ++c) {
                cap[c][g] = count[groupVertex[g]][c];
                left -= cap[c][g];
            }
            cap[colors][g] = left;
        }
        for (int i : active)
            buckets[chosen[i] < 0 ? colors : chosen[i]].push_back(i);
        for (int c = 0; c <= colors; ++c) refine(buckets[c], cap[c]);
    }

    void singleBatch(vector<int> active, int colors) {
        stable_sort(active.begin(), active.end(), [&](int a, int b) {
            return possible[a].count() < possible[b].count();
        });
        vector<Mask> used(colors);
        vector<int> chosen(m, -1);
        for (int i : active) {
            for (int c = 0; c < colors; ++c) {
                if ((used[c] & possible[i]).any()) continue;
                used[c] |= possible[i];
                chosen[i] = c;
                break;
            }
        }
        while (colors && used[colors - 1].none()) --colors;
        colorQuery(active, chosen, colors);
    }

    void tripleBatch(const vector<int>& active, int colors) {
        // The subset sums of 110, 101, 011, 111 are all distinct.
        // Thus three measurements per color identify four leaves,
        // even if several of them attach at the very same vertex.
        const int code[4] = {3, 5, 6, 7};
        int take = min((int)active.size(), 4 * colors);
        colors = (take + 3) / 4;
        vector<Mask> prior = possible;
        vector<vector<vector<int>>> counts;
        for (int b = 0; b < 3; ++b) {
            vector<int> chosen(m, -1), selected;
            for (int j = 0; j < take; ++j) {
                if (!(code[j % 4] & (1 << b))) continue;
                chosen[active[j]] = j / 4;
                selected.push_back(active[j]);
            }
            if (selected.empty())
                counts.emplace_back(n, vector<int>(colors));
            else
                counts.push_back(measure(projection(selected), colors, chosen));

            bool done = true;
            for (int c = 0; c < colors; ++c) {
                int size = min(4, take - 4 * c);
                for (int g = 0; g < (int)groupVertex.size(); ++g) {
                    int allowed = 0, support = 0;
                    for (int j = 0; j < size; ++j)
                        if (prior[active[4 * c + j]][g]) allowed |= 1 << j;
                    if (!allowed) continue;
                    for (int mask = allowed; mask; mask = (mask - 1) & allowed) {
                        if (popcount((unsigned)mask) > remaining[g]) continue;
                        bool ok = true;
                        for (int t = 0; t <= b; ++t) {
                            int sum = 0;
                            for (int j = 0; j < size; ++j)
                                if ((mask >> j & 1) && (code[j] >> t & 1)) ++sum;
                            if (sum != counts[t][groupVertex[g]][c]) {
                                ok = false;
                                break;
                            }
                        }
                        if (ok) support |= mask;
                    }
                    for (int j = 0; j < size; ++j)
                        if (!(support >> j & 1)) possible[active[4 * c + j]].reset(g);
                }
            }
            for (int j = 0; j < take; ++j)
                if (possible[active[j]].count() != 1) done = false;
            if (done) break;
        }
        vector<int> caps = remaining;
        refine(active, caps);
    }

    void resolveSingletons() {
        int hashRounds = 0;
        while (true) {
            auto active = activeLeaves();
            if (active.empty()) return;
            int u = (int)active.size();
            auto projected = projection(active);
            Codec shape = makeCodec(projected, 0, false);
            int base = shape.heads + shape.radix;
            auto extra = [&](int colors) {
                return max(0, base + colors - 1 - largest);
            };

            int bestCost = INT_MAX, bestK = INT_MAX, batchColors = 1;
            for (int s = 1; s <= u; ++s) {
                int units = (u + s - 1) / s;
                int k = 3 * (units / 4) + units % 4;
                int cost = extra(s) + k;
                if (pair(cost, k) < pair(bestCost, bestK)) {
                    bestCost = cost;
                    bestK = k;
                    batchColors = s;
                }
            }

            int maxDemand = *max_element(remaining.begin(), remaining.end());
            if (maxDemand == 1) {
                // Parents have at most one unresolved leaf each. Give
                // distinct signatures only to leaves that can compete.
                vector<int> label(m, -1);
                vector<Mask> used;
                auto order = active;
                stable_sort(order.begin(), order.end(), [&](int a, int b) {
                    return possible[a].count() > possible[b].count();
                });
                for (int i : order) {
                    int j = 0;
                    while (j < (int)used.size() && (used[j] & possible[i]).any()) ++j;
                    if (j == (int)used.size()) used.emplace_back();
                    used[j] |= possible[i];
                    label[i] = j;
                }

                int digitColors = -1, digitK = 0;
                for (int s = 1; s < (int)used.size(); ++s) {
                    int k = 0;
                    for (int x = (int)used.size() - 1; x; x /= s + 1) ++k;
                    int cost = extra(s) + k;
                    if (pair(cost, k) <= pair(bestCost, bestK)) {
                        bestCost = cost;
                        bestK = digitK = k;
                        digitColors = s;
                    }
                }
                if (digitColors > 0) {
                    int place = 1;
                    for (int d = 0; d < digitK; ++d) {
                        active = activeLeaves();
                        if (active.empty()) return;
                        vector<int> chosen(m, -1);
                        for (int i : active) {
                            int x = label[i] / place % (digitColors + 1);
                            if (x < digitColors) chosen[i] = x;
                        }
                        colorQuery(active, chosen, digitColors);
                        place *= digitColors + 1;
                    }
                    continue;
                }
            } else if (hashRounds < 6) {
                int gsz = (int)groupVertex.size();
                vector<int> candidates(gsz);
                for (int i : active)
                    for (int g = 0; g < gsz; ++g)
                        candidates[g] += possible[i][g];

                int hashColors = -1;
                double predicted = bestCost;
                for (int s = 1; s <= min(u - 1, 24); ++s) {
                    double effort = 0;
                    for (int g = 0; g < gsz; ++g) {
                        int d = remaining[g];
                        if (!d) continue;
                        double logMiss = d * log1p(-1.0 / (s + 1));
                        double shrink = -log(-expm1(logMiss));
                        effort += d * log(1.0 + candidates[g] - d) /
                                  max(1e-9, shrink);
                    }
                    double cost = extra(s) + max(1.0, effort / u + 0.7);
                    if (cost + 0.15 < predicted) {
                        predicted = cost;
                        hashColors = s;
                    }
                }
                if (hashColors > 0) {
                    shuffle(active.begin(), active.end(), rng);
                    vector<int> chosen(m, -1);
                    for (int j = 0; j < u; ++j) {
                        int c = j % (hashColors + 1);
                        if (c < hashColors) chosen[active[j]] = c;
                    }
                    colorQuery(active, chosen, hashColors);
                    ++hashRounds;
                    continue;
                }
            }

            if (u > bestK * batchColors)
                tripleBatch(active, batchColors);
            else
                singleBatch(active, batchColors);
        }
    }

public:
    void run() {
        if (!(cin >> m) || m <= 0) return;
        len.assign(m, 0);

        if (m == 1) {
            while (n < 1000 && insertBall(0, 0)) ++n;
            collect();
            parent.resize(n);
            parent[0] = n == 2 ? 1 : 1;
            for (int i = 1; i + 1 < n; ++i) parent[i] = i + 1;
            answer();
            return;
        }

        const bool direct = m <= 4;
        for (int i = 0; i < m; ++i) {
            while (n + (m - 1 - i) < 1000 || len[i] == 0) {
                int x = 0;
                if (direct)
                    x = len[i] ? m - 1 : max(0, i - 1);
                if (!insertBall(i, x)) break;
                ++len[i];
                ++n;
            }
        }
        auto first = collect();
        numberVertices();
        if (n == m + 1) {
            fill(parent.begin(), parent.end() - 1, n - 1);
            answer();
            return;
        }
        if (direct) {
            Codec c;
            c.heads = m - 1;
            c.word.resize(m);
            for (int i = 1; i < m; ++i) {
                c.word[i].assign(len[i], m - 1);
                c.word[i][0] = i - 1;
            }
            decode(first, c, len[0], 0);
            answer();
            return;
        }

        vector<int> skeleton = len;
        vector<int> chosen(m, -1);
        for (int i = 1; i < m; ++i) {
            if (len[i] != 1) continue;
            singles.push_back(i);
            chosen[i] = 0;
            skeleton[i] = 0;
        }
        auto count = measure(skeleton, !singles.empty(), chosen);
        if (!singles.empty()) {
            groupOf.assign(n, -1);
            for (int u = m; u < n; ++u) {
                if (count[u][0] == 0) continue;
                groupOf[u] = (int)groupVertex.size();
                groupVertex.push_back(u);
                remaining.push_back(count[u][0]);
            }
            possible.resize(m);
            for (int i : singles)
                for (int g = 0; g < (int)groupVertex.size(); ++g)
                    if (owner[groupVertex[g]] < i) possible[i].set(g);
            vector<int> caps = remaining;
            refine(singles, caps);
            resolveSingletons();
        }
        answer();
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    Solver solver;
    solver.run();
    return 0;
}
