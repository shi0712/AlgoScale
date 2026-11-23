#include <bits/stdc++.h>
using namespace std;

static bool blk[1005][1005];
static int rx, ry;
static long long turns = 0;

static inline bool blackAt(int x, int y) {
    if (x < 1 || y < 1 || x > 1000 || y > 1000) return false;
    return blk[x][y];
}
static void doMark(int x, int y) {
    if (x < 1) x = 1; if (x > 1000) x = 1000;
    if (y < 1) y = 1; if (y > 1000) y = 1000;
    printf("%d %d\n", x, y); fflush(stdout);
    blk[x][y] = true;
    if (++turns > 100000) exit(0);
    int a, b;
    if (scanf("%d %d", &a, &b) != 2) exit(0);
    if (a <= 0 || b <= 0) exit(0);
    rx = a; ry = b;
}
static int whiteNb(pair<int,int>* out) {
    int n = 0;
    for (int dx = -1; dx <= 1; dx++) for (int dy = -1; dy <= 1; dy++) {
        if (!dx && !dy) continue;
        int nx = rx + dx, ny = ry + dy;
        if (nx < 1 || ny < 1) continue;
        if (!blackAt(nx, ny)) out[n++] = make_pair(nx, ny);
    }
    return n;
}
static bool forcedTwo(int bx, int by, int ax, int ay, int ox, int oy) {
    for (int dx = -1; dx <= 1; dx++) for (int dy = -1; dy <= 1; dy++) {
        if (!dx && !dy) continue;
        int nx = bx + dx, ny = by + dy;
        if (nx < 1 || ny < 1) continue;
        if (nx == ax && ny == ay) continue;
        if (blackAt(nx, ny)) continue;
        if (!(nx == ox && ny == oy)) return false;
    }
    return true;
}
int main() {
    int sx, sy;
    if (scanf("%d %d", &sx, &sy) != 2) return 0;
    rx = sx; ry = sy;
    const int M = 8;
    int X = min(999, sx + M), Y = min(999, sy + M);
    long long toggle = 0;
    while (true) {
        pair<int,int> wn[8]; int c = whiteNb(wn);
        if (c == 0) { doMark(rx, ry); continue; }
        if (c == 1 && wn[0].first <= 1000 && wn[0].second <= 1000) { doMark(wn[0].first, wn[0].second); continue; }
        if (c == 2) {
            int ax = wn[0].first, ay = wn[0].second, bx = wn[1].first, by = wn[1].second;
            if (ax <= 1000 && ay <= 1000 && forcedTwo(bx, by, ax, ay, rx, ry)) { doMark(ax, ay); continue; }
            if (bx <= 1000 && by <= 1000 && forcedTwo(ax, ay, bx, by, rx, ry)) { doMark(bx, by); continue; }
        }
        if (rx > 999 || ry > 999) {
            bool ok = false;
            for (int i = 0; i < c; i++) if (wn[i].first <= 1000 && wn[i].second <= 1000) { doMark(wn[i].first, wn[i].second); ok = true; break; }
            if (!ok) doMark(1000, 1000);
            continue;
        }
        if (rx > X) X = min(999, rx + M);
        if (ry > Y) Y = min(999, ry + M);
        long long dv = LLONG_MAX, dv2 = LLONG_MAX; int vy = -1;
        for (int y = 1; y <= Y + 1; y++) {
            if (blk[X + 1][y]) continue;
            long long d = max((long long)abs(X + 1 - rx), (long long)abs(y - ry));
            long long d2 = (long long)(X + 1 - rx) * (X + 1 - rx) + (long long)(y - ry) * (y - ry);
            if (d < dv || (d == dv && d2 < dv2)) { dv = d; dv2 = d2; vy = y; }
        }
        long long dh = LLONG_MAX, dh2 = LLONG_MAX; int hx = -1;
        for (int x = 1; x <= X + 1; x++) {
            if (blk[x][Y + 1]) continue;
            long long d = max((long long)abs(x - rx), (long long)abs(Y + 1 - ry));
            long long d2 = (long long)(x - rx) * (x - rx) + (long long)(Y + 1 - ry) * (Y + 1 - ry);
            if (d < dh || (d == dh && d2 < dh2)) { dh = d; dh2 = d2; hx = x; }
        }
        if (vy < 0 && hx < 0) break;
        bool useV;
        if (vy < 0) useV = false;
        else if (hx < 0) useV = true;
        else if (dv != dh) useV = dv < dh;
        else if (dv2 != dh2) useV = dv2 < dh2;
        else useV = ((toggle++) & 1LL) == 0;
        if (useV) doMark(X + 1, vy); else doMark(hx, Y + 1);
    }
    int x1 = 1, y1 = 1, x2 = X, y2 = Y;
    vector<pair<int,int>> cut; bool vert = false; int cm = 0; int cstep = 0;
    while (true) {
        pair<int,int> wn[8]; int c = whiteNb(wn);
        if (c == 0) { doMark(rx, ry); continue; }
        if (c == 1) { doMark(wn[0].first, wn[0].second); continue; }
        if (c == 2) {
            int ax = wn[0].first, ay = wn[0].second, bx = wn[1].first, by = wn[1].second;
            if (forcedTwo(bx, by, ax, ay, rx, ry)) { doMark(ax, ay); continue; }
            if (forcedTwo(ax, ay, bx, by, rx, ry)) { doMark(bx, by); continue; }
        }
        if (!cut.empty()) {
            pair<int,int> p = cut.back(); cut.pop_back();
            if (!blk[p.first][p.second]) doMark(p.first, p.second);
            if (cut.empty()) {
                if (vert) { if (rx < cm) x2 = cm - 1; else x1 = cm + 1; }
                else { if (ry < cm) y2 = cm - 1; else y1 = cm + 1; }
            }
            continue;
        }
        if (x1 == x2 || y1 == y2) {
            pair<int,int> a = wn[0], b = wn[1];
            if (x1 == x2) { if (a.second > b.second) swap(a, b); }
            else { if (a.first > b.first) swap(a, b); }
            if (cstep == 0) { doMark(b.first, b.second); cstep = 1; }
            else { doMark(a.first, a.second); cstep = 2; }
            continue;
        }
        long long area = (long long)(x2 - x1 + 1) * (y2 - y1 + 1);
        if (area <= 4) { doMark(rx, ry); continue; }
        int w = x2 - x1 + 1, h = y2 - y1 + 1;
        if (w >= h) {
            vert = true; cm = (x1 + x2) / 2;
            for (int y = y1; y <= y2; y++) if (!blk[cm][y]) cut.push_back(make_pair(cm, y));
            int rry = ry;
            sort(cut.begin(), cut.end(), [&](const pair<int,int>& A, const pair<int,int>& B) {
                return abs(A.second - rry) > abs(B.second - rry); });
        } else {
            vert = false; cm = (y1 + y2) / 2;
            for (int x = x1; x <= x2; x++) if (!blk[x][cm]) cut.push_back(make_pair(x, cm));
            int rrx = rx;
            sort(cut.begin(), cut.end(), [&](const pair<int,int>& A, const pair<int,int>& B) {
                return abs(A.first - rrx) > abs(B.first - rrx); });
        }
        if (cut.empty()) {
            if (vert) { if (rx < cm) x2 = cm - 1; else x1 = cm + 1; }
            else { if (ry < cm) y2 = cm - 1; else y1 = cm + 1; }
        }
    }
    return 0;
}
