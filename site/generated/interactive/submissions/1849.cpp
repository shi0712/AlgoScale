#include <bits/stdc++.h>
using namespace std;

using U = unsigned long long;
using W = __uint128_t;

constexpr int B = 61;
constexpr int LIMIT = 9;
constexpr W INF = W(1) << 126;

U n, total;
int H;

U ask(U l, U r) {
    if (l > r) return 0;
    cout << "xor " << l << ' ' << r << endl;
    long long response;
    if (!(cin >> response) || response == -1) exit(0);
    return U(response);
}

void answer(U a, U b, U c) {
    cout << "ans " << a << ' ' << b << ' ' << c << endl;
}

int msb(U x) {
    return 63 - __builtin_clzll(x);
}

void solve_small() {
    vector<array<U, 3>> candidates;
    for (U a = 1; a <= n; ++a) {
        for (U b = a + 1; b <= n; ++b) {
            U c = total ^ a ^ b;
            if (b < c && c <= n) candidates.push_back({a, b, c});
        }
    }

    auto value = [](const array<U, 3>& a, U l, U r) {
        U v = 0;
        for (U x : a) if (l <= x && x <= r) v ^= x;
        return v;
    };

    while (candidates.size() > 1) {
        U bestL = 1, bestR = 1;
        int bestMax = int(candidates.size()) + 1;
        int bestSum = INT_MAX;
        for (U l = 1; l <= n && bestMax > 1; ++l) {
            for (U r = l; r <= n && bestMax > 1; ++r) {
                int freq[128] = {};
                int largest = 0, sum = 0;
                for (const auto& a : candidates) {
                    int f = ++freq[value(a, l, r)];
                    largest = max(largest, f);
                    sum += 2 * f - 1;
                    if (largest > bestMax) break;
                }
                if (make_pair(largest, sum) < make_pair(bestMax, bestSum)) {
                    bestMax = largest;
                    bestSum = sum;
                    bestL = l;
                    bestR = r;
                }
            }
        }
        U response = ask(bestL, bestR);
        erase_if(candidates, [&](const auto& a) {
            return value(a, bestL, bestR) != response;
        });
    }
    auto a = candidates.front();
    answer(a[0], a[1], a[2]);
}

// With d = total XOR singleton, t = msb(d). A pair lies in a
// dyadic block B_h opposite total, with h > t. Its members are in
// opposite halves of one group of size 2^(t+1) within that block.
struct Span {
    U first = 1, last = 0;
};

Span groups[B][B];
U firstS[B], sizeS[B], pairPrefix[B][B + 1];

U block_base(int bit) {
    return ((total >> bit) << bit) ^ (U(1) << bit);
}

struct Aggregate {
    U count = 0, firstDeficit = 0, lastDeficit = 0;

    void add(Span s, int t) {
        if (s.first > s.last) return;
        U half = U(1) << t;
        count += s.last - s.first + 1;
        if (s.first == 0) firstDeficit = 1;
        if (s.last == (n >> (t + 1))) {
            lastDeficit = half - 1 - (n & (half - 1));
        }
    }

    U choices(int t) const {
        return count * (U(1) << t) - firstDeficit - lastDeficit;
    }
};

void init_groups() {
    for (int t = 0; t <= H; ++t) {
        U half = U(1) << t;
        U base = block_base(t);
        U l = max(U(1), base);
        U r = min(n, base + half - 1);
        if (l <= r) {
            firstS[t] = l;
            sizeS[t] = r - l + 1;
        }
        for (int h = 0; h <= H; ++h) {
            if (h > t) {
                U p = block_base(h);
                U end = min(n, p + (U(1) << h) - 1);
                if (end >= p + half) {
                    U a = p >> (t + 1);
                    U b = (end - half) >> (t + 1);
                    if (t == 0) a = max(a, U(1));
                    groups[t][h] = {a, b};
                }
            }
            Aggregate ag;
            ag.add(groups[t][h], t);
            pairPrefix[t][h + 1] = pairPrefix[t][h] + ag.choices(t);
        }
    }
}

void solve_pair(U s, int lowH, int highH) {
    U d = total ^ s;
    int t = msb(d);
    U half = U(1) << t;
    vector<Span> spans;
    U count = 0;
    for (int h = max(lowH, t + 1); h <= highH; ++h) {
        Span z = groups[t][h];
        if (z.first <= z.last) {
            spans.push_back(z);
            count += z.last - z.first + 1;
        }
    }
    sort(spans.begin(), spans.end(), [](Span a, Span b) {
        return a.first < b.first;
    });

    auto group_at = [&](U rank) {
        for (Span z : spans) {
            U len = z.last - z.first + 1;
            if (rank < len) return z.first + rank;
            rank -= len;
        }
        return U(0);
    };

    U lo = 0, hi = count;
    while (lo < hi) {
        U pivot = lo + (hi - lo) / 2;
        U g = group_at(pivot);
        U start = g << (t + 1);
        if (hi - lo == 1) {
            Aggregate ag;
            ag.add({g, g}, t);
            if (ag.choices(t) == 1) {
                U x;
                if (t == 0) x = start;
                else if (g == 0) x = 1;
                else x = n ^ d;
                answer(s, x, x ^ d);
                return;
            }
        }
        U cut = start + half - 1;
        U response = ask(1, cut);
        if (s <= cut) response ^= s;
        if (response == 0) {
            lo = pivot + 1;
        } else if (response == d) {
            hi = pivot;
        } else {
            answer(s, response, response ^ d);
            return;
        }
    }
}

// Sum of the queries used by solve_pair over every pair for a fixed
// singleton. Groups have equal weights except at the ends of [1,n].
W pair_cost(const Aggregate& ag, int t) {
    if (ag.count == 0) return 0;
    W depthSum = 0;
    U remaining = ag.count;
    for (U slots = 1, depth = 1; remaining; slots *= 2, ++depth) {
        U occupied = min(slots, remaining);
        depthSum += W(occupied) * depth;
        if (t == 0) {
            U leaves = remaining <= slots ? remaining
                : (remaining < 2 * slots ? 2 * slots - remaining : 0);
            depthSum -= leaves;
        }
        remaining -= occupied;
    }
    if (t == 0) return depthSum;

    U half = U(1) << t;
    W cost = depthSum * half;
    auto discount = [&](U rank, U deficit) {
        if (!deficit) return;
        U size = ag.count;
        U depth = 1;
        while (size > 1) {
            U mid = size / 2;
            if (rank == mid) break;
            if (rank < mid) {
                size = mid;
            } else {
                rank -= mid + 1;
                size -= mid + 1;
            }
            ++depth;
        }
        cost -= W(deficit) * depth;
        if (size == 1 && half - deficit == 1) --cost;
    };
    discount(0, ag.firstDeficit);
    discount(ag.count - 1, ag.lastDeficit);
    return cost;
}

W finishing[B][B][B];
W weight[B][B], dp[LIMIT + 1][B][B];
int action[LIMIT + 1][B][B], fixedT[B][B];

void solve_nonzero() {
    for (int t = 0; t < H; ++t) {
        for (int k = t + 1; k <= H; ++k) {
            Aggregate ag;
            for (int u = k; u <= H; ++u) {
                ag.add(groups[t][u], t);
                finishing[t][k][u] = pair_cost(ag, t) * sizeS[t];
            }
        }
    }

    for (int l = 0; l <= H; ++l) {
        for (int u = l; u <= H; ++u) {
            U singletons = 0;
            int lastT = -1;
            for (int t = l; t < u; ++t) {
                U pairs = pairPrefix[t][u + 1];
                if (pairs != 0 && sizeS[t] != 0) {
                    weight[l][u] += W(pairs) * sizeS[t];
                    singletons += sizeS[t];
                    lastT = t;
                }
            }
            fixedT[l][u] = singletons == 1 ? lastT : -1;
        }
    }

    // A prefix block around total, of size 2^k, returns total if h<k,
    // zero if t>=k, and the singleton otherwise. Optimize those
    // choices by their exact candidate counts, with a depth cap.
    for (int q = 0; q <= LIMIT; ++q) {
        for (int l = 0; l <= H; ++l) {
            for (int u = l; u <= H; ++u) {
                if (weight[l][u] == 0) continue;
                int t0 = fixedT[l][u];
                if (t0 >= 0) {
                    dp[q][l][u] = finishing[t0][t0 + 1][u];
                    continue;
                }

                W best = INF;
                int chosen = 0;
                if (q > 0) {
                    for (int k = u; k > l; --k) {
                        if (weight[l][k - 1] == weight[l][u] ||
                            weight[k][u] == weight[l][u]) continue;
                        W a = dp[q - 1][l][k - 1];
                        W b = dp[q - 1][k][u];
                        if (a == INF || b == INF) continue;
                        W cost = weight[l][u] + a + b;
                        for (int t = l; t < k; ++t)
                            cost += finishing[t][k][u];
                        if (cost < best) {
                            best = cost;
                            chosen = k;
                        }
                    }
                }
                dp[q][l][u] = best;
                action[q][l][u] = chosen;
            }
        }
    }

    int l = 0, u = H, q = LIMIT;
    while (true) {
        int t = fixedT[l][u];
        if (t >= 0) {
            solve_pair(firstS[t], t + 1, u);
            return;
        }
        int k = action[q--][l][u];
        U base = (total >> k) << k;
        U response = ask(max(U(1), base), min(n, base + (U(1) << k) - 1));
        if (response == total) {
            u = k - 1;
        } else if (response == 0) {
            l = k;
        } else {
            solve_pair(response, k, u);
            return;
        }
    }
}

void solve_zero() {
    int u = H;
    if (n == (U(1) << u)) --u;

    for (int tries = 0; tries < 2 && u > 1; ++tries, --u) {
        U response = ask(U(1) << u, min(n, (U(1) << (u + 1)) - 1));
        if (response != 0) {
            solve_pair(response, u, u);
            return;
        }
    }

    // For total zero, the two occupied bit-length buckets h and t
    // each contribute s. After a zero reply they stay in the same
    // class. One interval can test a cut in each of two classes.
    vector<pair<int, int>> classes = {{0, u}};
    while (true) {
        if (classes.size() == 1 && classes[0] == make_pair(0, 1)) {
            solve_pair(1, 1, 1);
            return;
        }

        vector<pair<int, int>> cuts;
        if (classes.size() == 1) {
            auto [l, r] = classes[0];
            cuts.push_back({0, l + (r - l + 1) / 2});
        } else {
            int top = int(classes.size()) - 1;
            int other = 0;
            for (int i = 1; i < top; ++i) {
                if (classes[i].second - classes[i].first >=
                    classes[other].second - classes[other].first) other = i;
            }
            auto [l, r] = classes[other];
            cuts.push_back({other, l + (r - l + 1) / 2});
            cuts.push_back({top, classes[top].second});
        }

        U response = cuts.size() == 1
            ? ask(1, (U(1) << cuts[0].second) - 1)
            : ask(U(1) << cuts[0].second, (U(1) << cuts[1].second) - 1);
        if (response != 0) {
            int t = msb(response);
            for (auto [i, k] : cuts) {
                if (classes[i].first <= t && t < k) {
                    solve_pair(response, k, classes[i].second);
                    return;
                }
            }
        }

        vector<pair<int, int>> next;
        for (int i = 0; i < int(classes.size()); ++i) {
            auto [l, r] = classes[i];
            int k = -1;
            for (auto [j, cut] : cuts) if (i == j) k = cut;
            if (k == -1) {
                next.push_back({l, r});
            } else {
                if (l < k - 1) next.push_back({l, k - 1});
                if (k < r) next.push_back({k, r});
            }
        }
        classes.swap(next);
    }
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    if (!(cin >> n)) return 0;
    if (n == 3) {
        answer(1, 2, 3);
        return 0;
    }

    total = ask(1, n);
    if (n <= 64) {
        solve_small();
        return 0;
    }

    H = msb(n);
    init_groups();
    if (total == 0) solve_zero();
    else solve_nonzero();
    return 0;
}
