#include <array>
#include <cstdint>
#include <iostream>
#include <vector>

using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n)) return 0;

    const int width = 2 * n + 1;
    const array<int, 4> dx = {0, -1, 0, 1};
    const array<int, 4> dy = {1, 0, -1, 0};
    const array<char, 4> symbol = {'^', '<', 'v', '>'};
    vector<uint8_t> state(static_cast<size_t>(width) * width, 0);

    int x = 0, y = 0;
    int incoming = -1;
    int outgoing = 0;
    int forwardFailures = 0;
    int oppositeFailures = 0;
    int sideFailures = 0;

    auto cell = [&](int px, int py) -> size_t {
        return static_cast<size_t>(px + n) * width + (py + n);
    };
    auto inside = [&](int px, int py) {
        return px >= -n && px <= n && py >= -n && py <= n;
    };
    auto decode = [&](char response) {
        for (int d = 0; d < 4; ++d) {
            if (response == symbol[d]) return d;
        }
        return -1;
    };
    auto step = [&](int direction, char &response) {
        cout << symbol[direction] << '\n' << flush;
        if (!(cin >> response)) return false;
        x += dx[direction];
        y += dy[direction];
        return response != 'G';
    };

    state[cell(x, y)] = 2;

    while (true) {
        bool changedCell = false;

        auto probe = [&](int direction, int type) {
            const int oldOutgoing = outgoing;
            char response;
            if (!step(direction, response)) return false;
            int decoded = decode(response);

            if (decoded >= 0) {
                state[cell(x, y)] = 2;
                outgoing = decoded;
                incoming = -1;
                if (type == 0) forwardFailures = 0;
                else if (type == 1) oppositeFailures = 0;
                else sideFailures = 0;
                changedCell = true;
                return true;
            }
            if (response != '.') return false;

            state[cell(x, y)] = 1;
            if (type == 1) {
                ++oppositeFailures;
                if (!step(direction ^ 2, response)) return false;
                if (decode(response) < 0) return false;
                return true;
            }

            if (!step(oldOutgoing, response)) return false;
            decoded = decode(response);
            if (decoded >= 0) {
                state[cell(x, y)] = 2;
                outgoing = decoded;
                incoming = -1;
                if (type == 0) forwardFailures = 0;
                else sideFailures = 0;
                changedCell = true;
                return true;
            }
            if (response != '.') return false;

            state[cell(x, y)] = 1;
            if (!step(direction ^ 2, response)) return false;
            decoded = decode(response);
            if (decoded < 0) return false;
            state[cell(x, y)] = 2;
            outgoing = decoded;
            incoming = oldOutgoing;
            if (type == 0) ++forwardFailures;
            else ++sideFailures;
            changedCell = true;
            return true;
        };

        if (incoming >= 0) {
            if (outgoing != incoming) {
                const int oppositeDirection = outgoing ^ 2;
                const int qx = x + dx[oppositeDirection];
                const int qy = y + dy[oppositeDirection];
                if (oppositeFailures < 1 && inside(qx, qy) && state[cell(qx, qy)] == 0) {
                    if (!probe(oppositeDirection, 1)) return 0;
                }

                const int qxForward = x + dx[incoming];
                const int qyForward = y + dy[incoming];
                if (!changedCell && forwardFailures < 5 && inside(qxForward, qyForward)
                    && state[cell(qxForward, qyForward)] == 0) {
                    if (!probe(incoming, 0)) return 0;
                }
            } else if (sideFailures < 1) {
                const array<int, 2> directions = {(outgoing + 1) & 3, (outgoing + 3) & 3};
                for (int direction : directions) {
                    const int qx = x + dx[direction];
                    const int qy = y + dy[direction];
                    if (inside(qx, qy) && state[cell(qx, qy)] == 0) {
                        if (!probe(direction, 2)) return 0;
                        if (changedCell) break;
                    }
                }
            }
        }

        if (!changedCell) {
            const int oldOutgoing = outgoing;
            char response;
            if (!step(outgoing, response)) return 0;
            int decoded = decode(response);
            if (decoded < 0) return 0;
            state[cell(x, y)] = 2;
            outgoing = decoded;
            incoming = oldOutgoing;
        }
    }
}
