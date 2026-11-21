#include <bits/stdc++.h>
using namespace std;

int n, visible_cells;
int parity[11][11];

bool flip(int l, int r) {
    cout << l << ' ' << r << '\n';
    cout.flush();

    int x;
    if (!(cin >> x)) return false;
    if (x < 0 || x > n) return false;

    visible_cells = x;
    parity[l][r] ^= 1;

    if (visible_cells == n) exit(0);
    return true;
}

bool exhaustive_stage(int i) {
    int k = n - i + 1;
    int total = 1 << k;

    vector<int> cur(k, 0);
    for (int p = 0; p < k; ++p) {
        cur[p] = parity[i][i + p];
    }

    for (int r = 0; r < total; ++r) {
        int target = r ^ (r >> 1);

        for (int p = 0; p < k; ++p) {
            int bit = (target >> p) & 1;
            if (cur[p] != bit) {
                if (!flip(i, i + p)) return false;
                cur[p] ^= 1;
            }
        }

        int old_visible = visible_cells;
        if (!flip(i, i)) return false;
        int delta = visible_cells - old_visible;
        if (!flip(i, i)) return false;

        if (delta == -1) return true;
    }

    return false;
}

bool solve_stage(int i) {
    int k = n - i + 1;
    int pairs = 1 << (k - 1);

    vector<int> gray(pairs);
    for (int r = 0; r < pairs; ++r) {
        gray[r] = r ^ (r >> 1);
    }

    for (int r = 0; r < pairs; ++r) {
        int old_visible = visible_cells;

        if (!flip(i, i)) return false;
        int delta = visible_cells - old_visible;

        if (delta == 1) {
            return true;
        } else if (delta == -1) {
            if (!flip(i, i)) return false;
            return true;
        } else if (delta == 0) {
            if (r == pairs - 1) {
                return exhaustive_stage(i);
            }

            int diff = gray[r] ^ gray[r + 1];
            int bit = __builtin_ctz((unsigned)diff);
            int pos = k - 1 - bit;

            if (!flip(i, i + pos)) return false;
        } else {
            return exhaustive_stage(i);
        }
    }

    return exhaustive_stage(i);
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    if (!(cin >> n)) return 0;
    if (!(cin >> visible_cells)) return 0;

    if (visible_cells == n) return 0;

    for (int i = 1; i <= n; ++i) {
        if (!solve_stage(i)) return 0;
        if (visible_cells == n) return 0;
    }

    return 0;
}
