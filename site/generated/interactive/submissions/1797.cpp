#include <bits/stdc++.h>
using namespace std;

int n, N;
int curx = 0, cury = 0;
char lastReply = '^';

vector<char> cacheCell;
vector<unsigned char> fullCol;

char doMove(char d) {
    cout.put(d);
    cout.put('\n');
    cout.flush();

    char c;
    if (!(cin >> c)) exit(0);
    if (c == 'G') exit(0);

    if (d == '^') ++cury;
    else if (d == 'v') --cury;
    else if (d == '>') ++curx;
    else if (d == '<') --curx;

    lastReply = c;
    return c;
}

void moveTo(int tx, int ty) {
    while (curx < tx) doMove('>');
    while (curx > tx) doMove('<');
    while (cury < ty) doMove('^');
    while (cury > ty) doMove('v');
}

void scanColumnUp(int x) {
    moveTo(x, -n);
    int base = (x + n) * N;

    for (int y = -n; y <= n; ++y) {
        if (y > -n) lastReply = doMove('^');
        cacheCell[base + (y + n)] = lastReply;
    }

    fullCol[x + n] = 1;
}

void scanBothColumns(int mid) {
    int xl = mid;
    int xr = mid + 1;

    moveTo(xl, -n);
    int baseL = (xl + n) * N;

    for (int y = -n; y <= n; ++y) {
        if (y > -n) lastReply = doMove('^');
        cacheCell[baseL + (y + n)] = lastReply;
    }
    fullCol[xl + n] = 1;

    lastReply = doMove('>');

    int baseR = (xr + n) * N;
    for (int y = n; y >= -n; --y) {
        if (y < n) lastReply = doMove('v');
        cacheCell[baseR + (y + n)] = lastReply;
    }
    fullCol[xr + n] = 1;
}

int countVerticalCrossings(int mid) {
    int xl = mid;
    int xr = mid + 1;

    bool leftFull = fullCol[xl + n];
    bool rightFull = fullCol[xr + n];

    if (!leftFull && !rightFull) {
        scanBothColumns(mid);
    } else if (!leftFull) {
        scanColumnUp(xl);
    } else if (!rightFull) {
        scanColumnUp(xr);
    }

    int parity = 0;
    int baseL = (xl + n) * N;
    int baseR = (xr + n) * N;

    for (int y = -n; y <= n; ++y) {
        char l = cacheCell[baseL + (y + n)];
        char r = cacheCell[baseR + (y + n)];

        bool crosses = (l == '>') || (r == '<');
        parity ^= (int)crosses;
    }

    return parity;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    cin >> n;

    N = 2 * n + 1;
    cacheCell.assign((size_t)N * N, 0);
    fullCol.assign(N, 0);

    long long totalCells = 1LL * N * N;
    int followLimit = (int)min<long long>(totalCells, 5LL * N + 200);

    for (int i = 0; i < followLimit; ++i) {
        char d = lastReply;
        if (d != '^' && d != '<' && d != 'v' && d != '>') break;
        lastReply = doMove(d);
    }

    int lo = -n, hi = n;

    while (lo < hi) {
        int mid = lo + (hi - lo) / 2;
        int parity = countVerticalCrossings(mid);

        bool startLeft = (0 <= mid);
        bool treasureLeft;

        if (parity == 0) treasureLeft = startLeft;
        else treasureLeft = !startLeft;

        if (treasureLeft) hi = mid;
        else lo = mid + 1;
    }

    // The treasure is guaranteed to have been found during the scans above.
    // This is only a safety fallback.
    scanColumnUp(lo);

    return 0;
}
