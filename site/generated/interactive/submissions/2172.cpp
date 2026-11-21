#include <bits/stdc++.h>
using namespace std;

using ll = long long;

ll query(int i, int j, int k) {
    cout << "? " << i + 1 << ' ' << j + 1 << ' ' << k + 1 << '\n';
    cout.flush();

    ll ans;
    if (!(cin >> ans)) exit(0);
    if (ans == -1) exit(0);
    return ans;
}

bool gaussSolve(vector<array<long double, 6>> a, vector<long double>& x) {
    const int N = 5;
    int m = (int)a.size();
    int row = 0;
    vector<int> where(N, -1);

    for (int col = 0; col < N && row < m; ++col) {
        int sel = -1;
        for (int i = row; i < m; ++i) {
            if (fabsl(a[i][col]) > 1e-12L) {
                sel = i;
                break;
            }
        }
        if (sel == -1) continue;

        swap(a[sel], a[row]);
        where[col] = row;

        for (int i = 0; i < m; ++i) {
            if (i == row) continue;
            long double factor = a[i][col] / a[row][col];
            if (fabsl(factor) < 1e-18L) continue;
            for (int j = col; j <= N; ++j) {
                a[i][j] -= factor * a[row][j];
            }
        }
        ++row;
    }

    for (int i = 0; i < m; ++i) {
        bool allZero = true;
        for (int j = 0; j < N; ++j) {
            if (fabsl(a[i][j]) > 1e-12L) {
                allZero = false;
                break;
            }
        }
        if (allZero && fabsl(a[i][N]) > 1e-7L) return false;
    }

    x.assign(N, 0.0L);
    for (int col = 0; col < N; ++col) {
        if (where[col] != -1) x[col] = a[where[col]][N];
    }
    return true;
}

array<ll, 5> solve5(const ll ans[5][5][5]) {
    auto get = [&](int i, int j, int k) -> ll {
        return ans[i][j][k];
    };

    vector<array<ll, 5>> cand;

    auto addCand = [&](const array<ll, 5>& val) {
        for (const auto& v : cand) {
            if (v == val) return;
        }
        cand.push_back(val);
    };

    auto verify = [&](const array<ll, 5>& val) -> bool {
        for (int i = 0; i < 5; ++i) {
            for (int j = i + 1; j < 5; ++j) {
                for (int k = j + 1; k < 5; ++k) {
                    ll mn = min({val[i], val[j], val[k]});
                    ll mx = max({val[i], val[j], val[k]});
                    if (mn + mx != get(i, j, k)) return false;
                }
            }
        }
        return true;
    };

    auto tryVals = [&](ll A02, ll A03, ll A04, ll A13, ll A14, ll A24,
                       const array<int, 5>& perm) {
        ll num = A02 + A24 - A04;
        if (num % 2 != 0) return;

        ll u2 = num / 2;
        ll u0 = A02 - u2;
        ll u4 = A04 - u0;
        ll u3 = A03 - u0;
        ll u1 = A13 - u3;

        if (u1 != A14 - u4) return;

        ll vals[5] = {u0, u1, u2, u3, u4};

        for (int i = 0; i < 5; ++i) {
            if (vals[i] < 0 || vals[i] > 1000000000LL) return;
        }
        for (int i = 0; i < 4; ++i) {
            if (vals[i] > vals[i + 1]) return;
        }

        array<ll, 5> val;
        for (int i = 0; i < 5; ++i) val[perm[i]] = vals[i];

        if (!verify(val)) return;
        addCand(val);
    };

    array<int, 5> perm = {0, 1, 2, 3, 4};
    do {
        int p0 = perm[0], p1 = perm[1], p2 = perm[2], p3 = perm[3], p4 = perm[4];

        ll A02 = get(p0, p1, p2);
        ll A13 = get(p1, p2, p3);
        ll A24 = get(p2, p3, p4);

        vector<ll> A03s = {get(p0, p1, p3), get(p0, p2, p3)};
        vector<ll> A04s = {get(p0, p1, p4), get(p0, p2, p4), get(p0, p3, p4)};
        vector<ll> A14s = {get(p1, p2, p4), get(p1, p3, p4)};

        for (ll A03 : A03s) {
            for (ll A04 : A04s) {
                for (ll A14 : A14s) {
                    tryVals(A02, A03, A04, A13, A14, A24, perm);
                }
            }
        }
    } while (next_permutation(perm.begin(), perm.end()));

    if (!cand.empty()) return cand[0];

    // Fallback: solve the linear pair-sum system for each permutation.
    perm = {0, 1, 2, 3, 4};
    do {
        int pos[5];
        for (int i = 0; i < 5; ++i) pos[perm[i]] = i;

        vector<array<long double, 6>> eqs;
        for (int i = 0; i < 5; ++i) {
            for (int j = i + 1; j < 5; ++j) {
                for (int k = j + 1; k < 5; ++k) {
                    array<long double, 6> row{};
                    int pi = pos[i], pj = pos[j], pk = pos[k];
                    int mn = min({pi, pj, pk});
                    int mx = max({pi, pj, pk});
                    row[mn] = 1.0L;
                    row[mx] = 1.0L;
                    row[5] = (long double)get(i, j, k);
                    eqs.push_back(row);
                }
            }
        }

        vector<long double> x;
        if (gaussSolve(eqs, x)) {
            array<ll, 5> val;
            bool ok = true;
            for (int i = 0; i < 5; ++i) {
                if (x[i] < -1e-7L || x[i] > 1000000000.0L + 1e-7L) ok = false;
                val[perm[i]] = (ll)floor(x[i] + 0.5L);
            }
            if (ok) {
                for (int i = 0; i < 4; ++i) {
                    if (x[i] > x[i + 1] + 1e-7L) ok = false;
                }
            }
            if (ok && verify(val)) addCand(val);
        }
    } while (next_permutation(perm.begin(), perm.end()));

    if (!cand.empty()) return cand[0];

    // Should never happen for valid judge data.
    return array<ll, 5>{0, 0, 0, 0, 0};
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n)) return 0;

    ll ans[5][5][5] = {};

    for (int i = 0; i < 5; ++i) {
        for (int j = i + 1; j < 5; ++j) {
            for (int k = j + 1; k < 5; ++k) {
                ll a = query(i, j, k);
                int arr[3] = {i, j, k};
                do {
                    ans[arr[0]][arr[1]][arr[2]] = a;
                } while (next_permutation(arr, arr + 3));
            }
        }
    }

    vector<ll> c(n, 0);
    array<ll, 5> first5 = solve5(ans);
    for (int i = 0; i < 5; ++i) c[i] = first5[i];

    for (int idx = 5; idx < n; ++idx) {
        int dupA = -1, dupB = -1;

        for (int i = 0; i < idx && dupA == -1; ++i) {
            for (int j = i + 1; j < idx; ++j) {
                if (c[i] == c[j]) {
                    dupA = i;
                    dupB = j;
                    break;
                }
            }
        }

        if (dupA != -1) {
            ll a = query(dupA, dupB, idx);
            c[idx] = a - c[dupA];
            continue;
        }

        vector<int> ord(idx);
        iota(ord.begin(), ord.end(), 0);
        sort(ord.begin(), ord.end(), [&](int x, int y) {
            return c[x] < c[y];
        });

        int best = 0;
        ll bestGap = LLONG_MAX;
        for (int i = 0; i + 2 < idx; ++i) {
            ll gap = c[ord[i + 1]] - c[ord[i]];
            if (gap < bestGap) {
                bestGap = gap;
                best = i;
            }
        }

        int a = ord[best];
        int b = ord[best + 1];
        int cc = ord[best + 2];

        ll sum1 = c[a] + c[b];
        ll ans1 = query(a, b, idx);
        ll x;

        if (ans1 != sum1) {
            if (ans1 > sum1) x = ans1 - c[a];
            else x = ans1 - c[b];
        } else {
            ll sum2 = c[b] + c[cc];
            ll ans2 = query(b, cc, idx);

            if (ans2 != sum2) {
                if (ans2 > sum2) x = ans2 - c[b];
                else x = ans2 - c[cc];
            } else {
                x = c[b];
            }
        }

        c[idx] = x;
    }

    cout << "!";
    for (int i = 0; i < n; ++i) {
        cout << ' ' << c[i];
    }
    cout << '\n';
    cout.flush();

    return 0;
}
