#include <bits/stdc++.h>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int x, y;
    if (!(cin >> x >> y)) return 0;

    vector<vector<unsigned char>> black(1002, vector<unsigned char>(1002));
    int previousX = x, previousY = y;
    int phase = 0, wallIndex = 0;
    int directionX = 1, directionY = 0;
    int baseX = x, baseY = y, wallDistance = 2;

    auto isBlack = [&](int a, int b) {
        return a >= 1 && a <= 1000 && b >= 1 && b <= 1000 && black[a][b];
    };

    for (int turn = 0; ; ++turn) {
        vector<pair<int, int>> exits;
        for (int dx = -1; dx <= 1; ++dx) {
            for (int dy = -1; dy <= 1; ++dy) {
                if (dx == 0 && dy == 0) continue;
                int a = x + dx, b = y + dy;
                if (a > 0 && b > 0 && !isBlack(a, b)) exits.push_back({a, b});
            }
        }

        int markX = 0, markY = 0;
        int endgameThreshold = (turn % 200 < 100 ? 2 : 3);
        if ((int)exits.size() <= endgameThreshold) {
            int bestScore = -1, bestSum = INT_MAX;
            for (auto [a, b] : exits) {
                if (a > 1000 || b > 1000) continue;
                int score = 0;
                for (int dx = -1; dx <= 1; ++dx)
                    for (int dy = -1; dy <= 1; ++dy)
                        score += isBlack(a + dx, b + dy);
                if (score > bestScore || (score == bestScore && a + b < bestSum)) {
                    bestScore = score;
                    bestSum = a + b;
                    markX = a;
                    markY = b;
                }
            }
        }

        if (markX == 0) {
            if (phase == 0) {
                directionX = x - previousX;
                directionY = y - previousY;
                if (turn == 0) {
                    directionX = 1;
                    directionY = 0;
                }
                baseX = x;
                baseY = y;
                wallDistance = turn < 50 ? 2 : 3;
                wallIndex = 0;
                phase = 3;
            }

            static const int order[3] = {-1, 0, 1};
            int k = order[wallIndex++];
            if (directionX != 0 && directionY != 0) {
                markX = baseX + wallDistance * directionX - (wallDistance - 1) * directionX * (k == 1);
                markY = baseY + wallDistance * directionY - (wallDistance - 1) * directionY * (k == -1);
            } else if (directionX != 0) {
                markX = baseX + wallDistance * directionX;
                markY = baseY + k;
            } else {
                markX = baseX + k;
                markY = baseY + wallDistance * directionY;
            }
            --phase;
        }

        if (markX < 1 || markX > 1000 || markY < 1 || markY > 1000 || isBlack(markX, markY)) {
            markX = clamp(x, 1, 1000);
            markY = clamp(y, 1, 1000);
        }
        black[markX][markY] = 1;
        cout << markX << ' ' << markY << '\n' << flush;

        int nextX, nextY;
        if (!(cin >> nextX >> nextY)) return 0;
        if (nextX == 0 && nextY == 0) return 0;
        previousX = x;
        previousY = y;
        x = nextX;
        y = nextY;
    }
}
