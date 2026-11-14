#include <bits/stdc++.h>
using namespace std;

using ull = unsigned long long;
using ll = long long;

static constexpr ll INF = 4'000'000'000'000'000'000LL;

static ull splitmix64(ull x) {
    x += 0x9e3779b97f4a7c15ULL;
    x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9ULL;
    x = (x ^ (x >> 27)) * 0x94d049bb133111ebULL;
    return x ^ (x >> 31);
}

/*
Persistent randomized treap representing a multiset of run lengths.

The subtree hash is additive. Comparisons find the largest length whose
multiplicities differ. Hash collisions have negligible probability.

Reference counting recycles nodes that are no longer used.
*/
class Multisets {
    struct Key {
        ll value;
        ull weight, inverse, priority;
    };

    struct Node {
        ull hash;
        int left, right, key, references;
    };

    static constexpr int BLOCK_BITS = 16;
    static constexpr int BLOCK_SIZE = 1 << BLOCK_BITS;
    static constexpr int BLOCK_MASK = BLOCK_SIZE - 1;

    vector<unique_ptr<Node[]>> blocks;
    vector<Key> keys;
    int allocated = 0;
    int freeHead = 0;
    ull seed;

    Node& node(int p) {
        return blocks[p >> BLOCK_BITS][p & BLOCK_MASK];
    }

    const Node& node(int p) const {
        return blocks[p >> BLOCK_BITS][p & BLOCK_MASK];
    }

    ull weight(ll x) const {
        return splitmix64(static_cast<ull>(x) ^ seed) | 1ULL;
    }

    ull priority(ll x) const {
        return splitmix64(static_cast<ull>(x) ^ seed ^
                          0xd6e8feb86659fd93ULL);
    }

    static ull inverseOdd(ull x) {
        ull result = 1;
        for (int i = 0; i < 6; ++i)
            result *= 2 - x * result;
        return result;
    }

    int allocate() {
        if (freeHead) {
            int p = freeHead;
            freeHead = node(p).left;
            return p;
        }
        int p = ++allocated;
        if ((p >> BLOCK_BITS) >= static_cast<int>(blocks.size()))
            blocks.push_back(make_unique<Node[]>(BLOCK_SIZE));
        return p;
    }

    bool higher(int a, int b) const {
        if (keys[a].priority != keys[b].priority)
            return keys[a].priority > keys[b].priority;
        return keys[a].value > keys[b].value;
    }

    int makeNode(int left, int right, int key, int count) {
        int p = allocate();
        node(p) = {
            hash(left) + hash(right) + keys[key].weight * static_cast<ull>(count),
            left, right, key, 1
        };
        retain(left);
        retain(right);
        return p;
    }

    // The returned roots each own one reference.
    pair<int, int> split(int root, ll value) {
        if (!root) return {0, 0};

        int left = node(root).left;
        int right = node(root).right;
        int key = node(root).key;
        int count = multiplicity(root);

        if (keys[key].value < value) {
            auto [a, b] = split(right, value);
            int result = makeNode(left, a, key, count);
            release(a);
            return {result, b};
        } else {
            auto [a, b] = split(left, value);
            int result = makeNode(b, right, key, count);
            release(b);
            return {a, result};
        }
    }

    int insertImpl(int root, int key) {
        if (!root) return makeNode(0, 0, key, 1);

        int oldKey = node(root).key;
        int left = node(root).left;
        int right = node(root).right;
        int count = multiplicity(root);

        if (keys[oldKey].value == keys[key].value)
            return makeNode(left, right, oldKey, count + 1);

        if (higher(key, oldKey)) {
            auto [a, b] = split(root, keys[key].value);
            int result = makeNode(a, b, key, 1);
            release(a);
            release(b);
            return result;
        }

        if (keys[key].value < keys[oldKey].value) {
            int changed = insertImpl(left, key);
            int result = makeNode(changed, right, oldKey, count);
            release(changed);
            return result;
        } else {
            int changed = insertImpl(right, key);
            int result = makeNode(left, changed, oldKey, count);
            release(changed);
            return result;
        }
    }

    // Hash of all keys strictly greater than value.
    ull suffixHash(int root, ll value) const {
        ull result = 0;
        while (root) {
            const Node& n = node(root);
            if (keys[n.key].value > value) {
                result += n.hash - hash(n.left);
                root = n.left;
            } else {
                root = n.right;
            }
        }
        return result;
    }

    // Hash of all keys in the open interval (low, high).
    ull rangeHash(int root, ll low, ll high) const {
        ull result = suffixHash(root, low);
        if (high != INF)
            result -= suffixHash(root, high - 1);
        return result;
    }

    int normalize(int root, ll low, ll high) const {
        while (root) {
            ll value = keys[node(root).key].value;
            if (value <= low)
                root = node(root).right;
            else if (value >= high)
                root = node(root).left;
            else
                break;
        }
        return root;
    }

public:
    struct Difference {
        ll length;
        int sign;
    };

    explicit Multisets(int reserveKeys)
        : seed(chrono::steady_clock::now().time_since_epoch().count()) {
        blocks.push_back(make_unique<Node[]>(BLOCK_SIZE));
        keys.reserve(reserveKeys + 1);
        keys.push_back({0, 0, 0, 0});
    }

    ull hash(int root) const {
        return root ? node(root).hash : 0;
    }

    void retain(int root) {
        if (root) ++node(root).references;
    }

    void release(int root) {
        if (!root) return;
        if (--node(root).references) return;

        int left = node(root).left;
        int right = node(root).right;

        node(root).left = freeHead;
        freeHead = root;

        release(left);
        release(right);
    }

    int multiplicity(int root) const {
        const Node& n = node(root);
        ull own = n.hash - hash(n.left) - hash(n.right);
        return static_cast<int>(own * keys[n.key].inverse);
    }

    // Returns a new owning root.
    int insert(int root, ll length) {
        ull w = weight(length);
        int id = static_cast<int>(keys.size());
        keys.push_back({length, w, inverseOdd(w), priority(length)});
        return insertImpl(root, id);
    }

    /*
    Compare:
        multiset(a) + {extraA}, multiset(b) + {extraB}.
    An extra equal to zero means that nothing is added.

    Returns the largest differing length and the sign of its
    multiplicity difference.
    */
    Difference difference(int a, ll extraA, int b, ll extraB) const {
        ull wa = extraA ? weight(extraA) : 0;
        ull wb = extraB ? weight(extraB) : 0;

        if (hash(a) + wa == hash(b) + wb)
            return {0, 0};

        ull pa = extraA ? priority(extraA) : 0;
        ull pb = extraB ? priority(extraB) : 0;

        ll low = 0, high = INF;

        while (true) {
            a = normalize(a, low, high);
            b = normalize(b, low, high);

            bool useA = extraA > low && extraA < high;
            bool useB = extraB > low && extraB < high;

            ll pivot = 0;
            ull bestPriority = 0;
            bool found = false;

            auto consider = [&](ll value, ull pr) {
                if (!found || pr > bestPriority ||
                    (pr == bestPriority && value > pivot)) {
                    found = true;
                    pivot = value;
                    bestPriority = pr;
                }
            };

            if (a) {
                const Key& k = keys[node(a).key];
                consider(k.value, k.priority);
            }
            if (b) {
                const Key& k = keys[node(b).key];
                consider(k.value, k.priority);
            }
            if (useA) consider(extraA, pa);
            if (useB) consider(extraB, pb);

            ull rightA = rangeHash(a, pivot, high);
            ull rightB = rangeHash(b, pivot, high);

            if (useA && extraA > pivot) rightA += wa;
            if (useB && extraB > pivot) rightB += wb;

            if (rightA != rightB) {
                low = pivot;
                continue;
            }

            int countA = 0, countB = 0;
            if (a && keys[node(a).key].value == pivot)
                countA = multiplicity(a);
            if (b && keys[node(b).key].value == pivot)
                countB = multiplicity(b);

            if (useA && extraA == pivot) ++countA;
            if (useB && extraB == pivot) ++countB;

            if (countA != countB)
                return {pivot, countA > countB ? 1 : -1};

            high = pivot;
        }
    }

    vector<ll> extract(int root) const {
        vector<ll> result;
        vector<int> stack;

        while (root || !stack.empty()) {
            while (root) {
                stack.push_back(root);
                root = node(root).right;
            }

            root = stack.back();
            stack.pop_back();

            ll value = keys[node(root).key].value;
            int count = multiplicity(root);
            for (int i = 0; i < count; ++i)
                result.push_back(value);

            root = node(root).left;
        }
        return result;
    }
};

/*
For a candidate starting at s, its value at x is
    base + F(x-s).

Candidates in the deque describe the upper envelope for future integer x.
The front candidate is optimal until its 'until' coordinate.
*/
class Envelope {
public:
    struct Candidate {
        ll start;
        int root;
        ll until;
    };

private:
    Multisets& sets;
    deque<Candidate> candidates;

    ll firstOldWins(const Candidate& old, ll newStart, int newRoot) {
        auto d = sets.difference(old.root, 0, newRoot, 0);

        if (d.sign == 0)
            return newStart + 1;

        ll crossing = old.start + d.length;

        if (crossing <= newStart)
            return newStart + 1;

        int comparison = sets.difference(
            old.root, d.length,
            newRoot, crossing - newStart
        ).sign;

        return crossing + (comparison < 0);
    }

    void popFront() {
        sets.release(candidates.front().root);
        candidates.pop_front();
    }

public:
    explicit Envelope(Multisets& sets) : sets(sets) {}

    void clear() {
        while (!candidates.empty())
            popFront();
    }

    const Candidate& query(ll x) {
        while (candidates.front().until <= x)
            popFront();
        return candidates.front();
    }

    void add(ll start, int root) {
        while (!candidates.empty()) {
            ll crossing = firstOldWins(candidates.front(), start, root);

            // The new candidate is never better at a future integer position.
            if (crossing <= start + 1)
                return;

            if (crossing >= candidates.front().until) {
                popFront();
            } else {
                sets.retain(root);
                candidates.push_front({start, root, crossing});
                return;
            }
        }

        sets.retain(root);
        candidates.push_front({start, root, INF});
    }
};

struct Event {
    ll position;
    int column;
    int type; // +1: start, -1: end
    bool operator<(const Event& other) const {
        return position < other.position;
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int tests;
    cin >> tests;

    while (tests--) {
        int n, m;
        cin >> n >> m;

        vector<Event> events;
        events.reserve(2 * (n + m));

        for (int column = 0; column < 2; ++column) {
            int count = column == 0 ? n : m;
            for (int i = 0; i < count; ++i) {
                ll l, r;
                cin >> l >> r;
                events.push_back({l, column, +1});
                events.push_back({r + 1, column, -1});
            }
        }

        sort(events.begin(), events.end());

        Multisets sets(static_cast<int>(events.size()));
        Envelope first(sets), second(sets);
        Envelope* envelope[2] = {&first, &second};

        bool active[2] = {false, false};
        int dp = 0;

        int i = 0;
        while (i < static_cast<int>(events.size())) {
            ll x = events[i].position;
            int j = i;
            while (j < static_cast<int>(events.size()) &&
                   events[j].position == x)
                ++j;

            // Finish the best selection strictly before coordinate x.
            int bestRoot = dp;
            ll bestExtra = 0;

            for (int column = 0; column < 2; ++column) {
                if (!active[column]) continue;

                const auto& candidate = envelope[column]->query(x);
                ll length = x - candidate.start;

                if (sets.difference(
                        candidate.root, length,
                        bestRoot, bestExtra
                    ).sign > 0) {
                    bestRoot = candidate.root;
                    bestExtra = length;
                }
            }

            if (bestExtra) {
                int next = sets.insert(bestRoot, bestExtra);
                sets.release(dp);
                dp = next;
            }

            // Update which original black intervals are active.
            for (int k = i; k < j; ++k) {
                if (events[k].type == -1) {
                    int column = events[k].column;
                    envelope[column]->clear();
                    active[column] = false;
                }
            }
            for (int k = i; k < j; ++k) {
                if (events[k].type == +1)
                    active[events[k].column] = true;
            }

            // A new selected run may start at this event coordinate.
            for (int column = 0; column < 2; ++column) {
                if (active[column])
                    envelope[column]->add(x, dp);
            }

            i = j;
        }

        vector<ll> answer = sets.extract(dp);
        cout << answer.size() << '\n';
        for (int k = 0; k < static_cast<int>(answer.size()); ++k) {
            if (k) cout << ' ';
            cout << answer[k];
        }
        cout << '\n';

        first.clear();
        second.clear();
        sets.release(dp);
    }

    return 0;
}
