#include <bits/stdc++.h>
using namespace std;

class FastInput {
    static constexpr size_t SZ = 1 << 20;
    char buf[SZ];
    size_t pos = 0, len = 0;
public:
    inline int read() {
        int c;
        do {
            if (pos == len) {
                len = fread(buf, 1, SZ, stdin);
                pos = 0;
                if (!len) return -1;
            }
            c = buf[pos++];
        } while (c <= ' ');
        int x = c - '0';
        while (true) {
            if (pos == len) {
                len = fread(buf, 1, SZ, stdin);
                pos = 0;
                if (!len) break;
            }
            c = buf[pos];
            if (c <= ' ') { ++pos; break; }
            ++pos;
            x = x * 10 + c - '0';
        }
        return x;
    }
};

struct Event {
    int w, u, v; // u < 0 denotes town -u-1; otherwise an original road.
    bool operator<(const Event& other) const { return w < other.w; }
};
struct TownEdge { int w, u, v; unsigned bits; };

int main() {
    FastInput in;
    int n = in.read(), m = in.read(), k = in.read();
    if (n == 1) { puts("0"); return 0; }

    vector<Event> events;
    events.reserve(size_t(m) + size_t(n) * k);
    for (int i = 0; i < m; ++i) {
        int u = in.read() - 1, v = in.read() - 1, w = in.read();
        events.push_back({w, u, v});
    }
    long long activation[16];
    for (int j = 0; j < k; ++j) {
        activation[j] = in.read();
        for (int i = 0; i < n; ++i)
            events.push_back({in.read(), -j - 1, i});
    }
    sort(events.begin(), events.end());

    const unsigned full = (1u << k) - 1;
    vector<int> parent(n), sz(n, 1), last(n, 0);
    vector<unsigned> mask(n, 0);
    iota(parent.begin(), parent.end(), 0);
    auto find = [&](int v) {
        while (parent[v] != v) {
            parent[v] = parent[parent[v]];
            v = parent[v];
        }
        return v;
    };
    vector<long long> duration(1u << k, 0);
    unsigned unseen[16];
    for (int j = 0; j < k; ++j) unseen[j] = full ^ (1u << j);
    vector<TownEdge> townEdges;
    townEdges.reserve(k * (k - 1) / 2);

    // Record the first threshold where two towns touch the same original
    // component. Each pair is recorded once, in nondecreasing cost order.
    auto meet = [&](int a, unsigned others, int w) {
        unsigned fresh = others & unseen[a];
        unseen[a] &= ~fresh;
        while (fresh) {
            int b = __builtin_ctz(fresh);
            fresh &= fresh - 1;
            unseen[b] &= ~(1u << a);
            townEdges.push_back({w, a, b, (1u << a) | (1u << b)});
        }
    };

    long long originalMST = 0;
    for (const Event& e : events) {
        if (e.u < 0) {
            int j = -e.u - 1;
            int r = find(e.v);
            if (mask[r] & (1u << j)) continue;
            duration[mask[r]] += e.w - last[r];
            last[r] = e.w;
            meet(j, mask[r], e.w);
            mask[r] |= 1u << j;
        } else {
            int a = find(e.u), b = find(e.v);
            if (a == b) continue;
            originalMST += e.w;
            duration[mask[a]] += e.w - last[a];
            duration[mask[b]] += e.w - last[b];
            unsigned bits = mask[a];
            while (bits) {
                int j = __builtin_ctz(bits);
                bits &= bits - 1;
                meet(j, mask[b], e.w);
            }
            if (sz[a] < sz[b]) swap(a, b);
            parent[b] = a;
            sz[a] += sz[b];
            mask[a] |= mask[b];
            last[a] = e.w;
        }
    }

    // For nonempty S, duration[complement(S)] after this transform is
    // the integral of the number of original components touching no town in S.
    for (unsigned bit = 1; bit <= full; bit <<= 1)
        for (unsigned block = 0; block <= full; block += bit << 1)
            for (unsigned t = 0; t < bit; ++t)
                duration[block + bit + t] += duration[block + t];

    long long answer = originalMST;
    vector<long long> activationSum(1u << k, 0);
    for (unsigned s = 1; s <= full; ++s) {
        unsigned bit = s & -s;
        activationSum[s] = activationSum[s ^ bit] + activation[__builtin_ctz(bit)];
        long long cost = activationSum[s] + duration[full ^ s];
        if (cost >= answer) continue;
        int need = __builtin_popcount(s) - 1;
        if (need) {
            int p[16];
            iota(p, p + k, 0);
            for (const TownEdge& e : townEdges) {
                if ((s & e.bits) != e.bits) continue;
                int a = e.u, b = e.v;
                while (p[a] != a) { p[a] = p[p[a]]; a = p[a]; }
                while (p[b] != b) { p[b] = p[p[b]]; b = p[b]; }
                if (a == b) continue;
                p[a] = b;
                cost += e.w;
                if (--need == 0 || cost >= answer) break;
            }
        }
        answer = min(answer, cost);
    }
    printf("%lld\n", answer);
}
