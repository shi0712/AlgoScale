#include <bits/stdc++.h>
using namespace std;

int n, kcur;

void doMove(int i, int j) {
    cout << i << ' ' << j << '\n';
    cout.flush();
    int nk;
    if (!(cin >> nk)) exit(0);
    kcur = nk;
    if (kcur == 1) exit(0);
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    cin >> n;
    cin >> kcur;

    // If already solved (normally impossible by statement), terminate.
    if (kcur == 1) return 0;

    mt19937 rng(chrono::steady_clock::now().time_since_epoch().count());

    // First reduce the number of distinct symbols to a small value.
    int B = min(25, n - 2);
    if (B < 1) B = 1;

    while (kcur > B) {
        int i = int(rng() % n) + 1;
        int r = int(rng() % (n - 1)) + 1;
        int old = kcur;

        doMove(i, r);

        if (kcur > old) {
            doMove(i, -r);
        }
    }

    // Now kcur <= B, and there is at least one empty position.
    // Learn the occupied positions relative to wheel 1 by a full sweep.
    int val[55];
    val[0] = kcur;

    for (int s = 1; s < n; ++s) {
        doMove(1, 1);
        val[s] = kcur;
    }
    doMove(1, 1); // restore wheel 1 to its original position

    int mn = 1000000;
    for (int s = 0; s < n; ++s) mn = min(mn, val[s]);

    bool inG[55] = {};
    vector<int> Gpos;

    // O_1 = { s | val[s] == mn }, occupied set = O_1 union {0}
    for (int s = 0; s < n; ++s) {
        if (s == 0 || val[s] == mn) {
            if (!inG[s]) {
                inG[s] = true;
                Gpos.push_back(s);
            }
        }
    }

    int marker_pos = 0; // wheel 1 is currently at coordinate 0

    // Create a unique marker at an empty position using wheel 1.
    int empty_pos = -1;
    for (int p = 0; p < n; ++p) {
        if (!inG[p]) {
            empty_pos = p;
            break;
        }
    }

    int before = kcur;
    doMove(1, empty_pos);
    int after = kcur;

    if (after == before) {
        // wheel 1 was unique: remove old position, add new one
        inG[0] = false;
        inG[empty_pos] = true;
    } else {
        // wheel 1 was non-unique: old position stays occupied
        inG[empty_pos] = true;
    }

    marker_pos = empty_pos;

    Gpos.clear();
    for (int p = 0; p < n; ++p) {
        if (inG[p]) Gpos.push_back(p);
    }

    int pos[55] = {};
    pos[1] = marker_pos;

    auto determine_position = [&](int wheel) -> int {
        while (true) {
            vector<pair<int,int>> cand; // (position, unique_flag)

            for (int a : Gpos) {
                if (a == marker_pos) continue; // marker wheel occupies this uniquely
                cand.push_back({a, 0});
                cand.push_back({a, 1});
            }

            int current_t = 0;
            bool used[55] = {};
            bool restart = false;
            int found_a = -1;

            while (true) {
                set<int> distinct;
                for (auto [a, u] : cand) distinct.insert(a);

                if ((int)distinct.size() == 1) {
                    found_a = *distinct.begin();
                    break;
                }

                int szG = (int)Gpos.size();

                auto pred = [&](int a, int u, int t) -> int {
                    int newpos = (a + t) % n;

                    if (u == 0) {
                        // Other wheels occupy exactly Gpos
                        return szG + (inG[newpos] ? 0 : 1);
                    } else {
                        // wheel is unique at a, so its old position disappears
                        int base = szG - 1;
                        bool inO = (newpos != a && inG[newpos]);
                        return base + (inO ? 0 : 1);
                    }
                };

                int best_t = -1;
                int best_max = 1000000;

                for (int t = 0; t < n; ++t) {
                    if (used[t]) continue;

                    int delta = (t - current_t + n) % n;
                    if (delta == 0) continue;

                    map<int,int> mp;
                    for (auto [a, u] : cand) {
                        mp[pred(a, u, t)]++;
                    }

                    int mx = 0;
                    for (auto &it : mp) mx = max(mx, it.second);

                    if (mx < best_max) {
                        best_max = mx;
                        best_t = t;
                    }
                }

                if (best_t == -1 || best_max == (int)cand.size()) {
                    // No useful query remains; break symmetry by moving the marker.
                    if (current_t != 0) {
                        int delta = (-current_t % n + n) % n;
                        doMove(wheel, delta);
                    }

                    int e_new = -1;
                    for (int p = 0; p < n; ++p) {
                        if (!inG[p]) {
                            e_new = p;
                            break;
                        }
                    }

                    int delta = (e_new - marker_pos + n) % n;
                    doMove(1, delta);

                    inG[marker_pos] = false;
                    inG[e_new] = true;
                    marker_pos = e_new;

                    Gpos.clear();
                    for (int p = 0; p < n; ++p) {
                        if (inG[p]) Gpos.push_back(p);
                    }

                    restart = true;
                    break;
                }

                int delta = (best_t - current_t + n) % n;
                doMove(wheel, delta);

                int observed = kcur;
                vector<pair<int,int>> newcand;

                for (auto [a, u] : cand) {
                    if (pred(a, u, best_t) == observed) {
                        newcand.push_back({a, u});
                    }
                }

                cand.swap(newcand);
                current_t = best_t;
                used[best_t] = true;

                if (cand.empty()) {
                    // Should never happen; restart with a moved marker as a safety net.
                    if (current_t != 0) {
                        int delta2 = (-current_t % n + n) % n;
                        doMove(wheel, delta2);
                    }

                    int e_new = -1;
                    for (int p = 0; p < n; ++p) {
                        if (!inG[p]) {
                            e_new = p;
                            break;
                        }
                    }

                    int d = (e_new - marker_pos + n) % n;
                    doMove(1, d);

                    inG[marker_pos] = false;
                    inG[e_new] = true;
                    marker_pos = e_new;

                    Gpos.clear();
                    for (int p = 0; p < n; ++p) {
                        if (inG[p]) Gpos.push_back(p);
                    }

                    restart = true;
                    break;
                }
            }

            if (restart) continue;

            int result = found_a;

            if (current_t != 0) {
                int delta = (-current_t % n + n) % n;
                doMove(wheel, delta);
            }

            return result;
        }
    };

    for (int i = 2; i <= n; ++i) {
        pos[i] = determine_position(i);
    }

    int target = marker_pos;

    for (int i = 2; i <= n; ++i) {
        int delta = (target - pos[i] + n) % n;
        if (delta != 0) {
            doMove(i, delta);
        }
    }

    return 0;
}
