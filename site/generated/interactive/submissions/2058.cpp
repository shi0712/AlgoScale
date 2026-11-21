#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <random>
#include <string>
#include <vector>

using namespace std;

using Prob = array<double, 3>;
using Counts = array<int, 3>;
constexpr Prob opening{0.50, 0.27, 0.23};
constexpr int windows[] = {2, 3, 4, 5, 6, 8, 10, 16, 32};
constexpr int frequencyTypes = size(windows) + 3;
constexpr int orders[6][3] = {{0,1,2}, {0,2,1}, {1,0,2},
                              {1,2,0}, {2,0,1}, {2,1,0}};

struct History {
    int round = 0, wins = 0, streak = 0;
    array<int, 64> moves{}, won{};
    array<int, 2> lastByResult{-1, -1};
    array<Counts, frequencyTypes> counts{};
    array<Counts, 3> afterOne{};
    array<Counts, 9> afterTwo{};
    array<array<int, 2>, frequencyTypes> frequencyMask{};

    int move(int lag) const { return moves[(round - lag) & 63]; }
    int result(int lag) const { return won[(round - lag) & 63]; }

    void prepare() {
        counts[size(windows) + 1] = round ? afterOne[move(1)] : Counts{};
        counts[size(windows) + 2] = round >= 2 ? afterTwo[3*move(2) + move(1)] : Counts{};
        for (int i = 0; i < frequencyTypes; ++i) {
            const auto &c = counts[i];
            int hi = max({c[0], c[1], c[2]}), lo = min({c[0], c[1], c[2]});
            frequencyMask[i] = {};
            for (int j = 0; j < 3; ++j) {
                if (c[j] == hi) frequencyMask[i][0] |= 1 << j;
                if (c[j] == lo) frequencyMask[i][1] |= 1 << j;
            }
        }
    }

    void record(int m, bool success) {
        for (int i = 0; i < int(size(windows)); ++i) {
            if (round >= windows[i]) --counts[i][move(windows[i])];
            ++counts[i][m];
        }
        ++counts[size(windows)][m];
        if (round) ++afterOne[move(1)][m];
        if (round >= 2) ++afterTwo[move(2)*3 + move(1)][m];
        moves[round & 63] = m;
        won[round & 63] = success;
        lastByResult[success] = m;
        wins += success;
        streak = success ? streak + 1 : 0;
        ++round;
    }
};

enum class Kind { Run, Alternate, Response, React, Frequency, LastResult };

struct Rule {
    Kind kind;
    double weight;
    int a, b, c, d;
    int prediction = -1;

    int predict(const History& h) const {
        switch (kind) {
        case Kind::Run:       return (a + b*((h.round + d)/c)) % 3;
        case Kind::Alternate: return (h.round / c) % 2 ? b : a;
        case Kind::Response:  return h.round < a ? -1 : (b*h.move(a) + c) % 3;
        case Kind::React:     return a;
        case Kind::LastResult: {
            int m = h.lastByResult[a];
            return m < 0 ? -1 : (m + b) % 3;
        }
        case Kind::Frequency: {
            if (!h.round) return -1;
            int mask = h.frequencyMask[a][b];
            if (d == 6 && (mask & (1 << h.move(1)))) return (h.move(1) + c) % 3;
            for (int m : orders[d % 6])
                if (mask & (1 << m)) return (m + c) % 3;
        }
        }
        return -1;
    }

    void advance(int played) {
        if (kind == Kind::React) {
            int result = (played - a + 3) % 3;
            int shift = result == 0 ? b : result == 1 ? c : d;
            a = (a + shift) % 3;
        }
    }
};

enum class Key { Time, Wins, Losses, Streak, Moves, Results, History, PhaseMove };

struct Table {
    Key kind;
    int order;
    bool relative;
    double weight, prior, noise;
    vector<unsigned char> masks;
    vector<Prob> beliefs;
    int index = 0, offset = 0, warmup = 0;
    Prob p{};

    Table(Key k, int n, int cells, bool rel, double w, double error)
        : kind(k), order(n), relative(rel), weight(w), prior(w), noise(error) {
        if (noise) beliefs.assign(cells, Prob{1./3, 1./3, 1./3});
        else masks.assign(cells, 7);
    }

    int getIndex(const History& h) const {
        switch (kind) {
        case Key::Time:   return h.round % order;
        case Key::Wins:   return h.wins % order;
        case Key::Losses: return (h.round - h.wins) % order;
        case Key::Streak: return min(order, h.streak);
        case Key::PhaseMove:
            return h.round ? h.round % order * 3 + h.move(1) : -1;
        default: break;
        }
        if (h.round < order) return -1;
        int key = 0;
        for (int i = order; i >= 1; --i) {
            if (kind != Key::Results) key = key*3 + h.move(i);
            if (kind != Key::Moves) key = key*2 + h.result(i);
        }
        return key;
    }

    void predict(const History& h) {
        index = h.round < warmup ? -1 : getIndex(h);
        offset = relative && h.round ? h.move(1) : 0;
        if (index < 0 || (relative && !h.round)) {
            index = -1;
            offset = 0;
            p = h.round ? Prob{1./3, 1./3, 1./3} : opening;
        } else if (noise) {
            for (int m = 0; m < 3; ++m)
                p[m] = beliefs[index][m]*(1-noise) + noise/3;
        } else {
            unsigned mask = masks[index];
            double share = 1.0 / __builtin_popcount(mask);
            for (int m = 0; m < 3; ++m) p[m] = (mask >> m & 1) * share;
        }
    }

    void observe(int target, bool success) {
        int m = (target - offset + 3) % 3;
        double likelihood = success ? p[m] : 1 - p[m];
        weight *= likelihood;
        if (index < 0 || likelihood == 0) return;
        if (noise) {
            for (int i = 0; i < 3; ++i)
                beliefs[index][i] = ((i == m) == success) ? p[i]/likelihood : 0;
        } else {
            masks[index] &= success ? (1 << m) : ~(1 << m);
        }
    }
};

struct Predictor {
    History h;
    vector<Rule> rules;
    vector<Table> tables;
    mt19937 rng{random_device{}()};
    int nextRandom = 320, randomLeft = 0;

    Predictor() {
        auto add = [&](Kind k, double w, int a, int b=0, int c=0, int d=0) {
            rules.push_back({k, w, a, b, c, d});
        };
        for (int m = 0; m < 3; ++m) {
            add(Kind::Run, 0.23*opening[m], m, 0, 1);
            for (int dir : {1, 2}) {
                add(Kind::Run, opening[m]*(dir == 1 ? 0.098 : 0.057), m, dir, 1);
                for (int len = 2; len <= 12; ++len) {
                    for (int phase = 0; phase < len; ++phase)
                        add(Kind::Run, 0.045*opening[m]/(len*(len-1)) *
                            (0.5/len + (phase == 0 ? 0.5 : 0.0)), m, dir, len, phase);
                }
                for (int len = 1; len <= 6; ++len)
                    add(Kind::Alternate, 0.020*opening[m]/(len*len), m, (m+dir)%3, len);
            }
            // The opponent can stay or rotate independently after a tie, loss, or win.
            constexpr Prob onTie{0.34, 0.38, 0.28}, onLoss{0.22, 0.47, 0.31};
            constexpr Prob onWin{0.65, 0.20, 0.15};
            for (int tie = 0; tie < 3; ++tie)
                for (int loss = 0; loss < 3; ++loss)
                    for (int win = 0; win < 3; ++win)
                        add(Kind::React, 0.12*opening[m]*onTie[tie]*onLoss[loss]*onWin[win],
                            m, tie, loss, win);
        }
        constexpr Prob rotations{0.45, 0.39, 0.16};
        for (int lag = 1; lag <= 12; ++lag)
            for (int shift = 0; shift < 3; ++shift) {
                double w = (lag == 1 ? 0.16 : 0.065/(lag*(lag-1))) * rotations[shift];
                add(Kind::Response, w, lag, 1, shift);
                add(Kind::Response, 0.07*w, lag, 2, shift);
            }
        for (int won : {0,1})
            for (int shift = 0; shift < 3; ++shift)
                add(Kind::LastResult, 0.009*rotations[shift], won, shift);

        for (int window = 0; window < frequencyTypes; ++window) {
            double w = window == int(size(windows)) ? 0.045 : 0.008;
            for (int least : {0,1})
                for (int shift = 0; shift < 3; ++shift)
                    for (int tie = 0; tie < 7; ++tie)
                        add(Kind::Frequency, w * (least ? 0.22 : 0.78) *
                            (shift == 1 ? 0.60 : 0.20) / 7, window, least, shift, tie);
        }

        auto table = [&](Key k, int n, int cells, bool rel, double w, double noise = 0) {
            tables.emplace_back(k, n, cells, rel, w, noise);
        };
        for (int period = 2; period <= 128; ++period) {
            table(Key::Time, period, period, false, 0.15/(period*(period-1)));
            if (period <= 16) {
                table(Key::Time, period, period, true, 0.012/(period*(period-1)));
                table(Key::Time, period, period, false, 0.01/(period*period), 0.09);
            }
            if (period <= 12) {
                for (Key k : {Key::Wins, Key::Losses})
                    table(k, period, period, false, 0.017/(period*(period-1)));
            }
        }
        // Some sequences start only after the opponent has a move history.
        for (int prefix : {1, 3})
            for (int period = 1; period <= 16; ++period) {
                table(Key::Time, period, period, false, 0.015/(period*period*prefix));
                tables.back().warmup = prefix;
            }
        for (Key k : {Key::Moves, Key::Results, Key::History}) {
            int cells = 1;
            for (int depth = 1; depth <= 4; ++depth) {
                cells *= k == Key::Moves ? 3 : k == Key::Results ? 2 : 6;
                for (bool rel : {false, true}) {
                    if (rel && k == Key::Moves) continue;
                    double w = 0.011/(depth*depth);
                    table(k, depth, cells, rel, w);
                    table(k, depth, cells, rel, 0.32*w, 0.10);
                }
            }
        }
        for (bool rel : {false, true}) {
            table(Key::Streak, 9, 10, rel, 0.006);
            table(Key::Streak, 9, 10, rel, 0.002, 0.10);
        }
        for (int period = 2; period <= 6; ++period)
            table(Key::PhaseMove, period, period*3, false, 0.01/(period*period));
        table(Key::Time, 1, 1, false, 0.005, 0.23);
    }

    Prob forecast() {
        h.prepare();
        Prob votes{};
        const Prob unknown = h.round ? Prob{1./3, 1./3, 1./3} : opening;
        for (auto &r : rules) {
            r.prediction = r.predict(h);
            if (r.prediction >= 0) votes[r.prediction] += r.weight;
            else for (int m = 0; m < 3; ++m) votes[m] += unknown[m]*r.weight;
        }
        for (auto &t : tables) {
            t.predict(h);
            for (int m = 0; m < 3; ++m) votes[(m+t.offset)%3] += t.weight*t.p[m];
        }

        double sum = votes[0] + votes[1] + votes[2];
        for (double &v : votes) v /= sum;
        return votes;
    }

    // Score the expected streak at each lookahead depth. The immediate
    // streak gets extra weight so speculative sacrifices are less attractive.
    double plan(const Prob &votes, int depth, int &action) {
        int best = int(max_element(votes.begin(), votes.end()) - votes.begin());
        action = (best + 1) % 3;
        if (depth == 1) return votes[best]*(h.streak+1);

        double bestValue = -1;
        for (int i = 0; i < 3; ++i) {
            int target = (best + i) % 3;
            if (i && votes[best] > 0.995) break;
            int played = (target+1) % 3;
            double value = 0;
            for (bool win : {true, false}) {
                double prob = win ? votes[target] : 1-votes[target];
                if (prob < 1e-9) continue;
                if (win && h.streak == 9) {
                    value += prob*(10 + depth-1);
                } else {
                    Predictor child(*this);
                    child.update(played, win);
                    Prob next = child.forecast();
                    int unused;
                    value += prob * child.plan(next, depth-1, unused);
                }
            }
            value = 0.5*value + 0.5*votes[target]*(h.streak+1);
            if (value > bestValue) {
                bestValue = value;
                action = played;
            }
        }
        return bestValue;
    }

    int choose() {
        Prob votes = forecast();
        // A fresh random block also gives an unmodelled opponent a positive
        // conditional probability of ten straight wins from any history.
        if (h.round >= nextRandom && h.streak == 0) {
            randomLeft = h.round < 1024 ? 10 : 40;
            nextRandom = h.round + 80;
        }
        if (randomLeft > 0) {
            --randomLeft;
            return uniform_int_distribution<int>(0, 2)(rng);
        }

        if (h.round >= 1024 && *max_element(votes.begin(), votes.end()) < 0.43)
            return uniform_int_distribution<int>(0, 2)(rng);

        int action;
        int depth = h.round < 60 ? 3 : 1;
        plan(votes, depth, action);
        return action;
    }

    void update(int played, bool success) {
        // A positive reply identifies the opponent's move; a negative one
        // excludes exactly that move, leaving both the tie and the loss.
        int target = (played+2)%3;
        double unknown = h.round ? 1./3 : opening[target];
        double total = 0;
        for (auto &r : rules) {
            if (r.prediction < 0) r.weight *= success ? unknown : 1 - unknown;
            else if ((r.prediction == target) != success) r.weight = 0;
            r.advance(played);
            total += r.weight;
        }
        erase_if(rules, [](const Rule &r) { return r.weight == 0; });
        for (auto &t : tables) {
            t.observe(target, success);
            total += t.weight;
        }
        erase_if(tables, [](const Table &t) { return !t.noise && t.weight == 0; });
        for (auto &r : rules) r.weight /= total;
        for (auto &t : tables) {
            t.weight /= total;
            if (t.noise) t.weight = max(t.weight, 1e-7*t.prior);
        }
        h.record(played, success);
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    Predictor predictor;
    string reply;
    while (predictor.h.streak < 10) {
        int move = predictor.choose();
        cout << "RPS"[move] << '\n' << flush;
        if (!cout || !(cin >> reply)) return 0;
        if (reply != "player" && reply != "ai") return 0;
        predictor.update(move, reply == "player");
    }
}
