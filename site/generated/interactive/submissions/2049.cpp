#include <bits/stdc++.h>
using namespace std;

// Unused original zero ranks are initially 1..n. Unused ranks above the
// true number of zeros never affect a valid order-statistic request.
struct Ranks {
    vector<int> tree;
    explicit Ranks(int n) : tree(n + 1) {
        for (int i = 1; i <= n; ++i) tree[i] = i & -i;
    }
    int select(int k) const {
        int pos = 0;
        for (int step = bit_floor(unsigned(tree.size() - 1)); step; step >>= 1) {
            int next = pos + step;
            if (next < int(tree.size()) && tree[next] < k) {
                pos = next;
                k -= tree[next];
            }
        }
        return pos + 1;
    }
    void erase(int rank) {
        for (int i = rank; i < int(tree.size()); i += i & -i) --tree[i];
    }
};

// For short gaps, minimize expected work assuming uniformly placed zeros.
// visit[z] accounts for the chance that future rounds also use the side
// of a split that does not contain the current target.
struct Planner {
    static constexpr int limit = 72;
    static constexpr int N = limit + 1;
    using Cube = double[N][N][N];
    Cube exact{}, lower{};
    int cut[N][N][N]{}, lowerCut[N][N][N]{};
    double choose[N][N]{}, visit[N]{}, other[N][N]{}, otherLower[N][N]{};
    double tail[N][N]{};
    int built = 0;
    double futureShare;

    explicit Planner(double future) : futureShare(future) {
        for (int w = 0; w <= limit; ++w) {
            choose[w][0] = choose[w][w] = 1;
            for (int z = 1; z < w; ++z)
                choose[w][z] = choose[w-1][z-1] + choose[w-1][z];
            for (int z = w; z >= 0; --z)
                tail[w][z] = choose[w][z] + (z < w ? tail[w][z+1] : 0);
            visit[w] = 1 - pow(1 - future, w);
        }
    }

    void prepare(int width) {
        while (built < width) {
            int w = ++built;
            for (int z = 1; z < w; ++z) {
                fill_n(exact[w][z] + 1, z, 1e30);
                for (int s = 1; s < w; ++s) {
                    int r = w - s;
                    double cost[N]{};
                    for (int h = max(0, z-r); h <= min(z,s); ++h) {
                        double prob = choose[s][h] * choose[r][z-h] / choose[w][z];
                        for (int d = 1; d <= h; ++d)
                            cost[d] += prob * (exact[s][h][d] + other[r][z-h]);
                        for (int d = h+1; d <= z; ++d)
                            cost[d] += prob * (exact[r][z-h][d-h] + other[s][h]);
                    }
                    for (int d = 1; d <= z; ++d) {
                        if (1 + cost[d] < exact[w][z][d]) {
                            exact[w][z][d] = 1 + cost[d];
                            cut[w][z][d] = s;
                        }
                    }
                }
                other[w][z] = visit[z] * accumulate(exact[w][z]+1,
                                                    exact[w][z]+z+1, 0.0) / z;
            }

            // An unmeasured suffix has only a lower bound b on its count.
            // Use independent fair bits conditioned on this bound as a prior.
            for (int b = 1; b < w; ++b) {
                for (int d = 1; d <= b; ++d) {
                    double cost = 1;
                    for (int z = b; z <= w; ++z)
                        cost += choose[w][z] / tail[w][b] * exact[w][z][d];
                    lower[w][b][d] = cost;
                    lowerCut[w][b][d] = w;
                }
                for (int s = 1; s < w; ++s) {
                    int r = w-s;
                    double cost[N]{};
                    for (int h = max(0,b-r); h <= s; ++h) {
                        int rb = max(0,b-h);
                        double prob = choose[s][h] * tail[r][rb] / tail[w][b];
                        for (int d = 1; d <= min(b,h); ++d)
                            cost[d] += prob * (exact[s][h][d] + otherLower[r][rb]);
                        for (int d = h+1; d <= b; ++d)
                            cost[d] += prob * (lower[r][rb][d-h] + other[s][h]);
                    }
                    for (int d = 1; d <= b; ++d) {
                        if (1 + cost[d] < lower[w][b][d]) {
                            lower[w][b][d] = 1 + cost[d];
                            lowerCut[w][b][d] = s;
                        }
                    }
                }
            }
            for (int b = 0; b < w; ++b) {
                for (int z = max(1,b); z <= w; ++z) {
                    double sum = 0;
                    for (int d = 1; d <= z; ++d) sum += lower[w][max(b,d)][d];
                    otherLower[w][b] += choose[w][z] / tail[w][b] * visit[z] * sum / z;
                }
            }
        }
    }
};

// Prefixes with an equal original zero count form a run of ones. Only
// the extreme observed positions for each count can tighten a search.
struct Extent { int first, last; };
using Anchors = map<int, Extent>;

// A long homogeneous stretch beside a gap can indicate a single
// transition inside it. Its orientation determines the exact cut to try.
int transitionCut(const Anchors &prefix, Anchors::const_iterator lo,
                  Anchors::const_iterator hi) {
    int left = lo->second.last, right = hi->second.first;
    int w = right-left, z = hi->first-lo->first;
    int hint = 0;
    auto consider = [&](int length, bool zeros, bool onLeft) {
        int opposite = zeros ? w-z : z;
        if (1LL * length * opposite < 5LL*w || 8*length < w) return;
        int candidate = zeros == onLeft ? z : w-z;
        hint = hint == 0 || hint == candidate ? candidate : -1;
    };
    if (lo->second.first < left) {
        consider(left-lo->second.first, false, true);
    } else if (lo != prefix.begin()) {
        auto prior = prev(lo);
        int span = left-prior->second.last;
        if (span == lo->first-prior->first) consider(span, true, true);
    }
    if (hi->second.last > right) {
        consider(hi->second.last-right, false, false);
    } else if (next(hi) != prefix.end()) {
        auto after = next(hi);
        int span = after->second.first-right;
        if (span == after->first-hi->first) consider(span, true, false);
    }
    return hint;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, rounds;
    if (!(cin >> n >> rounds) || n < 1 || rounds < 1) return 0;
    Ranks ranks(n);
    Anchors prefix{{0, {0,0}}};
    auto record = [&](int count, int pos) {
        auto [it, added] = prefix.try_emplace(count, Extent{pos,pos});
        if (!added) {
            it->second.first = min(it->second.first, pos);
            it->second.last = max(it->second.last, pos);
        }
    };
    int total = -1, least = rounds;
    unique_ptr<Planner> planner, sequentialPlanner;
    int prevRank = 0, streak = 0;

    for (int turn = 0; turn < rounds; ++turn) {
        int k;
        if (!(cin >> k) || k < 1) return 0;
        int rank = ranks.select(k);
        least = max(least, rank);
        if (turn && rank == prevRank+1) streak = max(0,streak)+1;
        else if (turn && rank == prevRank-1) streak = min(0,streak)-1;
        else streak = 0;
        prevRank = rank;
        int pos = 0, slowDir = 0, slowCount = 0;
        bool triedRun = false;
        while (true) {
            auto hi = prefix.lower_bound(rank);
            auto lo = prev(hi);
            int left = lo->second.last;
            bool known = hi != prefix.end();
            int right = known ? hi->second.first : n;
            int w = right-left, d = rank-lo->first;
            int z = (known ? hi->first : least) - lo->first;
            if (w == z) {
                pos = left+d;
                break;
            }

            int s;
            if (w <= Planner::limit) {
                int remaining = rounds-turn-1;
                bool sequential = abs(streak) >= 2 && remaining >= max(5,z/2);
                auto &table = sequential ? sequentialPlanner : planner;
                double estimate = total >= 0 ? total : max(double(least), n * 0.5);
                double future = sequential ? 1.0 :
                    clamp(remaining / max(1.0,estimate-turn-1), 0.0, 1.0);
                if (!table || abs(table->futureShare-future) > max(0.025,table->futureShare*0.5))
                    table = make_unique<Planner>(future);
                table->prepare(w);
                s = known ? table->cut[w][z][d] : table->lowerCut[w][z][d];
            } else if (!known) {
                // Measure a large suffix when its density will aid this or
                // later searches. Near the start a local probe can suffice.
                if (rounds-turn > 4 || slowCount >= 2) s = w;
                else if (w-z <= 8) s = d + int(1LL * d * (w-z) / (w+1));
                else {
                    double density = (lo->first + 1.0) / (left + 2.0);
                    double estimate = d + (d-0.5) * (1-density) / density;
                    s = int(clamp(estimate, double(d), double(w)));
                }
            } else {
                int rem = rounds-turn-1;
                double density = double(z)/w;
                // Estimate how many of the ones precede the desired zero
                // using the median of a uniform order statistic.
                double fraction = (d-1.0/3) / (z+1.0/3);
                s = d + int(floor((w-z)*fraction + 0.5));
                if (fraction > 0.5) --s;
                double load = double(rem) / max(1, (total >= 0 ? total : max(least,n/2))-turn-1);
                if (load > 0.5 && z > 10)
                    s = w / 2;
                else if (streak >= 2 && d <= 2 && rem >= 5)
                    s = min(w/2, max(2, int(min(32,rem+1) / density)));
                else if (streak <= -2 && z-d <= 1 && rem >= 5)
                    s = w - min(w/2, max(2, int(min(32,rem+1) / density)));
                // If the prediction keeps falling in the same run of
                // ones, grow the distance probed past that run.
                int leadingOnes = left - lo->second.first;
                int trailingOnes = hi->second.last - right;
                if (leadingOnes >= s && s < w/4)
                    s = min(w/2, 2*leadingOnes);
                else if (trailingOnes >= w-s && w-s < w/4)
                    s = w-min(w/2, 2*trailingOnes);
                else if (slowCount >= 2)
                    s = w/2;

                if (!triedRun && slowCount == 0) {
                    int hint = transitionCut(prefix, lo, hi);
                    if (hint > 0) {
                        s = hint;
                        triedRun = true;
                    }
                }
                s = clamp(s, 1, w-1);
            }

            // All removed positions have prefix anchors on both sides, so
            // the open search gap contains no previously removed zero.
            cout << "? " << left+1 << ' ' << left+s << endl;
            int ones;
            if (!(cin >> ones) || ones < 0 || ones > s) return 0;
            int h = s-ones;
            record(lo->first+h, left+s);
            if (left+s == n) total = lo->first+h;
            int dir = h < d ? 1 : -1;
            int nextWidth = h < d ? w-s : s;
            if (nextWidth * 5 > w * 4 && s < w && (known || 2*h < d)) {
                slowCount = dir == slowDir ? slowCount+1 : 1;
                slowDir = dir;
            } else slowCount = 0;
        }
        // Finding rank at pos establishes both neighboring prefix counts.
        cout << "! " << pos << endl;
        record(rank-1, pos-1);
        record(rank, pos);
        ranks.erase(rank);
    }
}
