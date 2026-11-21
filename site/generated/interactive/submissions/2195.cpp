#include <bits/stdc++.h>
using namespace std;

int main() {
    int n;
    if (scanf("%d", &n) != 1) return 0;

    int H = 2 * n + 1;
    int cx = 0, cy = 0;
    char curResp = '^';

    auto do_move = [&](char d, auto& handle) {
        putchar(d);
        putchar('\n');
        fflush(stdout);

        int c;
        do {
            c = getchar();
            if (c == EOF) exit(0);
        } while (c != 'G' && c != '^' && c != '<' && c != 'v' && c != '>' && c != '.');

        char resp = (char)c;
        if (resp == 'G') exit(0);

        if (d == '^') ++cy;
        else if (d == 'v') --cy;
        else if (d == '<') --cx;
        else if (d == '>') ++cx;

        curResp = resp;
        handle(cx, cy, resp);
    };

    auto move_to = [&](int tx, int ty, auto& handle) {
        while (cx < tx) do_move('>', handle);
        while (cx > tx) do_move('<', handle);
        while (cy < ty) do_move('^', handle);
        while (cy > ty) do_move('v', handle);
    };

    auto query_vertical = [&](int k) -> bool {
        vector<unsigned char> markL(H, 0), markR(H, 0);
        int parity = 0;

        auto handle = [&](int x, int y, char resp) {
            if (x == k) {
                int idx = y + n;
                if (!markL[idx]) {
                    markL[idx] = 1;
                    if (resp == '>') parity ^= 1;
                }
            } else if (x == k + 1) {
                int idx = y + n;
                if (!markR[idx]) {
                    markR[idx] = 1;
                    if (resp == '<') parity ^= 1;
                }
            }
        };

        handle(cx, cy, curResp);

        int sx = k, sy = -n;
        int best = abs(cx - sx) + abs(cy - sy);

        auto consider = [&](int x, int y) {
            int d = abs(cx - x) + abs(cy - y);
            if (d < best || (d == best && (y > sy || (y == sy && x > sx)))) {
                best = d;
                sx = x;
                sy = y;
            }
        };

        consider(k, -n);
        consider(k + 1, -n);
        consider(k, n);
        consider(k + 1, n);

        move_to(sx, sy, handle);

        if (sy == -n) {
            if (sx == k) {
                for (int i = 0; i < H - 1; ++i) do_move('^', handle);
                do_move('>', handle);
                for (int i = 0; i < H - 1; ++i) do_move('v', handle);
            } else {
                for (int i = 0; i < H - 1; ++i) do_move('^', handle);
                do_move('<', handle);
                for (int i = 0; i < H - 1; ++i) do_move('v', handle);
            }
        } else {
            if (sx == k) {
                for (int i = 0; i < H - 1; ++i) do_move('v', handle);
                do_move('>', handle);
                for (int i = 0; i < H - 1; ++i) do_move('^', handle);
            } else {
                for (int i = 0; i < H - 1; ++i) do_move('v', handle);
                do_move('<', handle);
                for (int i = 0; i < H - 1; ++i) do_move('^', handle);
            }
        }

        bool start_right = (0 > k);
        bool end_right = start_right ^ (parity & 1);
        return end_right;
    };

    auto query_horizontal = [&](int k) -> bool {
        vector<unsigned char> markL(H, 0), markR(H, 0);
        int parity = 0;

        auto handle = [&](int x, int y, char resp) {
            if (y == k) {
                int idx = x + n;
                if (!markL[idx]) {
                    markL[idx] = 1;
                    if (resp == '^') parity ^= 1;
                }
            } else if (y == k + 1) {
                int idx = x + n;
                if (!markR[idx]) {
                    markR[idx] = 1;
                    if (resp == 'v') parity ^= 1;
                }
            }
        };

        handle(cx, cy, curResp);

        int sx = -n, sy = k;
        int best = abs(cx - sx) + abs(cy - sy);

        auto consider = [&](int x, int y) {
            int d = abs(cx - x) + abs(cy - y);
            if (d < best || (d == best && (x > sx || (x == sx && y > sy)))) {
                best = d;
                sx = x;
                sy = y;
            }
        };

        consider(-n, k);
        consider(n, k);
        consider(-n, k + 1);
        consider(n, k + 1);

        move_to(sx, sy, handle);

        if (sx == -n) {
            if (sy == k) {
                for (int i = 0; i < H - 1; ++i) do_move('>', handle);
                do_move('v', handle);
                for (int i = 0; i < H - 1; ++i) do_move('<', handle);
            } else {
                for (int i = 0; i < H - 1; ++i) do_move('>', handle);
                do_move('^', handle);
                for (int i = 0; i < H - 1; ++i) do_move('<', handle);
            }
        } else {
            if (sy == k) {
                for (int i = 0; i < H - 1; ++i) do_move('<', handle);
                do_move('v', handle);
                for (int i = 0; i < H - 1; ++i) do_move('>', handle);
            } else {
                for (int i = 0; i < H - 1; ++i) do_move('<', handle);
                do_move('^', handle);
                for (int i = 0; i < H - 1; ++i) do_move('>', handle);
            }
        }

        bool start_up = (0 > k);
        bool end_up = start_up ^ (parity & 1);
        return end_up;
    };

    int low = -n, high = n;
    while (low < high) {
        int mid = low + (high - low) / 2;
        bool p = query_vertical(mid);
        if (p) low = mid + 1;
        else high = mid;
    }
    int tx = low;

    low = -n;
    high = n;
    while (low < high) {
        int mid = low + (high - low) / 2;
        bool p = query_horizontal(mid);
        if (p) low = mid + 1;
        else high = mid;
    }
    int ty = low;

    auto dummy = [](int, int, char) {};
    move_to(tx, ty, dummy);

    return 0;
}
