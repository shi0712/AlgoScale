#include <bits/stdc++.h>
using namespace std;

/*
The six strategies are not specified, so a worst-case guarantee is
impossible. For any deterministic contestant, precompute its moves on
an all-"ai" transcript. A fixed opponent script playing those same
moves ties every round without seeing the contestant's current move.
It could be one of the six strategies. Randomization cannot guarantee
success within a finite deadline either.

This is a best-effort contestant: it learns periodic and contextual
move tables from binary feedback, and also tests stateful opponent
strategies. No particular set of six strategies is assumed.
*/

struct Table {
    unordered_map<int, int> masks;
    double prior, evidence = 0;
    int key = 0, base = 0, mask = 7;
    double probability[3]{};

    explicit Table(double p) : prior(p) {}
};

struct Rule {
    int opponent, delta[3];
    double prior;
    bool alive = true;
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    mt19937_64 rng(chrono::steady_clock::now().time_since_epoch().count());
    vector<Table> tables;
    vector<Rule> rules;

    // Opponent shifts its own move according to the previous outcome.
    for (int a = 0; a < 3; ++a)
        for (int b = 0; b < 3; ++b)
            for (int c = 0; c < 3; ++c)
                for (int first = 0; first < 3; ++first)
                    rules.push_back({first, {a, b, c},
                        (a == b && b == c ? 0.07 : 0.006)
                        * (first == 0 ? 1.5 : 1.0), true});

    vector<int> play, won;
    array<int, 3> counts{};
    int streak = 0, lastRevealed = 0;
    double ruleEvidence = 0;

    for (int t = 0; ; ++t) {
        array<double, 3> vote{};
        size_t idx = 0;

        // A table entry encodes the possible winning moves relative to base.
        auto use = [&](int key, int base, double prior) {
            if (idx == tables.size()) tables.emplace_back(prior);
            auto &m = tables[idx++];
            m.key = key;
            m.base = base;
            auto it = m.masks.find(key);
            m.mask = it == m.masks.end() ? 7 : it->second;
            double weight = m.prior * exp(m.evidence);
            int n = __builtin_popcount((unsigned)m.mask);
            for (int i = 0; i < 3; ++i) {
                int j = (base + i) % 3;
                m.probability[j] = 0.003 + ((m.mask >> i) & 1) * 0.991 / n;
                vote[j] += weight * m.probability[j];
            }
        };

        int last = t ? play.back() : 0;
        use(0, 0, 1.0);
        for (int p = 2; p <= 96; ++p) {
            use(t % p, 0, 1.2 / (p * p));
            if (p <= 12) use(t % p, last, 0.16 / (p * p));
        }

        for (int k = 1; k <= 12; ++k) {
            use(0, t >= k ? play[t-k] : 0, 0.9 / (k * k));
            use(0, (t / k) % 3, 0.24 / (k * k));
            use(0, (3 - (t / k) % 3) % 3, 0.24 / (k * k));
        }

        use(0, lastRevealed, 0.16);
        use(0, int(max_element(counts.begin(), counts.end()) - counts.begin()), 0.25);
        use(0, int(min_element(counts.begin(), counts.end()) - counts.begin()), 0.12);

        int moves = 0, results = 0, both = 0;
        array<int, 3> recent{};
        for (int k = 1; k <= 8; ++k) {
            int a = t >= k ? play[t-k] : 3;
            int w = t >= k ? won[t-k] : 2;
            moves = 4 * moves + a;
            results = 3 * results + w;
            both = 7 * both + (a == 3 ? 6 : 2 * a + w);
            use(moves, 0, 0.4 / (k * k));
            use(results, 0, 0.2 / (k * k));
            use(results, last, 0.3 / (k * k));
            use(both, 0, 0.35 / (k * k));
            if (a < 3) ++recent[a];
            use(0, int(max_element(recent.begin(), recent.end()) - recent.begin()),
                0.08 / k);
        }

        double rw = exp(ruleEvidence);
        for (const auto &r : rules)
            if (r.alive) vote[(r.opponent + 1) % 3] += r.prior * rw;

        int x = int(rng() % 3);
        for (int i = 0; i < 3; ++i)
            if (vote[i] > vote[x]) x = i;

        // Periodic random blocks if none of the predictors solves the game.
        if (t >= 256 && (t % 64) < 16) x = int(rng() % 3);

        cout << "RPS"[x] << '\n' << flush;
        string s;
        if (!(cin >> s) || (s != "player" && s != "ai")) return 0;
        bool win = s == "player";
        streak = win ? streak + 1 : 0;
        if (streak == 10) return 0;

        double baseline = win ? 1.0 / 3 : 2.0 / 3;
        for (auto &m : tables) {
            int bit = 1 << ((x - m.base + 3) % 3);
            int allowed = win ? bit : (7 ^ bit);
            int remaining = m.mask & allowed;
            m.masks[m.key] = remaining ? remaining : allowed;
            double p = win ? m.probability[x] : 1 - m.probability[x];
            m.evidence = 0.99 * m.evidence + log(p / baseline);
        }

        ruleEvidence = 0.99 * ruleEvidence - log(baseline);
        for (auto &r : rules) {
            if (!r.alive) continue;
            int outcome = (x - r.opponent + 3) % 3;
            if ((outcome == 1) != win) r.alive = false;
            else r.opponent = (r.opponent + r.delta[outcome]) % 3;
        }

        if (win) lastRevealed = (x + 2) % 3;
        ++counts[x];
        play.push_back(x);
        won.push_back(win);
    }
}
