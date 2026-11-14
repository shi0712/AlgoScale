#include <bits/stdc++.h>
using namespace std;

using int64 = long long;
using uint64 = unsigned long long;

struct Circle {
    int x, y, r, v;
};

struct Event {
    int x;
    int code; // 2*i: left endpoint, 2*i+1: right endpoint
};

struct Contact {
    int circle;
    int node;
    int dx, dy;
};

static inline int64 cross(const Contact& a, const Contact& b) {
    return int64(a.dx) * b.dy - int64(a.dy) * b.dx;
}

static inline int half(const Contact& a) {
    return (a.dy > 0 || (a.dy == 0 && a.dx >= 0)) ? 0 : 1;
}

/*
    The vertical order of two semicircles can be determined at the
    leftmost x-coordinate common to their horizontal ranges.

    At that x-coordinate, at least one of the semicircles is at its
    left endpoint, so comparing their heights needs only integer
    arithmetic and a comparison with a squared square root.
*/
struct ArcLess {
    const vector<Circle>* circles;

    int compareEndpointToArc(int newer, int older, int olderUpper) const {
        const auto& a = (*circles)[newer];
        const auto& b = (*circles)[older];

        int64 x = int64(a.x) - a.r;
        int64 dx = x - b.x;
        int64 q = int64(b.r) * b.r - dx * dx;
        int64 dy = int64(a.y) - b.y;

        if (olderUpper) {
            if (dy < 0) return -1;
            int64 s = dy * dy;
            return (s > q) - (s < q);
        } else {
            if (dy > 0) return 1;
            int64 s = dy * dy;
            return (q > s) - (q < s);
        }
    }

    bool operator()(int a, int b) const {
        if (a == b) return false;

        int ia = a >> 1, ib = b >> 1;
        int ua = a & 1, ub = b & 1;

        if (ia == ib) return ua < ub;

        const auto& ca = (*circles)[ia];
        const auto& cb = (*circles)[ib];

        int64 la = int64(ca.x) - ca.r;
        int64 lb = int64(cb.x) - cb.r;

        if (la == lb) {
            if (ca.y != cb.y) return ca.y < cb.y;

            // Both circles have the same leftmost point.
            if (ua != ub) return ua < ub;
            if (ca.r != cb.r) {
                if (ua) return ca.r < cb.r;
                return ca.r > cb.r;
            }
            return a < b;
        }

        int cmp;
        if (la > lb) {
            cmp = compareEndpointToArc(ia, ib, ub);
        } else {
            cmp = -compareEndpointToArc(ib, ia, ua);
        }

        if (cmp != 0) return cmp < 0;

        // For simultaneously active arcs, this case is excluded by
        // the non-crossing condition and the endpoint event ordering.
        return a < b;
    }
};

class DSU {
public:
    vector<int> p;

    explicit DSU(int n) : p(n, -1) {}

    int find(int x) {
        int root = x;
        while (p[root] >= 0) root = p[root];

        while (x != root) {
            int next = p[x];
            p[x] = root;
            x = next;
        }
        return root;
    }

    void unite(int a, int b) {
        a = find(a);
        b = find(b);
        if (a == b) return;
        if (p[a] > p[b]) swap(a, b);
        p[a] += p[b];
        p[b] = a;
    }
};

class IndexedHeap {
    vector<int> heap;
    vector<int>& position;
    vector<double>& distance;

public:
    IndexedHeap(vector<int>& position_, vector<double>& distance_)
        : position(position_), distance(distance_) {}

    bool empty() const {
        return heap.empty();
    }

    void decrease(int vertex) {
        int at = position[vertex];

        if (at == -1) {
            at = int(heap.size());
            heap.push_back(vertex);
            position[vertex] = at;
        }

        while (at > 0) {
            int parent = (at - 1) >> 1;
            if (distance[heap[parent]] <= distance[vertex]) break;

            heap[at] = heap[parent];
            position[heap[at]] = at;
            at = parent;
        }

        heap[at] = vertex;
        position[vertex] = at;
    }

    int pop() {
        int result = heap[0];
        int last = heap.back();
        heap.pop_back();

        position[result] = -2;

        if (!heap.empty()) {
            int at = 0;
            int size = int(heap.size());

            while (true) {
                int child = at * 2 + 1;
                if (child >= size) break;

                if (child + 1 < size &&
                    distance[heap[child + 1]] < distance[heap[child]]) {
                    ++child;
                }

                if (distance[last] <= distance[heap[child]]) break;

                heap[at] = heap[child];
                position[heap[at]] = at;
                at = child;
            }

            heap[at] = last;
            position[last] = at;
        }

        return result;
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    int sx, sy, tx, ty;
    cin >> n >> sx >> sy >> tx >> ty;

    vector<Circle> circles(n);
    int endpointMemberships = 0;

    auto onCircle = [](int x, int y, const Circle& c) {
        int64 dx = int64(x) - c.x;
        int64 dy = int64(y) - c.y;
        return dx * dx + dy * dy == int64(c.r) * c.r;
    };

    for (auto& c : circles) {
        cin >> c.x >> c.y >> c.r >> c.v;
        endpointMemberships += onCircle(sx, sy, c);
        endpointMemberships += onCircle(tx, ty, c);
    }

    if (sx == tx && sy == ty) {
        cout << "0\n";
        return 0;
    }

    vector<uint64> pairs;
    pairs.reserve(size_t(4) * n);

    auto appendPair = [&](int a, int b) {
        if (a == b) return;
        if (a > b) swap(a, b);
        pairs.push_back((uint64(uint32_t(a)) << 32) | uint32_t(b));
    };

    auto checkArcs = [&](int arcA, int arcB) {
        int a = arcA >> 1;
        int b = arcB >> 1;
        if (a == b) return;

        const auto& ca = circles[a];
        const auto& cb = circles[b];

        int64 dx = int64(ca.x) - cb.x;
        int64 dy = int64(ca.y) - cb.y;
        int64 d2 = dx * dx + dy * dy;
        int64 sum = int64(ca.r) + cb.r;
        int64 difference = int64(ca.r) - cb.r;

        if (d2 == sum * sum || d2 == difference * difference) {
            appendPair(a, b);
        }
    };

    {
        vector<Event> events;
        events.reserve(size_t(2) * n);

        for (int i = 0; i < n; ++i) {
            events.push_back({circles[i].x - circles[i].r, 2 * i});
            events.push_back({circles[i].x + circles[i].r, 2 * i + 1});
        }

        sort(events.begin(), events.end(), [&](const Event& a, const Event& b) {
            if (a.x != b.x) return a.x < b.x;
            int ya = circles[a.code >> 1].y;
            int yb = circles[b.code >> 1].y;
            if (ya != yb) return ya < yb;
            return a.code < b.code;
        });

        using ArcSet = set<int, ArcLess>;
        ArcSet active(ArcLess{&circles});
        vector<ArcSet::iterator> where(size_t(2) * n);

        auto insertArc = [&](int arc) {
            auto it = active.insert(arc).first;
            where[arc] = it;

            if (it != active.begin()) {
                checkArcs(*prev(it), *it);
            }

            auto nextIt = next(it);
            if (nextIt != active.end()) {
                checkArcs(*it, *nextIt);
            }
        };

        auto eraseArc = [&](int arc) {
            auto it = where[arc];
            auto nextIt = next(it);

            if (it != active.begin() && nextIt != active.end()) {
                checkArcs(*prev(it), *nextIt);
            }

            active.erase(it);
        };

        for (int begin = 0; begin < int(events.size()); ) {
            int end = begin + 1;
            int x = events[begin].x;
            int y = circles[events[begin].code >> 1].y;

            while (end < int(events.size()) &&
                   events[end].x == x &&
                   circles[events[end].code >> 1].y == y) {
                ++end;
            }

            // All these circle endpoints are the same geometric point.
            for (int j = begin + 1; j < end; ++j) {
                appendPair(events[j - 1].code >> 1, events[j].code >> 1);
            }

            // Remove ending circles before inserting starting circles
            // at the same point.
            for (int j = begin; j < end; ++j) {
                int code = events[j].code;
                if (code & 1) {
                    int circle = code >> 1;
                    eraseArc(2 * circle);
                    eraseArc(2 * circle + 1);
                }
            }

            for (int j = begin; j < end; ++j) {
                int code = events[j].code;
                if (!(code & 1)) {
                    int circle = code >> 1;
                    insertArc(2 * circle);
                    insertArc(2 * circle + 1);
                }
            }

            begin = end;
        }
    }

    sort(pairs.begin(), pairs.end());
    pairs.erase(unique(pairs.begin(), pairs.end()), pairs.end());

    int pairCount = int(pairs.size());
    int sourceNode = pairCount;
    int targetNode = pairCount + 1;

    vector<Contact> contacts;
    contacts.reserve(size_t(2) * pairCount + endpointMemberships);

    for (int node = 0; node < pairCount; ++node) {
        uint64 key = pairs[node];
        int a = int(key >> 32);
        int b = int(uint32_t(key));

        const auto& ca = circles[a];
        const auto& cb = circles[b];

        int dx = cb.x - ca.x;
        int dy = cb.y - ca.y;

        int64 d2 = int64(dx) * dx + int64(dy) * dy;
        int64 sum = int64(ca.r) + cb.r;

        if (d2 == sum * sum) {
            // External tangency.
            contacts.push_back({a, node, dx, dy});
            contacts.push_back({b, node, -dx, -dy});
        } else {
            // Internal tangency: both radius directions are equal.
            if (ca.r < cb.r) {
                dx = -dx;
                dy = -dy;
            }
            contacts.push_back({a, node, dx, dy});
            contacts.push_back({b, node, dx, dy});
        }
    }

    vector<uint64>().swap(pairs);

    for (int i = 0; i < n; ++i) {
        const auto& c = circles[i];

        if (onCircle(sx, sy, c)) {
            contacts.push_back({i, sourceNode, sx - c.x, sy - c.y});
        }

        if (onCircle(tx, ty, c)) {
            contacts.push_back({i, targetNode, tx - c.x, ty - c.y});
        }
    }

    sort(contacts.begin(), contacts.end(), [](const Contact& a, const Contact& b) {
        if (a.circle != b.circle) return a.circle < b.circle;

        int ha = half(a), hb = half(b);
        if (ha != hb) return ha < hb;

        return cross(a, b) > 0;
    });

    int nodeCount;
    {
        DSU dsu(pairCount + 2);

        size_t kept = 0;
        for (size_t i = 0; i < contacts.size(); ) {
            size_t j = i + 1;

            while (j < contacts.size() &&
                   contacts[j].circle == contacts[i].circle &&
                   half(contacts[j]) == half(contacts[i]) &&
                   cross(contacts[i], contacts[j]) == 0) {
                dsu.unite(contacts[i].node, contacts[j].node);
                ++j;
            }

            contacts[kept++] = contacts[i];
            i = j;
        }

        contacts.resize(kept);

        sourceNode = dsu.find(sourceNode);
        targetNode = dsu.find(targetNode);

        if (sourceNode == targetNode) {
            cout << "0\n";
            return 0;
        }

        for (auto& contact : contacts) {
            contact.node = dsu.find(contact.node);
        }

        nodeCount = 0;
        for (int i = 0; i < int(dsu.p.size()); ++i) {
            if (dsu.p[i] < 0) {
                dsu.p[i] = -1 - nodeCount++;
            }
        }

        sourceNode = -1 - dsu.p[sourceNode];
        targetNode = -1 - dsu.p[targetNode];

        for (auto& contact : contacts) {
            contact.node = -1 - dsu.p[contact.node];
        }

        // Release the capacity used by duplicate contacts.
        vector<Contact>(contacts).swap(contacts);
    }

    int m = int(contacts.size());

    vector<int> offset(n + 1);
    int at = 0;
    for (int i = 0; i < n; ++i) {
        offset[i] = at;
        while (at < m && contacts[at].circle == i) ++at;
    }
    offset[n] = m;

    vector<int> head(nodeCount, -1);
    vector<int> nextAtNode(m);
    vector<double> forwardCost(m);

    const long double PI = acosl(-1.0L);
    const long double TWO_PI = 2.0L * PI;

    for (int i = 0; i < m; ++i) {
        int node = contacts[i].node;
        nextAtNode[i] = head[node];
        head[node] = i;
    }

    for (int circle = 0; circle < n; ++circle) {
        int begin = offset[circle];
        int end = offset[circle + 1];

        if (end - begin <= 1) {
            if (begin < end) forwardCost[begin] = 0;
            continue;
        }

        const auto& c = circles[circle];

        for (int i = begin; i < end; ++i) {
            int j = (i + 1 == end) ? begin : i + 1;

            if (c.v == 0) {
                forwardCost[i] = 0;
                continue;
            }

            int64 cr = cross(contacts[i], contacts[j]);
            int64 dot =
                int64(contacts[i].dx) * contacts[j].dx +
                int64(contacts[i].dy) * contacts[j].dy;

            long double angle = atan2l((long double)cr, (long double)dot);
            if (angle < 0) angle += TWO_PI;

            forwardCost[i] =
                (double)(angle * (long double)c.r * (long double)c.v);
        }
    }

    const double INF = numeric_limits<double>::infinity();
    vector<double> distance(nodeCount, INF);
    vector<int> position(nodeCount, -1);

    IndexedHeap heap(position, distance);
    distance[sourceNode] = 0;
    heap.decrease(sourceNode);

    auto relax = [&](int vertex, double candidate) {
        if (position[vertex] == -2) return;
        if (candidate < distance[vertex]) {
            distance[vertex] = candidate;
            heap.decrease(vertex);
        }
    };

    while (!heap.empty()) {
        int node = heap.pop();

        if (node == targetNode) {
            cout << scientific << setprecision(18) << distance[node] << '\n';
            return 0;
        }

        double current = distance[node];

        for (int i = head[node]; i != -1; i = nextAtNode[i]) {
            int circle = contacts[i].circle;
            int begin = offset[circle];
            int end = offset[circle + 1];

            if (end - begin <= 1) continue;

            int next = (i + 1 == end) ? begin : i + 1;
            int previous = (i == begin) ? end - 1 : i - 1;

            relax(contacts[next].node, current + forwardCost[i]);
            relax(contacts[previous].node, current + forwardCost[previous]);
        }
    }

    cout << "-1\n";
    return 0;
}
