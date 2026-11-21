#include <bits/stdc++.h>
using namespace std;

static const int LOOKAHEAD_DEPTH = 6;
static const int LOOKAHEAD_NODES = 3000;
static const int LOOKAHEAD_SIZE = 5000;
static const int MAX_ROUNDS = 200000;

int r, c;
vector<string> grid;
vector<vector<int>> cellId;
vector<int> cellR, cellC;
int openCnt = 0;
int N = 0;

vector<int> distToWall;
vector<int> statePos;
vector<int> nextL, nextR, nextS;
vector<char> posAllZero;

vector<int> posMark;
int markTimer = 1;

uint64_t splitmix64(uint64_t x) {
    x += 0x9e3779b97f4a7c15ULL;
    x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9ULL;
    x = (x ^ (x >> 27)) * 0x94d049bb133111ebULL;
    return x ^ (x >> 31);
}

vector<uint64_t> stateRand;

struct Stats {
    int pc = 0;
    int zeroCount = 0;
    bool allZero = true;
};

Stats getStats(const vector<int>& states) {
    if (markTimer == INT_MAX) {
        fill(posMark.begin(), posMark.end(), 0);
        markTimer = 1;
    }
    ++markTimer;
    Stats st;
    for (int s : states) {
        int p = statePos[s];
        if (posMark[p] != markTimer) {
            posMark[p] = markTimer;
            ++st.pc;
            if (posAllZero[p]) ++st.zeroCount;
            else st.allZero = false;
        }
    }
    return st;
}

uint64_t hashBelief(const vector<int>& states) {
    uint64_t h = 0;
    for (int s : states) h ^= stateRand[s];
    return h;
}

struct Branch {
    int obs = 0;
    vector<int> states;
    int stateCount = 0;
    int posCount = 0;
    int zeroCount = 0;
    bool allZero = true;
};

vector<Branch> getBranches(const vector<int>& states, int action, int d) {
    array<vector<int>, 100> buckets;
    if (action == 2) {
        if (d <= 0) return {};
        for (int s : states) {
            int ns = nextS[s];
            if (ns >= 0) buckets[d - 1].push_back(ns);
        }
    } else {
        for (int s : states) {
            int ns = (action == 0 ? nextL[s] : nextR[s]);
            int o = distToWall[ns];
            buckets[o].push_back(ns);
        }
    }

    vector<Branch> res;
    res.reserve(100);
    for (int o = 0; o < 100; ++o) {
        if (buckets[o].empty()) continue;
        Branch b;
        b.obs = o;
        b.states = std::move(buckets[o]);
        b.stateCount = (int)b.states.size();
        Stats st = getStats(b.states);
        b.posCount = st.pc;
        b.zeroCount = st.zeroCount;
        b.allZero = st.allZero;
        res.push_back(std::move(b));
    }
    return res;
}

bool canProgress(const vector<int>& states, int d, int depth, int& nodes) {
    if (nodes > LOOKAHEAD_NODES) return true; // optimistic to avoid false negatives
    ++nodes;

    Stats st = getStats(states);
    if (st.pc <= 1) return true;
    if (depth == 0) return false;
    if (st.allZero || st.zeroCount >= 2) return false;

    int actions[3] = {0, 1, 2};
    for (int a : actions) {
        if (a == 2 && d == 0) continue;
        vector<Branch> brs = getBranches(states, a, d);
        if (brs.empty()) continue;

        bool ok = true;
        for (const Branch& br : brs) {
            if (br.posCount <= 1) continue;
            if (br.allZero || br.zeroCount >= 2) {
                ok = false;
                break;
            }
            if (!canProgress(br.states, br.obs, depth - 1, nodes)) {
                ok = false;
                break;
            }
        }
        if (ok) return true;
    }
    return false;
}

bool actionGood(const vector<Branch>& brs, int depth) {
    int nodes = 0;
    for (const Branch& br : brs) {
        if (br.posCount <= 1) continue;
        if (br.allZero || br.zeroCount >= 2) return false;
        if (!canProgress(br.states, br.obs, depth, nodes)) return false;
    }
    return true;
}

pair<int, vector<Branch>> selectAction(const vector<int>& belief, int d, int pc) {
    int bestAction = -1;
    vector<Branch> bestBranches;
    int bestPriority = 100;
    int bestMaxState = INT_MAX;
    int bestMaxPos = INT_MAX;

    bool doLookahead = ((int)belief.size() <= LOOKAHEAD_SIZE);

    int actions[3] = {0, 1, 2};
    for (int a : actions) {
        if (a == 2 && d == 0) continue;

        vector<Branch> brs = getBranches(belief, a, d);
        if (brs.empty()) continue;

        int maxState = 0, maxPos = 0, maxObs = 0;
        for (const Branch& br : brs) {
            maxState = max(maxState, br.stateCount);
            maxPos = max(maxPos, br.posCount);
            maxObs = max(maxObs, br.obs);
        }

        bool splits = brs.size() > 1;
        bool good = false;
        if (doLookahead) {
            good = actionGood(brs, LOOKAHEAD_DEPTH);
        } else {
            good = splits || (a == 2) || (d == 0 && maxObs > 0);
        }

        int priority;
        if (good && splits) priority = 0;
        else if (good && !splits && a == 2) priority = 1;
        else if (good && !splits && d == 0 && maxObs > 0) priority = 2;
        else if (splits) priority = 3;
        else if (a == 2) priority = 4;
        else if (d == 0 && maxObs > 0) priority = 5;
        else priority = 6;

        bool better = false;
        if (priority < bestPriority) better = true;
        else if (priority == bestPriority) {
            if (maxState < bestMaxState) better = true;
            else if (maxState == bestMaxState && maxPos < bestMaxPos) better = true;
        }

        if (better) {
            bestAction = a;
            bestBranches = std::move(brs);
            bestPriority = priority;
            bestMaxState = maxState;
            bestMaxPos = maxPos;
        }
    }

    return {bestAction, std::move(bestBranches)};
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    if (!(cin >> r >> c)) return 0;
    grid.resize(r);
    for (int i = 0; i < r; ++i) cin >> grid[i];

    cellId.assign(r, vector<int>(c, -1));
    for (int i = 0; i < r; ++i) {
        for (int j = 0; j < c; ++j) {
            if (grid[i][j] == '.') {
                cellId[i][j] = openCnt++;
                cellR.push_back(i);
                cellC.push_back(j);
            }
        }
    }

    N = openCnt * 4;
    distToWall.assign(N, 0);
    statePos.assign(N, 0);
    nextL.assign(N, -1);
    nextR.assign(N, -1);
    nextS.assign(N, -1);
    posAllZero.assign(openCnt, 1);
    stateRand.assign(N, 0);

    const int dr[4] = {-1, 0, 1, 0};
    const int dc[4] = {0, 1, 0, -1};

    for (int p = 0; p < openCnt; ++p) {
        for (int dir = 0; dir < 4; ++dir) {
            int s = p * 4 + dir;
            statePos[s] = p;
            nextL[s] = p * 4 + ((dir + 3) & 3);
            nextR[s] = p * 4 + ((dir + 1) & 3);

            int cnt = 0;
            int ni = cellR[p], nj = cellC[p];
            while (true) {
                ni += dr[dir];
                nj += dc[dir];
                if (ni < 0 || ni >= r || nj < 0 || nj >= c) break;
                if (grid[ni][nj] == '#') break;
                ++cnt;
            }
            distToWall[s] = cnt;

            if (cnt > 0) {
                int ni = cellR[p] + dr[dir];
                int nj = cellC[p] + dc[dir];
                int np = cellId[ni][nj];
                nextS[s] = np * 4 + dir;
            } else {
                nextS[s] = -1;
            }

            if (cnt == 0) posAllZero[p] = 0;
        }
    }

    for (int s = 0; s < N; ++s) stateRand[s] = splitmix64((uint64_t)s + 0x123456789abcdefULL);

    posMark.assign(openCnt, 0);

    int d;
    if (!(cin >> d)) return 0;

    vector<int> belief;
    belief.reserve(N);
    for (int s = 0; s < N; ++s) {
        if (distToWall[s] == d) belief.push_back(s);
    }

    if (belief.empty()) return 0;

    unordered_map<uint64_t, int> seenMask;
    seenMask.reserve(100000);

    int round = 0;
    while (true) {
        ++round;

        Stats st = getStats(belief);
        if (st.pc == 1) {
            int p = statePos[belief[0]];
            cout << "yes " << cellR[p] + 1 << ' ' << cellC[p] + 1 << '\n';
            cout.flush();
            return 0;
        }

        if (st.zeroCount >= 2 || st.allZero) {
            cout << "no\n";
            cout.flush();
            return 0;
        }

        if (round > MAX_ROUNDS) {
            cout << "no\n";
            cout.flush();
            return 0;
        }

        uint64_t h = hashBelief(belief);
        int mask = seenMask[h];

        auto [action, branches] = selectAction(belief, d, st.pc);
        if (action < 0) {
            cout << "no\n";
            cout.flush();
            return 0;
        }

        if (mask & (1 << action)) {
            cout << "no\n";
            cout.flush();
            return 0;
        }
        seenMask[h] = mask | (1 << action);

        if (action == 0) cout << "left\n";
        else if (action == 1) cout << "right\n";
        else cout << "step\n";
        cout.flush();

        int oldD = d;
        if (!(cin >> d)) return 0;

        const Branch* chosen = nullptr;
        for (const Branch& br : branches) {
            if (br.obs == d) {
                chosen = &br;
                break;
            }
        }
        if (!chosen) return 0;

        belief = std::move(chosen->states);
        if (belief.empty()) return 0;
    }
}
