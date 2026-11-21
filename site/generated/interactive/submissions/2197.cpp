#include <bits/stdc++.h>
using namespace std;

int n;

void submit_final(const vector<int>& key) {
    string s(n, 'F');
    for (int i = 0; i < n; ++i) {
        if (key[i]) s[i] = 'T';
    }
    cout << s << endl;
    int sc;
    if (!(cin >> sc)) return;
}

void fallback() {
    string s(n, 'F');
    cout << s << endl;
    int sc;
    if (!(cin >> sc)) return;
    if (sc == -1) return;
    if (sc == n) return;

    int k = n - sc;
    vector<int> key(n, 0);
    int sum = 0;

    for (int i = 0; i < n - 1; ++i) {
        string q(n, 'F');
        q[i] = 'T';
        cout << q << endl;
        if (!(cin >> sc)) return;
        if (sc == -1) return;
        if (sc == n) return;

        int v = sc + k - (n - 1);
        int bit = (v == 2) ? 1 : 0;
        key[i] = bit;
        sum += bit;
    }

    key[n - 1] = k - sum;
    submit_final(key);
}

bool decode_complement(int n, const vector<int>& s, vector<int>& key) {
    long long Z = 0;
    for (int x : s) Z += x;

    int d = n - 3;
    bool found = false;

    for (int last = 0; last <= 1; ++last) {
        long long num = Z - (n - 1) - 2LL * last;
        if (num % d != 0) continue;

        int k = (int)(num / d);
        if (k < 0 || k > n) continue;

        int sum = 0;
        bool ok = true;
        for (int i = 0; i < n - 1; ++i) {
            int v = k + 1 - s[i];
            if (v != 0 && v != 2) {
                ok = false;
                break;
            }
            int bit = v / 2;
            key[i] = bit;
            sum += bit;
        }
        if (!ok) continue;
        if (sum + last != k) continue;

        key[n - 1] = last;
        if (found) return false;
        found = true;
    }

    return found;
}

bool decode_block(int n, const vector<int>& s, const array<int, 3>& t, vector<int>& key) {
    int m = n - 5;
    long long Z = 0;
    for (int x : s) Z += x;

    int d = m - 2;
    bool found = false;

    for (int b = 0; b <= 5; ++b) {
        long long num = Z - m - 2LL * b;
        if (num % d != 0) continue;

        int k = (int)(num / d);
        if (k < 0 || k > n) continue;

        int first_sum = 0;
        bool ok = true;
        for (int i = 0; i < m; ++i) {
            int v = k + 1 - s[i];
            if (v != 0 && v != 2) {
                ok = false;
                break;
            }
            int bit = v / 2;
            key[i] = bit;
            first_sum += bit;
        }
        if (!ok) continue;
        if (first_sum + b != k) continue;

        int y[3];
        bool oky = true;
        for (int j = 0; j < 3; ++j) {
            long long v = t[j] + k + 2 - n;
            if (v % 2 != 0) {
                oky = false;
                break;
            }
            y[j] = (int)(v / 2);
            if (y[j] < 0 || y[j] > 2) {
                oky = false;
                break;
            }
        }
        if (!oky) continue;

        bool block_ok = false;
        for (int c = 0; c <= 1; ++c) {
            int l1 = y[0] - c;
            int l2 = y[1] - c;
            int l3 = y[2] - c;
            if (l1 < 0 || l1 > 1 || l2 < 0 || l2 > 1 || l3 < 0 || l3 > 1) continue;

            int b0 = b - c - l1 - l2 - l3;
            if (b0 < 0 || b0 > 1) continue;

            if (block_ok) {
                block_ok = false;
                break;
            }

            block_ok = true;
            key[n - 5] = b0;
            key[n - 4] = l1;
            key[n - 3] = l2;
            key[n - 2] = l3;
            key[n - 1] = c;
        }

        if (block_ok) {
            if (found) return false;
            found = true;
        }
    }

    return found;
}

void solve_small() {
    int N = 1 << n;
    vector<vector<int>> score(N, vector<int>(N, 0));

    for (int q = 0; q < N; ++q) {
        for (int x = 0; x < N; ++x) {
            int c = 0;
            for (int i = 0; i < n; ++i) {
                if (((q >> i) & 1) == ((x >> i) & 1)) ++c;
            }
            score[q][x] = c;
        }
    }

    int M = 1 << N;
    const int INF = 1000000;
    vector<int> f(M, INF), best(M, -1);

    for (int mask = 1; mask < M; ++mask) {
        int pc = __builtin_popcount((unsigned)mask);
        if (pc == 1) {
            f[mask] = 1;
            best[mask] = __builtin_ctz((unsigned)mask);
            continue;
        }

        int best_cost = INF;
        for (int q = 0; q < N; ++q) {
            int branch[11] = {};
            for (int x = 0; x < N; ++x) {
                if (mask & (1 << x)) {
                    branch[score[q][x]] |= (1 << x);
                }
            }

            bool useful = true;
            int max_cost = 0;

            for (int v = 0; v <= n; ++v) {
                int br = branch[v];
                if (!br) continue;
                if (br == mask) {
                    useful = false;
                    break;
                }

                int cost;
                if (br == (1 << q)) cost = 1;
                else cost = 1 + f[br];

                if (cost > max_cost) max_cost = cost;
            }

            if (useful && max_cost < best_cost) {
                best_cost = max_cost;
                best[mask] = q;
            }
        }

        f[mask] = best_cost;
        if (best[mask] == -1) best[mask] = 0;
    }

    int mask = M - 1;
    while (true) {
        int q = best[mask];
        string s(n, 'F');
        for (int i = 0; i < n; ++i) {
            if ((q >> i) & 1) s[i] = 'T';
        }

        cout << s << endl;
        int sc;
        if (!(cin >> sc)) return;
        if (sc == -1) return;
        if (sc == n) return;

        int new_mask = 0;
        for (int x = 0; x < N; ++x) {
            if ((mask & (1 << x)) && score[q][x] == sc) {
                new_mask |= (1 << x);
            }
        }

        mask = new_mask;
        if (mask == 0) return;
    }
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    if (!(cin >> n)) return 0;

    if (n <= 4) {
        solve_small();
        return 0;
    }

    if (n <= 7) {
        vector<int> s(n - 1);
        for (int i = 0; i < n - 1; ++i) {
            string q(n, 'T');
            q[i] = 'F';
            cout << q << endl;

            int sc;
            if (!(cin >> sc)) return 0;
            if (sc == -1) return 0;
            if (sc == n) return 0;

            s[i] = sc;
        }

        vector<int> key(n, 0);
        if (!decode_complement(n, s, key)) {
            fallback();
            return 0;
        }

        submit_final(key);
        return 0;
    }

    int m = n - 5;
    vector<int> s(m);

    for (int i = 0; i < m; ++i) {
        string q(n, 'T');
        q[i] = 'F';
        cout << q << endl;

        int sc;
        if (!(cin >> sc)) return 0;
        if (sc == -1) return 0;
        if (sc == n) return 0;

        s[i] = sc;
    }

    array<int, 3> t;

    string q(n, 'F');
    q[n - 4] = 'T';
    q[n - 1] = 'T';
    cout << q << endl;
    int sc;
    if (!(cin >> sc)) return 0;
    if (sc == -1) return 0;
    if (sc == n) return 0;
    t[0] = sc;

    q.assign(n, 'F');
    q[n - 3] = 'T';
    q[n - 1] = 'T';
    cout << q << endl;
    if (!(cin >> sc)) return 0;
    if (sc == -1) return 0;
    if (sc == n) return 0;
    t[1] = sc;

    q.assign(n, 'F');
    q[n - 2] = 'T';
    q[n - 1] = 'T';
    cout << q << endl;
    if (!(cin >> sc)) return 0;
    if (sc == -1) return 0;
    if (sc == n) return 0;
    t[2] = sc;

    vector<int> key(n, 0);
    if (!decode_block(n, s, t, key)) {
        fallback();
        return 0;
    }

    submit_final(key);
    return 0;
}
