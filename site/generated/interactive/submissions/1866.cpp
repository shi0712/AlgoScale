#include <bits/stdc++.h>
using namespace std;

/*
   The six strategies are not described, so the statement alone does not
   imply a bounded-round solution. Use feedback to test several families
   of predictors, then fall back to fresh uniform moves. Against any
   opponent that cannot see the current move, each uniform round has
   conditional win probability 1/3, so the fallback finishes almost surely.
*/

struct History {
    vector<int> moves, results;
    vector<array<int, 3>> prefix{{0, 0, 0}};
    int wins = 0, streak = 0;

    int size() const { return static_cast<int>(moves.size()); }

    void push(int move, bool win) {
        moves.push_back(move);
        results.push_back(win);
        auto counts = prefix.back();
        ++counts[move];
        prefix.push_back(counts);
        wins += win;
        streak = win ? streak + 1 : 0;
    }
};

using Predictor = function<int(const History&)>;

struct Context {
    struct Cell {
        int mask = 7;
        array<double, 3> counts{0.5, 0.5, 0.5};
    };

    Predictor key;
    vector<Cell> cells;
    double exact, soft;
    int current = -1;

    Context(int states, double weight, Predictor f)
        : key(move(f)), cells(states), exact(weight * 0.8),
          soft(weight * 0.2) {}

    void predict(const History& h, array<double, 3>& p) {
        current = key(h);
        if (current < 0) {
            for (double& x : p) x += (exact + soft) / 3.0;
            return;
        }
        const Cell& c = cells[current];
        int n = popcount(static_cast<unsigned>(c.mask));
        double sum = c.counts[0] + c.counts[1] + c.counts[2];
        for (int i = 0; i < 3; ++i) {
            if (c.mask & (1 << i)) p[i] += exact / n;
            p[i] += soft * c.counts[i] / sum;
        }
    }

    void update(int observed) {
        if (current < 0) {
            double likelihood = popcount(static_cast<unsigned>(observed)) / 3.0;
            exact *= likelihood;
            soft *= likelihood;
            return;
        }
        Cell& c = cells[current];
        if (c.mask) {
            int intersection = c.mask & observed;
            exact *= static_cast<double>(popcount(static_cast<unsigned>(intersection)))
                   / popcount(static_cast<unsigned>(c.mask));
            c.mask = intersection;
        }

        double sum = c.counts[0] + c.counts[1] + c.counts[2];
        double compatible = 0;
        for (int i = 0; i < 3; ++i) {
            if (observed & (1 << i)) compatible += c.counts[i];
        }
        soft *= compatible / sum;
        for (int i = 0; i < 3; ++i) {
            if (observed & (1 << i)) c.counts[i] += c.counts[i] / compatible;
        }
    }
};

struct Rule {
    Predictor predict;
    double weight;
    int current = -1;
};

struct Machine {
    int current;
    array<int, 3> next;
    int type;
    double weight;

    void update(int played, int observed) {
        if (!(observed & (1 << current))) {
            weight = 0;
            return;
        }
        if (type == 0) {
            current = next[current];
        } else {
            int outcome = (played - current + 3) % 3;
            int base = type == 1 ? current : played;
            current = (base + next[outcome]) % 3;
        }
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    random_device rng;
    uniform_int_distribution<int> uniformMove(0, 2);
    vector<Context> contexts;
    vector<Rule> rules;
    vector<Machine> machines;
    double uniformWeight = 0.15;

    for (int period : {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 12, 15, 16, 18,
                       20, 21, 24, 25, 27, 30, 32, 36, 40, 48}) {
        double weight = period == 1 ? 1.4 : period == 3 ? 0.65 : 0.8 / period;
        contexts.emplace_back(period, weight, [period](const History& h) {
            return h.size() % period;
        });
    }

    int combinations = 1;
    for (int length = 1; length <= 4; ++length) {
        combinations *= 3;
        auto key = [length](const History& h) {
            if (h.size() < length) return -1;
            int k = 0;
            for (int i = h.size() - length; i < h.size(); ++i)
                k = 3 * k + h.moves[i];
            return k;
        };
        contexts.emplace_back(combinations, 0.95 / (length * length), key);
        contexts.emplace_back(2 * combinations, 0.55 / (length * length),
                              [key](const History& h) {
            int k = key(h);
            return k < 0 ? -1 : 2 * k + h.results.back();
        });
        if (length <= 3) {
            for (int shift = 0; shift < 3; ++shift) {
                double weight = (shift == 1 ? 0.24 : 0.12) / length;
                rules.push_back({[length, shift](const History& h) {
                    if (h.size() < length) return -1;
                    return (h.moves[h.size() - length] + shift) % 3;
                }, weight});
            }
        }
    }

    contexts.emplace_back(2, 0.16, [](const History& h) {
        return h.size() ? h.results.back() : -1;
    });
    contexts.emplace_back(10, 0.10, [](const History& h) {
        return h.streak;
    });
    for (int period : {2, 3, 4, 5}) {
        contexts.emplace_back(period, 0.07, [period](const History& h) {
            return h.wins % period;
        });
        contexts.emplace_back(period, 0.07, [period](const History& h) {
            return (h.size() - h.wins) % period;
        });
    }

    for (int type = 0; type < 3; ++type) {
        for (int table = 0; table < 27; ++table) {
            array<int, 3> next{table % 3, table / 3 % 3, table / 9};
            for (int start = 0; start < 3; ++start) {
                machines.push_back({start, next, type, 0.38 / 81.0});
            }
        }
    }

    for (int block = 1; block <= 6; ++block) {
        for (int step : {1, 2}) {
            for (int phase = 0; phase < 3 * block; ++phase) {
                rules.push_back({[block, step, phase](const History& h) {
                    return (step * ((h.size() + phase) / block)) % 3;
                }, 0.14 / (block * 6 * block)});
            }
        }
    }

    for (int window : {0, 3, 5, 8, 12, 20, 50}) {
        array<int, 3> order{0, 1, 2};
        do {
            for (int sign : {-1, 1}) {
                for (int shift = 0; shift < 3; ++shift) {
                    rules.push_back({[window, order, sign, shift](const History& h) {
                        int n = h.size();
                        if (n == 0) return -1;
                        int begin = window ? max(0, n - window) : 0;
                        int best = order[0];
                        int score = sign * (h.prefix[n][best] - h.prefix[begin][best]);
                        for (int i : order) {
                            int s = sign * (h.prefix[n][i] - h.prefix[begin][i]);
                            if (s > score) {
                                score = s;
                                best = i;
                            }
                        }
                        return (best + shift) % 3;
                    }, (window ? 0.09 : 0.40) * (shift == 1 ? 0.5 : 0.25) / 12});
                }
            }
        } while (next_permutation(order.begin(), order.end()));
    }

    History history;
    int streak = 0;
    constexpr int learningRounds = 256;
    constexpr char letters[] = "RPS";

    for (int round = 0; streak < 10; round += (round < learningRounds)) {
        const bool learning = round < learningRounds;
        int target;
        if (!learning) {
            target = uniformMove(rng);
        } else {
            array<double, 3> p;
            p.fill(uniformWeight / 3.0);
            for (auto& c : contexts) c.predict(history, p);
            for (auto& r : rules) {
                if (r.weight == 0) continue;
                r.current = r.predict(history);
                if (r.current < 0) {
                    for (double& x : p) x += r.weight / 3.0;
                } else {
                    p[r.current] += r.weight;
                }
            }
            for (const auto& m : machines) p[m.current] += m.weight;

            double best = *max_element(p.begin(), p.end());
            array<int, 3> ties{};
            int count = 0;
            for (int i = 0; i < 3; ++i)
                if (best - p[i] <= 1e-12) ties[count++] = i;
            target = ties[count == 1 ? 0 : uniform_int_distribution<int>(0, count - 1)(rng)];
        }

        int played = (target + 1) % 3;
        cout << letters[played] << '\n' << flush;

        string response;
        if (!(cin >> response)) return 0;
        if (response != "player" && response != "ai") return 0;

        bool win = response == "player";
        streak = win ? streak + 1 : 0;
        if (streak == 10) return 0;
        if (!learning) continue;

        int bit = 1 << target;
        int observed = win ? bit : (7 ^ bit);
        double unknownLikelihood = win ? 1.0 / 3.0 : 2.0 / 3.0;
        uniformWeight *= unknownLikelihood;
        double total = uniformWeight;
        for (auto& c : contexts) {
            c.update(observed);
            total += c.exact + c.soft;
        }
        for (auto& r : rules) {
            if (r.current < 0) r.weight *= unknownLikelihood;
            else if (!(observed & (1 << r.current))) r.weight = 0;
            total += r.weight;
        }
        for (auto& m : machines) {
            if (m.weight != 0) m.update(played, observed);
            total += m.weight;
        }

        uniformWeight /= total;
        for (auto& c : contexts) {
            c.exact /= total;
            c.soft /= total;
        }
        for (auto& r : rules) r.weight /= total;
        for (auto& m : machines) m.weight /= total;
        history.push(played, win);
    }
    return 0;
}
