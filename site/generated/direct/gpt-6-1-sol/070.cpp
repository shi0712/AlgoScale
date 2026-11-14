#include <bits/stdc++.h>
using namespace std;

struct Hypothesis {
    int x;
    bool unique;
};

struct Action {
    // type 0: move the current wheel to total displacement 'value'
    // type 1: move the probe to position 'value'
    int type = -1;
    int value = -1;
};

class Solver {
    int n;
    int response;
    int actions = 0;

    int probePosition = 0;
    int K = 0;
    int target = 0;

    vector<bool> occupied;
    mt19937 rng;

    int mod(int x) const {
        x %= n;
        if (x < 0) x += n;
        return x;
    }

    void rotateWheel(int wheel, int delta) {
        delta = mod(delta);
        if (delta == 0) return;

        if (actions >= 1333) exit(0);
        ++actions;

        // Use a short signed representation.
        if (delta > n / 2) delta -= n;

        cout << wheel + 1 << ' ' << delta << endl;

        if (!(cin >> response)) exit(0);
        if (response <= 1) exit(0);
    }

    // Predict the answer for a hypothesis when the selected wheel has
    // displacement d and the probe is at p.
    int predict(const Hypothesis& h, int d, int p) const {
        int y = (h.x + d) % n;

        int count;
        if (d == 0) {
            count = K;
        } else {
            count = K - int(h.unique) + int(!occupied[y]);
        }

        bool probeCovered =
            (occupied[p] && !(h.unique && p == h.x)) || p == y;

        return count + int(!probeCovered);
    }

    // Every possible original position receives equal total weight.
    vector<int> makeWeights(const vector<Hypothesis>& hypotheses) const {
        vector<int> multiplicity(n, 0);
        for (const auto& h : hypotheses) {
            ++multiplicity[h.x];
        }

        vector<int> weights;
        weights.reserve(hypotheses.size());

        for (const auto& h : hypotheses) {
            weights.push_back(multiplicity[h.x] == 1 ? 2 : 1);
        }
        return weights;
    }

    long long score(
        const vector<Hypothesis>& hypotheses,
        const vector<int>& weights,
        int d,
        int p
    ) const {
        int buckets[52] = {};

        for (int i = 0; i < (int)hypotheses.size(); ++i) {
            int answer = predict(hypotheses[i], d, p);
            buckets[answer] += weights[i];
        }

        long long result = 0;
        for (int v : buckets) {
            result += 1LL * v * v;
        }
        return result;
    }

    Action chooseAction(
        const vector<Hypothesis>& hypotheses,
        int displacement
    ) {
        vector<int> weights = makeWeights(hypotheses);
        int totalWeight =
            accumulate(weights.begin(), weights.end(), 0);

        const long long uninformative =
            1LL * totalWeight * totalWeight;

        long long bestScore = LLONG_MAX;
        int ties = 0;
        Action best;

        auto consider = [&](long long value, Action action) {
            if (value < bestScore) {
                bestScore = value;
                best = action;
                ties = 1;
            } else if (value == bestScore) {
                ++ties;
                if (rng() % ties == 0) best = action;
            }
        };

        // Try moving the selected wheel.
        for (int d = 0; d < n; ++d) {
            if (d == displacement) continue;
            consider(
                score(hypotheses, weights, d, probePosition),
                {0, d}
            );
        }

        // Try moving the probe.
        for (int p = 0; p < n; ++p) {
            if (p == probePosition) continue;
            consider(
                score(hypotheses, weights, displacement, p),
                {1, p}
            );
        }

        if (bestScore < uninformative) return best;

        // A preparatory wheel movement may be needed before the probe
        // can distinguish the hypotheses.
        bestScore = LLONG_MAX;
        ties = 0;
        best = {};

        for (int d = 0; d < n; ++d) {
            if (d == displacement) continue;

            for (int p = 0; p < n; ++p) {
                if (p == probePosition) continue;

                consider(
                    score(hypotheses, weights, d, p),
                    {0, d}
                );
            }
        }

        return best;
    }

    void identifyAndAlign(int wheel, int remaining, int aligned) {
        vector<Hypothesis> hypotheses;

        if (aligned == 0) {
            // All non-probe wheels are still unprocessed.
            for (int x = 0; x < n; ++x) {
                if (!occupied[x]) continue;

                hypotheses.push_back({x, true});
                if (remaining > K) {
                    hypotheses.push_back({x, false});
                }
            }
        } else {
            // Each occupied position other than target needs at least
            // one unprocessed wheel.
            int surplus = remaining - (K - 1);

            for (int x = 0; x < n; ++x) {
                if (!occupied[x]) continue;

                if (x == target) {
                    // Processed wheels already keep target occupied.
                    if (surplus > 0) {
                        hypotheses.push_back({x, false});
                    }
                } else {
                    hypotheses.push_back({x, true});
                    if (surplus > 0) {
                        hypotheses.push_back({x, false});
                    }
                }
            }
        }

        int displacement = 0;

        while (true) {
            if (hypotheses.empty()) exit(0);

            int x = hypotheses.front().x;
            bool determined = true;

            for (const auto& h : hypotheses) {
                if (h.x != x) {
                    determined = false;
                    break;
                }
            }

            if (determined) {
                // Move this wheel to target.
                int currentPosition = (x + displacement) % n;
                rotateWheel(wheel, target - currentPosition);

                // With both the probe and this wheel at target,
                // the response equals the number of positions occupied
                // by non-probe wheels.
                if (probePosition != target) {
                    rotateWheel(0, target - probePosition);
                    probePosition = target;
                }

                if (x != target && response == K - 1) {
                    occupied[x] = false;
                }

                occupied[target] = true;
                K = response;
                return;
            }

            Action action = chooseAction(hypotheses, displacement);

            if (action.type == 0) {
                rotateWheel(wheel, action.value - displacement);
                displacement = action.value;
            } else {
                rotateWheel(0, action.value - probePosition);
                probePosition = action.value;
            }

            vector<Hypothesis> next;
            next.reserve(hypotheses.size());

            for (const auto& h : hypotheses) {
                if (predict(h, displacement, probePosition) == response) {
                    next.push_back(h);
                }
            }

            hypotheses.swap(next);
        }
    }

public:
    Solver()
        : rng((uint32_t)chrono::steady_clock::now()
                  .time_since_epoch().count()) {}

    void run() {
        if (!(cin >> n)) return;
        if (!(cin >> response)) return;
        if (response <= 1) return;

        // Scan every position of the probe. Its initial position is 0.
        vector<int> scan(n);
        scan[0] = response;

        for (int p = 1; p < n; ++p) {
            rotateWheel(0, 1);
            probePosition = p;
            scan[p] = response;
        }

        K = *min_element(scan.begin(), scan.end());
        occupied.assign(n, false);

        vector<int> possibleTargets;
        for (int p = 0; p < n; ++p) {
            occupied[p] = (scan[p] == K);
            if (occupied[p]) possibleTargets.push_back(p);
        }

        target = possibleTargets[rng() % possibleTargets.size()];

        vector<int> order;
        for (int wheel = 1; wheel < n; ++wheel) {
            order.push_back(wheel);
        }
        shuffle(order.begin(), order.end(), rng);

        int aligned = 0;
        int remaining = n - 1;

        for (int wheel : order) {
            identifyAndAlign(wheel, remaining, aligned);
            --remaining;
            ++aligned;
        }
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    Solver solver;
    solver.run();
    return 0;
}
