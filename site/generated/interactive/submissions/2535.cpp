#include <bits/stdc++.h>
using namespace std;

const int MAXN = 4005;
char grid[MAXN][MAXN];
int n, offset, cx, cy;

char move(char d) {
    printf("%c\n", d);
    fflush(stdout);
    char c;
    scanf(" %c", &c);
    if (c == 'G') exit(0);
    if (d == '^') cy++;
    else if (d == 'v') cy--;
    else if (d == '>') cx++;
    else if (d == '<') cx--;
    grid[cy + offset][cx + offset] = c;
    return c;
}

void move_to(int x, int y) {
    while (cx != x) {
        if (cx < x) move('>');
        else move('<');
    }
    while (cy != y) {
        if (cy < y) move('^');
        else move('v');
    }
}

int scan_vertical(int c) {
    int parity = 0;
    int startY, endY, step;
    if (cy >= 0) {
        startY = n;
        endY = -n;
        step = -1;
    } else {
        startY = -n;
        endY = n;
        step = 1;
    }
    move_to(c, startY);
    for (int y = startY;; y += step) {
        char ch = grid[y + offset][c + offset];
        if (ch == '>') parity ^= 1;
        if (y == endY) break;
        move(step == 1 ? '^' : 'v');
    }
    move('>');
    int step2 = -step;
    for (int y = endY;; y += step2) {
        char ch = grid[y + offset][(c + 1) + offset];
        if (ch == '<') parity ^= 1;
        if (y == startY) break;
        move(step2 == 1 ? '^' : 'v');
    }
    return parity;
}

int scan_horizontal(int d) {
    int parity = 0;
    int startX, endX, step;
    if (cx >= 0) {
        startX = n;
        endX = -n;
        step = -1;
    } else {
        startX = -n;
        endX = n;
        step = 1;
    }
    move_to(startX, d);
    for (int x = startX;; x += step) {
        char ch = grid[d + offset][x + offset];
        if (ch == '^') parity ^= 1;
        if (x == endX) break;
        move(step == 1 ? '>' : '<');
    }
    move('^');
    int step2 = -step;
    for (int x = endX;; x += step2) {
        char ch = grid[(d + 1) + offset][x + offset];
        if (ch == 'v') parity ^= 1;
        if (x == startX) break;
        move(step2 == 1 ? '>' : '<');
    }
    return parity;
}

int main() {
    scanf("%d", &n);
    offset = n;
    memset(grid, 0, sizeof(grid));
    cx = cy = 0;
    grid[0 + offset][0 + offset] = '^';

    int L = -n, R = n;
    while (L < R) {
        int mid = L + (R - L) / 2;
        int parity = scan_vertical(mid);
        bool startLeft = (0 <= mid);
        bool Tleft = (parity == 0) ? startLeft : !startLeft;
        if (Tleft) R = mid;
        else L = mid + 1;
    }
    int Tx = L;

    L = -n;
    R = n;
    while (L < R) {
        int mid = L + (R - L) / 2;
        int parity = scan_horizontal(mid);
        bool startBelow = (0 <= mid);
        bool Tbelow = (parity == 0) ? startBelow : !startBelow;
        if (Tbelow) R = mid;
        else L = mid + 1;
    }
    int Ty = L;

    move_to(Tx, Ty);
    return 0;
}
