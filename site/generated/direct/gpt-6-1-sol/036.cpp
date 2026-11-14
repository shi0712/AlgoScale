#include <bits/stdc++.h>
using namespace std;

class FastInput {
    static constexpr size_t SIZE = 1 << 16;
    char buffer[SIZE];
    size_t pos = 0, len = 0;

    char getChar() {
        if (pos == len) {
            len = fread(buffer, 1, SIZE, stdin);
            pos = 0;
            if (len == 0) return 0;
        }
        return buffer[pos++];
    }

public:
    int nextInt() {
        char c;
        do {
            c = getChar();
        } while (c <= ' ' && c != 0);

        int value = 0;
        while (c >= '0' && c <= '9') {
            value = value * 10 + (c - '0');
            c = getChar();
        }
        return value;
    }
};

struct Event {
    int weight;
    int u, v;
};

struct TownEdge {
    int u, v;
    unsigned endpoints;
    int weight;
};

struct DSU {
    vector<int> parent;

    explicit DSU(int n) : parent(n, -1) {}

    int find(int x) {
        if (parent[x] < 0) return x;
        return parent[x] = find(parent[x]);
    }
};

static inline int smallFind(int* parent, int x) {
    while (parent[x] != x) {
        parent[x] = parent[parent[x]];
        x = parent[x];
    }
    return x;
}

int main() {
    FastInput in;

    const int n = in.nextInt();
    const int m = in.nextInt();
    const int k = in.nextInt();

    vector<Event> events;
    events.reserve(static_cast<size_t>(m) + static_cast<size_t>(n) * k);

    for (int i = 0; i < m; ++i) {
        int u = in.nextInt() - 1;
        int v = in.nextInt() - 1;
        int w = in.nextInt();
        events.push_back({w, u, v});
    }

    vector<long long> activation(k);
    for (int j = 0; j < k; ++j) {
        activation[j] = in.nextInt();
        for (int i = 0; i < n; ++i) {
            int a = in.nextInt();
            events.push_back({a, i, n + j});
        }
    }

    sort(events.begin(), events.end(),
         [](const Event& a, const Event& b) {
             return a.weight < b.weight;
         });

    const unsigned states = 1u << k;
    const unsigned full = states - 1;
    constexpr int MAX_WEIGHT = 1'000'000'000;

    DSU dsu(n);

    // For each original-city component:
    // mask = towns having an available edge into it.
    // last = time when its current mask interval began.
    vector<unsigned> mask(n, 0);
    vector<int> last(n, 0);

    // Initially this stores integrals for exact masks.
    vector<long long> histogram(states, 0);

    // missing[j] contains towns whose pair threshold with j is not known yet.
    vector<unsigned> missing(k);
    for (int j = 0; j < k; ++j) {
        missing[j] = full ^ (1u << j);
    }

    vector<TownEdge> townEdges;
    townEdges.reserve(k * (k - 1) / 2);

    // Events are processed by increasing weight, so townEdges is also sorted.
    auto discoverPairs = [&](int j, unsigned partners, int weight) {
        missing[j] &= ~partners;
        while (partners) {
            int l = __builtin_ctz(partners);
            partners &= partners - 1;

            missing[l] &= ~(1u << j);
            townEdges.push_back({
                j, l, (1u << j) | (1u << l), weight
            });
        }
    };

    long long originalMST = 0;

    for (const Event& e : events) {
        if (e.v < n) {
            // An original road becomes available.
            int a = dsu.find(e.u);
            int b = dsu.find(e.v);
            if (a == b) continue;

            histogram[mask[a]] += static_cast<long long>(e.weight) - last[a];
            histogram[mask[b]] += static_cast<long long>(e.weight) - last[b];

            unsigned left = mask[a] & ~mask[b];
            unsigned right = mask[b] & ~mask[a];

            while (left) {
                int j = __builtin_ctz(left);
                left &= left - 1;

                unsigned partners = right & missing[j];
                if (partners) discoverPairs(j, partners, e.weight);
            }

            if (dsu.parent[a] > dsu.parent[b]) swap(a, b);
            dsu.parent[a] += dsu.parent[b];
            dsu.parent[b] = a;

            mask[a] |= mask[b];
            last[a] = e.weight;
            originalMST += e.weight;
        } else {
            // A town-to-city edge becomes available.
            int j = e.v - n;
            int root = dsu.find(e.u);
            unsigned bit = 1u << j;

            if (mask[root] & bit) continue;

            histogram[mask[root]] +=
                static_cast<long long>(e.weight) - last[root];

            unsigned partners = mask[root] & missing[j];
            if (partners) discoverPairs(j, partners, e.weight);

            mask[root] |= bit;
            last[root] = e.weight;
        }
    }

    for (int i = 0; i < n; ++i) {
        if (dsu.parent[i] < 0) {
            histogram[mask[i]] +=
                static_cast<long long>(MAX_WEIGHT) - last[i];
        }
    }

    // Subset zeta transform:
    // histogram[T] = sum of the original histogram[M] over M subset of T.
    for (int bit = 0; bit < k; ++bit) {
        unsigned step = 1u << bit;
        for (unsigned base = 0; base < states; base += step << 1) {
            for (unsigned offset = 0; offset < step; ++offset) {
                histogram[base + step + offset] += histogram[base + offset];
            }
        }
    }

    vector<long long> activationSum(states, 0);
    for (unsigned s = 1; s < states; ++s) {
        unsigned bit = s & -s;
        int j = __builtin_ctz(bit);
        activationSum[s] = activationSum[s ^ bit] + activation[j];
    }

    long long answer = originalMST;

    for (unsigned s = 1; s < states; ++s) {
        long long candidate = histogram[full ^ s] + activationSum[s];
        if (candidate >= answer) continue;

        int components = __builtin_popcount(s);
        if (components > 1) {
            int parent[16];
            for (int j = 0; j < k; ++j) parent[j] = j;

            for (const TownEdge& e : townEdges) {
                if ((s & e.endpoints) != e.endpoints) continue;

                int a = smallFind(parent, e.u);
                int b = smallFind(parent, e.v);
                if (a == b) continue;

                parent[a] = b;
                candidate += e.weight;

                if (--components == 1) break;
            }
        }

        answer = min(answer, candidate);
    }

    printf("%lld\n", answer);
    return 0;
}
