#include <bits/stdc++.h>
using namespace std;

using ll = long long;

const int LIMIT = 1000000000;
const int C30 = (1 << 30) - 1;

int ask(ll x) {
    cout << "- " << x << '\n';
    cout.flush();
    int r;
    if (!(cin >> r)) exit(0);
    if (r == -1) exit(0);
    return r;
}

int pc(int x) {
    return __builtin_popcount((unsigned)x);
}

/*
   Determines the current value m, knowing only:
   - popcount(m) = p
   - m <= U
   It outputs queries and reads responses.
*/
ll solve_value(int p, ll U) {
    ll S = 0; // total subtracted inside this solver

    while (true) {
        if (p == 0) return S;

        if (p == 1) {
            int q = ask(1);
            S += 1;
            // m was 2^q
            return S + ((1LL << q) - 1);
        }

        ll Ucur = U - S;

        // Safe "jump": if current value is guaranteed to be < 2 * (2^p - 1),
        // subtracting (2^p - 1) strictly decreases the popcount.
        if (p >= 2) {
            ll D = (1LL << p) - 1;
            if (Ucur < 2 * D) {
                int q = ask(D);
                S += D;
                if (q == 0) return S;
                p = q;
                continue;
            }
        }

        // Arbitrary state: subtract 1 to establish the invariant
        // "lower a bits are 1, bit a is 0".
        int q = ask(1);
        S += 1;
        int a;
        if (q == p - 1) {
            a = 0;
        } else {
            a = q - p + 1;
        }
        p = q;

        bool jumped = false;
        while (p > a) {
            Ucur = U - S;

            // If a == 0, the remaining low-to-high cost is p, so the same
            // safe jump can be used.
            if (a == 0 && p >= 2) {
                ll D = (1LL << p) - 1;
                if (Ucur < 2 * D) {
                    int q2 = ask(D);
                    S += D;
                    if (q2 == 0) return S;
                    p = q2;
                    jumped = true;
                    break;
                }
            }

            // Invariant step: subtract 2^a.
            ll x = 1LL << a;
            int q2 = ask(x);
            S += x;
            a = q2 - p + a + 1;
            p = q2;
        }

        if (jumped) continue;

        // p == a: current value is exactly 2^a - 1.
        return S + ((1LL << a) - 1);
    }
}

void gen_candidates(int pos, int left, int mask, vector<int>& cand) {
    if (left == 0) {
        int n = C30 - mask;
        if (1 <= n && n <= LIMIT) cand.push_back(n);
        return;
    }
    if (pos < 0) return;
    if (left > pos + 1) return;

    // Put a zero of n (a one of the complement) at bit pos.
    gen_candidates(pos - 1, left - 1, mask | (1 << pos), cand);
    // Do not put it there.
    gen_candidates(pos - 1, left, mask, cand);
}

vector<int> generate_candidates(int c) {
    int z = 30 - c;
    vector<int> cand;
    cand.reserve(700000);
    gen_candidates(29, z, 0, cand);
    return cand;
}

vector<int> xs_small, xs_full;

void build_query_sets() {
    for (int k = 0; k <= 30; ++k) {
        ll x = 1LL << k;
        if (1 <= x && x <= LIMIT) xs_small.push_back((int)x);
    }
    for (int k = 1; k <= 30; ++k) {
        ll x = (1LL << k) - 1;
        if (1 <= x && x <= LIMIT) xs_small.push_back((int)x);
    }

    xs_full = xs_small;
    for (int j = 0; j <= 30; ++j) {
        for (int k = j + 1; k <= 30; ++k) {
            ll x = (1LL << k) - (1LL << j);
            if (1 <= x && x <= LIMIT) xs_full.push_back((int)x);
        }
    }

    sort(xs_small.begin(), xs_small.end());
    xs_small.erase(unique(xs_small.begin(), xs_small.end()), xs_small.end());

    sort(xs_full.begin(), xs_full.end());
    xs_full.erase(unique(xs_full.begin(), xs_full.end()), xs_full.end());
}

void solve_by_candidates(int c) {
    vector<int> cand = generate_candidates(c);

    if (cand.size() == 1) {
        cout << "! " << cand[0] << '\n';
        cout.flush();
        return;
    }

    ll S = 0;
    int current_pop = c;
    int queries = 0;
    const int QUERY_LIMIT = 6;

    const vector<int>& base = (cand.size() <= 200000 ? xs_full : xs_small);

    while (cand.size() > 1 && queries < QUERY_LIMIT) {
        if (current_pop == 0) {
            cout << "! " << S << '\n';
            cout.flush();
            return;
        }

        int minc = *min_element(cand.begin(), cand.end());
        int min_cur = minc - (int)S;
        if (min_cur <= 0) break;

        vector<int> xs;
        xs.reserve(base.size() + 2);
        for (int x : base) {
            if (x > min_cur) break;
            xs.push_back(x);
        }
        xs.push_back(min_cur);
        if (min_cur > 1) xs.push_back(min_cur - 1);

        sort(xs.begin(), xs.end());
        xs.erase(unique(xs.begin(), xs.end()), xs.end());

        int best_x = 1;
        int best_max = (int)cand.size();
        int best_nonempty = 0;
        array<int, 31> bucket{};

        for (int x : xs) {
            if (x <= 0 || x > min_cur) continue;

            bucket.fill(0);
            int maxb = 0;
            bool bad = false;

            for (int n : cand) {
                int val = n - (int)S - x;
                int r = pc(val);
                int cnt = ++bucket[r];
                if (cnt > maxb) maxb = cnt;
                if (maxb > best_max) {
                    bad = true;
                    break;
                }
            }

            if (bad) continue;

            int nonempty = 0;
            for (int i = 0; i <= 30; ++i) {
                if (bucket[i] > 0) ++nonempty;
            }

            if (maxb < best_max || (maxb == best_max && nonempty > best_nonempty)) {
                best_max = maxb;
                best_nonempty = nonempty;
                best_x = x;
            }

            if (best_max == 1) break;
        }

        int oldS = (int)S;
        int resp = ask(best_x);
        S += best_x;
        current_pop = resp;
        ++queries;

        vector<int> nc;
        nc.reserve(cand.size());
        for (int n : cand) {
            int val = n - oldS - best_x;
            if (pc(val) == resp) nc.push_back(n);
        }
        cand.swap(nc);
    }

    if (cand.size() == 1) {
        cout << "! " << cand[0] << '\n';
        cout.flush();
        return;
    }

    if (current_pop == 0) {
        cout << "! " << S << '\n';
        cout.flush();
        return;
    }

    // Fallback: determine the current value directly.
    ll cur = solve_value(current_pop, (ll)LIMIT - S);
    ll n = S + cur;
    cout << "! " << n << '\n';
    cout.flush();
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    build_query_sets();

    int c;
    if (!(cin >> c)) return 0;

    // For large popcounts the number of candidates is small; use a
    // candidate-driven decision process. Otherwise use the guaranteed
    // low-to-high popcount reconstruction.
    if (c >= 24) {
        solve_by_candidates(c);
    } else {
        ll n = solve_value(c, (ll)LIMIT);
        cout << "! " << n << '\n';
        cout.flush();
    }

    return 0;
}
