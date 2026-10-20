#include <bits/stdc++.h>
using namespace std;

// ---------------------------------------------------------------------------
// Model
// ---------------------------------------------------------------------------
// A tromino = 1 "center" cell (a '*' or '+') + exactly two '.' cells.
//   L-shape : the two dots are perpendicular neighbours of the center
//             (allowed for both '*' and '+' centers).
//   I-shape : the two dots are opposite neighbours of the center
//             (allowed only when the center is '+').
// A center is never a '.', hence {possible centers} and {possible dots} are
// disjoint sets: the only resource shared by two trominoes is a dot cell
// (and every center can be used by at most one tromino).
//
// Max-flow network, all capacities 1:
//
//   S -> dotIn -> dotOut -> cenIn -> cenOut -> dotIn' -> dotOut' -> T
//
//   * dotIn -> dotOut  (cap 1): a dot belongs to at most one tromino.
//   * '*' center : dotOut[h] -> cenIn only from horizontal neighbours and
//                  cenOut -> dotIn[v] only to vertical neighbours, so the two
//                  dots are necessarily perpendicular  => L shape only.
//   * '+' center : every dot neighbour is both a possible entry and a
//                  possible exit  => every pair of distinct dot neighbours
//                  (perpendicular = L, opposite = I).
//   * cenIn -> cenOut  (cap 1): the center is used at most once.
//
// One unit of s-t flow therefore equals exactly one tromino, and every legal
// packing gives an integral flow of the same value, so max flow = answer.
// ---------------------------------------------------------------------------

struct Dinic {
    int n, ec = 0;
    vector<int> head, to, nxt, cap, level, it;

    explicit Dinic(int n_ = 0) : n(n_), head(n_, -1), level(n_), it(n_) {}

    void reserveEdges(size_t e) {
        to.reserve(e); nxt.reserve(e); cap.reserve(e);
    }
    inline void addEdge(int u, int v, int c) {
        to.push_back(v); cap.push_back(c); nxt.push_back(head[u]); head[u] = ec++;
        to.push_back(u); cap.push_back(0); nxt.push_back(head[v]); head[v] = ec++;
    }

    bool bfs(int s, int t) {
        fill(level.begin(), level.end(), -1);
        static vector<int> q;
        q.clear(); q.reserve(n);
        q.push_back(s); level[s] = 0;
        for (size_t i = 0; i < q.size(); i++) {
            int u = q[i], lu = level[u] + 1;
            for (int e = head[u]; e != -1; e = nxt[e])
                if (cap[e] > 0) {
                    int v = to[e];
                    if (level[v] < 0) { level[v] = lu; q.push_back(v); }
                }
        }
        return level[t] >= 0;
    }

    // Blocking flow in the current level graph.  Every capacity is 1, so each
    // augmenting path carries exactly one unit and becomes saturated.
    int maxflow(int s, int t) {
        int flow = 0;
        vector<int> path;
        path.reserve(64);
        while (bfs(s, t)) {
            it = head;
            path.clear();
            int u = s;
            while (true) {
                if (u == t) {
                    for (int e : path) { cap[e]--; cap[e ^ 1]++; }
                    flow++;
                    path.clear(); u = s;
                    continue;
                }
                int &e = it[u];
                while (e != -1 && !(cap[e] > 0 && level[to[e]] == level[u] + 1))
                    e = nxt[e];
                if (e == -1) {
                    if (u == s) break;      // blocking flow finished
                    level[u] = -1;          // dead end for this phase
                    path.pop_back();
                    u = path.empty() ? s : to[path.back()];
                } else {
                    path.push_back(e);
                    u = to[e];
                }
            }
        }
        return flow;
    }
};

int main() {
    int n, m;
    if (scanf("%d %d", &n, &m) != 2) return 0;
    vector<string> board(n);
    {
        vector<char> buf(m + 4);
        for (int i = 0; i < n; i++) {
            if (scanf("%s", buf.data()) != 1) return 0;
            board[i].assign(buf.data(), m);
        }
    }

    const int nm = n * m;
    const int DR[4] = {-1, 1, 0, 0};
    const int DC[4] = {0, 0, -1, 1};

    // node numbering: 0 = S, 1 = T, two nodes per cell afterwards
    vector<int> dotIn(nm, -1), dotOut(nm, -1), cenIn(nm, -1), cenOut(nm, -1);
    vector<int> role(2, 0), nodeCell(2, -1);   // 1 dotIn 2 dotOut 3 cenIn 4 cenOut
    int cnt = 2;
    for (int i = 0; i < nm; i++) {
        if (board[i / m][i % m] == '.') {
            dotIn[i] = cnt;  role.push_back(1); nodeCell.push_back(i); cnt++;
            dotOut[i] = cnt; role.push_back(2); nodeCell.push_back(i); cnt++;
        } else {
            cenIn[i] = cnt;  role.push_back(3); nodeCell.push_back(i); cnt++;
            cenOut[i] = cnt; role.push_back(4); nodeCell.push_back(i); cnt++;
        }
    }

    Dinic din(cnt);
    din.reserveEdges((size_t)nm * 8 + 16);

    const int S = 0, T = 1;
    for (int i = 0; i < nm; i++)
        if (dotIn[i] != -1) {
            din.addEdge(S, dotIn[i], 1);
            din.addEdge(dotIn[i], dotOut[i], 1);
            din.addEdge(dotOut[i], T, 1);
        }

    for (int i = 0; i < nm; i++) {
        if (dotIn[i] != -1) continue;               // dots are never centers
        int r = i / m, c = i % m;
        bool plus = (board[r][c] == '+');
        bool hasIn = false, hasOut = false;
        for (int d = 0; d < 4; d++) {
            int nr = r + DR[d], nc = c + DC[d];
            if (nr < 0 || nr >= n || nc < 0 || nc >= m) continue;
            int j = nr * m + nc;
            if (dotIn[j] == -1) continue;           // neighbour must be a dot
            bool horiz = (d >= 2);
            bool asIn = plus || horiz;
            bool asOut = plus || !horiz;
            if (asIn)  { din.addEdge(dotOut[j], cenIn[i], 1); hasIn = true; }
            if (asOut) { din.addEdge(cenOut[i], dotIn[j], 1); hasOut = true; }
        }
        if (hasIn && hasOut) din.addEdge(cenIn[i], cenOut[i], 1);
    }

    din.maxflow(S, T);

    // ---- extract the trominoes from the saturated arcs ----------------------
    vector<int> leftDot(nm, -1), rightDot(nm, -1);
    for (int e = 0; e < din.ec; e += 2) {
        if (din.cap[e] != 0) continue;              // arc carries no flow
        int u = din.to[e ^ 1], v = din.to[e];
        if (role[v] == 3 && role[u] == 2)        leftDot[nodeCell[v]]  = nodeCell[u];
        else if (role[u] == 4 && role[v] == 1)   rightDot[nodeCell[u]] = nodeCell[v];
    }

    vector<int> trominoOf(nm, -1);
    vector<array<int, 3>> trom;
    for (int i = 0; i < nm; i++)
        if (leftDot[i] != -1 && rightDot[i] != -1) {
            int k = (int)trom.size();
            trom.push_back({i, leftDot[i], rightDot[i]});
            trominoOf[i] = k;
            trominoOf[leftDot[i]] = k;
            trominoOf[rightDot[i]] = k;
        }

    // ---- greedy colouring: adjacent trominoes get different letters ---------
    vector<int> color(trom.size(), 0);
    for (int k = 0; k < (int)trom.size(); k++) {
        bool used[27] = {false};
        for (int t = 0; t < 3; t++) {
            int cell = trom[k][t], r = cell / m, c = cell % m;
            for (int d = 0; d < 4; d++) {
                int nr = r + DR[d], nc = c + DC[d];
                if (nr < 0 || nr >= n || nc < 0 || nc >= m) continue;
                int tk = trominoOf[nr * m + nc];
                if (tk != -1 && tk < k) used[color[tk]] = true;
            }
        }
        int col = 0;
        while (used[col]) col++;
        color[k] = col;
    }

    vector<string> ans = board;
    for (int k = 0; k < (int)trom.size(); k++) {
        char ch = (char)('a' + color[k]);
        for (int t = 0; t < 3; t++)
            ans[trom[k][t] / m][trom[k][t] % m] = ch;
    }

    string out;
    out.reserve((size_t)(m + 1) * n);
    for (int i = 0; i < n; i++) { out += ans[i]; out += '\n'; }
    fputs(out.c_str(), stdout);
    return 0;
}
