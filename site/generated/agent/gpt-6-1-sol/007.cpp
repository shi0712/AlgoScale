#include <bits/stdc++.h>
using namespace std;

struct Circle { int x, y, r, v; };
static vector<Circle> circles;

// The order just to the right of the larger of the two left endpoints.
// Noncrossing boundaries keep this order throughout their common x interval.
struct ArcLess {
    bool operator()(uint32_t a, uint32_t b) const {
        if (a == b) return false;
        const Circle &p = circles[a >> 1], &q = circles[b >> 1];
        int lp = p.x - p.r, lq = q.x - q.r;
        if (lp == lq) {
            if (p.y != q.y) return p.y < q.y;
            if ((a & 1) != (b & 1)) return (a & 1) < (b & 1);
            if (p.r != q.r) return (a & 1) ? p.r < q.r : p.r > q.r;
            return a < b;
        }
        if (lp < lq) {
            long long dy = (long long)q.y - p.y;
            if (a & 1) {
                if (dy <= 0) return false;
                long long dx = (long long)lq - p.x;
                return dx * dx + dy * dy > (long long)p.r * p.r;
            } else {
                if (dy >= 0) return true;
                long long dx = (long long)lq - p.x;
                return dx * dx + dy * dy < (long long)p.r * p.r;
            }
        } else {
            long long dy = (long long)p.y - q.y;
            if (b & 1) {
                if (dy <= 0) return true;
                long long dx = (long long)lp - q.x;
                return dx * dx + dy * dy < (long long)q.r * q.r;
            } else {
                if (dy >= 0) return false;
                long long dx = (long long)lp - q.x;
                return dx * dx + dy * dy > (long long)q.r * q.r;
            }
        }
    }
};

struct Event { int x, y; uint32_t arc; };
struct Incidence { uint32_t circle, vertex; int dx, dy; };
struct DSU {
    vector<int> p;
    explicit DSU(int n): p(n, -1) {}
    int find(int x) {
        int r = x;
        while (p[r] >= 0) r = p[r];
        while (x != r) { int nx = p[x]; p[x] = r; x = nx; }
        return r;
    }
    void unite(int x, int y) {
        x = find(x); y = find(y);
        if (x == y) return;
        if (p[x] > p[y]) swap(x,y);
        p[x] += p[y]; p[y] = x;
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    int n, sx, sy, tx, ty;
    if (!(cin >> n >> sx >> sy >> tx >> ty)) return 0;
    if (sx == tx && sy == ty) { cout << "0\n"; return 0; }
    circles.resize(n);
    int sc = -1, tc = -1;
    vector<Event> events;
    events.reserve(2ull*n);
    for (int i = 0; i < n; ++i) {
        Circle &c = circles[i];
        cin >> c.x >> c.y >> c.r >> c.v;
        long long dx = (long long)sx - c.x, dy = (long long)sy - c.y;
        if (dx*dx + dy*dy == (long long)c.r*c.r) sc = i;
        dx = (long long)tx - c.x; dy = (long long)ty - c.y;
        if (dx*dx + dy*dy == (long long)c.r*c.r) tc = i;
        events.push_back({c.x-c.r, c.y, uint32_t(2*i)});
        events.push_back({c.x+c.r, c.y, uint32_t(2*i+1)});
    }
    sort(events.begin(), events.end(), [](const Event &a, const Event &b) {
        if (a.x != b.x) return a.x < b.x;
        if (a.y != b.y) return a.y < b.y;
        return a.arc < b.arc;
    });
    vector<uint64_t> contacts;
    contacts.reserve(3ull*n);
    auto add_pair = [&](uint32_t a, uint32_t b) {
        if (a > b) swap(a,b);
        contacts.push_back((uint64_t(a) << 32) | b);
    };
    auto check = [&](uint32_t a, uint32_t b) {
        a >>= 1; b >>= 1;
        if (a == b) return;
        const Circle &p = circles[a], &q = circles[b];
        long long dx = (long long)p.x-q.x, dy = (long long)p.y-q.y;
        long long d2 = dx*dx + dy*dy;
        long long sum = p.r+q.r, diff = p.r-q.r;
        if (d2 == sum*sum || d2 == diff*diff) add_pair(a,b);
    };
    {
        set<uint32_t, ArcLess> active;
        vector<set<uint32_t, ArcLess>::iterator> where(2ull*n);
        size_t begin = 0;
        while (begin < events.size()) {
            size_t end = begin+1;
            while (end < events.size() && events[end].x == events[begin].x) ++end;
            // Every circle sharing an extremal point can transfer there.
            for (size_t j = begin+1; j < end; ++j)
                if (events[j].y == events[j-1].y)
                    add_pair(events[j].arc >> 1, events[j-1].arc >> 1);
            // Endings precede beginnings: the tree represents an open x interval.
            for (size_t j = begin; j < end; ++j) if (events[j].arc & 1) {
                uint32_t id = events[j].arc & ~1u;
                for (uint32_t a = id; a <= id+1; ++a) {
                    auto it = where[a];
                    auto nx = next(it);
                    if (it != active.begin() && nx != active.end()) check(*prev(it), *nx);
                    active.erase(it);
                }
            }
            for (size_t j = begin; j < end; ++j) if (!(events[j].arc & 1)) {
                uint32_t id = events[j].arc;
                for (uint32_t a = id; a <= id+1; ++a) {
                    auto it = active.insert(a).first;
                    where[a] = it;
                    if (it != active.begin()) check(*prev(it), *it);
                    auto nx = next(it);
                    if (nx != active.end()) check(*it, *nx);
                }
            }
            begin = end;
        }
    }
    vector<Event>().swap(events);
    sort(contacts.begin(), contacts.end());
    contacts.erase(unique(contacts.begin(), contacts.end()), contacts.end());
    {
        DSU dsu(n);
        for (uint64_t e : contacts) dsu.unite(e >> 32, uint32_t(e));
        int root = dsu.find(sc);
        if (dsu.find(tc) != root) { cout << "-1\n"; return 0; }
        size_t k = 0;
        for (uint64_t e : contacts)
            if (dsu.find(e >> 32) == root) contacts[k++] = e;
        contacts.resize(k);
    }
    uint32_t m = contacts.size(), source = m, target = m+1, vertices = m+2;
    vector<Incidence> inc;
    inc.reserve(2ull*m+2);
    for (uint32_t k = 0; k < m; ++k) {
        uint32_t a = contacts[k] >> 32, b = uint32_t(contacts[k]);
        const Circle &p = circles[a], &q = circles[b];
        int dx = q.x-p.x, dy = q.y-p.y;
        long long sum = p.r+q.r;
        bool external = (long long)dx*dx + (long long)dy*dy == sum*sum;
        if (external) {
            inc.push_back({a,k,dx,dy});
            inc.push_back({b,k,-dx,-dy});
        } else {
            if (p.r < q.r) { dx = -dx; dy = -dy; }
            inc.push_back({a,k,dx,dy});
            inc.push_back({b,k,dx,dy});
        }
    }
    inc.push_back({uint32_t(sc), source, sx-circles[sc].x, sy-circles[sc].y});
    inc.push_back({uint32_t(tc), target, tx-circles[tc].x, ty-circles[tc].y});
    vector<uint64_t>().swap(contacts);
    sort(inc.begin(), inc.end(), [](const Incidence &a, const Incidence &b) {
        if (a.circle != b.circle) return a.circle < b.circle;
        bool ha = a.dy < 0 || (a.dy == 0 && a.dx < 0);
        bool hb = b.dy < 0 || (b.dy == 0 && b.dx < 0);
        if (ha != hb) return ha < hb;
        long long cross = (long long)a.dx*b.dy - (long long)a.dy*b.dx;
        if (cross) return cross > 0;
        return a.vertex < b.vertex;
    });
    // Several discovered pairs can describe the same physical tangent point.
    // Equal directions on any one circle identify those vertices exactly.
    {
        DSU point_dsu(vertices);
        for (size_t j = 1; j < inc.size(); ++j) {
            const Incidence &a = inc[j-1], &b = inc[j];
            if (a.circle != b.circle) continue;
            long long cross = (long long)a.dx*b.dy - (long long)a.dy*b.dx;
            long long dot = (long long)a.dx*b.dx + (long long)a.dy*b.dy;
            if (cross == 0 && dot > 0) point_dsu.unite(a.vertex, b.vertex);
        }
        vector<uint32_t> mapping(vertices);
        for (uint32_t j = 0; j < vertices; ++j) mapping[j] = point_dsu.find(j);
        uint32_t count = 0;
        for (uint32_t j = 0; j < vertices; ++j)
            if (mapping[j] == j) point_dsu.p[j] = count++;
        for (uint32_t j = 0; j < vertices; ++j) mapping[j] = point_dsu.p[mapping[j]];
        source = mapping[source]; target = mapping[target]; vertices = count;
        size_t kept = 0;
        for (size_t j = 0; j < inc.size(); ++j) {
            Incidence a = inc[j];
            a.vertex = mapping[a.vertex];
            if (kept && inc[kept-1].circle == a.circle && inc[kept-1].vertex == a.vertex) continue;
            inc[kept++] = a;
        }
        inc.resize(kept);
    }
    vector<uint32_t> offset(vertices+1, 0);
    for (size_t l = 0; l < inc.size();) {
        size_t r = l+1;
        while (r < inc.size() && inc[r].circle == inc[l].circle) ++r;
        if (r-l == 2) {
            ++offset[inc[l].vertex+1]; ++offset[inc[l+1].vertex+1];
        } else if (r-l > 2) {
            for (size_t j = l; j < r; ++j) offset[inc[j].vertex+1] += 2;
        }
        l = r;
    }
    for (uint32_t j = 1; j <= vertices; ++j) offset[j] += offset[j-1];
    vector<uint32_t> to(offset.back());
    vector<double> weight(offset.back());
    {
        vector<uint32_t> cursor = offset;
        const long double pi = acosl(-1.0L);
        for (size_t l = 0; l < inc.size();) {
            size_t r = l+1;
            while (r < inc.size() && inc[r].circle == inc[l].circle) ++r;
            const Circle &c = circles[inc[l].circle];
            auto edge = [&](const Incidence &a, const Incidence &b, bool shortest) {
                double w = 0;
                if (c.v) {
                    long long cross = (long long)a.dx*b.dy - (long long)a.dy*b.dx;
                    long long dot = (long long)a.dx*b.dx + (long long)a.dy*b.dy;
                    if (shortest && cross < 0) cross = -cross;
                    long double angle = atan2l((long double)cross, (long double)dot);
                    if (angle < 0) angle += 2*pi;
                    w = (double)(angle * c.r * c.v);
                }
                uint32_t i = cursor[a.vertex]++, j = cursor[b.vertex]++;
                to[i] = b.vertex; weight[i] = w;
                to[j] = a.vertex; weight[j] = w;
            };
            if (r-l == 2) edge(inc[l], inc[l+1], true);
            else if (r-l > 2)
                for (size_t j = l; j < r; ++j) edge(inc[j], inc[j+1 == r ? l : j+1], false);
            l = r;
        }
    }
    vector<Incidence>().swap(inc);
    vector<Circle>().swap(circles);

    // An indexed binary heap bounds Dijkstra's queue memory by O(vertices).
    vector<double> dist(vertices, numeric_limits<double>::infinity());
    vector<int> position(vertices, -1);
    vector<uint32_t> heap;
    heap.reserve(vertices);
    auto decrease = [&](uint32_t u) {
        int i = position[u];
        if (i < 0) { i = heap.size(); heap.push_back(u); }
        while (i) {
            int p = (i-1)/2;
            if (dist[heap[p]] <= dist[u]) break;
            heap[i] = heap[p]; position[heap[i]] = i; i = p;
        }
        heap[i] = u; position[u] = i;
    };
    dist[source] = 0;
    decrease(source);
    while (!heap.empty()) {
        uint32_t u = heap[0], last = heap.back();
        heap.pop_back();
        position[u] = -2;
        if (!heap.empty()) {
            size_t i = 0;
            while (2*i+1 < heap.size()) {
                size_t j = 2*i+1;
                if (j+1 < heap.size() && dist[heap[j+1]] < dist[heap[j]]) ++j;
                if (dist[last] <= dist[heap[j]]) break;
                heap[i] = heap[j]; position[heap[i]] = i; i = j;
            }
            heap[i] = last; position[last] = i;
        }
        if (u == target) { cout << setprecision(17) << dist[u] << '\n'; return 0; }
        for (uint32_t j = offset[u]; j < offset[u+1]; ++j) {
            uint32_t v = to[j];
            double nd = dist[u] + weight[j];
            if (position[v] != -2 && nd < dist[v]) { dist[v] = nd; decrease(v); }
        }
    }
    cout << "-1\n";
}
