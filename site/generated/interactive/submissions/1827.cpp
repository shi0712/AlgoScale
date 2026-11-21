#include <bits/stdc++.h>
using namespace std;

int n;
int cur_k;

void send_action(int wheel, int delta) {
    cout << wheel << ' ' << delta << '\n';
    cout.flush();

    int k;
    if (!(cin >> k)) exit(0);
    if (k == -1) exit(0);
    if (k == 1) exit(0);

    cur_k = k;
}

vector<int> get_high_set(int wheel) {
    vector<int> vals(n);
    vals[0] = cur_k;

    for (int t = 1; t < n; ++t) {
        send_action(wheel, 1);
        vals[t] = cur_k;
    }

    // Return wheel to its original position.
    send_action(wheel, 1);

    int mx = *max_element(vals.begin(), vals.end());

    vector<int> res;
    for (int t = 0; t < n; ++t) {
        if (vals[t] == mx) res.push_back(t);
    }
    return res;
}

int stabilizer_size(const vector<char>& X) {
    int cnt = 0;
    for (int d = 0; d < n; ++d) {
        bool ok = true;
        for (int r = 0; r < n; ++r) {
            if (X[r] != X[(r + d) % n]) {
                ok = false;
                break;
            }
        }
        if (ok) ++cnt;
    }
    return cnt;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    if (!(cin >> n)) return 0;
    if (!(cin >> cur_k)) return 0;

    if (cur_k == -1) return 0;
    if (cur_k == 1) return 0;

    // Sweep wheel 1 once to learn the initial occupied set,
    // in the coordinate system where wheel 1 initially is at 0.
    vector<int> E1 = get_high_set(1);

    vector<char> e1(n, 0);
    for (int x : E1) e1[x] = 1;

    // Other-wheel occupied positions are exactly the complement of E1.
    vector<char> X(n, 0);
    for (int r = 0; r < n; ++r) {
        if (!e1[r]) X[r] = 1;
    }
    X[0] = 1;

    // Whether another wheel is currently together with wheel 1.
    bool wheel1_has_other = (!e1[0]);

    int wheel1_pos = 0;

    // If the occupied set has a non-trivial translation symmetry,
    // move wheel 1 once to break it.
    while (stabilizer_size(X) > 1) {
        int best_q = -1;
        int best_stab = stabilizer_size(X);
        vector<char> best_X;
        bool best_has_other = wheel1_has_other;

        for (int q = 0; q < n; ++q) {
            if (q == wheel1_pos) continue;

            vector<char> Xrem = X;

            // Remove wheel 1 from its old position.
            if (!wheel1_has_other) {
                Xrem[wheel1_pos] = 0;
            }

            bool occupied_at_q = Xrem[q];

            vector<char> Xnew = Xrem;
            Xnew[q] = 1;

            bool new_has_other = occupied_at_q;

            int st = stabilizer_size(Xnew);

            if (st < best_stab) {
                best_stab = st;
                best_q = q;
                best_X = Xnew;
                best_has_other = new_has_other;
            }
        }

        if (best_q == -1) break;

        int delta = (best_q - wheel1_pos) % n;
        if (delta < 0) delta += n;

        if (delta != 0) send_action(1, delta);

        wheel1_pos = best_q;
        X = best_X;
        wheel1_has_other = best_has_other;

        if (best_stab == 1) break;
    }

    vector<int> holes;
    for (int r = 0; r < n; ++r) {
        if (!X[r]) holes.push_back(r);
    }

    vector<int> pos(n + 1, -1);
    pos[1] = wheel1_pos;

    // Recover exact positions of all other wheels.
    for (int i = 2; i <= n; ++i) {
        vector<int> Evec = get_high_set(i);

        vector<char> Eb(n, 0);
        for (int x : Evec) Eb[x] = 1;

        int found = -1;

        for (int x = 0; x < n; ++x) {
            if (!X[x]) continue;

            vector<char> S(n, 0);

            for (int h : holes) {
                int val = h - x;
                val %= n;
                if (val < 0) val += n;
                S[val] = 1;
            }

            bool eq_group = true;
            bool eq_singleton = true;

            for (int r = 0; r < n; ++r) {
                if (S[r] != Eb[r]) eq_group = false;

                bool s0 = S[r] || (r == 0);
                if (s0 != Eb[r]) eq_singleton = false;
            }

            if (eq_group || eq_singleton) {
                found = x;
                break;
            }
        }

        // Numerical fallback; normally never needed.
        if (found == -1) {
            for (int x = 0; x < n && found == -1; ++x) {
                vector<char> S(n, 0);

                for (int h : holes) {
                    int val = h - x;
                    val %= n;
                    if (val < 0) val += n;
                    S[val] = 1;
                }

                bool ok = true;
                for (int r = 0; r < n; ++r) {
                    bool s0 = S[r] || (r == 0);
                    if (S[r] != Eb[r] && s0 != Eb[r]) {
                        ok = false;
                        break;
                    }
                }

                if (ok) found = x;
            }
        }

        pos[i] = found;
    }

    vector<int> cnt(n, 0);
    for (int i = 1; i <= n; ++i) {
        if (pos[i] >= 0) cnt[pos[i]]++;
    }

    int target = 0;
    for (int r = 1; r < n; ++r) {
        if (cnt[r] > cnt[target]) target = r;
    }

    for (int i = 1; i <= n; ++i) {
        if (pos[i] == target) continue;

        int delta = target - pos[i];
        delta %= n;
        if (delta < 0) delta += n;

        if (delta == 0) continue;

        send_action(i, delta);
    }

    return 0;
}
