#include <bits/stdc++.h>
using namespace std;

static bool blk[1003][1003];

static inline bool isBlack(int x, int y) {
    if (x < 1 || y < 1 || x > 1000 || y > 1000) return false;
    return blk[x][y];
}

static int whiteMoves(int x, int y, int *mx, int *my) {
    int c = 0;
    for (int dx = -1; dx <= 1; dx++)
        for (int dy = -1; dy <= 1; dy++) {
            if (!dx && !dy) continue;
            int nx = x + dx, ny = y + dy;
            if (nx < 1 || ny < 1) continue;
            if (isBlack(nx, ny)) continue;
            mx[c] = nx;
            my[c] = ny;
            c++;
        }
    return c;
}

// our turn, robot at (x,y); returns value (lower is better for us)
static int F(int x, int y, int d) {
    int wx[8], wy[8];
    int k = whiteMoves(x, y, wx, wy);
    if (d == 0) return k;
    // candidates: markable white neighbours
    int cx[8], cy[8], cc = 0;
    for (int i = 0; i < k; i++) {
        if (wx[i] <= 1000 && wy[i] <= 1000) {
            cx[cc] = wx[i];
            cy[cc] = wy[i];
            cc++;
        }
    }
    int best = INT_MAX;
    if (cc == 0) {
        // no useful marking; robot moves freely
        int val = -1000;
        for (int i = 0; i < k; i++) {
            int v = F(wx[i], wy[i], d - 1);
            val = max(val, v);
            if (val >= best) break;
        }
        return val;
    }
    for (int ci = 0; ci < cc; ci++) {
        int px = cx[ci], py = cy[ci];
        blk[px][py] = true;
        int mx[8], my[8];
        int m = whiteMoves(x, y, mx, my);
        int val;
        if (m == 0) {
            val = -1000 - d;
        } else {
            val = -100000;
            for (int i = 0; i < m; i++) {
                int v = F(mx[i], my[i], d - 1);
                if (v > val) val = v;
                if (val >= best) break;
            }
        }
        blk[px][py] = false;
        if (val < best) best = val;
    }
    return best;
}

int main() {
    int rx, ry;
    if (scanf("%d %d", &rx, &ry) != 2) return 0;
    const int DEPTH = 3;
    while (true) {
        int wx[8], wy[8];
        int k = whiteMoves(rx, ry, wx, wy);
        int cx[8], cy[8], cc = 0;
        for (int i = 0; i < k; i++) {
            if (wx[i] <= 1000 && wy[i] <= 1000) {
                cx[cc] = wx[i];
                cy[cc] = wy[i];
                cc++;
            }
        }
        int ox, oy;
        if (cc == 0) {
            // nothing useful adjacent; mark something harmless near origin
            ox = min(max(rx, 1), 1000);
            oy = min(max(ry, 1), 1000);
        } else {
            // sort by x+y descending (push robot toward origin)
            vector<int> idx(cc);
            iota(idx.begin(), idx.end(), 0);
            sort(idx.begin(), idx.end(), [&](int a, int b) {
                return cx[a] + cy[a] > cx[b] + cy[b];
            });
            int best = INT_MAX;
            ox = cx[idx[0]];
            oy = cy[idx[0]];
            for (int t = 0; t < cc; t++) {
                int ci = idx[t];
                int px = cx[ci], py = cy[ci];
                blk[px][py] = true;
                int mx[8], my[8];
                int m = whiteMoves(rx, ry, mx, my);
                int val;
                if (m == 0) {
                    val = -100000;
                } else {
                    val = -100000;
                    for (int i = 0; i < m; i++) {
                        int v = F(mx[i], my[i], DEPTH - 1);
                        if (v > val) val = v;
                        if (val >= best) break;
                    }
                }
                blk[px][py] = false;
                if (val < best) {
                    best = val;
                    ox = px;
                    oy = py;
                }
            }
        }
        printf("%d %d\n", ox, oy);
        fflush(stdout);
        blk[ox][oy] = true;
        int nx, ny;
        if (scanf("%d %d", &nx, &ny) != 2) return 0;
        if (nx == 0 && ny == 0) return 0;
        rx = nx;
        ry = ny;
    }
    return 0;
}
