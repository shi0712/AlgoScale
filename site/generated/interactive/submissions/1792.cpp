#include <bits/stdc++.h>
using namespace std;

const int INF = 1000;

int ai_six(int s, int last) {
    if (last == -1) {
        if (s < 3) return s;
        else return 0;
    } else {
        if (s == 0) return 0;
        if (s == 1) return 1;
        if (s == 2) return 2;
        if (s == 3) return last;
        if (s == 4) return (last + 1) % 3;
        if (s == 5) return (last + 2) % 3;
    }
    return 0;
}

bool computed[64][4][11];
int dp[64][4][11][6];
int bestMove[64][4][11];
bool inStack[64][4][11];

bool better(const vector<int>& v1, const vector<int>& v2, int mask) {
    vector<int> a, b;
    for (int s = 0; s < 6; s++) if (mask & (1 << s)) {
        a.push_back(v1[s]);
        b.push_back(v2[s]);
    }
    sort(a.rbegin(), a.rend());
    sort(b.rbegin(), b.rend());
    if (a != b) return a < b;
    int sum1 = 0, sum2 = 0;
    for (int x : a) sum1 += x;
    for (int x : b) sum2 += x;
    return sum1 < sum2;
}

vector<int> dfs(int mask, int lastIdx, int streak) {
    if (streak >= 10) {
        vector<int> res(6, INF);
        for (int s = 0; s < 6; s++) if (mask & (1 << s)) res[s] = 0;
        return res;
    }
    if (computed[mask][lastIdx][streak]) {
        vector<int> res(6);
        for (int s = 0; s < 6; s++) res[s] = dp[mask][lastIdx][streak][s];
        return res;
    }
    if (inStack[mask][lastIdx][streak]) {
        return vector<int>(6, INF);
    }
    inStack[mask][lastIdx][streak] = true;

    vector<int> best_v(6, INF);
    int best_m = -1;

    for (int m = 0; m < 3; m++) {
        int last = (lastIdx == 3 ? -1 : lastIdx);
        int W = 0, L = 0;
        for (int s = 0; s < 6; s++) if (mask & (1 << s)) {
            int ai = ai_six(s, last);
            if (ai == (m + 2) % 3) W |= (1 << s);
            else L |= (1 << s);
        }

        vector<int> v(6, INF);
        for (int s = 0; s < 6; s++) if (mask & (1 << s)) {
            if (W & (1 << s)) {
                vector<int> sub = dfs(W, m, min(10, streak + 1));
                if (sub[s] >= INF) v[s] = INF;
                else v[s] = 1 + sub[s];
            } else {
                vector<int> sub = dfs(L, m, 0);
                if (sub[s] >= INF) v[s] = INF;
                else v[s] = 1 + sub[s];
            }
        }

        if (best_m == -1 || better(v, best_v, mask)) {
            best_v = v;
            best_m = m;
        }
    }

    inStack[mask][lastIdx][streak] = false;
    computed[mask][lastIdx][streak] = true;
    for (int s = 0; s < 6; s++) dp[mask][lastIdx][streak][s] = best_v[s];
    bestMove[mask][lastIdx][streak] = best_m;
    return best_v;
}

struct Candidate {
    int type; // 0 const, 1 cycle, 2 prevfunc
    int c;
    int perm[3];
    int f[3];
};

vector<Candidate> L;

int ai_fallback(const Candidate& cand, int t, int lastMove) {
    if (cand.type == 0) return cand.c;
    if (cand.type == 1) return cand.perm[t % 3];
    if (t == 0) return cand.c;
    else return cand.f[lastMove];
}

int roundCnt = 0;
int lastMove = -1;
int streak = 0;
bool useFallback = false;
int mask = 63;
int lastIdx = 3;
vector<int> pastMoves;
vector<bool> pastWins;
vector<int> candidates;

void initFallback() {
    candidates.clear();
    for (int i = 0; i < (int)L.size(); i++) candidates.push_back(i);
    for (int i = 0; i < (int)pastMoves.size(); i++) {
        int m = pastMoves[i];
        bool win = pastWins[i];
        vector<int> newCand;
        for (int idx : candidates) {
            int ai = ai_fallback(L[idx], i, (i == 0 ? -1 : pastMoves[i - 1]));
            bool predWin = (ai == (m + 2) % 3);
            if (predWin == win) newCand.push_back(idx);
        }
        candidates = newCand;
    }
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    // Build fallback library
    for (int c = 0; c < 3; c++) {
        Candidate cand; cand.type = 0; cand.c = c;
        L.push_back(cand);
    }

    vector<array<int, 3>> perms = {
        {0, 1, 2}, {0, 2, 1}, {1, 0, 2},
        {1, 2, 0}, {2, 0, 1}, {2, 1, 0}
    };
    for (auto &p : perms) {
        Candidate cand; cand.type = 1;
        for (int i = 0; i < 3; i++) cand.perm[i] = p[i];
        L.push_back(cand);
    }

    for (int c0 = 0; c0 < 3; c0++) {
        for (int f0 = 0; f0 < 3; f0++)
        for (int f1 = 0; f1 < 3; f1++)
        for (int f2 = 0; f2 < 3; f2++) {
            Candidate cand; cand.type = 2; cand.c = c0;
            cand.f[0] = f0; cand.f[1] = f1; cand.f[2] = f2;
            L.push_back(cand);
        }
    }

    dfs(63, 3, 0);

    while (true) {
        int m;
        if (!useFallback) {
            m = bestMove[mask][lastIdx][streak];
        } else {
            if (candidates.size() == 1) {
                int ai = ai_fallback(L[candidates[0]], roundCnt, lastMove);
                m = (ai + 1) % 3;
            } else {
                int bestMax = INF;
                int best_m = 0;
                for (int mm = 0; mm < 3; mm++) {
                    int W = 0, Lc = 0;
                    for (int idx : candidates) {
                        int ai = ai_fallback(L[idx], roundCnt, lastMove);
                        if (ai == (mm + 2) % 3) W++;
                        else Lc++;
                    }
                    int mx = max(W, Lc);
                    if (mx < bestMax) {
                        bestMax = mx;
                        best_m = mm;
                    }
                }
                m = best_m;
            }
        }

        cout << "RPS"[m] << endl;

        string resp;
        if (!(cin >> resp)) return 0;
        if (resp == "-1") return 0;

        bool win = (resp == "player");
        if (win) streak++;
        else streak = 0;

        if (!useFallback) {
            int newMask = 0;
            for (int s = 0; s < 6; s++) if (mask & (1 << s)) {
                int ai = ai_six(s, lastIdx == 3 ? -1 : lastIdx);
                bool predWin = (ai == (m + 2) % 3);
                if (predWin == win) newMask |= (1 << s);
            }
            if (newMask == 0) {
                useFallback = true;
                pastMoves.push_back(m);
                pastWins.push_back(win);
                initFallback();
            } else {
                mask = newMask;
                lastIdx = m;
                pastMoves.push_back(m);
                pastWins.push_back(win);
            }
        } else {
            if (candidates.size() > 1) {
                vector<int> newCand;
                for (int idx : candidates) {
                    int ai = ai_fallback(L[idx], roundCnt, lastMove);
                    bool predWin = (ai == (m + 2) % 3);
                    if (predWin == win) newCand.push_back(idx);
                }
                candidates = newCand;
            }
            pastMoves.push_back(m);
            pastWins.push_back(win);
        }

        lastMove = m;
        roundCnt++;

        if (streak >= 10) return 0;
    }

    return 0;
}
