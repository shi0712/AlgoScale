#include <bits/stdc++.h>
using namespace std;

using ld = long double;

static const ld PHI = (1.0L + sqrtl(5.0L)) / 2.0L;

// States: 0 = H1, 1 = H2, 2 = D1, 3 = D2, 4 = no previous answer.
static int nextState(int s, bool honest) {
    if (s == 4) return honest ? 0 : 2;
    if (honest) {
        if (s == 0) return 1;
        if (s == 1) return -1;
        return 0;
    } else {
        if (s == 3) return -1;
        if (s == 0 || s == 1) return 2;
        return 3;
    }
}

static ld stateWeight(int s) {
    if (s < 0) return 0;
    if (s == 1 || s == 3) return 1.0L;
    return PHI;
}

struct Candidate {
    int roll;
    int state;
};

struct SmallMove {
    int i, j;
    string p, q;
    int maxSize, sumSize;
    ld maxWeight;
};

class SmallSolver {
    unordered_map<uint64_t, unsigned char> memo;

    static uint64_t encode(const string &s) {
        uint64_t x = 1;
        for (unsigned char c : s) x = (x << 3) | uint64_t(c + 1);
        return x;
    }

    static ld potential(const string &s) {
        ld result = 0;
        for (unsigned char c : s) result += stateWeight(int(c));
        return result;
    }

    static string applyMove(const string &s, int i, int j, bool p) {
        string result;
        result.reserve(s.size());
        for (int k = 0; k < int(s.size()); ++k) {
            bool inside = (i <= k && k <= j);
            bool honest = p ? inside : !inside;
            int ns = nextState(int(s[k]), honest);
            if (ns >= 0) result.push_back(char(ns));
        }
        return result;
    }

    static vector<SmallMove> buildMoves(const string &s) {
        vector<SmallMove> moves;
        int m = int(s.size());
        for (int i = 0; i < m; ++i) {
            for (int j = i; j < m; ++j) {
                string p = applyMove(s, i, j, true);
                string q = applyMove(s, i, j, false);
                if (p == s && q == s) continue;
                SmallMove mv;
                mv.i = i;
                mv.j = j;
                mv.p = std::move(p);
                mv.q = std::move(q);
                mv.maxSize = int(max(mv.p.size(), mv.q.size()));
                mv.sumSize = int(mv.p.size() + mv.q.size());
                mv.maxWeight = max(potential(mv.p), potential(mv.q));
                moves.push_back(std::move(mv));
            }
        }
        sort(moves.begin(), moves.end(), [](const SmallMove &a, const SmallMove &b) {
            if (a.maxSize != b.maxSize) return a.maxSize < b.maxSize;
            if (a.maxWeight != b.maxWeight) return a.maxWeight < b.maxWeight;
            return a.sumSize < b.sumSize;
        });
        return moves;
    }

public:
    SmallSolver() {
        memo.reserve(1 << 16);
    }

    bool canSolve(const string &s, int depth) {
        if (s.size() <= 2) return true;
        if (depth <= 0) return false;

        uint64_t key = (encode(s) << 6) | uint64_t(depth);
        auto it = memo.find(key);
        if (it != memo.end()) return it->second != 0;

        vector<SmallMove> moves = buildMoves(s);
        for (const SmallMove &mv : moves) {
            if (canSolve(mv.p, depth - 1) && canSolve(mv.q, depth - 1)) {
                memo[key] = 1;
                return true;
            }
        }
        memo[key] = 0;
        return false;
    }

    pair<int, int> getMove(const string &s) {
        if (s.size() <= 2) return {-1, -1};

        int depth = -1;
        for (int d = 1; d <= 32; ++d) {
            if (canSolve(s, d)) {
                depth = d;
                break;
            }
        }
        if (depth < 0) return {-1, -1};

        vector<SmallMove> moves = buildMoves(s);
        for (const SmallMove &mv : moves) {
            if (canSolve(mv.p, depth - 1) && canSolve(mv.q, depth - 1)) {
                return {mv.i, mv.j};
            }
        }
        return {-1, -1};
    }
};

class Player {
    int n;
    vector<Candidate> candidates;
    SmallSolver smallSolver;

    bool ask(int l, int r) {
        cout << "? " << l << " " << r << "\n" << flush;

        int hands;
        if (!(cin >> hands)) return false;
        if (hands == -1) return false;

        int len = r - l + 1;
        // c = 1 means: inside is honest, outside is dishonest.
        // c = 0 means the reverse.
        int c = (hands == len ? 0 : 1);
        bool p = (c == 1);

        vector<Candidate> next;
        next.reserve(candidates.size());
        for (const Candidate &cand : candidates) {
            bool inside = (l <= cand.roll && cand.roll <= r);
            bool honest = p ? inside : !inside;
            int ns = nextState(cand.state, honest);
            if (ns >= 0) next.push_back({cand.roll, ns});
        }
        candidates.swap(next);
        return true;
    }

    bool askBestPrefix() {
        int m = int(candidates.size());

        ld pWeight = 0, qWeight = 0;
        int pCount = 0, qCount = 0;

        vector<ld> deltaP(m), deltaQ(m);
        vector<int> deltaPC(m), deltaQC(m);

        for (int i = 0; i < m; ++i) {
            int s = candidates[i].state;

            int pOutState = nextState(s, false);
            int qOutState = nextState(s, true);
            int pInState = nextState(s, true);
            int qInState = nextState(s, false);

            ld pOut = stateWeight(pOutState);
            ld qOut = stateWeight(qOutState);
            ld pIn = stateWeight(pInState);
            ld qIn = stateWeight(qInState);

            int pOutAlive = (pOutState >= 0);
            int qOutAlive = (qOutState >= 0);
            int pInAlive = (pInState >= 0);
            int qInAlive = (qInState >= 0);

            pWeight += pOut;
            qWeight += qOut;
            pCount += pOutAlive;
            qCount += qOutAlive;

            deltaP[i] = pIn - pOut;
            deltaQ[i] = qIn - qOut;
            deltaPC[i] = pInAlive - pOutAlive;
            deltaQC[i] = qInAlive - qOutAlive;
        }

        int bestK = m;
        ld bestWeight = numeric_limits<ld>::infinity();
        int bestCount = INT_MAX;

        for (int k = 1; k <= m; ++k) {
            pWeight += deltaP[k - 1];
            qWeight += deltaQ[k - 1];
            pCount += deltaPC[k - 1];
            qCount += deltaQC[k - 1];

            ld worstWeight = max(pWeight, qWeight);
            int worstCount = max(pCount, qCount);

            if (worstWeight < bestWeight - 1e-12L ||
                (fabsl(worstWeight - bestWeight) <= 1e-12L && worstCount < bestCount)) {
                bestWeight = worstWeight;
                bestCount = worstCount;
                bestK = k;
            }
        }

        return ask(1, candidates[bestK - 1].roll);
    }

public:
    explicit Player(int n_) : n(n_) {
        candidates.reserve(n);
        for (int i = 1; i <= n; ++i) candidates.push_back({i, 4});
    }

    void run() {
        if (n > 8) {
            if (!ask(1, (n + 1) / 2)) return;
        }

        while (candidates.size() > 2) {
            bool ok = false;

            if (candidates.size() <= 8) {
                string state;
                state.reserve(candidates.size());
                for (const Candidate &cand : candidates) state.push_back(char(cand.state));

                pair<int, int> mv = smallSolver.getMove(state);
                if (mv.first >= 0) {
                    ok = ask(candidates[mv.first].roll, candidates[mv.second].roll);
                } else {
                    ok = askBestPrefix();
                }
            } else {
                ok = askBestPrefix();
            }

            if (!ok) return;
        }

        for (const Candidate &cand : candidates) {
            cout << "! " << cand.roll << "\n" << flush;
            int response;
            if (!(cin >> response)) return;
            if (response == -1) return;
            if (response == 1) break;
        }

        cout << "#\n" << flush;
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n)) return 0;

    Player player(n);
    player.run();
    return 0;
}
