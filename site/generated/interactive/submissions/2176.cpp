#include <bits/stdc++.h>
using namespace std;

int ask_plus(int id) {
    cout << '+' << id << '\n';
    cout.flush();
    int x;
    if (!(cin >> x)) exit(0);
    if (x == -1) exit(0);
    return x;
}

int ask_minus() {
    cout << "-\n";
    cout.flush();
    int x;
    if (!(cin >> x)) exit(0);
    if (x == -1) exit(0);
    return x;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int N;
    if (!(cin >> N)) return 0;

    if (N == 1) {
        cout << "! 1\n1 1\n";
        cout.flush();
        return 0;
    }

    vector<int> cls(N + 1, -1);
    vector<int> rep;
    vector<int> S, M;

    int K = 0;
    int cur = 0;

    int firstcnt = ask_plus(1);
    if (firstcnt != 1) exit(0);

    K = 1;
    cls[1] = 0;
    rep.push_back(1);
    cur = 1;
    S.push_back(0);

    bool single_mode = true;
    int single_count = 1;
    int last_single_id = 1;

    for (int x = 2; x <= N; ++x) {
        bool last = (x == N);

        if (single_mode) {
            int nc = ask_plus(x);

            if (nc == 1) {
                cls[x] = 0;
                ++single_count;
                last_single_id = x;
                cur = 1;
            } else if (nc == 2) {
                if (!last) {
                    for (int i = 0; i < single_count - 1; ++i) {
                        ask_minus();
                    }

                    int c1 = K++;
                    cls[x] = c1;
                    rep.push_back(x);
                    rep[0] = last_single_id;

                    S.clear();
                    S.push_back(0);
                    S.push_back(c1);
                    M.clear();

                    cur = 2;
                    single_mode = false;
                } else {
                    int c1 = K++;
                    cls[x] = c1;
                    rep.push_back(x);
                    single_mode = false;
                }
            } else {
                exit(0);
            }
            continue;
        }

        int newcnt = ask_plus(x);

        if (newcnt == cur + 1) {
            int t = -1;
            int prev = newcnt;

            for (int i = 0; i < (int)M.size(); ++i) {
                int d = ask_plus(rep[M[i]]);
                if (d == prev) {
                    t = i;
                    break;
                }
                prev = d;
            }

            if (t == -1) {
                int c = K++;
                cls[x] = c;
                rep.push_back(x);

                if (!last) {
                    S.push_back(c);
                    for (int c2 : M) S.push_back(c2);
                    M.clear();
                    cur = (int)S.size();
                }
            } else {
                int c = M[t];
                cls[x] = c;

                if (!last) {
                    int s = (int)S.size();

                    for (int i = 0; i < s; ++i) ask_minus();
                    ask_minus();

                    vector<int> newS, newM;
                    newS.reserve(t + 1);
                    newM.reserve(K);

                    for (int i = 0; i <= t; ++i) newS.push_back(M[i]);
                    for (int i = s - 1; i >= 0; --i) newM.push_back(S[i]);
                    for (int i = t + 1; i < (int)M.size(); ++i) newM.push_back(M[i]);

                    S.swap(newS);
                    M.swap(newM);
                    cur = (int)S.size();
                }
            }
        } else if (newcnt == cur) {
            int s = (int)S.size();
            int p = -1;
            int prev = newcnt;

            for (int i = 0; i < s; ++i) {
                if (last && i == s - 1) {
                    p = i;
                    break;
                }

                int d = ask_minus();
                if (d == prev) {
                    p = i;
                    break;
                }
                prev = d;
            }

            if (p == -1) exit(0);

            int c = S[p];
            cls[x] = c;

            if (!last) {
                rep[c] = x;

                vector<int> newS, newM;
                newS.reserve(s - p);
                newM.reserve(K);

                for (int i = p + 1; i < s; ++i) newS.push_back(S[i]);
                newS.push_back(c);

                for (int i = p - 1; i >= 0; --i) newM.push_back(S[i]);
                for (int c2 : M) newM.push_back(c2);

                S.swap(newS);
                M.swap(newM);
                cur = (int)S.size();
            }
        } else {
            exit(0);
        }
    }

    vector<vector<int>> groups(K);
    for (int i = 1; i <= N; ++i) {
        if (cls[i] < 0) cls[i] = 0;
        groups[cls[i]].push_back(i);
    }

    cout << "! " << K << "\n";
    cout.flush();

    for (int c = 0; c < K; ++c) {
        cout << groups[c].size();
        for (int id : groups[c]) cout << ' ' << id;
        cout << "\n";
        cout.flush();
    }

    return 0;
}
