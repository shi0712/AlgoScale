#include <bits/stdc++.h>
using namespace std;

int n, curK;

int ask(int i, int j) {
    cout << i << ' ' << j << '\n' << flush;
    int k;
    if (!(cin >> k)) exit(0);
    if (k < 0) exit(0);
    if (k == 1) exit(0);
    return k;
}

int rep(int d) {
    d %= n;
    if (d < 0) d += n;
    if (d > n / 2) d -= n;
    return d;
}

bool is_aperiodic(const vector<char>& E) {
    int cnt = 0;
    for (int i = 0; i < n; ++i) cnt += E[i];
    if (cnt == 0 || cnt == n) return false;
    for (int d = 1; d < n; ++d) {
        bool same = true;
        for (int v = 0; v < n; ++v) {
            if (E[v] != E[(v + d) % n]) {
                same = false;
                break;
            }
        }
        if (same) return false;
    }
    return true;
}

void make_distinct() {
    vector<char> processed(n + 1, 0);

    while (curK < n) {
        bool progress = false;

        for (int i = 1; i <= n; ++i) {
            if (processed[i]) continue;

            bool done = false;

            // Try offsets in increasing cyclic distance:
            // +1, -1, +2, -2, ...
            for (int d = 1; d <= n / 2; ++d) {
                int j1 = d;
                int j2 = (n % 2 == 0 && d == n / 2) ? 0 : n - d;

                if (!done) {
                    int k1 = ask(i, j1);
                    if (k1 == curK + 1) {
                        // It was in a duplicate group and moved to an empty value.
                        curK = k1;
                        processed[i] = 1;
                        progress = true;
                        done = true;
                    } else if (k1 == curK - 1) {
                        // It was a singleton and moved to an occupied value. Undo.
                        curK = ask(i, -j1);
                        processed[i] = 1;
                        done = true;
                    } else {
                        // Unchanged: ambiguous. Undo and try another offset.
                        curK = ask(i, -j1);
                    }
                }

                if (!done && j2 != 0) {
                    int k1 = ask(i, j2);
                    if (k1 == curK + 1) {
                        curK = k1;
                        processed[i] = 1;
                        progress = true;
                        done = true;
                    } else if (k1 == curK - 1) {
                        curK = ask(i, -j2);
                        processed[i] = 1;
                        done = true;
                    } else {
                        curK = ask(i, -j2);
                    }
                }

                if (done) break;
            }

            // Safety fallback: try every non-zero offset.
            if (!done) {
                for (int j = 1; j < n; ++j) {
                    int k1 = ask(i, j);
                    if (k1 == curK + 1) {
                        curK = k1;
                        processed[i] = 1;
                        progress = true;
                        done = true;
                        break;
                    } else if (k1 == curK - 1) {
                        curK = ask(i, -j);
                        processed[i] = 1;
                        done = true;
                        break;
                    } else {
                        curK = ask(i, -j);
                    }
                }
            }

            if (curK == n) break;
        }

        // Theoretically, if curK < n, some unprocessed wheel is still in a
        // duplicate group, so progress must have been made.
        if (curK < n && !progress) {
            // As a last resort, restart the scan.
            for (int i = 1; i <= n; ++i) processed[i] = 0;
        }
    }
}

void solve_all_distinct() {
    // curK == n here.
    // Move wheel 1 by +1.  Now:
    //   known empty e = 0
    //   known pair value g = 1 (wheel 1 and one unknown wheel A)
    curK = ask(1, 1);

    int g = 1;
    int A = -1;

    // Identify A using non-destructive queries with offset -1.
    for (int i = 2; i <= n; ++i) {
        int j = n - 1; // -1 modulo n
        int k1 = ask(i, j);

        if (k1 == curK + 1) {
            // This is A. Undo to restore the pair at g.
            curK = ask(i, -j);
            A = i;
            break;
        } else {
            // Singleton. Undo.
            curK = ask(i, -j);
        }
    }

    // Should never happen for valid input, but keep a fallback.
    if (A == -1) A = 2;

    vector<char> E(n, 0);
    E[0] = 1; // known empty value

    // wheelVal[w]:
    //   -1 : unknown
    //   -2 : already in the known group at g
    //   >=0: known value, not yet in group
    vector<int> wheelVal(n + 1, -1);
    vector<int> valToWheel(n, 0);
    vector<int> knownVals;

    wheelVal[1] = -2;
    wheelVal[A] = -2;
    int groupSize = 2;

    auto unknown_count = [&]() -> int {
        return n - groupSize - (int)knownVals.size();
    };

    auto add_known = [&](int x) {
        int w = valToWheel[x];
        int d = (g - x) % n;
        if (d < 0) d += n;
        curK = ask(w, rep(d));

        E[x] = 1;
        wheelVal[w] = -2;
        valToWheel[x] = 0;

        for (int idx = 0; idx < (int)knownVals.size(); ++idx) {
            if (knownVals[idx] == x) {
                knownVals[idx] = knownVals.back();
                knownVals.pop_back();
                break;
            }
        }
        ++groupSize;
    };

    while (curK > 1) {
        // If possible, add a known wheel to the group.
        bool added = false;
        int uc = unknown_count();

        for (int idx = 0; idx < (int)knownVals.size(); ++idx) {
            int x = knownVals[idx];
            vector<char> E2 = E;
            E2[x] = 1;
            bool ok = is_aperiodic(E2);

            // If no unknown wheels remain, periodicity no longer matters.
            if (ok || uc == 0) {
                add_known(x);
                added = true;
                break;
            }
        }
        if (added) continue;

        // Pick an unknown wheel.
        int i = -1;
        for (int w = 1; w <= n; ++w) {
            if (wheelVal[w] == -1) {
                i = w;
                break;
            }
        }

        if (i == -1) {
            // No unknown wheels left; add any known wheel.
            if (!knownVals.empty()) {
                add_known(knownVals[0]);
                continue;
            }
            break;
        }

        // Candidate values for this wheel.
        vector<int> C;
        for (int v = 0; v < n; ++v) {
            if (!E[v] && v != g && valToWheel[v] == 0) {
                C.push_back(v);
            }
        }

        // Identify its value by group tests using translates of E.
        int guard = 0;
        while (C.size() > 1) {
            if (++guard > 1000) break;

            int best_j = -1;
            int best_score = 1e9;

            for (int j = 1; j < n; ++j) {
                int s = 0;
                for (int c : C) {
                    if (E[(c + j) % n]) ++s;
                }
                if (s > 0 && s < (int)C.size()) {
                    int score = max(s, (int)C.size() - s);
                    if (score < best_score) {
                        best_score = score;
                        best_j = j;
                    }
                }
            }

            if (best_j == -1) {
                // Fallback: add a known value to change E, if possible.
                if (!knownVals.empty()) {
                    add_known(knownVals[0]);
                    C.clear();
                    for (int v = 0; v < n; ++v) {
                        if (!E[v] && v != g && valToWheel[v] == 0) {
                            C.push_back(v);
                        }
                    }
                    continue;
                }
                // Should not happen; choose a default to avoid hanging.
                best_j = 1;
            }

            int j = best_j;
            int jr = rep(j);

            int k1 = ask(i, jr);
            bool in;
            if (k1 == curK) in = true;
            else if (k1 == curK - 1) in = false;
            else in = (k1 != curK - 1); // unexpected, but keep going

            curK = ask(i, -jr); // undo

            vector<int> C2;
            for (int c : C) {
                bool cin = E[(c + j) % n];
                if (in == cin) C2.push_back(c);
            }

            if (C2.empty()) C2 = C;
            C.swap(C2);
        }

        if (C.empty()) {
            if (!knownVals.empty()) {
                add_known(knownVals[0]);
                continue;
            }
            break;
        }

        int x = C[0];
        wheelVal[i] = x;
        valToWheel[x] = i;
        knownVals.push_back(x);
    }
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    if (!(cin >> n)) return 0;
    if (!(cin >> curK)) return 0;
    if (curK == 1) return 0;

    if (curK < n) {
        make_distinct();
    }

    if (curK == 1) return 0;

    if (curK == n) {
        solve_all_distinct();
    } else {
        // Fallback: try once more to reach all distinct.
        make_distinct();
        if (curK == n) solve_all_distinct();
    }

    return 0;
}
