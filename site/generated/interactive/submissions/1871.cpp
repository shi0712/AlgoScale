#include <bits/stdc++.h>
using namespace std;

constexpr int SMALL = 48;
constexpr int BUDGET = 60000;
constexpr int LEVELS = 20;
constexpr double DENSITIES[] = {
    0.0625, 0.125, 0.25, 0.375, 0.5, 0.625, 0.75, 0.875, 0.9375
};
constexpr int MODELS = size(DENSITIES);

struct Decision {
    double cost = 0;
    int cut = 0;
};

double comb[SMALL + 1][SMALL + 1];
Decision exactDP[SMALL + 1][SMALL + 1][SMALL + 1];
Decision tailDP[MODELS][SMALL + 1][SMALL + 1][SMALL + 1];
bool exactReady = false;
bool tailReady[MODELS];

void consider(Decision& best, double cost, int cut, int center) {
    constexpr double EPS = 1e-10;
    if (cost < best.cost - EPS ||
        (abs(cost - best.cost) <= EPS &&
         abs(2 * cut - center) < abs(2 * best.cut - center))) {
        best = {cost, cut};
    }
}

void buildExact() {
    if (exactReady) return;
    exactReady = true;

    comb[0][0] = 1;
    for (int m = 1; m <= SMALL; ++m) {
        comb[m][0] = comb[m][m] = 1;
        for (int z = 1; z < m; ++z)
            comb[m][z] = comb[m - 1][z - 1] + comb[m - 1][z];
    }

    for (int m = 2; m <= SMALL; ++m) {
        for (int z = 1; z < m; ++z) {
            for (int k = 1; k <= z; ++k) {
                Decision best{1e100, 0};
                int center = 2 * k + m - z - 1;
                for (int q = 1; q < m; ++q) {
                    double expected = 0;
                    for (int a = max(0, z - (m - q)); a <= min(q, z); ++a) {
                        double next = (a >= k)
                            ? exactDP[q][a][k].cost
                            : exactDP[m - q][z - a][k - a].cost;
                        expected += comb[q][a] * comb[m - q][z - a] * next;
                    }
                    consider(best, 1 + expected / comb[m][z], q, center);
                }
                exactDP[m][z][k] = best;
            }
        }
    }
}

void buildTail(int model) {
    if (tailReady[model]) return;
    tailReady[model] = true;
    buildExact();

    double p = DENSITIES[model];
    double mass[SMALL + 1][SMALL + 1]{};
    double atLeast[SMALL + 1][SMALL + 2]{};
    mass[0][0] = atLeast[0][0] = 1;
    for (int m = 1; m <= SMALL; ++m) {
        for (int z = 0; z <= m; ++z) {
            mass[m][z] = mass[m - 1][z] * (1 - p);
            if (z) mass[m][z] += mass[m - 1][z - 1] * p;
        }
        for (int z = m; z >= 0; --z)
            atLeast[m][z] = mass[m][z] + atLeast[m][z + 1];
    }

    auto& dp = tailDP[model];
    for (int m = 2; m <= SMALL; ++m) {
        for (int low = 1; low < m; ++low) {
            for (int k = 1; k <= low; ++k) {
                Decision best{1e100, 0};
                int center = 2 * k + m - low - 1;
                for (int q = 1; q <= m; ++q) {
                    double expected = 0;
                    for (int a = max(0, low - (m - q)); a <= q; ++a) {
                        double next = (a >= k)
                            ? exactDP[q][a][k].cost
                            : dp[m - q][low - a][k - a].cost;
                        expected += mass[q][a] *
                                    atLeast[m - q][max(0, low - a)] * next;
                    }
                    consider(best, 1 + expected / atLeast[m][low], q, center);
                }
                dp[m][low][k] = best;
            }
        }
    }
}

struct Fenwick {
    int n, top;
    vector<int> tree;

    explicit Fenwick(int n) : n(n), top(bit_floor((unsigned)n)), tree(n + 1) {}

    void add(int i, int delta) {
        for (; i <= n; i += i & -i) tree[i] += delta;
    }

    // Returns the endpoint of the gap containing rank k and the rank in that gap.
    pair<int, int> select(int k) const {
        int i = 0;
        for (int step = top; step; step >>= 1) {
            int j = i + step;
            if (j <= n && tree[j] < k) {
                i = j;
                k -= tree[j];
            }
        }
        return {i + 1, k};
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, t;
    if (!(cin >> n >> t) || n <= 0 || t <= 0) return 0;

    Fenwick fw(n);
    set<int> ends{0, n};
    vector<int> weight(n + 1);
    weight[n] = n;
    fw.add(n, n);

    // An endpoint q has canonical level v2(q); larger levels split first.
    array<int, LEVELS> unrecorded{};
    for (int h = 0; h < LEVELS; ++h)
        unrecorded[h] = ((n - 1) >> h) - ((n - 1) >> (h + 1));

    auto record = [&](int pos, int zeros) {
        if (ends.insert(pos).second && pos < n)
            --unrecorded[countr_zero((unsigned)pos)];
        fw.add(pos, zeros - weight[pos]);
        weight[pos] = zeros;
    };

    bool totalKnown = false;
    int globalLower = t;
    int queries = 0;

    for (int round = 0; round < t; ++round) {
        int rank;
        if (!(cin >> rank) || rank <= 0) return 0;
        const int remaining = t - round;
        globalLower = max(globalLower, rank);

        int priorWidth = -1;
        int slow = 0;

        while (true) {
            auto [r, k] = fw.select(rank);
            int l = *prev(ends.find(r));
            int m = r - l;
            bool exact = r < n || totalKnown;

            // Until the last gap is measured, its Fenwick weight is its
            // length. This routes ranks correctly without claiming it is exact.
            int z = exact ? weight[r] : globalLower - (rank - k);
            int width = m - z;
            int a = l + k;
            int b = a + width;

            if (width == 0) {
                if (r == n) totalKnown = true;
                if (a - 1 > l) record(a - 1, k - 1);
                record(a, 0);
                if (a < r) record(r, r - a);

                cout << "! " << a << '\n' << flush;
                --globalLower;
                break;
            }

            if (priorWidth >= 0) {
                if (4 * width > 3 * priorWidth) ++slow;
                else slow = 0;
            }

            unsigned different = (unsigned)(a - 1) ^ (unsigned)(b - 1);
            int level = (int)bit_width(different) - 1;
            int canonical = ((b - 1) >> level) << level;
            int q;

            double p = 0.5;
            if (!exact) {
                // All previous removals are to the left of the unknown tail.
                p = (rank - k + round + 6.0) / (l + 12.0);
            }

            if (m <= SMALL) {
                if (exact) {
                    buildExact();
                    q = l + exactDP[m][z][k].cut;
                } else {
                    int model = 0;
                    for (int i = 1; i < MODELS; ++i) {
                        if (abs(p - DENSITIES[i]) < abs(p - DENSITIES[model]))
                            model = i;
                    }
                    buildTail(model);
                    q = l + tailDP[model][m][z][k].cut;
                }
            } else if (!exact && remaining >= 8 && width >= 4 && slow < 2) {
                // Measure the total once when many later rounds can reuse it.
                q = r;
            } else if (slow >= 2) {
                q = a + (width - 1) / 2;
                if (canonical - a >= width / 4 && b - 1 - canonical >= width / 4)
                    q = canonical;
            } else {
                double estimatedZeros = exact ? z : max<double>(z, m * p);
                double fraction = (k - 1.0 / 3) / (estimatedZeros + 1.0 / 3);
                double location = k - 0.5 + (m - estimatedZeros + 1) * fraction;
                q = clamp(l + (int)floor(location), a, b - 1);
                if (width >= 24 && abs(q - canonical) <= 1) q = canonical;
            }

            if (q != canonical) {
                // Reserve enough queries to finish with canonical cuts.
                // This round can use only levels at or below its current split.
                int reserve = 0;
                for (int h = 0; h < LEVELS; ++h)
                    reserve += min(unrecorded[h], remaining - (h > level));
                if (queries + 1 + reserve > BUDGET) q = canonical;
            }

            cout << "? " << l + 1 << ' ' << q << '\n' << flush;
            int ones;
            if (!(cin >> ones) || ones < 0 || ones > q - l) return 0;
            ++queries;
            priorWidth = width;

            int got = q - l - ones;
            if (q == r) {
                totalKnown = true;
                record(r, got);
            } else {
                int rightZeros = exact ? weight[r] - got : r - q;
                record(q, got);
                record(r, rightZeros);
            }
        }
    }
    return 0;
}
