#include <bits/stdc++.h>
using namespace std;

/* Give every rank a priority.  If priorities strictly increase from the
   bottom to the top of each stack, the currently greatest remaining rank
   can always be removed.  Patience sorting finds the fewest stacks for
   any chosen priority order (equal priorities must use different stacks).

   For ranks ordered by their first occurrence, form consecutive groups.
   Inside a group priorities follow the second occurrence.  The first
   occurrences can then be put into as many increasing subsequences as
   the longest decreasing subsequence of their second positions; all
   second occurrences of a group fit into one additional stack.

   In the principal construction group g has a budget of 179-g increasing
   subsequences.  The reserved stack of group g can subsequently be used
   by the second occurrences of that group.  Uniform group budgets and
   reversing the deck provide alternative orders for structured decks.
*/
int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    int n;
    if (!(cin >> n)) return 0;
    int m = 2 * n;
    vector<int> a(m), first(n + 1, -1), second(n + 1), answer(m);
    for (int i = 0; i < m; ++i) {
        cin >> a[i];
        if (first[a[i]] == -1) first[a[i]] = i;
        else second[a[i]] = i;
    }
    vector<long long> priority(n + 1), piles;
    vector<int> tails;
    tails.reserve(181);
    piles.reserve(181);

    auto attempt = [&](int l, int r, bool reverse, int budget, int slope) {
        tails.clear();
        int group = 0;
        const long long stride = r - l + 1LL;
        int begin = reverse ? r - 1 : l;
        int end = reverse ? l - 1 : r;
        int step = reverse ? -1 : 1;
        for (int i = begin; i != end; i += step) {
            int x = a[i];
            if (i != (reverse ? second[x] : first[x])) continue;
            int q = reverse ? r - 1 - first[x] : second[x] - l;
            int v = -q;
            int k = upper_bound(tails.begin(), tails.end(), v) - tails.begin();
            int capacity = max(1, budget - slope * group);
            if (k == (int)tails.size()) {
                if (k == capacity) {
                    ++group;
                    tails.clear();
                }
                tails.push_back(v);
            } else {
                tails[k] = v;
            }
            priority[x] = stride * group + q;
        }
        piles.clear();
        for (int i = begin; i != end; i += step) {
            long long v = -priority[a[i]];
            int k = upper_bound(piles.begin(), piles.end(), v) - piles.begin();
            if (k == (int)piles.size()) {
                if (k == 180) return false;
                piles.push_back(v);
            } else {
                piles[k] = v;
            }
            answer[i] = k + 1;
        }
        return true;
    };

    auto compressed_attempts = [&](int l, int r) {
        struct Block { int p, q, size, direction; };
        vector<Block> blocks;
        vector<int> owner(n + 1, -1), offset(n + 1);
        for (int i = l; i < r; ++i) {
            int x = a[i];
            if (first[x] != i) continue;
            int direction = 0;
            if (i + 1 < r && first[a[i + 1]] == i + 1) {
                int delta = second[a[i + 1]] - second[x];
                if (abs(delta) == 1) direction = delta;
            }
            int j = i + 1;
            if (direction) {
                while (j < r && first[a[j]] == j &&
                       second[a[j]] == second[x] + direction * (j - i)) ++j;
            }
            int id = blocks.size();
            for (int k = i; k < j; ++k) {
                owner[a[k]] = id;
                offset[a[k]] = k - i;
            }
            int q = direction == -1 ? second[a[j - 1]] : second[x];
            blocks.push_back({i, q, j - i, direction});
            i = j - 1;
        }
        int common = 0;
        for (const auto &b : blocks) if (b.direction == -1)
            common = gcd(common, b.size);
        if (common > 1) {
            vector<Block> split;
            for (const auto &b : blocks) {
                int chunk = b.direction == -1 ? common : b.size;
                for (int off = 0; off < b.size; off += chunk) {
                    int sz = min(chunk, b.size - off), id = split.size();
                    int q = b.direction == -1 ? b.q + b.size - off - sz : b.q + off;
                    split.push_back({b.p + off, q, sz, b.direction});
                    for (int j = 0; j < sz; ++j) {
                        owner[a[b.p + off + j]] = id;
                        offset[a[b.p + off + j]] = j;
                    }
                }
            }
            blocks.swap(split);
        }
        int count = blocks.size();
        if (count == (r - l) / 2) return false;
        vector<int> endpoints(r - l, -1), events, bp(count), bq(count), root(count);
        vector<long long> key(count);
        int maxroot = 1;
        for (int i = 0; i < count; ++i) {
            endpoints[blocks[i].p - l] = endpoints[blocks[i].q - l] = i;
            int b = 1;
            if (blocks[i].direction == -1) {
                b = sqrt(blocks[i].size);
                while (b * b < blocks[i].size) ++b;
            }
            root[i] = b;
            maxroot = max(maxroot, b);
            bp[i] = -1;
        }
        for (int x : endpoints) if (x != -1) {
            int p = events.size();
            if (bp[x] == -1) bp[x] = p;
            else bq[x] = p;
            events.push_back(x);
        }
        auto expand = [&](bool rev) {
            for (int i = l; i < r; ++i) if (first[a[i]] == i) {
                int x = a[i], b = owner[x], j = offset[x];
                if (rev && blocks[b].direction != -1) j = blocks[b].size - 1 - j;
                int w = root[b];
                int inner = (j / w) * w + w - 1 - j % w;
                priority[x] = key[b] * (r - l + 1000LL) + inner;
            }
            piles.clear();
            for (int z = 0; z < r - l; ++z) {
                int i = rev ? r - 1 - z : l + z;
                long long v = -priority[a[i]];
                int at = upper_bound(piles.begin(), piles.end(), v) - piles.begin();
                if (at == (int)piles.size()) {
                    if (at == 180) return false;
                    piles.push_back(v);
                } else piles[at] = v;
                answer[i] = at + 1;
            }
            return true;
        };
        auto test = [&](bool rev, int budget, int slope) {
            tails.clear();
            int group = 0, len = events.size();
            for (int z = 0; z < len; ++z) {
                int i = rev ? len - 1 - z : z;
                int x = events[i];
                if (i != (rev ? bq[x] : bp[x])) continue;
                int q = rev ? len - 1 - bp[x] : bq[x];
                int v = -q;
                int at = upper_bound(tails.begin(), tails.end(), v) - tails.begin();
                if (at == (int)tails.size()) {
                    if (at == max(1, budget - slope * group)) {
                        ++group;
                        tails.clear();
                    }
                    tails.push_back(v);
                } else tails[at] = v;
                key[x] = (len + 1LL) * group + q;
            }
            return expand(rev);
        };
        vector<int> order(count), label(count, n + 1);
        iota(order.begin(), order.end(), 0);
        for (int i = l; i < r; ++i) if (first[a[i]] == i)
            label[owner[a[i]]] = min(label[owner[a[i]]], a[i]);
        for (int mode = 0; mode < 4; ++mode) {
            sort(order.begin(), order.end(), [&](int x, int y) {
                if (mode == 0) return label[x] < label[y];
                if (mode == 1) return bp[x] + bq[x] < bp[y] + bq[y];
                if (mode == 2) return bp[x] < bp[y];
                return bq[x] < bq[y];
            });
            for (int i = 0; i < count; ++i) key[order[i]] = i;
            if (expand(false) || expand(true)) return true;
        }
        int budget = max(1, 180 / maxroot - 1);
        if (test(false, budget, 1) || test(true, budget, 1)) return true;
        for (int b = 1; b <= min(20, budget + 3); ++b) {
            if (test(false, b, 1) || test(true, b, 1)) return true;
            if (test(false, b, 0) || test(true, b, 0)) return true;
        }
        return false;
    };

    auto solve = [&](int l, int r) {
        if (r - l == 2) {
            answer[l] = 1;
            answer[l + 1] = 2;
            return;
        }
        if (attempt(l, r, false, 179, 1)) return;
        if (attempt(l, r, true, 179, 1)) return;
        const int budgets[] = {90, 120, 60, 150, 30, 170, 15, 8, 4, 2, 1, 179};
        for (int b : budgets) {
            if (attempt(l, r, false, b, 0)) return;
            if (attempt(l, r, true, b, 0)) return;
        }
        for (int b : {150, 120, 90, 60}) {
            for (int s : {1, 2}) {
                if (attempt(l, r, false, b, s)) return;
                if (attempt(l, r, true, b, s)) return;
            }
        }
        if (compressed_attempts(l, r)) return;
        // Further group widths only matter on unusually structured inputs.
        for (int b = 1; b < 180; ++b) {
            if (attempt(l, r, false, b, 0)) return;
            if (attempt(l, r, true, b, 0)) return;
        }
        assert(false && "no suitable priority order found");
    };

    // No rank crosses these boundaries, so all 180 stacks can be reused.
    int l = 0, far = -1;
    for (int i = 0; i < m; ++i) {
        far = max(far, second[a[i]]);
        if (i == far) {
            solve(l, i + 1);
            l = i + 1;
        }
    }
    for (int i = 0; i < m; ++i) {
        if (i) cout << ' ';
        cout << answer[i];
    }
    cout << '\n';
}
