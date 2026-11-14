#include <bits/stdc++.h>
using namespace std;

using ll = long long;
using u64 = uint64_t;

struct Event {
    ll pos;
    int delta;
    bool operator<(const Event& other) const {
        return pos < other.pos;
    }
};

struct Segment {
    int coverage;             // 1: column 1, 2: column 2, 3: both
    ll len;
};

static u64 splitmix64(u64& state) {
    u64 z = (state += 0x9e3779b97f4a7c15ULL);
    z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
    z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
    return z ^ (z >> 31);
}

// Persistent multiset of run lengths, ordered by compressed length index.
// Fingerprints allow the highest differing length to be found in O(log U).
class MultisetDS {
    struct Node {
        int l = 0, r = 0;
        u64 hash = 0;
    };

    const vector<ll>& values;
    int U;
    vector<u64> rnd, inv;
    vector<Node> tr;

    int insertRec(int previous, int l, int r, int id) {
        Node copy = tr[previous];
        int cur = (int)tr.size();
        tr.push_back(copy);

        if (l == r) {
            tr[cur].hash += rnd[id];
            return cur;
        }

        int mid = (l + r) >> 1;
        if (id <= mid) {
            int child = insertRec(tr[cur].l, l, mid, id);
            tr[cur].l = child;
        } else {
            int child = insertRec(tr[cur].r, mid + 1, r, id);
            tr[cur].r = child;
        }
        tr[cur].hash = tr[tr[cur].l].hash + tr[tr[cur].r].hash;
        return cur;
    }

    void collectRec(int root, int l, int r, vector<ll>& out) const {
        if (root == 0) return;
        if (l == r) {
            // At a leaf, hash = count * rnd[l] (mod 2^64).
            // rnd[l] is odd, hence invertible modulo 2^64.
            u64 count = tr[root].hash * inv[l];
            for (u64 i = 0; i < count; ++i) out.push_back(values[l]);
            return;
        }
        int mid = (l + r) >> 1;
        collectRec(tr[root].l, l, mid, out);
        collectRec(tr[root].r, mid + 1, r, out);
    }

public:
    MultisetDS(const vector<ll>& vals, int k)
        : values(vals), U((int)vals.size()), rnd(U), inv(U) {
        u64 state = 0x123456789abcdef0ULL ^ (u64)U ^ ((u64)k << 32);
        for (int i = 0; i < U; ++i) {
            rnd[i] = splitmix64(state) | 1ULL;

            // Newton iteration for the inverse modulo 2^64.
            u64 x = 1;
            for (int it = 0; it < 6; ++it)
                x *= 2ULL - rnd[i] * x;
            inv[i] = x;
        }

        int depth = 0;
        while ((1LL << depth) < U) ++depth;
        size_t estimated = (size_t)(2 * k + 4) * (depth + 2);
        tr.reserve(estimated);
        tr.push_back(Node{}); // empty node
    }

    int insert(int root, int id) {
        return insertRec(root, 0, U - 1, id);
    }

    // Compares multiset(root1) + id1 with multiset(root2) + id2.
    // Returns 1, 0, or -1. Larger means lexicographically larger when
    // lengths are written in non-increasing order.
    int compareAugmented(int root1, int id1, int root2, int id2) const {
        int l = 0, r = U - 1;

        while (true) {
            u64 h1 = tr[root1].hash;
            u64 h2 = tr[root2].hash;

            if (l <= id1 && id1 <= r) h1 += rnd[id1];
            if (l <= id2 && id2 <= r) h2 += rnd[id2];

            if (h1 == h2) return 0;
            if (l == r) break;

            int mid = (l + r) >> 1;
            int right1 = tr[root1].r;
            int right2 = tr[root2].r;

            u64 hr1 = tr[right1].hash;
            u64 hr2 = tr[right2].hash;

            if (id1 > mid && id1 <= r) hr1 += rnd[id1];
            if (id2 > mid && id2 <= r) hr2 += rnd[id2];

            if (hr1 != hr2) {
                root1 = right1;
                root2 = right2;
                l = mid + 1;
            } else {
                root1 = tr[root1].l;
                root2 = tr[root2].l;
                r = mid;
            }
        }

        u64 count1 = tr[root1].hash * inv[l] + (id1 == l);
        u64 count2 = tr[root2].hash * inv[l] + (id2 == l);

        if (count1 == count2) return 0;
        return count1 > count2 ? 1 : -1;
    }

    void collect(int root, vector<ll>& out) const {
        collectRec(root, 0, U - 1, out);
    }
};

void solveComponent(const vector<Segment>& segments, vector<ll>& answer) {
    vector<ll> a, gaps;
    ll pendingFree = 0;
    ll base = 0;
    int anchorColor = -1;

    for (const Segment& seg : segments) {
        if (seg.coverage == 3) {
            pendingFree += seg.len;
            continue;
        }

        int color = seg.coverage;
        if (anchorColor == -1) {
            anchorColor = color;
            base = pendingFree + seg.len;
            pendingFree = 0;
        } else if (color == anchorColor) {
            // Free cells between equal forced colors join everything into
            // one longer run.
            base += pendingFree + seg.len;
            pendingFree = 0;
        } else {
            a.push_back(base);
            gaps.push_back(pendingFree);
            base = seg.len;
            pendingFree = 0;
            anchorColor = color;
        }
    }

    if (anchorColor == -1) {
        // The whole component consists of doubly black rows.
        answer.push_back(pendingFree);
        return;
    }

    base += pendingFree;
    a.push_back(base);

    int k = (int)a.size();
    if (k == 1) {
        answer.push_back(a[0]);
        return;
    }

    // All lengths that can occur in any DP transition.
    vector<ll> possible;
    possible.reserve(4 * k);

    possible.push_back(a[0]);
    possible.push_back(a[0] + gaps[0]);

    for (int i = 1; i <= k - 2; ++i) {
        possible.push_back(a[i]);
        possible.push_back(a[i] + gaps[i - 1]);
        possible.push_back(a[i] + gaps[i]);
        possible.push_back(a[i] + gaps[i - 1] + gaps[i]);
    }

    possible.push_back(a[k - 1]);
    possible.push_back(a[k - 1] + gaps[k - 2]);

    sort(possible.begin(), possible.end());
    possible.erase(unique(possible.begin(), possible.end()), possible.end());

    auto getId = [&](ll x) {
        return int(lower_bound(possible.begin(), possible.end(), x)
                   - possible.begin());
    };

    MultisetDS ds(possible, k);

    // State q describes the orientation of the gap immediately to the right:
    // q = 0: oriented left, q = 1: oriented right.
    array<int, 2> dp{}, nextDp{};

    int firstLeft = getId(a[0] + gaps[0]);
    int firstRight = getId(a[0]);
    dp[0] = ds.insert(0, firstLeft);
    dp[1] = ds.insert(0, firstRight);

    for (int i = 1; i <= k - 2; ++i) {
        int ids[2][2];

        for (int p = 0; p <= 1; ++p) {
            for (int q = 0; q <= 1; ++q) {
                ll len = a[i];
                if (p == 1) len += gaps[i - 1];
                if (q == 0) len += gaps[i];
                ids[p][q] = getId(len);
            }
        }

        for (int q = 0; q <= 1; ++q) {
            int cmp = ds.compareAugmented(
                dp[0], ids[0][q],
                dp[1], ids[1][q]
            );
            int bestPrevious = (cmp >= 0 ? 0 : 1);
            nextDp[q] = ds.insert(
                dp[bestPrevious],
                ids[bestPrevious][q]
            );
        }
        dp = nextDp;
    }

    int lastIds[2];
    lastIds[0] = getId(a[k - 1]);
    lastIds[1] = getId(a[k - 1] + gaps[k - 2]);

    int cmp = ds.compareAugmented(
        dp[0], lastIds[0],
        dp[1], lastIds[1]
    );
    int best = (cmp >= 0 ? 0 : 1);
    int finalRoot = ds.insert(dp[best], lastIds[best]);

    ds.collect(finalRoot, answer);
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int t;
    cin >> t;

    while (t--) {
        int n, m;
        cin >> n >> m;

        vector<Event> events;
        events.reserve(2 * (n + m));

        for (int i = 0; i < n; ++i) {
            ll l, r;
            cin >> l >> r;
            events.push_back({l, 1});
            events.push_back({r + 1, -1});
        }
        for (int i = 0; i < m; ++i) {
            ll l, r;
            cin >> l >> r;
            events.push_back({l, 2});
            events.push_back({r + 1, -2});
        }

        sort(events.begin(), events.end());

        vector<ll> answer;
        vector<Segment> component;

        auto flushComponent = [&]() {
            if (component.empty()) return;
            solveComponent(component, answer);
            component.clear();
        };

        size_t p = 0;
        ll previous = events[0].pos;
        int coverage = 0;

        while (p < events.size()) {
            ll pos = events[p].pos;

            if (pos > previous) {
                if (coverage == 0) {
                    flushComponent();
                } else {
                    component.push_back({coverage, pos - previous});
                }
            }

            while (p < events.size() && events[p].pos == pos) {
                coverage += events[p].delta;
                ++p;
            }
            previous = pos;
        }
        flushComponent();

        sort(answer.begin(), answer.end(), greater<ll>());

        cout << answer.size() << '\n';
        for (size_t i = 0; i < answer.size(); ++i) {
            if (i) cout << ' ';
            cout << answer[i];
        }
        cout << '\n';
    }

    return 0;
}
