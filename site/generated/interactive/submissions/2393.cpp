#include <bits/stdc++.h>
using namespace std;

static bool blk[1003][1003];

inline bool isBlack(int x, int y) {
    if (x < 1 || y < 1 || x > 1000 || y > 1000) return false;
    return blk[x][y];
}

inline int cntWhite(int x, int y) {
    int c = 0;
    for (int dx = -1; dx <= 1; dx++)
        for (int dy = -1; dy <= 1; dy++) {
            if (!dx && !dy) continue;
            int nx = x + dx, ny = y + dy;
            if (nx < 1 || ny < 1) continue;
            if (!isBlack(nx, ny)) c++;
        }
    return c;
}

int bestMX, bestMY;

long long ourMove(int x, int y, int depth, int R, bool top) {
    long long best = LLONG_MAX;
    bool any = false;
    // candidates: ordered neighbors first
    vector<pair<int,int>> cands;
    for (int dx = -R; dx <= R; dx++)
        for (int dy = -R; dy <= R; dy++) {
            int cx = x + dx, cy = y + dy;
            if (cx < 1 || cy < 1 || cx > 1000 || cy > 1000) continue;
            if (blk[cx][cy]) continue;
            cands.push_back({cx, cy});
        }
    for (auto &cd : cands) {
        int cx = cd.first, cy = cd.second;
        any = true;
        blk[cx][cy] = true;
        int c = cntWhite(x, y);
        long long val;
        if (c == 0) {
            val = -1000000000LL;
        } else if (depth == 1) {
            val = (long long)c * 100 + x + y;
        } else {
            long long mx = LLONG_MIN;
            for (int dx = -1; dx <= 1; dx++)
                for (int dy = -1; dy <= 1; dy++) {
                    if (!dx && !dy) continue;
                    int nx = x + dx, ny = y + dy;
                    if (nx < 1 || ny < 1) continue;
                    if (isBlack(nx, ny)) continue;
                    long long v = ourMove(nx, ny, depth - 1, 1, false);
                    if (v > mx) mx = v;
                }
            val = mx;
        }
        blk[cx][cy] = false;
        if (val < best) {
            best = val;
            if (top) { bestMX = cx; bestMY = cy; }
        }
    }
    if (!any) {
        // pass
        int c = cntWhite(x, y);
        if (c == 0) return -1000000000LL;
        if (depth == 1) return (long long)c * 100 + x + y;
        long long mx = LLONG_MIN;
        for (int dx = -1; dx <= 1; dx++)
            for (int dy = -1; dy <= 1; dy++) {
                if (!dx && !dy) continue;
                int nx = x + dx, ny = y + dy;
                if (nx < 1 || ny < 1) continue;
                if (isBlack(nx, ny)) continue;
                long long v = ourMove(nx, ny, depth - 1, 1, false);
                if (v > mx) mx = v;
            }
        return mx;
    }
    return best;
}

int main() {
    int x, y;
    if (!(cin >> x >> y)) return 0;
    while (true) {
        bestMX = -1; bestMY = -1;
        ourMove(x, y, 3, 2, true);
        if (bestMX < 0) {
            // fallback: any white neighbor or any cell
            bestMX = min(1000, max(1, x + 1));
            bestMY = min(1000, max(1, y));
        }
        blk[bestMX][bestMY] = true;
        printf("%d %d\n", bestMX, bestMY);
        fflush(stdout);
        int nx, ny;
        if (!(cin >> nx >> ny)) return 0;
        if (nx == 0 && ny == 0) return 0;
        if (nx == -1) return 0;
        x = nx; y = ny;
    }
    return 0;
}
