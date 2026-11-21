#include <bits/stdc++.h>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    const int M = 33;
    static bool black[1005][1005] = {};

    int rx, ry;
    if (!(cin >> rx >> ry)) return 0;

    int last_dx = 0, last_dy = 0;

    auto output_mark = [&](int x, int y) -> bool {
        cout << x << ' ' << y << '\n' << flush;
        black[x][y] = true;

        int nx, ny;
        if (!(cin >> nx >> ny)) return false;
        if (nx == 0 && ny == 0) return false;
        if (nx < 0 || ny < 0) return false;

        last_dx = nx - rx;
        last_dy = ny - ry;
        rx = nx;
        ry = ny;
        return true;
    };

    // Initial corner block: x, y in [31, 33].
    // During these 9 moves the robot cannot reach coordinate 30.
    for (int x = M - 2; x <= M; ++x) {
        for (int y = M - 2; y <= M; ++y) {
            if (!output_mark(x, y)) return 0;
        }
    }

    auto closest_to = [](const vector<int>& v, int target) -> int {
        int best = v[0];
        for (int z : v) {
            if (abs(z - target) < abs(best - target)) best = z;
        }
        return best;
    };

    while (true) {
        vector<int> rmis, tmis;

        int radR = 0;
        if (rx >= M - 2) radR = 1;
        else if (rx >= M - 10) radR = 2;

        if (radR > 0) {
            int lo = max(1, ry - radR);
            int hi = min(M, ry + radR);
            for (int y = lo; y <= hi; ++y) {
                if (!black[M][y]) rmis.push_back(y);
            }
        }

        int radT = 0;
        if (ry >= M - 2) radT = 1;
        else if (ry >= M - 10) radT = 2;

        if (radT > 0) {
            int lo = max(1, rx - radT);
            int hi = min(M, rx + radT);
            for (int x = lo; x <= hi; ++x) {
                if (!black[x][M]) tmis.push_back(x);
            }
        }

        int mx = -1, my = -1;
        bool chosen = false;

        bool rightCritical = (rx >= M - 1);
        bool topCritical = (ry >= M - 1);

        if (!rmis.empty() && rightCritical) {
            int pick;
            if (last_dy > 0) pick = rmis.back();
            else if (last_dy < 0) pick = rmis.front();
            else pick = closest_to(rmis, ry);
            mx = M;
            my = pick;
            chosen = true;
        } else if (!tmis.empty() && topCritical) {
            int pick;
            if (last_dx > 0) pick = tmis.back();
            else if (last_dx < 0) pick = tmis.front();
            else pick = closest_to(tmis, rx);
            mx = pick;
            my = M;
            chosen = true;
        } else if (!rmis.empty() || !tmis.empty()) {
            bool chooseRight;
            if (rmis.empty()) chooseRight = false;
            else if (tmis.empty()) chooseRight = true;
            else {
                int dr = M - rx;
                int dt = M - ry;
                chooseRight = (dr <= dt);
            }

            if (chooseRight) {
                int pick;
                if (last_dy > 0) pick = rmis.back();
                else if (last_dy < 0) pick = rmis.front();
                else pick = closest_to(rmis, ry);
                mx = M;
                my = pick;
            } else {
                int pick;
                if (last_dx > 0) pick = tmis.back();
                else if (last_dx < 0) pick = tmis.front();
                else pick = closest_to(tmis, rx);
                mx = pick;
                my = M;
            }
            chosen = true;
        }

        // Complete the two boundary walls when there is no urgent local work.
        if (!chosen) {
            for (int y = 1; y <= M && !chosen; ++y) {
                if (!black[M][y]) {
                    mx = M;
                    my = y;
                    chosen = true;
                }
            }
            for (int x = 1; x <= M && !chosen; ++x) {
                if (!black[x][M]) {
                    mx = x;
                    my = M;
                    chosen = true;
                }
            }
        }

        // Once the finite box is closed, mark the robot's current cell.
        if (!chosen) {
            mx = rx;
            my = ry;

            // Safety fallback; normally not needed.
            if (black[mx][my]) {
                for (int x = 1; x <= M - 1 && !chosen; ++x) {
                    for (int y = 1; y <= M - 1; ++y) {
                        if (!black[x][y]) {
                            mx = x;
                            my = y;
                            chosen = true;
                            break;
                        }
                    }
                }
            }
        }

        if (!output_mark(mx, my)) return 0;
    }

    return 0;
}
