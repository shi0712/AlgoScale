#include <bits/stdc++.h>
using namespace std;

static const int MAXN = 5005;

int N;
int eq0[MAXN], eq1[MAXN];
bool ineq0[MAXN], ineq1[MAXN];

bool used[MAXN];
int cAssign[MAXN];
int doorOf[MAXN];

vector<bitset<MAXN>> queryBits;
vector<vector<int>> removeAt;

int lastCBit = 0;

bool allKnown = false;
int cGlob[MAXN];

int readResp() {
    int x;
    if (!(cin >> x)) exit(0);
    return x;
}

void printAnswer() {
    cout << "! ";
    for (int i = 0; i < N; i++) {
        cout << char('0' + cAssign[i]);
    }
    for (int i = 0; i < N; i++) {
        cout << ' ' << doorOf[i];
    }
    cout << '\n' << flush;
    exit(0);
}

void addQuery(const bitset<MAXN>& bs, int r, int j) {
    if (r <= j) return;
    int idx = (int)queryBits.size();
    queryBits.push_back(bs);

    for (int i = 0; i < N; i++) {
        if (bs[i]) eq1[i]++;
        else eq0[i]++;
    }

    if (r < N) removeAt[r].push_back(idx);
}

void isolationFinishWithGlob() {
    for (int i = 0; i < N; i++) cAssign[i] = cGlob[i];

    vector<int> unassigned;
    vector<char> doorAssigned(N, 0);

    for (int i = 0; i < N; i++) {
        if (doorOf[i] != -1) doorAssigned[doorOf[i]] = 1;
        else unassigned.push_back(i);
    }

    int m = (int)unassigned.size();

    for (int idx = 0; idx + 1 < m; idx++) {
        int sw = unassigned[idx];
        string str(N, '0');

        for (int t = 0; t < N; t++) {
            int bit = cGlob[t] ^ (t == sw ? 1 : 0);
            str[t] = char('0' + bit);
        }

        cout << "? " << str << '\n' << flush;
        int resp = readResp();

        doorOf[sw] = resp;
        if (0 <= resp && resp < N) doorAssigned[resp] = 1;
    }

    if (m > 0) {
        int last = unassigned.back();
        int d = 0;
        while (d < N && doorAssigned[d]) d++;
        doorOf[last] = d;
    }

    printAnswer();
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    cin >> N;

    memset(eq0, 0, sizeof(eq0));
    memset(eq1, 0, sizeof(eq1));

    fill(used, used + N, false);
    fill(cAssign, cAssign + N, -1);
    fill(doorOf, doorOf + N, -1);

    removeAt.assign(N, vector<int>());

    for (int j = 0; j < N; j++) {
        fill(ineq0, ineq0 + N, false);
        fill(ineq1, ineq1 + N, false);

        for (int idx : removeAt[j]) {
            const auto& bs = queryBits[idx];

            for (int i = 0; i < N; i++) {
                if (bs[i]) {
                    eq1[i]--;
                    ineq1[i] = true;
                } else {
                    eq0[i]--;
                    ineq0[i] = true;
                }
            }
        }

        vector<pair<int, int>> S;
        S.reserve(2 * N);

        for (int i = 0; i < N; i++) {
            if (used[i]) continue;

            bool allow0 = true, allow1 = true;

            if (eq0[i] > 0) allow1 = false;
            if (eq1[i] > 0) allow0 = false;

            if (ineq0[i]) allow0 = false;
            if (ineq1[i]) allow1 = false;

            if (allKnown) {
                if (cGlob[i] == 0) allow1 = false;
                else allow0 = false;
            }

            if (allow0) S.push_back({i, 0});
            if (allow1) S.push_back({i, 1});
        }

        if (allKnown) {
            if (S.size() == 1) {
                int p = S[0].first;
                int c = cGlob[p];

                used[p] = true;
                cAssign[p] = c;
                doorOf[p] = j;
                lastCBit = c;

                continue;
            } else {
                isolationFinishWithGlob();
            }
        }

        bool known = true;
        int cbit = 0;

        if (!S.empty()) {
            cbit = S[0].second;
            for (auto& pr : S) {
                if (pr.second != cbit) {
                    known = false;
                    break;
                }
            }
        }

        if (!known) {
            int g = lastCBit;

            bitset<MAXN> bs;
            for (int i = 0; i < N; i++) {
                if (used[i]) bs[i] = cAssign[i];
                else bs[i] = g;
            }

            string str(N, '0');
            for (int i = 0; i < N; i++) {
                if (bs[i]) str[i] = '1';
            }

            cout << "? " << str << '\n' << flush;
            int resp = readResp();

            if (resp == -1) {
                allKnown = true;
                for (int i = 0; i < N; i++) cGlob[i] = bs[i];

                vector<pair<int, int>> newS;
                newS.reserve(S.size());

                for (auto& pr : S) {
                    if (pr.second == cGlob[pr.first]) {
                        newS.push_back(pr);
                    }
                }

                S.swap(newS);

                if (S.size() > 1) isolationFinishWithGlob();

                int p = S[0].first;
                int c = cGlob[p];

                used[p] = true;
                cAssign[p] = c;
                doorOf[p] = j;
                lastCBit = c;

                continue;
            }

            int r = resp;
            int actual = (r == j ? 1 - g : g);

            vector<pair<int, int>> newS;
            newS.reserve(S.size());

            for (auto& pr : S) {
                if (pr.second == actual) newS.push_back(pr);
            }

            S.swap(newS);
            cbit = actual;

            if (r > j) addQuery(bs, r, j);
        }

        bool assignedAllOpen = false;

        while (S.size() > 1) {
            int m = (int)S.size();
            int ksize = m / 2;

            vector<char> inK(N, 0);
            for (int idx = 0; idx < ksize; idx++) {
                inK[S[idx].first] = 1;
            }

            bitset<MAXN> bs;
            for (int i = 0; i < N; i++) {
                if (used[i]) bs[i] = cAssign[i];
                else if (inK[i]) bs[i] = 1 - cbit;
                else bs[i] = cbit;
            }

            string str(N, '0');
            for (int i = 0; i < N; i++) {
                if (bs[i]) str[i] = '1';
            }

            cout << "? " << str << '\n' << flush;
            int resp = readResp();

            if (resp == -1) {
                allKnown = true;
                for (int i = 0; i < N; i++) cGlob[i] = bs[i];

                vector<pair<int, int>> newS;
                newS.reserve(S.size());

                for (auto& pr : S) {
                    if (pr.second == cGlob[pr.first]) {
                        newS.push_back(pr);
                    }
                }

                S.swap(newS);

                if (S.size() > 1) isolationFinishWithGlob();

                int p = S[0].first;
                int c = cGlob[p];

                used[p] = true;
                cAssign[p] = c;
                doorOf[p] = j;
                lastCBit = c;

                assignedAllOpen = true;
                break;
            }

            int r = resp;

            if (r == j) {
                S.resize(ksize);
            } else {
                vector<pair<int, int>> newS;
                newS.reserve(m - ksize);

                for (int idx = ksize; idx < m; idx++) {
                    newS.push_back(S[idx]);
                }

                S.swap(newS);

                if (r > j) addQuery(bs, r, j);
            }
        }

        if (assignedAllOpen) continue;

        int p = S[0].first;
        int c = cbit;

        used[p] = true;
        cAssign[p] = c;
        doorOf[p] = j;
        lastCBit = c;
    }

    printAnswer();
    return 0;
}
