#include <algorithm>
#include <array>
#include <bit>
#include <cassert>
#include <cstdlib>
#include <iostream>
#include <numeric>
#include <random>
#include <vector>
using namespace std;

int n;
mt19937 rng(173913);

int ask(const vector<int>& indices) {
    cout << "? " << indices.size();
    for (int i : indices) cout << ' ' << i + 1;
    cout << endl;
    int answer;
    if (!(cin >> answer) || (answer != 0 && answer != 1)) exit(0);
    return answer;
}

void finish(vector<int> values) {
    if (values[0] > n / 2)
        for (int& x : values) x = n + 1 - x;
    cout << '!';
    for (int x : values) cout << ' ' << x;
    cout << endl;
}

// For small cases, choose the most evenly splitting subset at each node.
void small_case() {
    vector<int> p(n);
    iota(p.begin(), p.end(), 1);
    vector<vector<int>> candidates;
    do {
        if (p[0] <= n / 2) candidates.push_back(p);
    } while (next_permutation(p.begin(), p.end()));
    while (candidates.size() > 1) {
        int limit = 1 << n;
        vector<int> yes(limit), sums(limit);
        for (const auto& v : candidates) {
            for (int mask = 1; mask < limit; ++mask) {
                sums[mask] = sums[mask & (mask - 1)] + v[countr_zero(unsigned(mask))];
                yes[mask] += sums[mask] % popcount(unsigned(mask)) == 0;
            }
        }
        int best = 0, score = 0, total = int(candidates.size());
        for (int mask = 1; mask < limit; ++mask) {
            int split = min(yes[mask], total - yes[mask]);
            if (split > score) {
                best = mask;
                score = split;
            }
        }
        assert(score > 0);
        vector<int> indices;
        for (int i = 0; i < n; ++i)
            if ((best >> i) & 1) indices.push_back(i);
        int answer = ask(indices);
        erase_if(candidates, [&](const auto& v) {
            int sum = 0;
            for (int i : indices) sum += v[i];
            return (sum % int(indices.size()) == 0) != answer;
        });
    }
    finish(candidates[0]);
}

constexpr int words = (800 + 63) / 64;
struct Equation {
    array<uint64_t, words> bits{};
    int rhs = 0;
    void flip(int i) { bits[i / 64] ^= uint64_t(1) << (i % 64); }
};

struct Basis {
    vector<Equation> rows = vector<Equation>(n);
    vector<bool> used = vector<bool>(n);

    int reduce(Equation& e) const {
        for (int w = (n - 1) / 64; w >= 0; --w) {
            while (e.bits[w]) {
                int pivot = 64 * w + 63 - countl_zero(e.bits[w]);
                if (!used[pivot]) return pivot;
                for (int j = 0; j <= w; ++j) e.bits[j] ^= rows[pivot].bits[j];
                e.rhs ^= rows[pivot].rhs;
            }
        }
        return -1;
    }
    void add(Equation e) {
        int p = reduce(e);
        if (p >= 0) {
            used[p] = true;
            rows[p] = e;
        } else assert(e.rhs == 0);
    }
    int value(Equation e) const { return reduce(e) < 0 ? e.rhs : -1; }
};

Equation variables(initializer_list<int> ids) {
    Equation e;
    for (int i : ids) e.flip(i);
    return e;
}

// Before the integer labels are oriented, represent each value by
// (p[i]-p[0])/(p[unit]-p[0]) modulo a power of two. The denominator
// is odd, and this affine change preserves every query of that size.
struct BinaryLift {
    int m;
    vector<int>& residue;
    const vector<vector<int>>& groups;
    bool absolute;
    int unit;
    Basis basis;

    // Lifting r_i to r_i + m*b_i makes a size-2m query with sum(r_i)
    // divisible by m one linear equation in the bits b_i.
    void observe(const vector<int>& fill, initializer_list<int> tail) {
        Equation e;
        int sum = 0;
        for (int i : fill) {
            e.flip(i);
            sum += residue[i];
        }
        for (int i : tail) {
            e.flip(i);
            sum += residue[i];
        }
        assert(sum % m == 0);
        int pivot = basis.reduce(e);
        if (pivot < 0) return;
        vector<int> q = fill;
        q.insert(q.end(), tail);
        e.rhs ^= (sum / m) & 1;
        e.rhs ^= 1 - ask(q);
        basis.used[pivot] = true;
        basis.rows[pivot] = e;
    }

    // Build a common part for two queries of size 2m. Prefer to leave
    // the entire target class outside so the same part can be reused.
    vector<int> filler(vector<int> excluded, int size, int target, int avoid) {
        vector<int> pool;
        for (int i = 0; i < n; ++i)
            if (find(excluded.begin(), excluded.end(), i) == excluded.end()) pool.push_back(i);
        stable_partition(pool.begin(), pool.end(), [&](int i) { return residue[i] != avoid; });
        int sum = 0;
        for (int j = 0; j < size; ++j) sum += residue[pool[j]];
        int delta = (target - sum) & (m - 1);
        if (delta) {
            vector<int> outside(m, -1);
            for (int j = int(pool.size()) - 1; j >= size; --j) outside[residue[pool[j]]] = j;
            // Try changes that keep the target class outside first.
            int best = -1, other = -1, penalty = 2;
            for (int j = 0; j < size; ++j) {
                int k = outside[(residue[pool[j]] + delta) & (m - 1)];
                if (k < 0) continue;
                int cost = int(residue[pool[k]] == avoid) - int(residue[pool[j]] == avoid);
                if (cost < penalty) {
                    penalty = cost;
                    best = j;
                    other = k;
                }
            }
            if (best >= 0) swap(pool[best], pool[other]);
            else {
                // Reserve one of every residue for the final adjustment.
                int available = int(pool.size());
                bool complement = size > available - m + 1;
                int take = complement ? available - size : size;
                int wanted = target;
                if (complement) {
                    wanted = -target;
                    for (int i : pool) wanted += residue[i];
                }
                vector<int> reserve(m, -1), chosen;
                for (int i : pool) {
                    int r = residue[i];
                    if (reserve[r] < 0) reserve[r] = i;
                    else if (int(chosen.size()) < take - 1) {
                        chosen.push_back(i);
                        wanted -= r;
                    }
                }
                assert(take >= 1 && int(chosen.size()) == take - 1);
                int last = reserve[wanted & (m - 1)];
                assert(last >= 0);
                chosen.push_back(last);
                if (!complement) return chosen;
                vector<bool> removed(n);
                for (int i : chosen) removed[i] = true;
                erase_if(pool, [&](int i) { return removed[i]; });
                return pool;
            }
        }
        pool.resize(size);
        return pool;
    }

    void split(int r) {
        const auto& g = groups[r];
        int ref = g[0], sz = int(g.size());
        array<int, 2> seen{1, 0};
        vector<int> fill;
        vector<bool> inside(n);
        vector<int> order(sz - 1);
        iota(order.begin(), order.end(), 1);
        for (int t = 0; t < sz - 1; ++t) {
            int i = g[order[t]];
            Equation diff = variables({ref, i});
            int bit = basis.value(diff);
            if (bit < 0) {
                array<int, 2> cap{(sz + 1) / 2, (sz + 1) / 2};
                if (absolute && sz % 2) {
                    int b = basis.value(variables({ref}));
                    if (b >= 0) {
                        int high = 1 + (n - (m + r)) / (2 * m);
                        cap = {sz - high, high};
                        if (b) swap(cap[0], cap[1]);
                    }
                }
                if (seen[0] == cap[0] || seen[1] == cap[1]) {
                    bit = seen[0] == cap[0];
                    diff.rhs = bit;
                    basis.add(diff);
                } else {
                    if (fill.empty() || inside[i]) {
                        fill = filler({ref, i}, 2 * m - 1, -r, r);
                        inside.assign(n, false);
                        for (int f : fill) inside[f] = true;
                        observe(fill, {ref});
                    }
                    observe(fill, {i});
                    bit = basis.value(diff);
                    assert(bit >= 0);
                }
            }
            ++seen[bit];
            if (t == 0 && bit == 1)
                stable_partition(order.begin() + 1, order.end(), [](int j) { return j % 2 == 0; });
        }
    }

    void run() {
        if (!absolute) {
            basis.add(variables({0}));
            basis.add(variables({unit}));
        }
        for (int r = 0; r < m; ++r) {
            int sz = int(groups[r].size());
            if (absolute || sz % 2 == 0) {
                int ones = absolute ? 1 + (n - (m + r)) / (2 * m) : sz / 2;
                Equation e;
                for (int i : groups[r]) e.flip(i);
                e.rhs = ones & 1;
                basis.add(e);
            }
        }
        for (int r = 0; r < m; ++r) split(r);

        // Equal-sum pair substitutions link the class orientations.
        for (int r = 2; r < m; ++r) {
            int a = groups[r][0], b = groups[0][0];
            int c = groups[r - 1][0], d = groups[1][r == 2 ? 1 : 0];
            if (basis.value(variables({a,b,c,d})) >= 0) continue;
            auto fill = filler({a,b,c,d}, 2 * m - 2, -r, -1);
            observe(fill, {a,b});
            observe(fill, {c,d});
        }
        for (int i = 0; i < n; ++i) {
            int b = basis.value(variables({i}));
            assert(b >= 0);
            residue[i] += b * m;
        }
    }
};

vector<vector<int>> group_by(const vector<int>& residue, int m) {
    vector<vector<int>> groups(m);
    for (int i = 0; i < n; ++i) groups[residue[i]].push_back(i);
    return groups;
}

// When the longer classes form a proper interval of length 2..m-2,
// its two boundary directions determine the absolute coordinates up to reversal.
void orient(vector<int>& residue, int m) {
    auto groups = group_by(residue, m);
    for (int step = 1; step < m; step += 2) {
        int start = -1, boundaries = 0;
        for (int r = 0; r < m; ++r) {
            if (groups[r].size() > groups[(r - step) & (m - 1)].size()) {
                start = r;
                if (++boundaries > 1) break;
            }
        }
        if (boundaries == 1) {
            vector<int> to(m);
            for (int k = 1; k <= m; ++k) {
                to[start] = k & (m - 1);
                start = (start + step) & (m - 1);
            }
            for (int& r : residue) r = to[r];
            return;
        }
    }
    abort();
}

// A complete two-element class supplies a pair of witnesses for its
// missing residue modulo three, without identifying either member.
void finish_absolute(const vector<int>& residue, int m) {
    auto groups = group_by(residue,m);
    array<array<int,2>,3> witness{};
    for (auto& w : witness) w = {-1,-1};
    array<int,3> known{-1,-1,-1};
    auto add_known = [&](int i, int x) {
        int c = x % 3;
        known[c] = i;
        for (int d = 0; d < 3; ++d)
            if (d != c && known[d] >= 0) witness[3-c-d] = {i,known[d]};
    };
    vector<int> result(n);
    for (int r = 0; r < m; ++r) {
        auto& g = groups[r];
        int first = r ? r : m;
        if (g.size() == 1) {
            result[g[0]] = first;
            add_known(g[0],first);
        }
        if (g.size() == 2) witness[(first + 2*m) % 3] = {g[0],g[1]};
    }

    int trend = 0;
    for (int i = 1; i < n; ++i) {
        int d = (residue[i] - residue[i-1]) & (m-1);
        trend += (d == 1) - (d == m-1);
    }
    for (int r = 0; r < m; ++r) {
        const auto& g = groups[r];
        if (g.size() == 1) continue;
        vector<int> options;
        for (int x = r ? r : m; x <= n; x += m) options.push_back(x);
        if (trend < 0) reverse(options.begin(),options.end());
        for (int j = 0; j < int(g.size()); ++j) {
            vector<int> choices = options;
            int k = 0;
            while (choices.size() > 1) {
                while (witness[choices[k] % 3][0] < 0) ++k;
                auto w = witness[choices[k] % 3];
                if (ask({g[j],w[0],w[1]})) break;
                choices.erase(choices.begin()+k);
                k = 0;
            }
            int x = choices[k];
            erase(options,x);
            result[g[j]] = x;
            add_known(g[j],x);
        }
    }
    finish(result);
}

struct Scenario { int a, b, color_map; };

// If 3m is exactly n, the two residues give the entire cyclic order up
// to an affine bijection. Non-divisor query sizes distinguish its cuts.
void finish_full_cycle(const vector<int>& residue, int m,
                       const vector<vector<int>>& groups, array<int,3> anchor) {
    vector<int> coordinate(n), position(n);
    for (const auto& g : groups) {
        array<int,3> c{0,1,2};
        if (g[0] != anchor[0]) {
            if (!ask({g[0],anchor[1],anchor[2]})) {
                if (ask({g[0],anchor[0],anchor[2]})) swap(c[0],c[1]);
                else swap(c[0],c[2]);
            }
            if (!ask({g[1],anchor[c[0]],anchor[c[2]]})) swap(c[1],c[2]);
        }
        for (int j = 0; j < 3; ++j) {
            int z = residue[g[j]];
            while (z % 3 != c[j]) z += m;
            coordinate[g[j]] = z;
            position[z] = g[j];
        }
    }
    // Use x = 1 + ((a*z + b) mod n), keeping one sign of a.
    struct Map { int a, b; };
    vector<Map> maps;
    for (int a = 1; a < n / 2; ++a)
        if (gcd(a,n) == 1)
            for (int b = 0; b < n; ++b) maps.push_back({a,b});
    shuffle(maps.begin(),maps.end(),rng);
    auto value = [&](Map f, int z) { return 1 + (f.a * z + f.b) % n; };
    while (maps.size() > 1) {
        int sample = min(96,int(maps.size()));
        vector<int> best;
        int split = 0;
        for (int trial = 0; trial < 120 && 2 * split < sample; ++trial) {
            int k = trial % 4 == 0 ? 7 : 5;
            vector<int> q;
            while (int(q.size()) < k) {
                int z = rng() % n;
                if (find(q.begin(),q.end(),z) == q.end()) q.push_back(z);
            }
            int yes = 0;
            for (int j = 0; j < sample; ++j) {
                int sum = 0;
                for (int z : q) sum += value(maps[j],z);
                yes += sum % k == 0;
            }
            int score = min(yes,sample-yes);
            if (score > split) {
                best = q;
                split = score;
            }
        }
        if (split > 0) {
            vector<int> q;
            for (int z : best) q.push_back(position[z]);
            int response = ask(q);
            erase_if(maps,[&](Map f) {
                int sum = 0;
                for (int z : best) sum += value(f,z);
                return (sum % int(q.size()) == 0) != response;
            });
        } else {
            // Omitting one position tests whether it is an endpoint.
            // The two endpoints' coordinates determine this affine map
            // up to reflection since their difference is a unit modulo n.
            int z = 0;
            while (z < n) {
                int x = value(maps[0],z), y = value(maps[1],z);
                if ((x == 1 || x == n) != (y == 1 || y == n)) break;
                ++z;
            }
            assert(z < n);
            vector<int> q;
            for (int i = 0; i < n; ++i)
                if (i != position[z]) q.push_back(i);
            int response = ask(q);
            erase_if(maps,[&](Map f) {
                int x = value(f,z);
                return (x == 1 || x == n) != response;
            });
        }
    }
    vector<int> result(n);
    for (int i = 0; i < n; ++i) result[i] = value(maps.front(),coordinate[i]);
    finish(result);
}

void finish_mod_three(const vector<int>& residue, int m) {
    auto groups = group_by(residue, m);
    array<int,3> anchor{groups[0][0],groups[0][1],-1};
    if (3 * m == n) {
        anchor[2] = groups[0][2];
        finish_full_cycle(residue, m, groups, anchor);
        return;
    }

    // Here n = 2m. Each pair has two different colors modulo three.
    vector<int> domain(n,7);
    domain[anchor[0]] = 1;
    domain[anchor[1]] = 2;
    vector<int> order(n);
    iota(order.begin(),order.end(),0);
    shuffle(order.begin(),order.end(),rng);
    for (int i : order) {
        if (i == anchor[0] || i == anchor[1]) continue;
        if (ask({anchor[0],anchor[1],i})) {
            anchor[2] = i;
            domain[i] = 4;
            break;
        }
        domain[i] = 3;
    }

    // Record which color can be missing from each pair.
    vector<int> allowed(m);
    for (int r = 0; r < m; ++r) {
        const auto& g = groups[r];
        for (int u = 0; u < 3; ++u)
            for (int v = 0; v < 3; ++v)
                if (u != v && (domain[g[0]] & (1 << u)) && (domain[g[1]] & (1 << v)))
                    allowed[r] |= 1 << (3 - u - v);
    }

    array<vector<int>, 6> signature;
    auto color = [](int map, int x) { return ((map / 3 + 1) * x + map % 3) % 3; };
    for (int h = 0; h < 6; ++h) {
        signature[h].resize(m);
        for (int r = 0; r < m; ++r) {
            int first = r == 0 ? m : r;
            signature[h][r] = color(h, first + 2 * m);
        }
    }
    auto sig = [&](Scenario s, int r) {
        return signature[s.color_map][(s.a * r + s.b) & (m - 1)];
    };

    vector<int> restricted;
    for (int r = 0; r < m; ++r)
        if (allowed[r] != 7) restricted.push_back(r);
    vector<Scenario> possible;
    for (int a = 1; a < m / 2; a += 2) { // keep one of each reflected pair
        for (int b = 0; b < m; ++b) {
            for (int h = 0; h < 6; ++h) {
                Scenario s{a,b,h};
                if (all_of(restricted.begin(), restricted.end(), [&](int r) {
                    return allowed[r] & (1 << sig(s,r));
                })) possible.push_back(s);
            }
        }
    }
    assert(!possible.empty());
    shuffle(possible.begin(), possible.end(), rng);
    while (possible.size() > 1) {
        int best_r = -1, best_c = -1, best = 0;
        int sample = min(256, int(possible.size()));
        for (int pass = 0; pass < 2 && best == 0; ++pass) {
            if (pass) sample = int(possible.size());
            for (int r = 0; r < m; ++r) {
                if (has_single_bit(unsigned(allowed[r]))) continue;
                array<int, 3> count{};
                for (int j = 0; j < sample; ++j) ++count[sig(possible[j], r)];
                for (int c = 0; c < 3; ++c) {
                    int score = min(count[c], sample - count[c]);
                    if (score > best) {
                        best = score;
                        best_r = r;
                        best_c = c;
                    }
                }
            }
        }
        // In CRT coordinates these signatures specify an interval of
        // length 2m in Z/(3m). Only a unit step of +1 or -1 overlaps
        // that interval in 2m-1 places, so only reflection preserves it.
        assert(best > 0);
        const auto& g = groups[best_r];
        int c = best_c;
        int yes = ask({g[0], g[1], anchor[c]});
        if (yes) allowed[best_r] = 1 << c;
        else allowed[best_r] &= ~(1 << c);
        erase_if(possible, [&](Scenario s) { return (sig(s,best_r) == c) != bool(yes); });
        assert(!possible.empty());
    }

    Scenario s = possible.front();
    vector<int> result(n);
    for (int r = 0; r < m; ++r) {
        const auto& g = groups[r];
        int low = (s.a * r + s.b) & (m - 1);
        int x = low ? low : m;
        int u = color(s.color_map,x), v = color(s.color_map,x+m);
        bool forward = (domain[g[0]] & (1 << u)) && (domain[g[1]] & (1 << v));
        bool backward = (domain[g[0]] & (1 << v)) && (domain[g[1]] & (1 << u));
        if (forward && backward)
            forward = ask({g[0],anchor[(u+1)%3],anchor[(u+2)%3]});
        result[g[0]] = forward ? x : x + m;
        result[g[1]] = forward ? x + m : x;
    }
    finish(result);
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    if (!(cin >> n) || n < 2) return 0;
    if (n <= 8) {
        small_case();
        return 0;
    }
    int m = 2, unit = -1;
    vector<int> residue(n);
    array<int,2> seen{1,0};
    vector<int> order(n - 1);
    iota(order.begin(), order.end(), 1);
    for (int t = 0; t < n - 1; ++t) {
        int i = order[t];
        if (seen[0] == n/2 || seen[1] == n/2) residue[i] = seen[0] == n/2;
        else residue[i] = 1 - ask({0,i});
        ++seen[residue[i]];
        if (residue[i] && unit < 0) unit = i;
        if (t == 0 && residue[i])
            stable_partition(order.begin() + 1, order.end(), [](int j) { return j % 2 == 0; });
    }
    bool absolute = false;
    while (3 * m < n) {
        auto groups = group_by(residue, m);
        BinaryLift{m, residue, groups, absolute, unit, {}}.run();
        m *= 2;
        if (!absolute && n % m) {
            orient(residue, m);
            absolute = true;
        }
    }
    if (absolute) finish_absolute(residue,m);
    else finish_mod_three(residue,m);
}
