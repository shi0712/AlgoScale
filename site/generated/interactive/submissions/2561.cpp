#include <bits/stdc++.h>
using namespace std;

struct Cand {
    int id;
    uint8_t mask;
};

static inline uint8_t trans(uint8_t mask, int h) {
    uint8_t nm = 0;
    for (int s = 0; s < 4; ++s) {
        if (!(mask & (1 << s))) continue;
        int a = (s >> 1) & 1;
        int b = s & 1;
        if (a == b && b == h) continue;
        int ns = ((b << 1) | h);
        nm |= (1 << ns);
    }
    return nm;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n)) return 0;

    vector<Cand> cands;
    cands.reserve(n);
    for (int i = 1; i <= n; ++i) cands.push_back({i, 0b1111});

    while ((int)cands.size() > 2) {
        int m = (int)cands.size();

        int best_max = m + 1;
        int best_l = 1, best_r = n;

        vector<int> diff0(n + 2, 0), diff1(n + 2, 0);
        int total0_0 = 0, total0_1 = 0;

        for (auto &c : cands) {
            uint8_t mask = c.mask;
            int s0_0 = trans(mask, 0) != 0;
            int s1_0 = trans(mask, 1) != 0;
            int s0_1 = trans(mask, 1) != 0;
            int s1_1 = trans(mask, 0) != 0;

            total0_0 += s0_0;
            total0_1 += s0_1;
            diff0[c.id] += s1_0 - s0_0;
            diff1[c.id] += s1_1 - s0_1;
        }

        int cur0 = total0_0, cur1 = total0_1;
        for (int mid = 1; mid <= n; ++mid) {
            cur0 += diff0[mid];
            cur1 += diff1[mid];
            int worst = max(cur0, cur1);
            if (worst < best_max) {
                best_max = worst;
                best_l = 1;
                best_r = mid;
            }
        }

        fill(diff0.begin(), diff0.end(), 0);
        fill(diff1.begin(), diff1.end(), 0);
        for (auto &c : cands) {
            uint8_t mask = c.mask;
            int s0_0 = trans(mask, 0) != 0;
            int s1_0 = trans(mask, 1) != 0;
            int s0_1 = trans(mask, 1) != 0;
            int s1_1 = trans(mask, 0) != 0;

            diff0[c.id] += s1_0 - s0_0;
            diff1[c.id] += s1_1 - s0_1;
        }

        cur0 = total0_0;
        cur1 = total0_1;
        for (int mid = n; mid >= 1; --mid) {
            cur0 += diff0[mid];
            cur1 += diff1[mid];
            int worst = max(cur0, cur1);
            if (worst < best_max) {
                best_max = worst;
                best_l = mid;
                best_r = n;
            }
        }

        int l = best_l, r = best_r;
        cout << "? " << l << " " << r << "\n" << flush;

        int c;
        if (!(cin >> c) || c == -1) return 0;

        int L = r - l + 1;
        int b = c - L + 1;

        vector<Cand> newc;
        newc.reserve(cands.size());
        for (auto &cand : cands) {
            int x = (l <= cand.id && cand.id <= r) ? 1 : 0;
            int h = b ^ x;
            uint8_t nm = trans(cand.mask, h);
            if (nm) newc.push_back({cand.id, nm});
        }
        cands = move(newc);
    }

    if (cands.empty()) {
        cout << "! 1\n" << flush;
        int resp;
        if (!(cin >> resp) || resp == -1) return 0;
        cout << "#\n" << flush;
        return 0;
    }

    int a1 = cands[0].id;
    cout << "! " << a1 << "\n" << flush;
    int resp;
    if (!(cin >> resp) || resp == -1) return 0;

    if (resp == 1) {
        cout << "#\n" << flush;
        return 0;
    }

    if (cands.size() >= 2) {
        int a2 = cands[1].id;
        cout << "! " << a2 << "\n" << flush;
        if (!(cin >> resp) || resp == -1) return 0;
    }

    cout << "#\n" << flush;
    return 0;
}
