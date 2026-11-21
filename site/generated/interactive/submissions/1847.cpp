#include <bits/stdc++.h>
using namespace std;

using ll = long long;

constexpr ll LIM = 1000000000LL;
constexpr ll INF = (1LL << 60);

struct Observation {
    int l, r;
    ll value;
};

vector<Observation> observations;
vector<ll> positive;
int zeroRun = 0;

ll ask(int l, int r, bool save = false) {
    cout << "? " << l << ' ' << r << '\n' << flush;
    ll value;
    if (!(cin >> value) || value < 0) exit(0);
    if (save) observations.push_back({l, r, value});
    return value;
}

void setPositive(int i, ll value) {
    positive[i] = value;
    if (value == 0) ++zeroRun;
    else zeroRun = 0;
}

// Recover max(a[i], 0), reusing the known MSS of this interval.
void discover(int l, int r, ll value) {
    int len = r - l + 1;
    if (value == 0) {
        zeroRun += len;
        return;
    }
    if (value == LIM * len) {
        fill(positive.begin() + l, positive.begin() + r + 1, LIM);
        zeroRun = 0;
        return;
    }
    if (len == 1) {
        setPositive(l, value);
        return;
    }
    if (len == 2) {
        ll first = ask(l, l, true);
        setPositive(l, first);
        setPositive(r, value - first);
        return;
    }

    ll total = 0, suffix = 0;
    int p = l;
    while (p <= r) {
        int left = r - p + 1;
        if (suffix == value) {
            // A positive neighbor would extend a run of sum value.
            setPositive(p++, 0);
            suffix = 0;
            continue;
        }
        if (left == 1 && total == suffix) {
            setPositive(p, value - suffix);
            return;
        }
        if (left <= 2 && total == 0) {
            // Removing a nonpositive prefix leaves MSS unchanged.
            discover(p, r, value);
            return;
        }

        int take = min(left, zeroRun + 2);
        if (p == l || total == 0) {
            take = min(take, max(2, (left + 1) / 2));
        }
        if (left == 2 && total == suffix) take = 1;

        int end = p + take - 1;
        discover(p, end, ask(p, end, true));
        while (p <= end) {
            total += positive[p];
            if (positive[p] == 0) suffix = 0;
            else suffix += positive[p];
            ++p;
        }
    }
}

// An edge u -> v of weight w means L[v] - L[u] <= w.
class DifferenceBounds {
    struct Edge {
        int to;
        ll w;
    };

    vector<vector<Edge>> adj;
    vector<ll> pot, dist;
    vector<int> seen;
    vector<unsigned char> queued;
    deque<int> pending;
    int stamp = 0;

    // The searches are optional. Bound their total work, independently
    // of how many implications the hidden instance happens to admit.
    int budget = 8000000;
    bool enabled = true;

    void enqueue(int v) {
        if (queued[v]) return;
        queued[v] = 1;
        if (!pending.empty() && pot[v] < pot[pending.front()])
            pending.push_front(v);
        else
            pending.push_back(v);
    }

public:
    explicit DifferenceBounds(int n)
        : adj(n), pot(n), dist(n), seen(n), queued(n) {}

    void add(int u, int v, ll w) {
        adj[u].push_back({v, w});
        if (enabled && pot[v] > pot[u] + w) {
            pot[v] = pot[u] + w;
            enqueue(v);
        }
    }

    bool settle() {
        if (!enabled) return false;
        while (!pending.empty()) {
            int u = pending.front();
            pending.pop_front();
            queued[u] = 0;
            budget -= 1 + static_cast<int>(adj[u].size());
            if (budget < 0) {
                enabled = false;
                return false;
            }
            for (const auto &e : adj[u]) {
                ll candidate = pot[u] + e.w;
                if (candidate < pot[e.to]) {
                    pot[e.to] = candidate;
                    enqueue(e.to);
                }
            }
        }
        return true;
    }

    ll difference(int u, int v) const {
        return pot[v] - pot[u];
    }

    // Called with a settled potential. A path within the limit proves
    // that the bound holds for every feasible assignment.
    bool proves(int s, int t, ll limit) {
        if (!enabled) return false;
        ll slack = limit - difference(s, t);
        if (slack < 0) return false;

        using State = pair<ll, int>;
        priority_queue<State, vector<State>, greater<State>> pq;
        ++stamp;
        seen[s] = stamp;
        dist[s] = 0;
        pq.emplace(0, s);
        while (!pq.empty()) {
            auto [du, u] = pq.top();
            pq.pop();
            if (du != dist[u]) continue;
            budget -= 1 + static_cast<int>(adj[u].size());
            if (budget < 0) {
                enabled = false;
                return false;
            }
            for (const auto &e : adj[u]) {
                ll d = du + e.w + pot[u] - pot[e.to];
                if (d > slack) continue;
                if (e.to == t) return true;
                if (seen[e.to] != stamp || d < dist[e.to]) {
                    seen[e.to] = stamp;
                    dist[e.to] = d;
                    pq.emplace(d, e.to);
                }
            }
        }
        return false;
    }

    vector<ll> solution() {
        if (settle()) return pot;

        // All constraints are satisfied by the hidden coordinates.
        // With a zero-weight super-source, ordinary Bellman-Ford suffices.
        int n = static_cast<int>(adj.size());
        vector<ll> result(n, 0);
        for (int it = 1; it < n; ++it) {
            bool changed = false;
            for (int u = 0; u < n; ++u) {
                for (const auto &e : adj[u]) {
                    ll candidate = result[u] + e.w;
                    if (candidate < result[e.to]) {
                        result[e.to] = candidate;
                        changed = true;
                    }
                }
            }
            if (!changed) break;
        }
        return result;
    }
};

struct Run {
    int l, r;
    ll sum;
};

struct Witness {
    ll value;
    int first, last;
};

struct TrimmedObservation {
    ll value, cutLeft, cutRight;
    int next;
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n) || n <= 0) return 0;
    positive.assign(n + 1, 0);
    discover(1, n, ask(1, n, true));

    vector<Run> runs;
    vector<int> runAt(n + 1, -1);
    vector<ll> prefix(n + 1, 0);
    for (int i = 1; i <= n; ++i) {
        prefix[i] = prefix[i - 1] + positive[i];
        if (positive[i] == 0) continue;
        if (i == 1 || positive[i - 1] == 0)
            runs.push_back({i, i, 0});
        runs.back().r = i;
        runs.back().sum += positive[i];
        runAt[i] = static_cast<int>(runs.size()) - 1;
    }

    int m = static_cast<int>(runs.size());
    vector<ll> answer = positive;

    if (m >= 2) {
        // A run has coordinates L[i] and R[i] = L[i] + sum[i].
        // The sum from the start of run i to the end of run j is R[j]-L[i].
        DifferenceBounds bounds(m);
        for (int i = 0; i + 1 < m; ++i) {
            ll gap = runs[i + 1].l - runs[i].r - 1;
            bounds.add(i, i + 1, runs[i].sum);
            bounds.add(i + 1, i, gap * LIM - runs[i].sum);
        }

        vector<int> nextPositive(n + 2, n + 1), prevPositive(n + 1, 0);
        for (int i = n; i >= 1; --i)
            nextPositive[i] = positive[i] ? i : nextPositive[i + 1];
        for (int i = 1; i <= n; ++i)
            prevPositive[i] = positive[i] ? i : prevPositive[i - 1];

        auto id = [m](int i, int j) { return i * m + j; };
        vector<ll> cap(m * m, INF);
        vector<int> head(m * m, -1);
        vector<TrimmedObservation> saved;

        auto lowerCap = [&](int i, int j, ll value) {
            if (i <= j) cap[id(i, j)] = min(cap[id(i, j)], value);
        };

        for (const auto &o : observations) {
            int l = nextPositive[o.l], r = prevPositive[o.r];
            if (l > r) continue;
            int i = runAt[l], j = runAt[r];
            if (i == j) continue;

            ll cl = prefix[l - 1] - prefix[runs[i].l - 1];
            ll cr = prefix[runs[j].r] - prefix[r];
            int k = id(i, j);
            saved.push_back({o.value, cl, cr, head[k]});
            head[k] = static_cast<int>(saved.size()) - 1;
            bounds.add(i, j, o.value + cl + cr - runs[j].sum);

            lowerCap(i, j, o.value + cl + cr);
            if (cl) lowerCap(i + 1, j, o.value + cr);
            if (cr) lowerCap(i, j - 1, o.value + cl);
            if (cl && cr) lowerCap(i + 1, j - 1, o.value);
        }

        for (int i = 0; i < m; ++i) {
            for (int j = m - 1; j >= i; --j) {
                ll &v = cap[id(i, j)];
                if (i > 0) v = min(v, cap[id(i - 1, j)]);
                if (j + 1 < m) v = min(v, cap[id(i, j + 1)]);
            }
        }

        vector<int> leftBlock(m, m), rightBlock(m, -1);
        vector<int> component(m);
        vector<ll> coord(m, 0);
        vector<vector<int>> members(m);
        for (int i = 0; i < m; ++i) {
            component[i] = i;
            members[i].push_back(i);
        }

        auto join = [&](int i, int j, ll difference) {
            int a = component[i], b = component[j];
            if (a == b) return;
            bounds.add(i, j, difference);
            bounds.add(j, i, -difference);

            ll shift = coord[i] + difference - coord[j];
            if (members[a].size() < members[b].size()) {
                swap(a, b);
                shift = -shift;
            }
            for (int v : members[b]) coord[v] += shift;
            for (int u : members[a]) {
                for (int v : members[b]) {
                    int x = min(u, v), y = max(u, v);
                    if (coord[x] >= coord[y])
                        leftBlock[x] = min(leftBlock[x], y);
                    if (coord[x] + runs[x].sum >= coord[y] + runs[y].sum)
                        rightBlock[y] = max(rightBlock[y], x);
                }
            }
            for (int v : members[b]) {
                component[v] = a;
                members[a].push_back(v);
            }
            members[b].clear();
        };

        vector<Witness> previous(m), older(m), current(m);
        vector<ll> startBest(m), endBest(m);
        for (int i = 0; i < m; ++i) {
            previous[i] = {runs[i].sum, i, i};
            startBest[i] = endBest[i] = runs[i].sum;
        }

        for (int width = 1; width < m; ++width) {
            for (int i = 0; i + width < m; ++i) {
                int j = i + width;
                Witness left = previous[i], right = previous[i + 1];
                Witness middle = width == 1 ? Witness{0, -1, -1} : older[i + 1];
                Witness best = left.value >= right.value ? left : right;
                ll threshold = best.value;
                ll exact = -1;

                // A known comparison with a nearer endpoint makes the
                // whole span unnecessary, including with trimmed ends.
                bool blocked = leftBlock[i] <= j || rightBlock[j] >= i ||
                               middle.value == threshold;

                if (!blocked) {
                    if (component[i] == component[j]) {
                        exact = coord[j] - coord[i] + runs[j].sum;
                    } else if (cap[id(i, j)] > threshold) {
                        for (int k = head[id(i, j)]; k != -1; k = saved[k].next) {
                            const auto &o = saved[k];
                            ll proper = max({middle.value,
                                             startBest[i] - o.cutLeft,
                                             endBest[j] - o.cutRight});
                            if (o.value > proper) {
                                exact = o.value + o.cutLeft + o.cutRight;
                                break;
                            }
                        }

                        bool need = exact < 0;
                        if (need && bounds.settle()) {
                            ll d = bounds.difference(i, j);
                            ll limit = threshold - runs[j].sum;
                            if (d <= limit) {
                                if (bounds.proves(i, j, limit)) need = false;
                            } else if (bounds.proves(i, j, d) &&
                                       bounds.proves(j, i, -d)) {
                                exact = d + runs[j].sum;
                                need = false;
                            }
                        }
                        if (need) exact = ask(runs[i].l, runs[j].r);
                    }
                }

                if (exact > threshold) {
                    join(i, j, exact - runs[j].sum);
                    current[i] = {exact, i, j};
                    startBest[i] = endBest[j] = exact;
                    continue;
                }

                current[i] = best;

                // If the whole span is not better than a proper witness,
                // retain a coordinate comparison that certifies dominance.
                // Each row/column needs at most one such explicit edge.
                int u = -1, v = -1;
                if (middle.value == threshold) {
                    u = middle.first;
                    v = middle.last;
                } else {
                    if (right.value == threshold) u = right.first;
                    if (left.value == threshold) v = left.last;
                }
                if (u != -1 && leftBlock[i] > j) {
                    leftBlock[i] = u;
                    bounds.add(i, u, 0);
                }
                if (v != -1 && rightBlock[j] < i) {
                    rightBlock[j] = v;
                    bounds.add(v, j, runs[v].sum - runs[j].sum);
                }
            }
            older.swap(previous);
            previous.swap(current);
        }

        vector<ll> L = bounds.solution();
        for (int i = 0; i + 1 < m; ++i)
            answer[runs[i].r + 1] = L[i + 1] - L[i] - runs[i].sum;
    }

    cout << '!';
    for (int i = 1; i <= n; ++i) cout << ' ' << answer[i];
    cout << '\n' << flush;
    return 0;
}
