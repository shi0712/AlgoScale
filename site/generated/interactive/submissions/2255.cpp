#include <bits/stdc++.h>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int rx, ry;
    if (!(cin >> rx >> ry)) return 0;

    const int R = 3;
    const int D = 50;
    const int C = D + R + 5;

    int N = max(rx, ry) + 170;
    int M = 2 * N - 1;

    vector<vector<unsigned char>> black(1005, vector<unsigned char>(1005, 0));
    vector<pair<int, int>> wallCell(M + 1);

    for (int i = 1; i <= N; i++) {
        wallCell[i] = {i, N};
    }
    for (int i = N + 1; i <= M; i++) {
        wallCell[i] = {N, 2 * N - i};
    }

    auto outputMark = [&](pair<int, int> p) -> bool {
        black[p.first][p.second] = 1;
        cout << p.first << ' ' << p.second << '\n' << flush;

        int nx, ny;
        if (!(cin >> nx >> ny)) return false;
        if (nx <= 0 || ny <= 0) return false;

        rx = nx;
        ry = ny;
        return true;
    };

    auto wallBlack = [&](int i) -> bool {
        auto [x, y] = wallCell[i];
        return black[x][y];
    };

    auto windowFull = [&](int c) -> bool {
        for (int i = max(1, c - R); i <= min(M, c + R); i++) {
            if (!wallBlack(i)) return false;
        }
        return true;
    };

    int wallCount = 0;

    for (int i = max(1, N - C); i <= min(M, N + C); i++) {
        if (wallBlack(i)) continue;
        if (!outputMark(wallCell[i])) return 0;
        wallCount++;
    }

    auto projection = [&]() -> int {
        if (ry >= rx) return rx;
        return 2 * N - ry;
    };

    int previousCenter = projection();
    int lastDelta = 0;
    bool full = windowFull(previousCenter);
    int sequential = 1;

    while (wallCount < M) {
        int center = projection();
        int delta = center - previousCenter;

        if (abs(delta) > 1) {
            full = windowFull(center);
            lastDelta = 0;
        } else {
            lastDelta = delta;
        }

        vector<int> white;
        for (int i = max(1, center - R); i <= min(M, center + R); i++) {
            if (!wallBlack(i)) white.push_back(i);
        }

        int chosen = -1;

        if (white.empty()) {
            full = true;
        } else if (full) {
            chosen = white.front();
        } else if (lastDelta < 0) {
            chosen = white.front();
        } else {
            chosen = white.back();
        }

        if (chosen == -1) {
            while (sequential <= M && wallBlack(sequential)) sequential++;
            if (sequential > M) break;
            chosen = sequential;
        }

        previousCenter = center;

        if (!outputMark(wallCell[chosen])) return 0;
        wallCount++;
        full = windowFull(center);
    }

    int xl = 1, xr = N - 1;
    int yl = 1, yr = N - 1;

    bool preferVertical = true;
    bool active = false;
    bool vertical = true;
    int middle = 0;
    vector<pair<int, int>> pending;

    while (true) {
        if (!active) {
            if (xl == xr && yl == yr) {
                if (!outputMark({rx, ry})) return 0;
                continue;
            }

            bool canVertical = xl < xr;
            bool canHorizontal = yl < yr;

            if (canVertical && (!canHorizontal || preferVertical)) {
                vertical = true;
            } else {
                vertical = false;
            }

            preferVertical = !preferVertical;
            pending.clear();

            if (vertical) {
                middle = (xl + xr) / 2;
                for (int y = yl; y <= yr; y++) {
                    if (!black[middle][y]) pending.push_back({middle, y});
                }
            } else {
                middle = (yl + yr) / 2;
                for (int x = xl; x <= xr; x++) {
                    if (!black[x][middle]) pending.push_back({x, middle});
                }
            }

            active = true;

            if (pending.empty()) {
                if (vertical) {
                    if (rx < middle) xr = middle - 1;
                    else xl = middle + 1;
                } else {
                    if (ry < middle) yr = middle - 1;
                    else yl = middle + 1;
                }
                active = false;
            }
            continue;
        }

        int best = 0;
        int bestDistance = INT_MAX;

        for (int i = 0; i < (int)pending.size(); i++) {
            int distance = max(abs(pending[i].first - rx),
                               abs(pending[i].second - ry));
            if (distance < bestDistance) {
                bestDistance = distance;
                best = i;
            }
        }

        pair<int, int> chosen = pending[best];
        pending[best] = pending.back();
        pending.pop_back();

        if (!outputMark(chosen)) return 0;

        if (pending.empty()) {
            if (vertical) {
                if (rx < middle) xr = middle - 1;
                else xl = middle + 1;
            } else {
                if (ry < middle) yr = middle - 1;
                else yl = middle + 1;
            }
            active = false;
        }
    }
}
