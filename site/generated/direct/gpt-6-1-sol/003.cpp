#include <bits/stdc++.h>
using namespace std;

static constexpr int MOD = 998244353;

static inline int addmod(int a, int b) {
    int s = a + b;
    if (s >= MOD) s -= MOD;
    return s;
}

static inline int submod(int a, int b) {
    int s = a - b;
    if (s < 0) s += MOD;
    return s;
}

static inline int mulmod(int a, int b) {
    return int(1LL * a * b % MOD);
}

static int modpow(int a, int e) {
    int r = 1;
    while (e) {
        if (e & 1) r = mulmod(r, a);
        a = mulmod(a, a);
        e >>= 1;
    }
    return r;
}

// Upper-triangular matrix:
// [ p r ]
// [ 0 q ]
struct Tri {
    int p = 1, q = 1, r = 0;

    bool operator==(const Tri& other) const {
        return p == other.p && q == other.q && r == other.r;
    }
};

static inline Tri multiply(const Tri& a, const Tri& b) {
    return {
        mulmod(a.p, b.p),
        mulmod(a.q, b.q),
        int((1LL * a.p * b.r + 1LL * a.r * b.q) % MOD)
    };
}

// Affine transformation of (p, q, r):
// p' = ap * p + bp
// q' = aq * q + bq
// r' = cp * p + cq * q + ar * r + br
struct Aff {
    int ap, bp, aq, bq, cp, cq, ar, br;

    bool operator==(const Aff& other) const {
        return memcmp(this, &other, sizeof(Aff)) == 0;
    }
};

static inline Aff compose(const Aff& a, const Aff& b) {
    return {
        mulmod(a.ap, b.ap),
        int((1LL * a.ap * b.bp + a.bp) % MOD),
        mulmod(a.aq, b.aq),
        int((1LL * a.aq * b.bq + a.bq) % MOD),
        int((1LL * a.cp * b.ap + 1LL * a.ar * b.cp) % MOD),
        int((1LL * a.cq * b.aq + 1LL * a.ar * b.cq) % MOD),
        mulmod(a.ar, b.ar),
        int((1LL * a.cp * b.bp + 1LL * a.cq * b.bq +
             1LL * a.ar * b.br + a.br) % MOD)
    };
}

static inline Tri evaluate(const Aff& a) {
    // Apply to the empty forest: (1, 1, 0).
    return {
        addmod(a.ap, a.bp),
        addmod(a.aq, a.bq),
        int((1LL * a.cp + a.cq + a.br) % MOD)
    };
}

struct Node {
    int parent = -1, heavy = -1, size = 1, height = -1;
    int first = -1, last = -1, next = -1;
    int leftRoot = -1, rightRoot = -1;
    int affLeaf = -1, triLeaf = -1;

    // kind / 2:
    // 0 = inactive, 1 = boundary, 2 = active, 3 = dummy.
    // kind % 2 indicates an element from the initial prefix.
    int kind = 6;

    Tri dp;
    int marked = 0;
};

struct TriSegment {
    Tri value;
    int left = -1, right = -1, parent = -1, owner = -1;
};

struct AffSegment {
    Aff value;
    int left = -1, right = -1, parent = -1, owner = -1;
};

// Counts valid ways to process every prefix of a fixed word.
// Entries equal to 1 do not append anything.
struct CoverCounter {
    vector<pair<int, int>> stk;
    int sum = 0, complete = 1, total = 1, trailingOnes = 0;

    void clear() {
        stk.clear();
        sum = 0;
        complete = 1;
        total = 1;
        trailingOnes = 0;
    }

    void feed(int h) {
        int before = sum;
        while (!stk.empty() && stk.back().first < h) stk.pop_back();

        if (h >= 2) {
            complete = stk.empty()
                ? addmod(sum, 1)
                : submod(sum, stk.back().second);
            sum = addmod(sum, complete);
            trailingOnes = 0;
        } else {
            ++trailingOnes;
        }

        total = addmod(total, complete);
        stk.emplace_back(h, before);
    }
};

// Counts distinct valid prunings of a word whose entries are positive.
struct ChoiceCounter {
    vector<pair<int, int>> stk;
    int sum = 0, leadingOnes = 0;
    bool seenLarger = false;

    void clear() {
        stk.clear();
        sum = 0;
        leadingOnes = 0;
        seenLarger = false;
    }

    void feed(int h) {
        int before = sum;
        while (!stk.empty() && stk.back().first < h) stk.pop_back();

        int ways = stk.empty()
            ? addmod(sum, 1)
            : submod(sum, stk.back().second);

        sum = addmod(sum, ways);
        stk.emplace_back(h, before);

        if (!seenLarger && h == 1) ++leadingOnes;
        if (h > 1) seenLarger = true;
    }

    int count() const {
        return addmod(sum, 1);
    }
};

class Solver {
    vector<Node> nodes;
    vector<TriSegment> triSeg;
    vector<AffSegment> affSeg;

    vector<vector<int>> byHeight;
    vector<int> initialChoices, initialBase;

    int roots = 0;
    int globalRoot = -1;

    Tri lightValue(int root) const {
        return root == -1 ? Tri{} : triSeg[root].value;
    }

    // Weighted balancing is important: it makes the total cost of
    // propagating one change through all heavy/light boundaries O(log n).
    static int splitPoint(const vector<int>& pref, int l, int r) {
        int target = pref[l] + (pref[r] - pref[l]) / 2;

        int mid = int(lower_bound(
            pref.begin() + l + 1, pref.begin() + r, target
        ) - pref.begin());

        if (mid >= r) mid = r - 1;

        if (mid > l + 1) {
            long long whole = pref[r] - pref[l];
            long long d1 = llabs(2LL * (pref[mid] - pref[l]) - whole);
            long long d2 = llabs(2LL * (pref[mid - 1] - pref[l]) - whole);
            if (d2 < d1) --mid;
        }

        return mid;
    }

    int buildTriRec(
        const vector<int>& ids,
        const vector<int>& pref,
        int l, int r,
        bool global
    ) {
        if (r - l == 1) {
            int u = ids[l];
            Tri value = nodes[u].dp;
            if (global) value.r = addmod(value.q, value.r);

            int id = int(triSeg.size());
            triSeg.push_back({value, -1, -1, -1, -1});
            nodes[u].triLeaf = id;
            return id;
        }

        int mid = splitPoint(pref, l, r);
        int a = buildTriRec(ids, pref, l, mid, global);
        int b = buildTriRec(ids, pref, mid, r, global);

        int id = int(triSeg.size());
        triSeg.push_back({
            multiply(triSeg[a].value, triSeg[b].value),
            a, b, -1, -1
        });

        triSeg[a].parent = id;
        triSeg[b].parent = id;
        return id;
    }

    int buildTri(const vector<int>& ids, bool global, int owner) {
        if (ids.empty()) return -1;

        vector<int> pref(ids.size() + 1, 0);
        for (int i = 0; i < int(ids.size()); ++i) {
            pref[i + 1] = pref[i] + nodes[ids[i]].size;
        }

        int root = buildTriRec(ids, pref, 0, int(ids.size()), global);
        triSeg[root].owner = owner;
        return root;
    }

    Aff coefficients(int u) const {
        const Node& x = nodes[u];
        Tri a = lightValue(x.leftRoot);
        Tri b = lightValue(x.rightRoot);

        int ap = mulmod(a.p, b.p);
        int aq = mulmod(a.q, b.q);
        int cp = mulmod(a.p, b.r);
        int cq = mulmod(a.r, b.q);
        int ar = mulmod(a.p, b.q);

        int state = x.kind >> 1;

        if (state == 3) {
            return {ap, 0, aq, 0, cp, cq, ar, 0};
        }
        if (state == 2) {
            return {ap, 1, aq, 1, cp, addmod(cq, aq), ar, 0};
        }
        if (state == 1) {
            return {0, 1, aq, 1, 0, 0, 0, 0};
        }
        return {0, 1, 0, 1, 0, 0, 0, 0};
    }

    int buildAffRec(
        const vector<int>& ids,
        const vector<int>& pref,
        int l, int r
    ) {
        if (r - l == 1) {
            int u = ids[l];
            int id = int(affSeg.size());
            affSeg.push_back({coefficients(u), -1, -1, -1, -1});
            nodes[u].affLeaf = id;
            return id;
        }

        int mid = splitPoint(pref, l, r);
        int a = buildAffRec(ids, pref, l, mid);
        int b = buildAffRec(ids, pref, mid, r);

        int id = int(affSeg.size());
        affSeg.push_back({
            compose(affSeg[a].value, affSeg[b].value),
            a, b, -1, -1
        });

        affSeg[a].parent = id;
        affSeg[b].parent = id;
        return id;
    }

    void buildAffChain(const vector<int>& ids) {
        vector<int> pref(ids.size() + 1, 0);

        for (int i = 0; i < int(ids.size()); ++i) {
            int u = ids[i];
            int weight = nodes[u].size;
            if (nodes[u].heavy != -1) {
                weight -= nodes[nodes[u].heavy].size;
            }
            pref[i + 1] = pref[i] + weight;
        }

        int root = buildAffRec(ids, pref, 0, int(ids.size()));
        affSeg[root].owner = ids[0];
    }

    void changeNode(int u) {
        while (true) {
            int pos = nodes[u].affLeaf;
            Aff value = coefficients(u);
            if (affSeg[pos].value == value) return;
            affSeg[pos].value = value;

            while (affSeg[pos].parent != -1) {
                pos = affSeg[pos].parent;
                const auto& s = affSeg[pos];
                value = compose(
                    affSeg[s.left].value,
                    affSeg[s.right].value
                );

                if (affSeg[pos].value == value) return;
                affSeg[pos].value = value;
            }

            int head = affSeg[pos].owner;
            Tri dp = evaluate(affSeg[pos].value);

            if (nodes[head].dp == dp) return;
            nodes[head].dp = dp;

            if (nodes[head].height == -1) {
                dp.r = addmod(dp.q, dp.r);
            }

            pos = nodes[head].triLeaf;
            if (triSeg[pos].value == dp) return;
            triSeg[pos].value = dp;

            while (triSeg[pos].parent != -1) {
                pos = triSeg[pos].parent;
                const auto& s = triSeg[pos];
                dp = multiply(
                    triSeg[s.left].value,
                    triSeg[s.right].value
                );

                if (triSeg[pos].value == dp) return;
                triSeg[pos].value = dp;
            }

            int owner = triSeg[pos].owner;
            if (owner == -2) return;
            u = owner;
        }
    }

public:
    int solve(const vector<int>& a) {
        int n = int(a.size());
        int maximum = *max_element(a.begin(), a.end());

        if (maximum <= 2) {
            long long answer = 0;
            for (int x : a) answer += x;
            return int(answer % MOD);
        }

        int anchor = n - 1;
        while (a[anchor] != maximum) --anchor;
        int level = maximum - 1;

        CoverCounter before;
        for (int i = 0; i < anchor; ++i) before.feed(a[i]);
        int answer = before.total;

        for (int x : a) {
            if (x == maximum) ++roots;
        }
        for (int i = anchor + 1; i < n; ++i) {
            if (a[i] == level) ++roots;
        }

        nodes.reserve(n);
        nodes.resize(roots);
        byHeight.resize(maximum);
        initialChoices.resize(roots);
        initialBase.resize(roots);

        CoverCounter fixed;
        ChoiceCounter freelyPrunable;
        vector<int> forestStack;

        int gap = 0;

        auto finishGap = [&]() {
            int choices = freelyPrunable.count();
            int base = mulmod(fixed.total, choices);

            // A trailing run of fixed ones can merge with a freely
            // prunable leading run of ones. Remove that overlap.
            if (fixed.trailingOnes && freelyPrunable.leadingOnes) {
                int rest = mulmod(
                    choices,
                    modpow(freelyPrunable.leadingOnes + 1, MOD - 2)
                );

                int overlap = mulmod(
                    fixed.complete,
                    mulmod(
                        fixed.trailingOnes,
                        mulmod(freelyPrunable.leadingOnes, rest)
                    )
                );
                base = submod(base, overlap);
            }

            initialChoices[gap] = choices;
            initialBase[gap] = base;

            ++gap;
            fixed.clear();
            freelyPrunable.clear();
            forestStack.clear();
        };

        auto consume = [&](int value, bool prefix) {
            int h = value - int(prefix);

            if (h == level) {
                finishGap();
                return;
            }
            if (h == 0) return;

            if (prefix) freelyPrunable.feed(h);
            else fixed.feed(h);

            while (!forestStack.empty() &&
                   nodes[forestStack.back()].height < h) {
                forestStack.pop_back();
            }

            int parent = forestStack.empty() ? gap : forestStack.back();

            Node x;
            x.parent = parent;
            x.height = h;
            x.kind = 2 * (h == 1 ? 1 : 2) + int(prefix);

            int id = int(nodes.size());
            nodes.push_back(x);

            if (nodes[parent].first == -1) {
                nodes[parent].first = id;
            } else {
                nodes[nodes[parent].last].next = id;
            }
            nodes[parent].last = id;

            forestStack.push_back(id);
            byHeight[h].push_back(id);
        };

        for (int i = anchor + 1; i < n; ++i) consume(a[i], false);
        for (int i = 0; i < anchor; ++i) consume(a[i], true);
        finishGap(); // The anchor itself is the final mandatory root.

        int count = int(nodes.size());

        for (int u = count - 1; u >= 0; --u) {
            int p = nodes[u].parent;
            if (p != -1) {
                nodes[p].size += nodes[u].size;
                if (nodes[p].heavy == -1 ||
                    nodes[u].size > nodes[nodes[p].heavy].size) {
                    nodes[p].heavy = u;
                }
            }
        }

        int heads = 0;
        for (int u = 0; u < count; ++u) {
            int p = nodes[u].parent;
            if (p == -1 || nodes[p].heavy != u) ++heads;
        }

        triSeg.reserve(2 * heads);
        affSeg.reserve(2 * count - heads);

        vector<int> leftChildren, rightChildren;

        for (int u = count - 1; u >= 0; --u) {
            leftChildren.clear();
            rightChildren.clear();

            bool afterHeavy = false;
            Tri children, markedChildren;

            for (int v = nodes[u].first; v != -1; v = nodes[v].next) {
                children = multiply(children, nodes[v].dp);
                markedChildren = multiply(
                    markedChildren,
                    Tri{nodes[v].dp.p, nodes[v].dp.q, nodes[v].marked}
                );

                if (v == nodes[u].heavy) {
                    afterHeavy = true;
                } else if (!afterHeavy) {
                    leftChildren.push_back(v);
                } else {
                    rightChildren.push_back(v);
                }
            }

            nodes[u].leftRoot = buildTri(leftChildren, false, u);
            nodes[u].rightRoot = buildTri(rightChildren, false, u);

            int state = nodes[u].kind >> 1;
            if (state == 3) {
                nodes[u].dp = children;
                nodes[u].marked = markedChildren.r;
            } else if (state == 2) {
                nodes[u].dp = {
                    addmod(children.p, 1),
                    addmod(children.q, 1),
                    addmod(children.q, children.r)
                };
                nodes[u].marked = markedChildren.r;
                if (nodes[u].kind & 1) {
                    nodes[u].marked =
                        addmod(nodes[u].marked, children.q);
                }
            } else {
                nodes[u].dp = {1, addmod(children.q, 1), 0};
                nodes[u].marked = 0;
            }
        }

        Tri initialProduct;
        for (int u = 0; u < roots; ++u) {
            int ways = addmod(initialBase[u], nodes[u].marked);
            initialProduct = multiply(
                initialProduct,
                Tri{nodes[u].dp.p, initialChoices[u], ways}
            );
        }

        answer = addmod(answer, initialProduct.r);
        answer = addmod(answer, roots);

        vector<int> chain;
        for (int u = 0; u < count; ++u) {
            int p = nodes[u].parent;
            if (p != -1 && nodes[p].heavy == u) continue;

            chain.clear();
            for (int v = u; v != -1; v = nodes[v].heavy) {
                chain.push_back(v);
            }
            buildAffChain(chain);
        }

        vector<int> rootIds(roots);
        iota(rootIds.begin(), rootIds.end(), 0);
        globalRoot = buildTri(rootIds, true, -2);

        for (int threshold = 1; threshold <= level - 2; ++threshold) {
            for (int u : byHeight[threshold]) {
                nodes[u].kind &= 1; // Boundary -> inactive.
                changeNode(u);
            }

            for (int u : byHeight[threshold + 1]) {
                nodes[u].kind = 2 + (nodes[u].kind & 1);
                changeNode(u); // Active -> boundary.
            }

            answer = addmod(answer, triSeg[globalRoot].value.r);
        }

        return answer;
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int c, T;
    cin >> c >> T;

    while (T--) {
        int n, m;
        cin >> n >> m;

        vector<int> a(n);
        for (int& x : a) cin >> x;

        Solver solver;
        cout << solver.solve(a) << '\n';
    }

    return 0;
}
