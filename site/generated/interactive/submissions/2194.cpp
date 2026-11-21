#include <bits/stdc++.h>
using namespace std;

int n;
long long totalSum;
vector<int> valAtPos, posOfVal;
vector<char> knownVal;
vector<int> knownPos;
vector<char> mark;

int MAX_M;
vector<vector<vector<int>>> tests;
vector<int> chosenModuli;

int query_all_except(const vector<int>& S, int i) {
    vector<int> touched;
    for (int p : S) {
        if (!mark[p]) {
            mark[p] = 1;
            touched.push_back(p);
        }
    }
    if (!mark[i]) {
        mark[i] = 1;
        touched.push_back(i);
    }

    int k = 0;
    for (int j = 1; j <= n; ++j) {
        if (!mark[j]) ++k;
    }

    if (k <= 0) {
        for (int p : touched) mark[p] = 0;
        return -1;
    }
    if (k == 1) {
        for (int p : touched) mark[p] = 0;
        return 1;
    }

    cout << "? " << k;
    for (int j = 1; j <= n; ++j) {
        if (!mark[j]) cout << ' ' << j;
    }
    cout << '\n';
    cout.flush();

    int resp;
    if (!(cin >> resp)) exit(0);
    if (resp == -1) exit(0);

    for (int p : touched) mark[p] = 0;
    return resp;
}

int query_small(const vector<int>& sub, int i) {
    int k = (int)sub.size() + 1;
    if (k == 1) return 1;

    cout << "? " << k;
    for (int p : sub) cout << ' ' << p;
    cout << ' ' << i << '\n';
    cout.flush();

    int resp;
    if (!(cin >> resp)) exit(0);
    if (resp == -1) exit(0);
    return resp;
}

void add_known(int pos, int val) {
    if (valAtPos[pos] != -1) return;
    valAtPos[pos] = val;
    posOfVal[val] = pos;
    knownVal[val] = 1;
    knownPos.push_back(pos);
}

vector<int> get_candidates(int s, long long sumS) {
    int m = n - s - 1;
    if (m <= 1) return {};

    long long R = (totalSum - sumS) % m;
    if (R < 0) R += m;

    vector<int> c;
    if (m * 2 > n) {
        if (R == 0) {
            if (m <= n) c.push_back(m);
            if (2 * m <= n) c.push_back(2 * m);
        } else {
            if (R <= n) c.push_back((int)R);
            if (R + m <= n) c.push_back((int)(R + m));
        }
    } else {
        for (int v = 1; v <= n; ++v) {
            if (!knownVal[v] && v % m == R) c.push_back(v);
        }
    }

    vector<int> f;
    for (int v : c) {
        if (!knownVal[v]) f.push_back(v);
    }
    return f;
}

bool find_subset_dp(int m, int target, vector<int>& out) {
    int need = m - 1;
    int L = (int)knownPos.size();
    if (L < need) return false;

    vector<vector<char>> dp(need + 1, vector<char>(m, 0));
    vector<vector<int>> par(need + 1, vector<int>(m, -1));
    vector<vector<int>> prev(need + 1, vector<int>(m, 0));

    dp[0][0] = 1;

    for (int i = 0; i < L; ++i) {
        int val = valAtPos[knownPos[i]] % m;
        int maxc = min(i + 1, need);
        for (int c = maxc; c >= 1; --c) {
            for (int mod = 0; mod < m; ++mod) {
                if (!dp[c][mod]) {
                    int pm = (mod - val + m) % m;
                    if (dp[c - 1][pm]) {
                        dp[c][mod] = 1;
                        par[c][mod] = i;
                        prev[c][mod] = pm;
                    }
                }
            }
        }
    }

    if (!dp[need][target]) return false;

    out.clear();
    int c = need;
    int mod = target;
    while (c > 0) {
        int idx = par[c][mod];
        if (idx < 0) return false;
        out.push_back(knownPos[idx]);
        mod = prev[c][mod];
        --c;
    }
    return true;
}

bool build_tests() {
    tests.assign(MAX_M + 1, vector<vector<int>>(MAX_M + 1));
    vector<char> complete(MAX_M + 1, 0);

    for (int m = 2; m <= MAX_M; ++m) {
        bool ok = true;
        for (int r = 0; r < m; ++r) {
            int target = (m - r) % m;
            vector<int> sub;
            if (!find_subset_dp(m, target, sub)) {
                ok = false;
                break;
            }
            tests[m][r] = sub;
        }
        complete[m] = ok;
    }

    int bestMask = 0;
    int bestCost = INT_MAX;
    int masks = 1 << MAX_M;

    for (int mask = 1; mask < masks; ++mask) {
        int l = 1;
        int cost = 0;
        bool ok = true;

        for (int m = 2; m <= MAX_M; ++m) {
            if (mask & (1 << (m - 2))) {
                if (!complete[m]) {
                    ok = false;
                    break;
                }
                cost += m;
                int g = gcd(l, m);
                l = l / g * m;
                if (l > n) l = n + 1;
            }
        }

        if (ok && l > n) {
            if (cost < bestCost) {
                bestCost = cost;
                bestMask = mask;
            }
        }
    }

    if (bestMask == 0) return false;

    chosenModuli.clear();
    for (int m = 2; m <= MAX_M; ++m) {
        if (bestMask & (1 << (m - 2))) chosenModuli.push_back(m);
    }
    return true;
}

bool resolve_pair(const vector<int>& S, const vector<int>& C) {
    int u = C[0], v = C[1];
    int x = -1, y = -1;

    for (int i = 1; i <= n; ++i) {
        if (valAtPos[i] != -1) continue;
        int resp = query_all_except(S, i);
        if (resp == 1) {
            if (x == -1) x = i;
            else {
                y = i;
                break;
            }
        }
    }

    if (x == -1 || y == -1) return false;

    int L = (int)knownPos.size();
    int maxS = min(10, L);
    vector<int> cur;
    vector<int> sepS;
    int sepVal = -1;

    function<void(int, int, long long)> gen = [&](int start, int left, long long sum) {
        if (sepVal != -1) return;
        if (left == 0) {
            int s = (int)cur.size();
            vector<int> C2 = get_candidates(s, sum);
            int cnt = 0;
            int valIn = -1;
            for (int val : C2) {
                if (val == u || val == v) {
                    ++cnt;
                    valIn = val;
                }
            }
            if (cnt == 1) {
                sepS = cur;
                sepVal = valIn;
            }
            return;
        }

        for (int i = start; i <= L - left; ++i) {
            cur.push_back(knownPos[i]);
            gen(i + 1, left - 1, sum + valAtPos[knownPos[i]]);
            cur.pop_back();
            if (sepVal != -1) return;
        }
    };

    for (int sz = 0; sz <= maxS; ++sz) {
        gen(0, sz, 0);
        if (sepVal != -1) break;
    }

    if (sepVal != -1) {
        int other = (sepVal == u ? v : u);
        int resp = query_all_except(sepS, x);
        if (resp == 1) {
            add_known(x, sepVal);
            add_known(y, other);
        } else {
            add_known(x, other);
            add_known(y, sepVal);
        }
        return true;
    }

    for (int d = 2; d <= MAX_M; ++d) {
        if (u % d == v % d) continue;
        if (L < d - 1) continue;

        vector<int> sub;
        int targetU = (d - (u % d)) % d;
        if (find_subset_dp(d, targetU, sub)) {
            int resp = query_small(sub, x);
            if (resp == 1) {
                add_known(x, u);
                add_known(y, v);
            } else {
                add_known(x, v);
                add_known(y, u);
            }
            return true;
        }

        int targetV = (d - (v % d)) % d;
        if (find_subset_dp(d, targetV, sub)) {
            int resp = query_small(sub, x);
            if (resp == 1) {
                add_known(x, v);
                add_known(y, u);
            } else {
                add_known(x, u);
                add_known(y, v);
            }
            return true;
        }
    }

    return false;
}

bool find_new_value(int maxS) {
    int bestVal = INT_MAX;
    vector<int> bestS;

    int bestPairMin = INT_MAX;
    vector<int> bestPairS, bestPairC;

    int minUnknown = n + 1;
    for (int v = 1; v <= n; ++v) {
        if (!knownVal[v]) {
            minUnknown = v;
            break;
        }
    }

    int L = (int)knownPos.size();
    maxS = min(maxS, L);

    vector<int> cur;
    bool stop = false;

    function<void(int, int, long long)> gen = [&](int start, int left, long long sum) {
        if (stop) return;
        if (left == 0) {
            int s = (int)cur.size();
            vector<int> C = get_candidates(s, sum);

            if (C.size() == 1) {
                int val = C[0];
                if (val < bestVal) {
                    bestVal = val;
                    bestS = cur;
                    if (bestVal == minUnknown) stop = true;
                }
            } else if (C.size() == 2) {
                int mn = min(C[0], C[1]);
                if (mn < bestPairMin) {
                    bestPairMin = mn;
                    bestPairS = cur;
                    bestPairC = C;
                }
            }
            return;
        }

        for (int i = start; i <= L - left; ++i) {
            cur.push_back(knownPos[i]);
            gen(i + 1, left - 1, sum + valAtPos[knownPos[i]]);
            cur.pop_back();
            if (stop) return;
        }
    };

    for (int sz = 0; sz <= maxS; ++sz) {
        gen(0, sz, 0);
        if (stop) break;
    }

    if (bestVal != INT_MAX) {
        for (int i = 1; i <= n; ++i) {
            if (valAtPos[i] != -1) continue;
            int resp = query_all_except(bestS, i);
            if (resp == 1) {
                add_known(i, bestVal);
                return true;
            }
        }
        return false;
    }

    if (!bestPairS.empty()) {
        return resolve_pair(bestPairS, bestPairC);
    }

    return false;
}

void solve_small() {
    int N = 1 << n;
    vector<int> ans(N, -1);
    vector<vector<int>> idxs(N);
    vector<int> ks(N, 0);

    for (int mask = 1; mask < N; ++mask) {
        int k = __builtin_popcount((unsigned)mask);
        ks[mask] = k;
        if (k == 1) {
            ans[mask] = 1;
            continue;
        }
        for (int i = 0; i < n; ++i) {
            if (mask & (1 << i)) idxs[mask].push_back(i);
        }

        cout << "? " << k;
        for (int i : idxs[mask]) cout << ' ' << i + 1;
        cout << '\n';
        cout.flush();

        int resp;
        if (!(cin >> resp)) exit(0);
        if (resp == -1) exit(0);
        ans[mask] = resp;
    }

    vector<int> perm(n);
    iota(perm.begin(), perm.end(), 1);

    do {
        if (perm[0] > n / 2) continue;

        bool ok = true;
        for (int mask = 1; mask < N && ok; ++mask) {
            int k = ks[mask];
            if (k == 1) continue;
            int sum = 0;
            for (int i : idxs[mask]) sum += perm[i];
            if (sum % k != 0) ok = false;
        }

        if (ok) {
            cout << "!";
            for (int x : perm) cout << ' ' << x;
            cout << '\n';
            cout.flush();
            return;
        }
    } while (next_permutation(perm.begin(), perm.end()));
}

void solve_large() {
    valAtPos.assign(n + 1, -1);
    posOfVal.assign(n + 1, -1);
    knownVal.assign(n + 1, 0);
    knownPos.clear();
    mark.assign(n + 1, 0);

    vector<int> cand;
    vector<int> empty;
    for (int i = 2; i <= n; ++i) {
        int resp = query_all_except(empty, i);
        if (resp == 1) cand.push_back(i);
    }

    vector<int> pair;
    if (cand.size() == 1) {
        pair = {1, cand[0]};
    } else if (cand.size() >= 2) {
        pair = {cand[0], cand[1]};
    } else {
        pair = {1, 2};
    }

    int a = pair[0], b = pair[1];
    if (a == 1 || b == 1) {
        int other = (a == 1 ? b : a);
        add_known(1, 1);
        add_known(other, n);
    } else {
        add_known(a, 1);
        add_known(b, n);
    }

    MAX_M = min(12, n - 1);
    int maxS = 10;

    while (true) {
        if (build_tests()) break;

        bool ok = find_new_value(maxS);
        if (!ok) {
            if (maxS < 12) {
                maxS = 12;
                continue;
            }
            if ((int)knownPos.size() <= 20) {
                ok = find_new_value((int)knownPos.size());
            }
            if (!ok && (int)knownPos.size() <= 25) {
                ok = find_new_value(15);
            }
            if (!ok && (int)knownPos.size() <= 30) {
                ok = find_new_value((int)knownPos.size());
            }
            if (!ok) {
                exit(0);
            }
        }

        if ((int)knownPos.size() >= n) break;
    }

    for (int i = 1; i <= n; ++i) {
        if (valAtPos[i] != -1) continue;

        int res[13] = {};

        for (int m : chosenModuli) {
            bool found = false;
            for (int r = 0; r < m - 1; ++r) {
                const vector<int>& sub = tests[m][r];
                int resp = query_small(sub, i);
                if (resp == 1) {
                    res[m] = r;
                    found = true;
                    break;
                }
            }
            if (!found) res[m] = m - 1;
        }

        int val = -1;
        for (int v = 1; v <= n; ++v) {
            if (knownVal[v]) continue;
            bool ok = true;
            for (int m : chosenModuli) {
                if (v % m != res[m]) {
                    ok = false;
                    break;
                }
            }
            if (ok) {
                val = v;
                break;
            }
        }

        if (val == -1) {
            for (int v = 1; v <= n; ++v) {
                if (!knownVal[v]) {
                    val = v;
                    break;
                }
            }
        }

        add_known(i, val);
    }

    if (valAtPos[1] > n / 2) {
        for (int i = 1; i <= n; ++i) {
            valAtPos[i] = n + 1 - valAtPos[i];
        }
    }

    cout << "!";
    for (int i = 1; i <= n; ++i) cout << ' ' << valAtPos[i];
    cout << '\n';
    cout.flush();
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    if (!(cin >> n)) return 0;
    totalSum = 1LL * n * (n + 1) / 2;

    if (n == 2) {
        cout << "! 1 2\n";
        cout.flush();
        return 0;
    }

    if (n <= 8) {
        solve_small();
        return 0;
    }

    solve_large();
    return 0;
}
