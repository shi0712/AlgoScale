#include <bits/stdc++.h>
using namespace std;

int N;
int F[5][5][5];

int area16(int a, int b, int c) {
    if (a + b <= c || a + c <= b || b + c <= a) return 0;
    return (a + b + c) * (-a + b + c) * (a - b + c) * (a + b - c);
}

int ask(int i, int j, int k) {
    int v[3] = {i, j, k};
    sort(v, v + 3);
    cout << "? " << v[0] + 1 << ' ' << v[1] + 1 << ' ' << v[2] + 1 << '\n' << flush;
    int r;
    if (!(cin >> r)) exit(0);
    if (r < 0) exit(0);
    return r;
}

void print_answer(const vector<int>& a) {
    cout << "!";
    for (int x : a) cout << ' ' << x;
    cout << '\n' << flush;
}

void print_minus() {
    cout << "! -1\n" << flush;
}

int eqValFromOne(int r) {
    if (r == 15) return 2;
    if (r == 35) return 3;
    if (r == 63) return 4;
    return 0;
}

void solve_small(int n) {
    static int resp[8][8][8];
    for (int i = 0; i < n; i++)
        for (int j = i + 1; j < n; j++)
            for (int k = j + 1; k < n; k++)
                resp[i][j][k] = ask(i, j, k);

    int total = 1 << (2 * n);
    vector<int> first;
    int cnt = 0;

    for (int mask = 0; mask < total; mask++) {
        int tmp = mask;
        vector<int> a(n);
        for (int i = 0; i < n; i++) {
            a[i] = tmp % 4 + 1;
            tmp /= 4;
        }

        bool ok = true;
        for (int i = 0; i < n && ok; i++)
            for (int j = i + 1; j < n && ok; j++)
                for (int k = j + 1; k < n; k++)
                    if (F[a[i]][a[j]][a[k]] != resp[i][j][k]) {
                        ok = false;
                        break;
                    }

        if (ok) {
            cnt++;
            if (cnt == 1) first = a;
            if (cnt >= 2) break;
        }
    }

    if (cnt == 1) print_answer(first);
    else print_minus();
}

void solve_ones(int n, int p, int q) {
    vector<int> val(n, 1);
    vector<int> L;

    for (int i = 0; i < n; i++) {
        if (i == p || i == q) continue;
        int r = ask(p, q, i);
        if (r != 3) {
            L.push_back(i);
        }
    }

    val[p] = val[q] = 1;

    if (L.empty()) {
        print_answer(val);
        return;
    }

    if ((int)L.size() == 1) {
        print_minus();
        return;
    }

    if ((int)L.size() == 2) {
        int a = L[0], b = L[1];
        int r = ask(p, a, b);

        if (r == 0) {
            print_minus();
            return;
        }

        int v = eqValFromOne(r);
        val[a] = val[b] = v;
        print_answer(val);
        return;
    }

    if ((int)L.size() == 3) {
        int a = L[0], b = L[1], c = L[2];

        int rab = ask(p, a, b);
        int rac = ask(p, a, c);
        int rbc = ask(p, b, c);

        int vab = eqValFromOne(rab);
        int vac = eqValFromOne(rac);
        int vbc = eqValFromOne(rbc);

        int pos = (vab > 0) + (vac > 0) + (vbc > 0);

        if (pos == 3) {
            int v = vab;
            val[a] = val[b] = val[c] = v;
            print_answer(val);
            return;
        }

        if (pos == 0) {
            print_minus();
            return;
        }

        int same1, same2, single, v;
        if (vab > 0) {
            same1 = a;
            same2 = b;
            single = c;
            v = vab;
        } else if (vac > 0) {
            same1 = a;
            same2 = c;
            single = b;
            v = vac;
        } else {
            same1 = b;
            same2 = c;
            single = a;
            v = vbc;
        }

        val[same1] = val[same2] = v;

        int r = ask(a, b, c);
        int w = 0;

        for (int x = 2; x <= 4; x++) {
            if (x == v) continue;
            int tmp[3] = {v, v, x};
            if (F[tmp[0]][tmp[1]][tmp[2]] == r) {
                w = x;
                break;
            }
        }

        val[single] = w;
        print_answer(val);
        return;
    }

    // |L| >= 4
    int idx[4] = {L[0], L[1], L[2], L[3]};
    int A = -1, B = -1, v = 0;

    for (int i = 0; i < 4 && A == -1; i++) {
        for (int j = i + 1; j < 4; j++) {
            int r = ask(p, idx[i], idx[j]);
            int z = eqValFromOne(r);
            if (z > 0) {
                A = idx[i];
                B = idx[j];
                v = z;
                break;
            }
        }
    }

    if (A == -1) {
        print_minus();
        return;
    }

    vector<int> ans(n, 0);
    ans[A] = ans[B] = v;

    for (int i = 0; i < n; i++) {
        if (i == A || i == B) continue;

        int r = ask(A, B, i);
        int got = -1;

        for (int x = 1; x <= 4; x++) {
            if (F[v][v][x] == r) {
                got = x;
                break;
            }
        }

        ans[i] = got;
    }

    print_answer(ans);
}

bool find_witness(const vector<int>& A, const vector<int>& B, int& ri, int& rj, int& rk) {
    vector<int> ex[16];

    for (int i = 0; i < N; i++) {
        int ta = A[i];
        int tb = B[i];
        int type = (ta - 1) * 4 + (tb - 1);
        if ((int)ex[type].size() < 3) ex[type].push_back(i);
    }

    for (int t1 = 0; t1 < 16; t1++) {
        for (int t2 = t1; t2 < 16; t2++) {
            for (int t3 = t2; t3 < 16; t3++) {
                int need[16] = {};
                need[t1]++;
                need[t2]++;
                need[t3]++;

                bool ok = true;
                for (int t = 0; t < 16; t++) {
                    if ((int)ex[t].size() < need[t]) {
                        ok = false;
                        break;
                    }
                }
                if (!ok) continue;

                vector<int> ids;
                for (int t = 0; t < 16; t++) {
                    for (int c = 0; c < need[t]; c++) {
                        ids.push_back(ex[t][c]);
                    }
                }

                int ra = F[A[ids[0]]][A[ids[1]]][A[ids[2]]];
                int rb = F[B[ids[0]]][B[ids[1]]][B[ids[2]]];

                if (ra != rb) {
                    sort(ids.begin(), ids.end());
                    ri = ids[0];
                    rj = ids[1];
                    rk = ids[2];
                    return true;
                }
            }
        }
    }

    return false;
}

void finalize(vector<vector<int>> fulls) {
    vector<vector<int>> uniq;

    for (auto& v : fulls) {
        bool found = false;
        for (auto& u : uniq) {
            if (u == v) {
                found = true;
                break;
            }
        }
        if (!found) uniq.push_back(v);
    }

    while ((int)uniq.size() > 1) {
        bool has = false;
        int qi = -1, qj = -1, qk = -1;

        for (int i = 0; i < (int)uniq.size() && !has; i++) {
            for (int j = i + 1; j < (int)uniq.size(); j++) {
                if (find_witness(uniq[i], uniq[j], qi, qj, qk)) {
                    has = true;
                    break;
                }
            }
        }

        if (!has) {
            print_minus();
            return;
        }

        int r = ask(qi, qj, qk);

        vector<vector<int>> nxt;
        for (auto& v : uniq) {
            if (F[v[qi]][v[qj]][v[qk]] == r) {
                nxt.push_back(v);
            }
        }

        uniq.swap(nxt);

        if (uniq.empty()) {
            print_minus();
            return;
        }
    }

    if ((int)uniq.size() == 1) print_answer(uniq[0]);
    else print_minus();
}

void solve_shared(int n, int P, int p, int q, const vector<array<int, 8>>& cand) {
    vector<int> rvals(n - P);

    for (int i = P; i < n; i++) {
        rvals[i - P] = ask(p, q, i);
    }

    vector<vector<int>> fulls;

    for (auto& c : cand) {
        int a = c[p];
        int b = c[q];

        vector<int> full(n);
        for (int i = 0; i < P; i++) full[i] = c[i];

        bool ok = true;

        for (int i = P; i < n; i++) {
            int r = rvals[i - P];
            int got = 0;

            for (int x = 1; x <= 4; x++) {
                if (F[a][b][x] == r) {
                    got = x;
                    break;
                }
            }

            if (got == 0) {
                ok = false;
                break;
            }

            full[i] = got;
        }

        if (ok) fulls.push_back(full);
    }

    finalize(fulls);
}

void solve_per_candidate(int n, int P, const vector<array<int, 8>>& cand) {
    vector<vector<int>> tmpFulls;
    vector<tuple<int, int, int, int>> queries;

    for (auto& c : cand) {
        int p = -1, q = -1;
        int a = 0, b = 0;

        for (int i = 0; i < P && p == -1; i++) {
            if (c[i] < 2) continue;

            for (int j = i + 1; j < P; j++) {
                if (c[j] < 2) continue;

                int x = c[i], y = c[j];
                if ((x == 2 && y == 4) || (x == 4 && y == 2)) continue;

                p = i;
                q = j;
                a = x;
                b = y;
                break;
            }
        }

        if (p == -1) {
            // This should not happen in the no-shared-ones branch,
            // because then every candidate has at least 6 non-1 values.
            continue;
        }

        vector<int> full(n);
        for (int i = 0; i < P; i++) full[i] = c[i];

        bool ok = true;

        for (int i = P; i < n; i++) {
            int r = ask(p, q, i);
            queries.emplace_back(p, q, i, r);

            int got = 0;

            for (int x = 1; x <= 4; x++) {
                if (F[a][b][x] == r) {
                    got = x;
                    break;
                }
            }

            if (got == 0) {
                ok = false;
                break;
            }

            full[i] = got;
        }

        if (ok) tmpFulls.push_back(full);
    }

    vector<vector<int>> fulls;

    for (auto& v : tmpFulls) {
        bool ok = true;

        for (auto [x, y, z, r] : queries) {
            if (F[v[x]][v[y]][v[z]] != r) {
                ok = false;
                break;
            }
        }

        if (ok) fulls.push_back(v);
    }

    finalize(fulls);
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    for (int a = 1; a <= 4; a++)
        for (int b = 1; b <= 4; b++)
            for (int c = 1; c <= 4; c++)
                F[a][b][c] = area16(a, b, c);

    cin >> N;

    if (N <= 8) {
        solve_small(N);
        return 0;
    }

    int P = 8;
    static int resp[8][8][8];

    for (int i = 0; i < P; i++)
        for (int j = i + 1; j < P; j++)
            for (int k = j + 1; k < P; k++)
                resp[i][j][k] = ask(i, j, k);

    vector<array<int, 8>> cand;

    int total = 1 << (2 * P);

    for (int mask = 0; mask < total; mask++) {
        int tmp = mask;
        array<int, 8> a;

        for (int i = 0; i < P; i++) {
            a[i] = tmp % 4 + 1;
            tmp /= 4;
        }

        bool ok = true;

        for (int i = 0; i < P && ok; i++) {
            for (int j = i + 1; j < P && ok; j++) {
                for (int k = j + 1; k < P; k++) {
                    if (F[a[i]][a[j]][a[k]] != resp[i][j][k]) {
                        ok = false;
                        break;
                    }
                }
            }
        }

        if (ok) cand.push_back(a);
    }

    int p1 = -1, q1 = -1;

    for (int i = 0; i < P && p1 == -1; i++) {
        for (int j = i + 1; j < P; j++) {
            bool ok = true;

            for (auto& c : cand) {
                if (c[i] != 1 || c[j] != 1) {
                    ok = false;
                    break;
                }
            }

            if (ok) {
                p1 = i;
                q1 = j;
                break;
            }
        }
    }

    if (p1 != -1) {
        solve_ones(N, p1, q1);
        return 0;
    }

    int sp = -1, sq = -1;

    for (int i = 0; i < P && sp == -1; i++) {
        for (int j = i + 1; j < P; j++) {
            bool ok = true;

            for (auto& c : cand) {
                int a = c[i], b = c[j];

                if (a < 2 || b < 2) {
                    ok = false;
                    break;
                }

                if ((a == 2 && b == 4) || (a == 4 && b == 2)) {
                    ok = false;
                    break;
                }
            }

            if (ok) {
                sp = i;
                sq = j;
                break;
            }
        }
    }

    if (sp != -1) {
        solve_shared(N, P, sp, sq, cand);
    } else {
        solve_per_candidate(N, P, cand);
    }

    return 0;
}
