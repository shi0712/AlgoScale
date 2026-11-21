#include <bits/stdc++.h>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    int r, c;
    if (!(cin >> r >> c)) return 0;
    vector<string> grid(r);
    for (int i = 0; i < r; ++i) cin >> grid[i];

    const int dx[4] = {-1, 0, 1, 0};
    const int dy[4] = {0, 1, 0, -1};

    // enumerate open cells
    vector<vector<int>> cellId(r, vector<int>(c, -1));
    vector<int> cellRow, cellCol;
    for (int i = 0; i < r; ++i) {
        for (int j = 0; j < c; ++j) {
            if (grid[i][j] == '.') {
                int id = (int)cellRow.size();
                cellId[i][j] = id;
                cellRow.push_back(i);
                cellCol.push_back(j);
            }
        }
    }
    int openCnt = (int)cellRow.size();
    int N = openCnt * 4; // states = (cell, direction)

    // precompute distances and transitions
    vector<int> dist(N);
    vector<int> leftTrans(N), rightTrans(N), stepTrans(N);
    for (int cell = 0; cell < openCnt; ++cell) {
        int i = cellRow[cell];
        int j = cellCol[cell];
        for (int d = 0; d < 4; ++d) {
            int sid = cell * 4 + d;
            int dcnt = 0;
            int ni = i + dx[d];
            int nj = j + dy[d];
            while (ni >= 0 && ni < r && nj >= 0 && nj < c && grid[ni][nj] == '.') {
                ++dcnt;
                ni += dx[d];
                nj += dy[d];
            }
            dist[sid] = dcnt;
            leftTrans[sid] = cell * 4 + (d + 3) % 4;
            rightTrans[sid] = cell * 4 + (d + 1) % 4;
            if (dcnt >= 1) {
                int ncell = cellId[i + dx[d]][j + dy[d]];
                stepTrans[sid] = ncell * 4 + d;
            } else {
                stepTrans[sid] = -1;
            }
        }
    }

    // compute equivalence classes (bisimulation)
    vector<int> cls(N), newCls(N);
    for (int i = 0; i < N; ++i) cls[i] = dist[i];
    struct Info {
        int id, dist, left, right, step;
    };
    vector<Info> infos(N);
    bool changed = true;
    while (changed) {
        changed = false;
        for (int i = 0; i < N; ++i) {
            infos[i] = {i, dist[i],
                        cls[leftTrans[i]],
                        cls[rightTrans[i]],
                        stepTrans[i] == -1 ? -1 : cls[stepTrans[i]]};
        }
        sort(infos.begin(), infos.end(), [](const Info& a, const Info& b) {
            if (a.dist != b.dist) return a.dist < b.dist;
            if (a.left != b.left) return a.left < b.left;
            if (a.right != b.right) return a.right < b.right;
            return a.step < b.step;
        });
        int cur = 0;
        newCls[infos[0].id] = 0;
        for (int i = 1; i < N; ++i) {
            if (infos[i].dist != infos[i-1].dist ||
                infos[i].left != infos[i-1].left ||
                infos[i].right != infos[i-1].right ||
                infos[i].step != infos[i-1].step) {
                ++cur;
            }
            newCls[infos[i].id] = cur;
        }
        if (newCls != cls) {
            changed = true;
            cls.swap(newCls);
        }
    }

    int maxCls = 0;
    for (int x : cls) maxCls = max(maxCls, x);
    int numCls = maxCls + 1;
    vector<int> classPosRep(numCls, -1);
    vector<char> classMultiple(numCls, false);
    for (int i = 0; i < N; ++i) {
        int cell = i / 4;
        int cl = cls[i];
        if (classPosRep[cl] == -1) classPosRep[cl] = cell;
        else if (classPosRep[cl] != cell) classMultiple[cl] = true;
    }
    bool impossible = false;
    for (int cl = 0; cl < numCls; ++cl) {
        if (classMultiple[cl]) {
            impossible = true;
            break;
        }
    }

    // interaction
    int d;
    if (!(cin >> d)) return 0;
    vector<int> S;
    S.reserve(N);
    for (int i = 0; i < N; ++i) if (dist[i] == d) S.push_back(i);

    if (impossible) {
        cout << "no\n" << flush;
        return 0;
    }

    while (true) {
        // compute possible positions
        vector<char> posPossible(openCnt, 0);
        int posCnt = 0;
        for (int s : S) {
            int cell = s / 4;
            if (!posPossible[cell]) {
                posPossible[cell] = 1;
                ++posCnt;
            }
        }
        if (posCnt == 1) {
            int cell = -1;
            for (int i = 0; i < openCnt; ++i) if (posPossible[i]) { cell = i; break; }
            int row = cellRow[cell] + 1;
            int col = cellCol[cell] + 1;
            cout << "yes " << row << " " << col << "\n" << flush;
            return 0;
        }

        // safety for step
        bool stepSafe = true;
        for (int s : S) if (dist[s] == 0) { stepSafe = false; break; }

        // evaluate actions
        struct Candidate {
            string act;
            vector<int>* vec;
            int worst;
        };
        vector<Candidate> cand;

        // left
        vector<int> Sleft;
        Sleft.reserve(S.size());
        unordered_map<int,int> freqL;
        for (int s : S) {
            int ns = leftTrans[s];
            Sleft.push_back(ns);
            ++freqL[dist[ns]];
        }
        int worstL = 0;
        for (auto &p : freqL) worstL = max(worstL, p.second);
        cand.push_back({"left", &Sleft, worstL});

        // right
        vector<int> Sright;
        Sright.reserve(S.size());
        unordered_map<int,int> freqR;
        for (int s : S) {
            int ns = rightTrans[s];
            Sright.push_back(ns);
            ++freqR[dist[ns]];
        }
        int worstR = 0;
        for (auto &p : freqR) worstR = max(worstR, p.second);
        cand.push_back({"right", &Sright, worstR});

        // step
        vector<int> Sstep;
        int worstS = INT_MAX;
        if (stepSafe) {
            Sstep.reserve(S.size());
            unordered_map<int,int> freqS;
            for (int s : S) {
                int ns = stepTrans[s];
                Sstep.push_back(ns);
                ++freqS[dist[ns]];
            }
            worstS = 0;
            for (auto &p : freqS) worstS = max(worstS, p.second);
            cand.push_back({"step", &Sstep, worstS});
        }

        // choose best action (minimize worst, tie: step > left > right)
        string bestAct;
        vector<int>* bestVec = nullptr;
        int bestWorst = INT_MAX;
        for (auto &c : cand) {
            if (c.worst < bestWorst) {
                bestWorst = c.worst;
                bestAct = c.act;
                bestVec = c.vec;
            } else if (c.worst == bestWorst) {
                if (bestAct != "step" && c.act == "step") {
                    bestAct = c.act;
                    bestVec = c.vec;
                } else if (bestAct == "right" && c.act == "left") {
                    bestAct = c.act;
                    bestVec = c.vec;
                }
            }
        }

        cout << bestAct << "\n" << flush;
        int nd;
        if (!(cin >> nd)) return 0;
        vector<int> newS;
        newS.reserve(bestVec->size());
        for (int ns : *bestVec) if (dist[ns] == nd) newS.push_back(ns);
        S.swap(newS);
    }
    return 0;
}
