#include <bits/stdc++.h>
using namespace std;

struct H {
    int a1, b1, a2, b2, c, i0;
    double w;
    bool alive;
};

int main() {
    vector<H> hs;
    for (int a1 = 0; a1 < 3; a1++)
        for (int b1 = 0; b1 < 3; b1++)
            for (int a2 = 0; a2 < 3; a2++)
                for (int b2 = 0; b2 < 3; b2++)
                    for (int c = 0; c < 3; c++)
                        for (int i0 = 0; i0 < 3; i0++) {
                            int nz = (a1 != 0) + (b1 != 0) + (a2 != 0) + (b2 != 0);
                            H h;
                            h.a1 = a1; h.b1 = b1; h.a2 = a2; h.b2 = b2; h.c = c; h.i0 = i0;
                            h.w = pow(0.25, nz);
                            h.alive = true;
                            hs.push_back(h);
                        }
    int n = hs.size();
    vector<vector<int>> ai(n);
    vector<int> me;
    const char L[3] = {'R', 'P', 'S'};
    int streak = 0;
    int t = 0;
    while (true) {
        // predict
        vector<int> pred(n, -1);
        double score[3] = {0, 0, 0};
        bool any = false;
        for (int k = 0; k < n; k++) {
            if (!hs[k].alive) continue;
            any = true;
            int p;
            if (t == 0) p = hs[k].i0;
            else {
                int x1 = ai[k][t - 1];
                int y1 = me[t - 1];
                int x2 = t >= 2 ? ai[k][t - 2] : 0;
                int y2 = t >= 2 ? me[t - 2] : 0;
                p = (hs[k].a1 * x1 + hs[k].b1 * y1 + hs[k].a2 * x2 + hs[k].b2 * y2 + hs[k].c) % 3;
            }
            pred[k] = p;
            score[(p + 1) % 3] += hs[k].w;
        }
        int mv = t % 3;
        if (any) {
            double best = -1;
            for (int m = 0; m < 3; m++) {
                if (score[m] > best + 1e-12) {
                    best = score[m];
                    mv = m;
                }
            }
        }
        printf("%c\n", L[mv]);
        fflush(stdout);
        string s;
        if (!(cin >> s)) return 0;
        bool win = (s == "player");
        if (s != "player" && s != "ai") return 0;
        for (int k = 0; k < n; k++) {
            if (!hs[k].alive) {
                ai[k].push_back(0);
                continue;
            }
            int p = pred[k];
            bool w = (mv == (p + 1) % 3);
            if (w != win) hs[k].alive = false;
            ai[k].push_back(p);
        }
        me.push_back(mv);
        t++;
        if (win) streak++;
        else streak = 0;
        if (streak >= 10) return 0;
    }
}
