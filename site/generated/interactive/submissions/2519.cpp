#include <bits/stdc++.h>
using namespace std;

const int MAXR = 105, MAXC = 105;
int r, c;
vector<string> grid;
int cellIndex[MAXR][MAXC];
vector<pair<int,int>> cellCoord;
int M;

int dirR[4] = {-1, 0, 1, 0};
int dirC[4] = {0, 1, 0, -1};

int Nstates;
vector<int> d, nextL, nextR, nextF, pos;

vector<int> cls;
int C;
vector<int> classRep, dClass, posClass;
vector<bool> uniquePos;
vector<array<int,3>> nextClass;
vector<vector<int>> classesByD;
vector<uint64_t> hashClass;

vector<int> seen;
int stamp = 0;

uint64_t splitmix64(uint64_t x) {
    x += 0x9e3779b97f4a7c15;
    x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9;
    x = (x ^ (x >> 27)) * 0x94d049bb133111eb;
    return x ^ (x >> 31);
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    cin >> r >> c;
    grid.resize(r);
    for (int i = 0; i < r; ++i) cin >> grid[i];

    memset(cellIndex, -1, sizeof(cellIndex));
    cellCoord.clear();
    for (int i = 0; i < r; ++i) {
        for (int j = 0; j < c; ++j) {
            if (grid[i][j] == '.') {
                cellIndex[i][j] = M++;
                cellCoord.push_back({i, j});
            }
        }
    }

    Nstates = M * 4;
    d.resize(Nstates);
    nextL.resize(Nstates);
    nextR.resize(Nstates);
    nextF.resize(Nstates);
    pos.resize(Nstates);

    for (int idx = 0; idx < M; ++idx) {
        int i = cellCoord[idx].first;
        int j = cellCoord[idx].second;
        for (int dir = 0; dir < 4; ++dir) {
            int s = idx * 4 + dir;
            pos[s] = idx;
            int ni = i + dirR[dir], nj = j + dirC[dir];
            int dist = 0;
            while (ni >= 0 && ni < r && nj >= 0 && nj < c && grid[ni][nj] == '.') {
                ++dist;
                ni += dirR[dir];
                nj += dirC[dir];
            }
            d[s] = dist;
            nextL[s] = idx * 4 + (dir + 3) % 4;
            nextR[s] = idx * 4 + (dir + 1) % 4;
            if (dist > 0) {
                ni = i + dirR[dir];
                nj = j + dirC[dir];
                int nidx = cellIndex[ni][nj];
                nextF[s] = nidx * 4 + dir;
            } else {
                nextF[s] = -1;
            }
        }
    }

    // initial partition by d
    vector<int> dvals;
    for (int s = 0; s < Nstates; ++s) dvals.push_back(d[s]);
    sort(dvals.begin(), dvals.end());
    dvals.erase(unique(dvals.begin(), dvals.end()), dvals.end());
    auto getDId = [&](int val) {
        return lower_bound(dvals.begin(), dvals.end(), val) - dvals.begin();
    };
    cls.resize(Nstates);
    for (int s = 0; s < Nstates; ++s) cls[s] = getDId(d[s]);
    int numClasses = dvals.size();

    while (true) {
        vector<tuple<int,int,int,int>> sig(Nstates);
        for (int s = 0; s < Nstates; ++s) {
            int c = cls[s];
            int l = nextL[s] != -1 ? cls[nextL[s]] : -1;
            int rr = nextR[s] != -1 ? cls[nextR[s]] : -1;
            int f = nextF[s] != -1 ? cls[nextF[s]] : -1;
            sig[s] = {c, l, rr, f};
        }
        vector<int> idx(Nstates);
        iota(idx.begin(), idx.end(), 0);
        sort(idx.begin(), idx.end(), [&](int a, int b) { return sig[a] < sig[b]; });
        vector<int> newCls(Nstates);
        int newNum = 0;
        for (int i = 0; i < Nstates; ++i) {
            if (i == 0 || sig[idx[i]] != sig[idx[i-1]]) ++newNum;
            newCls[idx[i]] = newNum - 1;
        }
        if (newNum == numClasses) {
            cls = newCls;
            numClasses = newNum;
            break;
        }
        cls = newCls;
        numClasses = newNum;
    }

    C = numClasses;
    classRep.assign(C, -1);
    dClass.resize(C);
    posClass.assign(C, -1);
    uniquePos.assign(C, true);
    for (int s = 0; s < Nstates; ++s) {
        int c = cls[s];
        if (classRep[c] == -1) {
            classRep[c] = s;
            dClass[c] = d[s];
            posClass[c] = pos[s];
        } else {
            if (pos[s] != posClass[c]) uniquePos[c] = false;
        }
    }

    nextClass.resize(C);
    for (int c = 0; c < C; ++c) {
        int s = classRep[c];
        for (int a = 0; a < 3; ++a) {
            if (a == 2 && d[s] == 0) nextClass[c][a] = -1;
            else {
                int ns;
                if (a == 0) ns = nextL[s];
                else if (a == 1) ns = nextR[s];
                else ns = nextF[s];
                nextClass[c][a] = cls[ns];
            }
        }
    }

    classesByD.assign(100, {});
    for (int c = 0; c < C; ++c) classesByD[dClass[c]].push_back(c);

    hashClass.resize(C);
    for (int c = 0; c < C; ++c) hashClass[c] = splitmix64(c);

    seen.assign(C, 0);

    int d_cur;
    if (!(cin >> d_cur)) return 0;
    vector<int> belief = classesByD[d_cur];

    unordered_set<uint64_t> visitedBeliefs;
    int fallbackOffset = 0;
    int stepCount = 0;

    while (true) {
        // check if all possible positions are the same
        bool allSame = true;
        int firstPos = -1;
        for (int c : belief) {
            if (!uniquePos[c]) { allSame = false; break; }
            if (firstPos == -1) firstPos = posClass[c];
            else if (posClass[c] != firstPos) { allSame = false; break; }
        }
        if (allSame) {
            auto [r0, c0] = cellCoord[firstPos];
            cout << "yes " << r0 + 1 << " " << c0 + 1 << endl;
            return 0;
        }

        // check if no state can ever be identified
        bool allBad = true;
        for (int c : belief) {
            if (uniquePos[c]) { allBad = false; break; }
        }
        if (allBad) {
            cout << "no" << endl;
            return 0;
        }

        // hash current belief to detect loops
        uint64_t h = 0;
        for (int c : belief) h ^= hashClass[c];
        if (visitedBeliefs.count(h)) {
            fallbackOffset = (fallbackOffset + 1) % 3;
        }
        visitedBeliefs.insert(h);

        // choose an action
        int a = -1;
        int bestMax = belief.size();
        long long bestSum = LLONG_MAX;

        for (int act = 0; act < 3; ++act) {
            if (act == 2 && d_cur == 0) continue;
            ++stamp;
            vector<int> cnt(100, 0);
            for (int c : belief) {
                int nc = nextClass[c][act];
                if (seen[nc] != stamp) {
                    seen[nc] = stamp;
                    cnt[dClass[nc]]++;
                }
            }
            int mx = 0;
            long long sum = 0;
            for (int obs = 0; obs < 100; ++obs) {
                int sz = cnt[obs];
                if (sz > mx) mx = sz;
                sum += 1LL * sz * sz;
            }
            if (mx < bestMax || (mx == bestMax && sum < bestSum)) {
                bestMax = mx;
                bestSum = sum;
                a = act;
            }
        }

        if (a == -1 || bestMax >= (int)belief.size()) {
            a = (stepCount + fallbackOffset) % 3;
            if (a == 2 && d_cur == 0) a = 0;
        }

        if (a == 0) cout << "left" << endl;
        else if (a == 1) cout << "right" << endl;
        else cout << "step" << endl;

        int d_next;
        if (!(cin >> d_next)) return 0;

        ++stamp;
        vector<int> newBelief;
        newBelief.reserve(belief.size());
        for (int c : belief) {
            int nc = nextClass[c][a];
            if (dClass[nc] == d_next && seen[nc] != stamp) {
                seen[nc] = stamp;
                newBelief.push_back(nc);
            }
        }
        belief = std::move(newBelief);
        d_cur = d_next;
        ++stepCount;
    }

    return 0;
}
