#include <bits/stdc++.h>
using namespace std;

struct Action {
    int z;
    int delta;
};

struct Particle {
    uint64_t initial;
    uint64_t current;
    int visible;
};

class Solver {
public:
    Solver(int n_, int initialVisible) : n(n_), targetInitial(initialVisible), actualVisible(initialVisible), rng(0x9e3779b97f4a7c15ULL) {
        for (int i = 0; i < n; ++i) {
            for (int j = i; j < n; ++j) {
                intervals.push_back({i, j});
                uint64_t mask = 0;
                for (int k = i; k <= j; ++k) mask |= 1ULL << k;
                masks.push_back(mask);
            }
        }
        m = intervals.size();
        knownOff.assign(m, false);
        simpleOrder.resize(m);
        iota(simpleOrder.begin(), simpleOrder.end(), 0);
        sort(simpleOrder.begin(), simpleOrder.end(), [&](int a, int b) {
            return intervals[a].second - intervals[a].first > intervals[b].second - intervals[b].first;
        });
        opening = initialVisible == 0;
        addParticle((1ULL << m) - 1);
        for (int z = 0; z < m; ++z) addParticle(1ULL << z);
        uint64_t singletonState = 0;
        for (int z = 0; z < m; ++z) {
            if (intervals[z].first == intervals[z].second) singletonState |= 1ULL << z;
        }
        addParticle(singletonState);
        for (int i = 0; i < 512; ++i) addParticle(randomState());
    }

    int choose() {
        if (opening) {
            while (openingPos < m && knownOff[simpleOrder[openingPos]]) ++openingPos;
            if (openingPos < m) return simpleOrder[openingPos++];
            opening = false;
        }
        if (particles.empty()) rebuild();
        vector<int> active(m, 0), positive(m, 0);
        for (const auto &p : particles) {
            int coverage[10] = {};
            for (int z = 0; z < m; ++z) {
                if ((p.current >> z) & 1ULL) {
                    auto [l, r] = intervals[z];
                    for (int k = l; k <= r; ++k) ++coverage[k];
                }
            }
            uint64_t privateCells = 0;
            for (int k = 0; k < n; ++k) {
                if (coverage[k] == 1) privateCells |= 1ULL << k;
            }
            for (int z = 0; z < m; ++z) {
                if ((p.current >> z) & 1ULL) {
                    ++active[z];
                    if (masks[z] & privateCells) ++positive[z];
                }
            }
        }
        int best = -1;
        array<long long, 4> bestScore = {-1, -1, -1, -1};
        for (int z = 0; z < m; ++z) {
            if (!active[z] || knownOff[z]) continue;
            if (!history.empty() && history.back().z == z && history.back().delta == 0 && m > 1) continue;
            int len = intervals[z].second - intervals[z].first + 1;
            bool recent = find(recentZeros.begin(), recentZeros.end(), z) != recentZeros.end();
            array<long long, 4> score = {
                positive[z],
                active[z] - (recent ? 10000 : 0),
                len,
                -z
            };
            if (score > bestScore) {
                bestScore = score;
                best = z;
            }
        }
        if (best < 0) best = 0;
        return best;
    }

    void observe(int z, int delta) {
        actualVisible += delta;
        if (delta != 0) opening = false;
        if (delta > 0) stagnant = 0;
        else ++stagnant;
        history.push_back({z, delta});
        if (delta != 0) {
            knownOff[z] = true;
            if (intervals[z].first == intervals[z].second) {
                int cell = intervals[z].first;
                for (int b = 0; b < m; ++b) {
                    if (intervals[b].first <= cell && cell <= intervals[b].second) knownOff[b] = true;
                }
            }
        }
        if (delta == 0) {
            recentZeros.push_back(z);
            if (recentZeros.size() > 10) recentZeros.erase(recentZeros.begin());
        } else {
            recentZeros.clear();
        }
        vector<Particle> kept;
        kept.reserve(particles.size());
        for (auto p : particles) {
            int old = p.visible;
            p.current ^= 1ULL << z;
            p.visible = visibleCount(p.current);
            if (p.visible - old == delta) kept.push_back(p);
        }
        particles.swap(kept);
        if (!opening && (particles.empty() || stagnant >= 30)) {
            rebuild();
            stagnant = 0;
        }
    }

private:
    int n, m, targetInitial, actualVisible;
    vector<pair<int, int>> intervals;
    vector<uint64_t> masks;
    vector<Action> history;
    vector<Particle> particles;
    vector<int> recentZeros;
    vector<bool> knownOff;
    vector<int> simpleOrder;
    int stagnant = 0;
    bool opening;
    int openingPos = 0;
    mt19937_64 rng;

    uint64_t randomState() {
        uint64_t allowed = 0;
        for (int z = 0; z < m; ++z) {
            if (!knownOff[z]) allowed |= 1ULL << z;
        }
        return rng() & allowed;
    }

    int visibleCount(uint64_t state) const {
        uint64_t covered = 0;
        for (int z = 0; z < m; ++z) {
            if ((state >> z) & 1ULL) covered |= masks[z];
        }
        return n - __builtin_popcountll(covered);
    }

    int cost(uint64_t state) const {
        int visible = visibleCount(state);
        int result = abs(visible - targetInitial) * 4;
        for (const auto &a : history) {
            int old = visible;
            state ^= 1ULL << a.z;
            visible = visibleCount(state);
            result += abs(visible - old - a.delta) * 4;
        }
        return result;
    }

    uint64_t anneal(uint64_t state) {
        int currentCost = cost(state);
        if (!currentCost) return state;
        const int steps = 300 + min(700, (int)history.size() * 4);
        for (int step = 0; step < steps; ++step) {
            int z = rng() % m;
            if (knownOff[z]) continue;
            uint64_t next = state ^ (1ULL << z);
            int nextCost = cost(next);
            int diff = nextCost - currentCost;
            bool accept = diff <= 0;
            if (!accept) {
                double temperature = 3.0 * (steps - step) / steps + 0.08;
                accept = uniform_real_distribution<double>(0.0, 1.0)(rng) < exp(-diff / temperature);
            }
            if (accept) {
                state = next;
                currentCost = nextCost;
                if (!currentCost) return state;
            }
        }
        return state;
    }

    void addParticle(uint64_t seed) {
        for (int z = 0; z < m; ++z) {
            if (knownOff[z]) seed &= ~(1ULL << z);
        }
        uint64_t initial = anneal(seed);
        if (cost(initial) != 0) return;
        for (const auto &p : particles) {
            if (p.initial == initial) return;
        }
        uint64_t current = initial;
        int visible = visibleCount(current);
        for (const auto &a : history) {
            current ^= 1ULL << a.z;
            visible = visibleCount(current);
        }
        particles.push_back({initial, current, visible});
    }

    void rebuild() {
        history.clear();
        particles.clear();
        targetInitial = actualVisible;
        recentZeros.clear();
        uint64_t allowed = 0;
        for (int z = 0; z < m; ++z) {
            if (!knownOff[z]) allowed |= 1ULL << z;
        }
        addParticle(allowed);
        for (int z = 0; z < m; ++z) {
            if (!knownOff[z]) addParticle(1ULL << z);
        }
        for (int i = 0; i < 512; ++i) addParticle(randomState());
        for (int attempt = 0; attempt < 200 && particles.empty(); ++attempt) {
            addParticle(randomState());
        }
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, visible;
    if (!(cin >> n)) return 0;
    if (!(cin >> visible)) return 0;
    Solver solver(n, visible);
    while (visible < n) {
        int z = solver.choose();
        auto [i, j] = [&]() {
            int index = z, row = 0;
            while (index >= n - row) {
                index -= n - row;
                ++row;
            }
            return pair<int, int>{row + 1, row + index + 1};
        }();
        cout << i << ' ' << j << '\n' << flush;
        int next;
        if (!(cin >> next)) return 0;
        int delta = next - visible;
        visible = next;
        solver.observe(z, delta);
        if (delta < 0 && visible < n) {
            cout << i << ' ' << j << '\n' << flush;
            if (!(cin >> next)) return 0;
            int undoDelta = next - visible;
            visible = next;
            solver.observe(z, undoDelta);
        }
    }
    return 0;
}
