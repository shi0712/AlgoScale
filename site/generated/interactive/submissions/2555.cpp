#include <bits/stdc++.h>
using namespace std;

const int INF = 1e9;

int getMove(int s, int last, int mod) {
    switch (s) {
        case 0: return 0; // R
        case 1: return 1; // P
        case 2: return 2; // S
        case 3: return mod; // R,P,S
        case 4: return (mod == 0 ? 0 : mod == 1 ? 2 : 1); // R,S,P
        case 5: return (last + 1) % 3; // counter
        case 6: return (mod == 0 ? 1 : mod == 1 ? 0 : 2); // P,R,S
        case 7: return (mod == 0 ? 1 : mod == 1 ? 2 : 0); // P,S,R
        case 8: return (mod == 0 ? 2 : mod == 1 ? 0 : 1); // S,R,P
        case 9: return (mod == 0 ? 2 : mod == 1 ? 1 : 0); // S,P,R
    }
    return 0;
}

void computeDP(int* strategies, int cost[64][11][3][3], int best[64][11][3][3]) {
    for (int mask = 0; mask < 64; ++mask)
        for (int streak = 0; streak < 11; ++streak)
            for (int last = 0; last < 3; ++last)
                for (int mod = 0; mod < 3; ++mod) {
                    cost[mask][streak][last][mod] = INF;
                    best[mask][streak][last][mod] = -1;
                }

    for (int mask = 0; mask < 64; ++mask)
        for (int last = 0; last < 3; ++last)
            for (int mod = 0; mod < 3; ++mod)
                cost[mask][10][last][mod] = 0;

    bool changed = true;
    int iter = 0;
    while (changed && iter < 1000) {
        changed = false;
        ++iter;
        for (int mask = 0; mask < 64; ++mask) {
            for (int streak = 0; streak < 10; ++streak) {
                for (int last = 0; last < 3; ++last) {
                    for (int mod = 0; mod < 3; ++mod) {
                        int bestVal = INF;
                        int bestM = -1;
                        for (int m = 0; m < 3; ++m) {
                            int maxc = 0;
                            bool possible = false;
                            for (int outcome = 0; outcome < 2; ++outcome) {
                                int newmask = 0;
                                for (int s = 0; s < 6; ++s) {
                                    if (mask & (1 << s)) {
                                        int global = strategies[s];
                                        int opp = getMove(global, last, mod);
                                        bool win = (m == (opp + 1) % 3);
                                        if (win == outcome)
                                            newmask |= (1 << s);
                                    }
                                }
                                if (newmask == 0) continue;
                                possible = true;
                                int newstreak = (outcome == 1 ? streak + 1 : 0);
                                int newlast = m;
                                int newmod = (mod + 1) % 3;
                                int c = cost[newmask][newstreak][newlast][newmod];
                                if (c > maxc) maxc = c;
                            }
                            if (possible) {
                                int val = 1 + maxc;
                                if (val < bestVal) {
                                    bestVal = val;
                                    bestM = m;
                                }
                            }
                        }
                        if (bestVal < cost[mask][streak][last][mod]) {
                            cost[mask][streak][last][mod] = bestVal;
                            best[mask][streak][last][mod] = bestM;
                            changed = true;
                        }
                    }
                }
            }
        }
    }
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(0);

    int A_strat[6] = {0, 1, 2, 3, 4, 5};
    int B_strat[6] = {3, 4, 6, 7, 8, 9};

    static int costA[64][11][3][3], bestA[64][11][3][3];
    static int costB[64][11][3][3], bestB[64][11][3][3];

    computeDP(A_strat, costA, bestA);
    computeDP(B_strat, costB, bestB);

    int maskA = 63, maskB = 63;
    int streak = 0, last = 2, mod = 0;
    bool setA = true;

    char moves[3] = {'R', 'P', 'S'};

    while (streak < 10) {
        int m;
        if (setA)
            m = bestA[maskA][streak][last][mod];
        else
            m = bestB[maskB][streak][last][mod];

        if (m == -1) m = 0;

        cout << moves[m] << endl;

        string resp;
        if (!(cin >> resp)) break;
        int outcome;
        if (resp == "player") outcome = 1;
        else if (resp == "ai") outcome = 0;
        else break;

        int newmaskA = 0;
        for (int s = 0; s < 6; ++s) {
            if (maskA & (1 << s)) {
                int global = A_strat[s];
                int opp = getMove(global, last, mod);
                bool win = (m == (opp + 1) % 3);
                if (win == outcome)
                    newmaskA |= (1 << s);
            }
        }
        maskA = newmaskA;

        int newmaskB = 0;
        for (int s = 0; s < 6; ++s) {
            if (maskB & (1 << s)) {
                int global = B_strat[s];
                int opp = getMove(global, last, mod);
                bool win = (m == (opp + 1) % 3);
                if (win == outcome)
                    newmaskB |= (1 << s);
            }
        }
        maskB = newmaskB;

        streak = (outcome == 1 ? streak + 1 : 0);
        last = m;
        mod = (mod + 1) % 3;

        if (setA && maskA == 0) {
            setA = false;
        }
    }

    return 0;
}
