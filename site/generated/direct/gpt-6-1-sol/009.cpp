#include <bits/stdc++.h>
using namespace std;

using int64 = long long;

static constexpr int MAX_ENTRY = 1'000'000'000;
static constexpr int64 INF = (1LL << 60);

class FastInput {
    static constexpr size_t SIZE = 1 << 20;
    char buffer[SIZE];
    size_t pos = 0, len = 0;

public:
    inline char getChar() {
        if (pos == len) {
            len = fread(buffer, 1, SIZE, stdin);
            pos = 0;
            if (len == 0) return 0;
        }
        return buffer[pos++];
    }

    template<class T>
    bool readInt(T& x) {
        char c;
        do {
            c = getChar();
            if (!c) return false;
        } while (c <= ' ');

        x = 0;
        while (c >= '0' && c <= '9') {
            x = x * 10 + (c - '0');
            c = getChar();
        }
        return true;
    }
};

/*
Events in Bob's order:
  0: missing weight, fixed value
  1: fixed weight and fixed value
  2: missing value, its possible selected position (value MAX_ENTRY)
  3: missing value, its possible excluded position (value 1)
*/
struct Event {
    int value;
    int id;
    int type;
};

class SegmentTree {
    int n;
    vector<int64> mn, lazy;

    void apply(int v, int64 x) {
        mn[v] += x;
        lazy[v] += x;
    }

    void push(int v) {
        if (lazy[v] != 0) {
            apply(v * 2, lazy[v]);
            apply(v * 2 + 1, lazy[v]);
            lazy[v] = 0;
        }
    }

    void build(int v, int l, int r, const vector<int64>& a) {
        if (l == r) {
            mn[v] = a[l];
            return;
        }
        int mid = (l + r) / 2;
        build(v * 2, l, mid, a);
        build(v * 2 + 1, mid + 1, r, a);
        mn[v] = min(mn[v * 2], mn[v * 2 + 1]);
    }

    void erasePoint(int v, int l, int r, int p) {
        if (l == r) {
            mn[v] = INF;
            lazy[v] = 0;
            return;
        }
        push(v);
        int mid = (l + r) / 2;
        if (p <= mid) erasePoint(v * 2, l, mid, p);
        else erasePoint(v * 2 + 1, mid + 1, r, p);
        mn[v] = min(mn[v * 2], mn[v * 2 + 1]);
    }

    void addSuffix(int v, int l, int r, int p, int64 x) {
        if (p <= l) {
            apply(v, x);
            return;
        }
        push(v);
        int mid = (l + r) / 2;
        if (p <= mid) {
            addSuffix(v * 2, l, mid, p, x);
            apply(v * 2 + 1, x);
        } else {
            addSuffix(v * 2 + 1, mid + 1, r, p, x);
        }
        mn[v] = min(mn[v * 2], mn[v * 2 + 1]);
    }

    int minimumPosition(int v, int l, int r) {
        if (l == r) return l;
        push(v);
        int mid = (l + r) / 2;
        if (mn[v * 2] <= mn[v * 2 + 1]) {
            return minimumPosition(v * 2, l, mid);
        }
        return minimumPosition(v * 2 + 1, mid + 1, r);
    }

public:
    explicit SegmentTree(const vector<int64>& a)
        : n((int)a.size()), mn(4 * max(1, n), INF),
          lazy(4 * max(1, n), 0) {
        if (n) build(1, 0, n - 1, a);
    }

    int64 minimum() const {
        return n ? mn[1] : INF;
    }

    int minimumPosition() {
        return minimumPosition(1, 0, n - 1);
    }

    void erasePoint(int p) {
        erasePoint(1, 0, n - 1, p);
    }

    void addSuffix(int p, int64 x) {
        if (p < n) addSuffix(1, 0, n - 1, p, x);
    }
};

int main() {
    FastInput in;

    int T;
    in.readInt(T);

    string output;
    output.reserve(1 << 20);

    while (T--) {
        int n, W;
        in.readInt(n);
        in.readInt(W);

        vector<int> w(n), r(n);
        for (int& x : w) in.readInt(x);
        for (int& x : r) in.readInt(x);

        vector<int> known;
        known.reserve(n);

        vector<Event> events;
        events.reserve(2 * n);

        for (int i = 0; i < n; ++i) {
            if (w[i] == 0) {
                events.push_back({r[i], i, 0});
            } else {
                known.push_back(i);
                if (r[i] == 0) {
                    events.push_back({MAX_ENTRY, i, 2});
                    events.push_back({1, i, 3});
                } else {
                    events.push_back({r[i], i, 1});
                }
            }
        }

        sort(known.begin(), known.end(), [&](int a, int b) {
            if (w[a] != w[b]) return w[a] < w[b];
            return a < b;
        });

        sort(events.begin(), events.end(), [](const Event& a, const Event& b) {
            if (a.value != b.value) return a.value > b.value;
            return a.id < b.id;
        });

        int m = (int)known.size();

        vector<int> rank(n, -1);
        vector<int64> prefix(m + 1, 0);
        for (int i = 0; i < m; ++i) {
            rank[known[i]] = i;
            prefix[i + 1] = prefix[i] + w[known[i]];
        }

        int maxP = int(upper_bound(prefix.begin(), prefix.end(), (int64)W)
                       - prefix.begin()) - 1;

        /*
        If W == MAX_ENTRY, a selected missing-weight item need not have
        weight MAX_ENTRY. Such an item would be the only selected item,
        and reducing its weight by one preserves a valid solution.
        */
        int selectedWeightLimit = MAX_ENTRY - (W == MAX_ENTRY);

        auto capFor = [&](int id, int p) -> int {
            if (p == m) return selectedWeightLimit;
            int next = known[p];
            int cap = w[next] - (id > next);
            return min(cap, selectedWeightLimit);
        };

        /*
        For p selected known-weight items, let:
          C = total weight of selected known-weight items preceding j;
          U = sum of the allowed weights of missing-weight items
              preceding j.

        One necessary condition for every excluded known-weight item j:
                         w[j] + C + U > W.

        It is monotone in p.
        */
        auto secondCondition = [&](int p) -> bool {
            if (p == m) return true;

            int64 C = 0, U = 0;

            for (const Event& e : events) {
                int id = e.id;

                if (e.type == 0) {
                    U += capFor(id, p);
                } else if (e.type == 1) {
                    if (rank[id] < p) {
                        C += w[id];
                    } else if ((int64)w[id] + C + U <= W) {
                        return false;
                    }
                } else if (e.type == 2) {
                    if (rank[id] < p) C += w[id];
                } else {
                    if (rank[id] >= p &&
                        (int64)w[id] + C + U <= W) {
                        return false;
                    }
                }
            }
            return true;
        };

        if (!secondCondition(maxP)) {
            output += "No\n";
            continue;
        }

        int lo = 0, hi = maxP;
        while (lo < hi) {
            int mid = (lo + hi) / 2;
            if (secondCondition(mid)) hi = mid;
            else lo = mid + 1;
        }
        int p = lo;

        /*
        The other necessary condition is:
                         w[j] + C > prefix[p].

        Maintain w[j] + C for every excluded known-weight item.
        Selecting an item adds its weight to a suffix of Bob's order.
        */
        vector<int> leafOf(n, -1), suffixStart(n, -1);
        vector<int> leafIds;
        leafIds.reserve(m);

        int leafCount = 0;
        for (const Event& e : events) {
            if (e.type == 1 || e.type == 3) {
                leafOf[e.id] = leafCount++;
                leafIds.push_back(e.id);
                if (e.type == 1) suffixStart[e.id] = leafCount;
            } else if (e.type == 2) {
                suffixStart[e.id] = leafCount;
            }
        }

        vector<int64> initial(m, INF);
        int64 C = 0;

        for (const Event& e : events) {
            int id = e.id;

            if (e.type == 1) {
                if (rank[id] < p) {
                    C += w[id];
                } else {
                    initial[leafOf[id]] = (int64)w[id] + C;
                }
            } else if (e.type == 2) {
                if (rank[id] < p) C += w[id];
            } else if (e.type == 3) {
                if (rank[id] >= p) {
                    initial[leafOf[id]] = (int64)w[id] + C;
                }
            }
        }

        SegmentTree seg(initial);
        bool possible = true;

        while (seg.minimum() <= prefix[p]) {
            int badLeaf = seg.minimumPosition();
            int badItem = leafIds[badLeaf];
            int target = rank[badItem] + 1;

            /*
            This excluded item must become selected: its violated
            condition cannot improve while it remains excluded.
            */
            if (target > maxP) {
                possible = false;
                break;
            }

            while (p < target) {
                int id = known[p];
                seg.erasePoint(leafOf[id]);
                seg.addSuffix(suffixStart[id], w[id]);
                ++p;
            }
        }

        if (!possible) {
            output += "No\n";
            continue;
        }

        vector<int> answerW = w, answerR = r;
        vector<unsigned char> selected(n, 0);

        for (int i = 0; i < m; ++i) {
            int id = known[i];
            selected[id] = (i < p);
            if (answerR[id] == 0) {
                answerR[id] = selected[id] ? MAX_ENTRY : 1;
            }
        }

        /*
        Allocate the remaining weight budget to missing-weight items
        as early as possible in Bob's order.
        */
        int64 budget = (int64)W - prefix[p];

        for (const Event& e : events) {
            if (e.type != 0) continue;

            int id = e.id;
            int cap = capFor(id, p);
            int take = (int)min<int64>(budget, cap);

            if (take > 0) {
                answerW[id] = take;
                selected[id] = 1;
                budget -= take;
            } else {
                answerW[id] = MAX_ENTRY;
            }
        }

        /*
        Verify Alice without sorting:
        selected items must precede all excluded items, and the first
        excluded item must not fit after the selected prefix.
        */
        int64 totalSelected = 0;
        pair<int, int> lastSelected = {-1, -1};
        pair<int, int> firstExcluded = {MAX_ENTRY + 1, n};

        for (int i = 0; i < n; ++i) {
            pair<int, int> key = {answerW[i], i};
            if (selected[i]) {
                totalSelected += answerW[i];
                lastSelected = max(lastSelected, key);
            } else {
                firstExcluded = min(firstExcluded, key);
            }
        }

        if (totalSelected > W || lastSelected >= firstExcluded) {
            possible = false;
        }

        if (firstExcluded.first <= MAX_ENTRY &&
            totalSelected + firstExcluded.first <= W) {
            possible = false;
        }

        // Verify Bob using the already sorted event list.
        int64 remaining = W;
        if (possible) {
            for (const Event& e : events) {
                int id = e.id;

                if (e.type == 2 && !selected[id]) continue;
                if (e.type == 3 && selected[id]) continue;

                bool takes = answerW[id] <= remaining;
                if (takes != (bool)selected[id]) {
                    possible = false;
                    break;
                }
                if (takes) remaining -= answerW[id];
            }
        }

        if (!possible) {
            output += "No\n";
            continue;
        }

        output += "Yes\n";
        for (int i = 0; i < n; ++i) {
            if (i) output += ' ';
            output += to_string(answerW[i]);
        }
        output += '\n';

        for (int i = 0; i < n; ++i) {
            if (i) output += ' ';
            output += to_string(answerR[i]);
        }
        output += '\n';
    }

    fwrite(output.data(), 1, output.size(), stdout);
    return 0;
}
