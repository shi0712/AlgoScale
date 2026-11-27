#include <bits/stdc++.h>
using namespace std;

struct Candidate {
    int lb;
    uint16_t l, T;
};

int main() {
    int n, m;
    if (scanf("%d%d", &n, &m) != 2) return 0;

    const int cells = n * n;
    const int L = 2 * n - 1;
    const int INF = cells + 1;

    vector<int> a(cells);
    for (int &x : a) {
        if (scanf("%d", &x) != 1) return 0;
    }

    vector<int> rowPref(n + 1, 0);
    for (int c = 1; c <= n; ++c) {
        rowPref[c] = max(rowPref[c - 1], a[c - 1]);
    }

    auto print_answer = [&](vector<int> answer) {
        sort(answer.begin(), answer.end());
        string out = "!";
        for (int x : answer) {
            out += ' ';
            out += to_string(x);
        }
        out += '\n';
        fwrite(out.data(), 1, out.size(), stdout);
        fflush(stdout);
    };

    if (n == 2) {
        int h[4] = {a[0], a[2], a[3]};
        int pref[4] = {};
        for (int t = 1; t <= 3; ++t) {
            pref[t] = max(pref[t - 1], h[t]);
        }

        vector<int> values;
        for (int l = 1; l <= 2; ++l) {
            for (int T = 1; T <= 3; ++T) {
                int value;
                if (T <= l) {
                    value = max(pref[T], rowPref[l - T + 1]);
                } else {
                    value = 0;
                    for (int k = T - l + 1; k <= T; ++k) {
                        value = max(value, h[k]);
                    }
                }
                values.push_back(value);
            }
        }

        if (m < (int)values.size()) {
            nth_element(values.begin(), values.begin() + m, values.end());
        }
        values.resize(m);
        print_answer(values);
        return 0;
    }

    // Minimum possible maximum on a valid head prefix ending at each diagonal.
    vector<int> prefixDP(cells, INF);
    vector<int> prefixDiag(L + 1, INF);
    prefixDP[0] = a[0];
    prefixDiag[1] = a[0];

    for (int r = 1; r < n; ++r) {
        for (int c = 0; c < n; ++c) {
            int best = INF;
            best = min(best, prefixDP[(r - 1) * n + c]);
            if (c > 0) best = min(best, prefixDP[r * n + c - 1]);

            int value = max(a[r * n + c], best);
            prefixDP[r * n + c] = value;
            int diagonal = r + c + 1;
            prefixDiag[diagonal] = min(prefixDiag[diagonal], value);
        }
    }

    vector<Candidate> candidates;
    candidates.reserve(n * (L - 2));

    // generic[k]: minimum bottleneck of a k-cell down/right path lying in rows 2..n.
    // forced[k]: the same, but the path must start at (2,1).
    vector<int> genericPrev(cells, INF), genericCur(cells, INF);
    vector<int> forcedPrev(cells, INF), forcedCur(cells, INF);

    for (int r = 1; r < n; ++r) {
        for (int c = 0; c < n; ++c) {
            genericPrev[r * n + c] = a[r * n + c];
        }
    }
    forcedPrev[n] = a[n];

    for (int length = 1; length <= n; ++length) {
        vector<int> genericDiag(L + 1, INF);
        vector<int> forcedDiag(L + 1, INF);

        if (length == 1) {
            for (int r = 0; r < n; ++r) {
                for (int c = 0; c < n; ++c) {
                    int id = r * n + c;
                    int diagonal = r + c + 1;

                    int g = (r >= 1 ? a[id] : INF);
                    genericDiag[diagonal] = min(genericDiag[diagonal], g);

                    int f = (r == 1 && c == 0 ? a[id] : INF);
                    forcedDiag[diagonal] = min(forcedDiag[diagonal], f);
                }
            }
        } else {
            for (int r = 0; r < n; ++r) {
                for (int c = 0; c < n; ++c) {
                    int id = r * n + c;
                    int diagonal = r + c + 1;

                    int gBest = INF;
                    if (r > 0) gBest = min(gBest, genericPrev[(r - 1) * n + c]);
                    if (c > 0) gBest = min(gBest, genericPrev[r * n + c - 1]);
                    int g = max(a[id], gBest);
                    genericCur[id] = g;
                    genericDiag[diagonal] = min(genericDiag[diagonal], g);

                    int fBest = INF;
                    if (r > 0) fBest = min(fBest, forcedPrev[(r - 1) * n + c]);
                    if (c > 0) fBest = min(fBest, forcedPrev[r * n + c - 1]);
                    int f = max(a[id], fBest);
                    forcedCur[id] = f;
                    forcedDiag[diagonal] = min(forcedDiag[diagonal], f);
                }
            }
        }

        for (int T = 3; T <= L; ++T) {
            if (length == 1 && T == L) continue;

            int lb;
            if (T < length) {
                lb = max(prefixDiag[T], rowPref[length - T + 1]);
            } else if (T == length) {
                lb = prefixDiag[T];
            } else if (T == length + 1) {
                lb = forcedDiag[T];
            } else {
                lb = genericDiag[T];
            }

            candidates.push_back({
                lb,
                static_cast<uint16_t>(length),
                static_cast<uint16_t>(T)
            });
        }

        if (length < n) {
            genericPrev.swap(genericCur);
            forcedPrev.swap(forcedCur);
        }
    }

    sort(candidates.begin(), candidates.end(),
         [](const Candidate &x, const Candidate &y) {
             if (x.lb != y.lb) return x.lb < y.lb;
             if (x.l != y.l) return x.l < y.l;
             return x.T < y.T;
         });

    priority_queue<int> best;

    auto add = [&](int value) {
        if ((int)best.size() < m) {
            best.push(value);
        } else if (value < best.top()) {
            best.pop();
            best.push(value);
        }
    };

    // f(l,1) and f(l,2) are known without querying.
    for (int l = 1; l <= n; ++l) {
        add(rowPref[l]);
        add(max(a[n], rowPref[l - 1]));
    }

    // The length-1 snake ends at (n,n).
    add(a[cells - 1]);

    for (const Candidate &candidate : candidates) {
        if ((int)best.size() == m && candidate.lb >= best.top()) {
            break;
        }

        printf("? %d %d\n", (int)candidate.l, (int)candidate.T);
        fflush(stdout);

        int response;
        if (scanf("%d", &response) != 1) return 0;
        if (response == -1) return 0;

        add(response);
    }

    vector<int> answer;
    answer.reserve(m);
    while (!best.empty()) {
        answer.push_back(best.top());
        best.pop();
    }
    print_answer(answer);
    return 0;
}
