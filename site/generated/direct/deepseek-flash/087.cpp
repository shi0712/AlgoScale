#include <bits/stdc++.h>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int R, C;
    cin >> R >> C;
    vector<string> grid(R);
    for (int i = 0; i < R; ++i) cin >> grid[i];

    vector<vector<int>> cid(R, vector<int>(C, -1));
    vector<pair<int,int>> cells;

    for (int i = 0; i < R; ++i) {
        for (int j = 0; j < C; ++j) {
            if (grid[i][j] == '.') {
                cid[i][j] = (int)cells.size();
                cells.push_back({i, j});
            }
        }
    }

    int K = (int)cells.size();
    int N = K * 4;

    const int dx[4] = {-1, 0, 1, 0};
    const int dy[4] = {0, 1, 0, -1}; // up, right, down, left

    vector<array<int,4>> ray(K);
    for (auto &a : ray) a = {0,0,0,0};

    // up
    for (int i = 0; i < R; ++i) {
        for (int j = 0; j < C; ++j) {
            if (grid[i][j] != '.') continue;
            int id = cid[i][j];
            if (i > 0 && grid[i-1][j] == '.') {
                ray[id][0] = 1 + ray[cid[i-1][j]][0];
            }
        }
    }

    // right
    for (int i = 0; i < R; ++i) {
        for (int j = C - 1; j >= 0; --j) {
            if (grid[i][j] != '.') continue;
            int id = cid[i][j];
            if (j + 1 < C && grid[i][j+1] == '.') {
                ray[id][1] = 1 + ray[cid[i][j+1]][1];
            }
        }
    }

    // down
    for (int i = R - 1; i >= 0; --i) {
        for (int j = 0; j < C; ++j) {
            if (grid[i][j] != '.') continue;
            int id = cid[i][j];
            if (i + 1 < R && grid[i+1][j] == '.') {
                ray[id][2] = 1 + ray[cid[i+1][j]][2];
            }
        }
    }

    // left
    for (int i = 0; i < R; ++i) {
        for (int j = 0; j < C; ++j) {
            if (grid[i][j] != '.') continue;
            int id = cid[i][j];
            if (j > 0 && grid[i][j-1] == '.') {
                ray[id][3] = 1 + ray[cid[i][j-1]][3];
            }
        }
    }

    vector<int> rayState(N), cellOfState(N);
    vector<int> transL(N), transR(N), transF(N, -1);

    vector<vector<int>> byRay(100);

    for (int cell = 0; cell < K; ++cell) {
        for (int dir = 0; dir < 4; ++dir) {
            int s = cell * 4 + dir;
            int d = ray[cell][dir];
            rayState[s] = d;
            cellOfState[s] = cell;
            byRay[d].push_back(s);

            transL[s] = cell * 4 + (dir + 3) % 4;
            transR[s] = cell * 4 + (dir + 1) % 4;

            if (d > 0) {
                int i = cells[cell].first;
                int j = cells[cell].second;
                int ni = i + dx[dir];
                int nj = j + dy[dir];
                int nc = cid[ni][nj];
                transF[s] = nc * 4 + dir;
            }
        }
    }

    // Compute bisimulation equivalence classes.
    vector<vector<int>> pred[3];
    for (int a = 0; a < 3; ++a) pred[a].resize(N);

    for (int s = 0; s < N; ++s) {
        pred[0][transL[s]].push_back(s);
        pred[1][transR[s]].push_back(s);
        if (transF[s] != -1) {
            pred[2][transF[s]].push_back(s);
        }
    }

    vector<vector<int>> init(100);
    for (int s = 0; s < N; ++s) {
        init[rayState[s]].push_back(s);
    }

    vector<set<int>> blocks;
    blocks.reserve(N + 5);
    vector<int> blockId(N);

    queue<int> q;

    for (int d = 0; d < 100; ++d) {
        if (init[d].empty()) continue;
        int b = (int)blocks.size();
        blocks.emplace_back();
        for (int s : init[d]) {
            blocks.back().insert(s);
            blockId[s] = b;
        }
        q.push(b);
    }

    vector<int> markBlock(N + 5, 0), headMark(N + 5, -1), nextMark(N, -1);
    vector<int> markState(N, 0);

    int iterBlock = 0;
    int iterState = 0;

    while (!q.empty()) {
        int b = q.front();
        q.pop();

        if (blocks[b].empty()) continue;

        vector<int> splitter(blocks[b].begin(), blocks[b].end());

        for (int a = 0; a < 3; ++a) {
            vector<int> X;
            X.reserve(splitter.size());

            for (int v : splitter) {
                for (int u : pred[a][v]) {
                    X.push_back(u);
                }
            }

            if (X.empty()) continue;

            ++iterBlock;
            vector<int> affected;

            for (int u : X) {
                int pb = blockId[u];

                if (markBlock[pb] != iterBlock) {
                    markBlock[pb] = iterBlock;
                    headMark[pb] = -1;
                    affected.push_back(pb);
                }

                nextMark[u] = headMark[pb];
                headMark[pb] = u;
            }

            for (int pb : affected) {
                int cnt = 0;
                for (int u = headMark[pb]; u != -1; u = nextMark[u]) {
                    ++cnt;
                }

                int sz = (int)blocks[pb].size();
                if (cnt == 0 || cnt == sz) continue;

                int nb = (int)blocks.size();
                blocks.emplace_back();

                if (cnt <= sz - cnt) {
                    // Move marked part to new block.
                    for (int u = headMark[pb]; u != -1; u = nextMark[u]) {
                        blocks[pb].erase(u);
                        blocks[nb].insert(u);
                        blockId[u] = nb;
                    }
                } else {
                    // Move unmarked part to new block.
                    ++iterState;

                    for (int u = headMark[pb]; u != -1; u = nextMark[u]) {
                        markState[u] = iterState;
                    }

                    vector<int> unmarked;
                    unmarked.reserve(sz - cnt);

                    for (int u : blocks[pb]) {
                        if (markState[u] != iterState) {
                            unmarked.push_back(u);
                        }
                    }

                    for (int u : unmarked) {
                        blocks[pb].erase(u);
                        blocks[nb].insert(u);
                        blockId[u] = nb;
                    }
                }

                q.push(nb);
            }
        }
    }

    vector<int> classId(N);
    for (int s = 0; s < N; ++s) classId[s] = blockId[s];

    int curD;
    cin >> curD;

    vector<int> B = byRay[curD];

    while (true) {
        int firstPos = cellOfState[B[0]];
        int firstClass = classId[B[0]];

        bool samePos = true;
        bool sameClass = true;

        for (int s : B) {
            if (cellOfState[s] != firstPos) samePos = false;
            if (classId[s] != firstClass) sameClass = false;
        }

        if (samePos) {
            cout << "yes " << cells[firstPos].first + 1 << " "
                 << cells[firstPos].second + 1 << endl;
            return 0;
        }

        if (sameClass) {
            cout << "no" << endl;
            return 0;
        }

        int bestAction = -1;
        int bestMax = INT_MAX;
        long long bestSq = LLONG_MAX;

        auto priority = [](int a) {
            if (a == 2) return 2; // step
            if (a == 1) return 1; // right
            return 0;             // left
        };

        for (int a = 0; a < 3; ++a) {
            if (a == 2 && curD == 0) continue;

            int cnt[100] = {};
            int mx = 0;

            for (int s : B) {
                int t;
                if (a == 0) t = transL[s];
                else if (a == 1) t = transR[s];
                else t = transF[s];

                int nd = rayState[t];
                ++cnt[nd];
                if (cnt[nd] > mx) mx = cnt[nd];
            }

            long long sq = 0;
            for (int v = 0; v < 100; ++v) {
                if (cnt[v]) sq += 1LL * cnt[v] * cnt[v];
            }

            bool better = false;
            if (mx < bestMax) {
                better = true;
            } else if (mx == bestMax) {
                if (sq < bestSq) {
                    better = true;
                } else if (sq == bestSq) {
                    if (bestAction == -1 || priority(a) > priority(bestAction)) {
                        better = true;
                    }
                }
            }

            if (better) {
                bestMax = mx;
                bestSq = sq;
                bestAction = a;
            }
        }

        if (bestAction == 0) {
            cout << "left" << endl;
        } else if (bestAction == 1) {
            cout << "right" << endl;
        } else {
            cout << "step" << endl;
        }

        int nd;
        cin >> nd;

        vector<int> newB;
        newB.reserve(B.size());

        for (int s : B) {
            int t;
            if (bestAction == 0) t = transL[s];
            else if (bestAction == 1) t = transR[s];
            else t = transF[s];

            if (t != -1 && rayState[t] == nd) {
                newB.push_back(t);
            }
        }

        B.swap(newB);
        curD = nd;
    }

    return 0;
}
