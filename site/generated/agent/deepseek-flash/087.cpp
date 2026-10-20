#include <bits/stdc++.h>
using namespace std;

static int R, C;
static vector<string> g;
static const int DR[4] = {-1, 0, 1, 0};
static const int DC[4] = {0, 1, 0, -1};
static const int OFF = 256, SZ = 512;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    if (!(cin >> R >> C)) return 0;
    g.resize(R);
    for (auto &s : g) cin >> s;

    // index open cells
    vector<vector<int>> cid(R, vector<int>(C, -1));
    vector<pair<int,int>> cells;
    for (int i = 0; i < R; i++)
        for (int j = 0; j < C; j++)
            if (g[i][j] == '.') { cid[i][j] = (int)cells.size(); cells.push_back({i, j}); }
    int V = (int)cells.size();
    int N = V * 4;  // states: cell*4 + dir

    // observation distance
    vector<int> obs(N);
    for (int c = 0; c < V; c++)
        for (int d = 0; d < 4; d++) {
            int r = cells[c].first, cc = cells[c].second, k = 0;
            while (true) {
                int nr = r + DR[d] * (k + 1), nc = cc + DC[d] * (k + 1);
                if (nr < 0 || nr >= R || nc < 0 || nc >= C || g[nr][nc] == '#') break;
                k++;
            }
            obs[c * 4 + d] = k;
        }

    // transitions (0 = left, 1 = right, 2 = step); dead state index N
    int n = N + 1;
    vector<array<int,3>> tr(n);
    for (int c = 0; c < V; c++)
        for (int d = 0; d < 4; d++) {
            int s = c * 4 + d;
            tr[s][0] = c * 4 + (d + 3) % 4;
            tr[s][1] = c * 4 + (d + 1) % 4;
            if (obs[s] > 0) {
                int nr = cells[c].first + DR[d], nc = cells[c].second + DC[d];
                tr[s][2] = cid[nr][nc] * 4 + d;
            } else tr[s][2] = N;
        }
    for (int a = 0; a < 3; a++) tr[N][a] = N;

    // ---------------- Hopcroft partition refinement (bisimulation) ----------------
    vector<int> lab(n);
    for (int s = 0; s < N; s++) lab[s] = obs[s];
    lab[N] = 100000;  // unique label for dead state

    vector<int> blk(n);
    vector<vector<int>> blocks;
    {
        unordered_map<int,int> m2;
        m2.reserve((size_t)n * 2);
        for (int s = 0; s < n; s++) {
            auto it = m2.find(lab[s]);
            if (it == m2.end()) {
                int id = (int)blocks.size();
                m2.emplace(lab[s], id);
                blocks.push_back({s});
                blk[s] = id;
            } else {
                blk[s] = it->second;
                blocks[it->second].push_back(s);
            }
        }
    }
    int maxb = 2 * n + 5;
    vector<vector<int>> pre[3];
    for (int a = 0; a < 3; a++) pre[a].assign(n, {});
    for (int s = 0; s < n; s++)
        for (int a = 0; a < 3; a++) pre[a][tr[s][a]].push_back(s);

    vector<array<char,3>> inQ(maxb);
    for (int i = 0; i < maxb; i++) inQ[i] = {0, 0, 0};
    vector<int> cnt(maxb, 0), bucketIdx(maxb, -1);
    vector<int> posInBlock(n, 0);
    for (int b = 0; b < (int)blocks.size(); b++)
        for (int i = 0; i < (int)blocks[b].size(); i++) posInBlock[blocks[b][i]] = i;

    deque<pair<int,int>> dq;
    for (int b = 0; b < (int)blocks.size(); b++)
        for (int a = 0; a < 3; a++) { dq.push_back({b, a}); inQ[b][a] = 1; }

    vector<int> touched;
    vector<vector<int>> bv;
    while (!dq.empty()) {
        auto pr = dq.front(); dq.pop_front();
        int b = pr.first, a = pr.second;
        if (!inQ[b][a]) continue;
        inQ[b][a] = 0;
        if (blocks[b].empty()) continue;
        touched.clear();
        bv.clear();
        for (int s : blocks[b])
            for (int p : pre[a][s]) {
                int bb = blk[p];
                if (cnt[bb] == 0) { bucketIdx[bb] = (int)bv.size(); bv.push_back({}); touched.push_back(bb); }
                cnt[bb]++;
                bv[bucketIdx[bb]].push_back(p);
            }
        for (int y : touched) {
            if (cnt[y] == (int)blocks[y].size()) { cnt[y] = 0; bucketIdx[y] = -1; continue; }
            int nid = (int)blocks.size();
            blocks.push_back({});
            const vector<int> &vec = bv[bucketIdx[y]];
            for (int x : vec) {
                int px = posInBlock[x];
                int last = blocks[y].back();
                blocks[y][px] = last; posInBlock[last] = px;
                blocks[y].pop_back();
                blk[x] = nid;
                posInBlock[x] = (int)blocks[nid].size();
                blocks[nid].push_back(x);
            }
            int small = ((int)blocks[nid].size() <= (int)blocks[y].size()) ? nid : y;
            for (int a2 = 0; a2 < 3; a2++) {
                if (inQ[y][a2]) {
                    dq.push_back({y, a2});
                    dq.push_back({nid, a2});
                    inQ[nid][a2] = 1;
                } else {
                    dq.push_back({small, a2});
                    inQ[small][a2] = 1;
                }
            }
            cnt[y] = 0; bucketIdx[y] = -1;
        }
    }
    // block properties
    int nb = (int)blocks.size();
    vector<int> bcell(nb, -1);
    vector<char> bspan(nb, 0);
    for (int s = 0; s < N; s++) {
        int b = blk[s], c = s / 4;
        if (bcell[b] < 0) bcell[b] = c;
        else if (bcell[b] != c) bspan[b] = 1;
    }

    // ---------------- interaction ----------------
    int d;
    if (!(cin >> d)) return 0;
    vector<int> alive;
    alive.reserve(N);
    for (int s = 0; s < N; s++) if (obs[s] == d) alive.push_back(s);

    auto emitYes = [&]() {
        int c = alive[0] / 4;
        cout << "yes " << cells[c].first + 1 << " " << cells[c].second + 1 << "\n";
        cout.flush();
        exit(0);
    };
    auto emitNo = [&]() {
        cout << "no\n";
        cout.flush();
        exit(0);
    };
    auto determined = [&]() -> bool {
        int c = alive[0] / 4;
        for (int s : alive) if (s / 4 != c) return false;
        return true;
    };

    for (int s : alive) if (bspan[blk[s]]) emitNo();
    if (determined()) emitYes();

    // local map in relative coordinates
    vector<vector<int>> mp(SZ, vector<int>(SZ, 0));   // 0 unknown, 1 open, 2 wall
    vector<vector<int>> kn(SZ, vector<int>(SZ, 0));   // bitmask of directions whose obs is known
    vector<vector<char>> vis(SZ, vector<char>(SZ, 0));

    int rr = OFF, rc = OFF, cur = 0;

    auto applyObs = [&](int dd) {
        for (int i = 0; i <= dd; i++) {
            int r2 = rr + DR[cur] * i, c2 = rc + DC[cur] * i;
            mp[r2][c2] = 1;
            kn[r2][c2] |= (1 << cur);
        }
        int r3 = rr + DR[cur] * (dd + 1), c3 = rc + DC[cur] * (dd + 1);
        mp[r3][c3] = 2;
    };
    applyObs(d);

    auto doAction = [&](int a) {
        if (a == 0) { cout << "left\n"; cur = (cur + 3) % 4; }
        else if (a == 1) { cout << "right\n"; cur = (cur + 1) % 4; }
        else { cout << "step\n"; rr += DR[cur]; rc += DC[cur]; }
        cout.flush();
        int nd;
        if (!(cin >> nd)) exit(0);
        int na = 0;
        for (int i = 0; i < (int)alive.size(); i++) {
            int s2 = tr[alive[i]][a];
            if (obs[s2] == nd) alive[na++] = s2;
        }
        alive.resize(na);
        d = nd;
        applyObs(nd);
        vis[rr][rc] = 1;
    };

    auto rotateTo = [&](int dir) {
        int cw = (dir - cur + 4) % 4, ccw = (cur - dir + 4) % 4;
        if (cw <= ccw) {
            for (int i = 0; i < cw; i++) { doAction(1); if (determined()) emitYes(); }
        } else {
            for (int i = 0; i < ccw; i++) { doAction(0); if (determined()) emitYes(); }
        }
    };

    vis[rr][rc] = 1;
    vector<pair<int,int>> path;
    path.push_back({rr, rc});

    while (true) {
        // make sure all four observation distances at the current cell are known
        for (int guard = 0; guard < 5 && (kn[rr][rc] & 15) != 15; guard++) {
            doAction(1);
            if (determined()) emitYes();
        }
        if (determined()) emitYes();
        int cand = -1;
        for (int i = 0; i < 4; i++) {
            int dir = (cur + i) % 4;
            int nr = rr + DR[dir], nc = rc + DC[dir];
            if (mp[nr][nc] == 1 && !vis[nr][nc]) { cand = dir; break; }
        }
        if (cand >= 0) {
            rotateTo(cand);
            if (determined()) emitYes();
            doAction(2);
            if (determined()) emitYes();
            path.push_back({rr, rc});
        } else {
            if (path.size() <= 1) break;
            auto p = path[path.size() - 2];
            int dir = -1;
            for (int i = 0; i < 4; i++)
                if (rr + DR[i] == p.first && rc + DC[i] == p.second) { dir = i; break; }
            if (dir < 0) break;  // should not happen
            rotateTo(dir);
            if (determined()) emitYes();
            doAction(2);
            if (determined()) emitYes();
            path.pop_back();
        }
    }
    emitYes();
    return 0;
}
