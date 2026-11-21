#include <bits/stdc++.h>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n)) return 0;

    if (n == 1) {
        cout << "C 1\n";
        cout.flush();
        return 0;
    }

    vector<int> primes;
    vector<char> comp(n + 1, 0);
    for (int i = 2; i <= n; ++i) {
        if (!comp[i]) {
            primes.push_back(i);
            if (1LL * i * i <= n) {
                for (long long j = 1LL * i * i; j <= n; j += i) comp[(int)j] = 1;
            }
        }
    }

    int m = (int)primes.size();
    if (m == 0) {
        cout << "C 1\n";
        cout.flush();
        return 0;
    }

    vector<char> inR(n + 1, 1);
    int totalR = n;

    auto read_int = []() {
        int x;
        if (!(cin >> x)) exit(0);
        if (x < 0) exit(0);
        return x;
    };

    auto queryA = [&](int a) -> int {
        cout << "A " << a << '\n';
        cout.flush();
        return read_int();
    };

    auto queryB = [&](int a, bool needBaseline, bool doUpdate) -> int {
        int base = 0;
        if (needBaseline) {
            for (int v = a; v <= n; v += a) {
                if (inR[v]) ++base;
            }
        }

        cout << "B " << a << '\n';
        cout.flush();
        int cnt = read_int();

        if (doUpdate) {
            for (int v = a; v <= n; v += a) {
                if (inR[v]) {
                    inR[v] = 0;
                    --totalR;
                }
            }
        }

        return cnt;
    };

    int S = max(1, (int)(sqrt((double)m) + 0.5));

    vector<vector<int>> groups;
    if (m == 1) {
        groups.push_back(primes);
    } else {
        groups.push_back({primes[0]});
        int start = 1;
        while (start < m) {
            int end = min(m, start + S);
            groups.emplace_back(primes.begin() + start, primes.begin() + end);
            start = end;
        }
    }

    int foundGroup = -1;
    vector<int> diff;

    for (int gi = 0; gi < (int)groups.size(); ++gi) {
        const auto &g = groups[gi];
        int s = (int)g.size();
        diff.assign(s, 0);
        bool hasDiff1 = false;

        for (int i = 0; i < s; ++i) {
            int p = g[i];
            int base = 0;

            if (i > 0) {
                for (int v = p; v <= n; v += p) {
                    if (inR[v]) ++base;
                }
            }

            cout << "B " << p << '\n';
            cout.flush();
            int cnt = read_int();

            for (int v = p; v <= n; v += p) {
                if (inR[v]) {
                    inR[v] = 0;
                    --totalR;
                }
            }

            if (i > 0 && cnt > base) {
                diff[i] = 1;
                hasDiff1 = true;
            }
        }

        if (!hasDiff1) {
            int sz = queryA(1);
            if (sz == totalR) {
                continue;
            }
        }

        foundGroup = gi;
        break;
    }

    if (foundGroup == -1) {
        cout << "C 1\n";
        cout.flush();
        return 0;
    }

    const auto &g = groups[foundGroup];
    int s = (int)g.size();

    int Lidx = 0;
    for (int i = 0; i < foundGroup; ++i) Lidx += (int)groups[i].size();
    int Ridx = Lidx + s - 1;

    int limit = s;
    for (int i = 0; i < s; ++i) {
        if (diff[i] > 0) {
            limit = i;
            break;
        }
    }
    if (limit == 0) limit = 1;

    int firstLocal = -1;
    for (int i = 0; i < limit; ++i) {
        if (i == limit - 1) {
            firstLocal = i;
            break;
        }

        int val = queryA(g[i]);
        if (val == 1) {
            firstLocal = i;
            break;
        }
    }
    if (firstLocal == -1) firstLocal = 0;

    int p = g[firstLocal];
    long long x_val = p;

    long long power = 1LL * p * p;
    while (power <= n) {
        int cnt = queryB((int)power, false, false);
        if (cnt == 1) {
            x_val *= p;
            power *= p;
        } else {
            break;
        }
    }

    int idx = Lidx + firstLocal + 1;
    while (idx < m) {
        int q = primes[idx];
        if (x_val * q > n) break;

        bool divides = false;

        if (idx <= Ridx) {
            int local = idx - Lidx;
            divides = (diff[local] > 0);
        } else {
            int base = 0;
            for (int v = q; v <= n; v += q) {
                if (inR[v]) ++base;
            }

            cout << "B " << q << '\n';
            cout.flush();
            int cnt = read_int();

            for (int v = q; v <= n; v += q) {
                if (inR[v]) {
                    inR[v] = 0;
                    --totalR;
                }
            }

            divides = (cnt > base);
        }

        if (divides) {
            x_val *= q;

            long long powq = 1LL * q * q;
            while (powq <= n) {
                int cnt = queryB((int)powq, false, false);
                if (cnt == 1) {
                    x_val *= q;
                    powq *= q;
                } else {
                    break;
                }
            }
        }

        ++idx;
    }

    cout << "C " << x_val << '\n';
    cout.flush();
    return 0;
}
