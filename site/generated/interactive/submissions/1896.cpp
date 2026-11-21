#include <bits/stdc++.h>
using namespace std;

using U = uint64_t;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n)) return 0;
    const int W = (n + 63) / 64;
    vector<U> pool(W, ~U(0)), fixedBits(W), q(W), locked(W);
    if (n % 64) pool.back() = (U(1) << (n % 64)) - 1;

    // For an observed door i, each candidate switch has exactly one
    // possible correct position, stored in good[i].
    vector<vector<U>> cand(n), good(n);
    vector<int> door(n, -1);
    set<int> active;
    int seen = 0, solved = 0;
    mt19937_64 rng(0x817ca9b746e5321dULL);

    auto finishDoor = [&](int i, int s) {
        U bit = U(1) << (s % 64);
        pool[s / 64] &= ~bit;
        fixedBits[s / 64] |= good[i][s / 64] & bit;
        door[s] = i;
        ++solved;
    };

    auto entropy = [](int k, int c) {
        if (!k || k == c) return 0.0;
        double p = double(k) / c;
        return -p * log2(p) - (1 - p) * log2(1 - p);
    };

    while (solved < n) {
        vector<int> ids, counts;
        // Propagate forced assignments before spending another query.
        for (;;) {
            ids.clear();
            counts.clear();
            bool changed = false;
            for (auto it = active.begin(); it != active.end();) {
                int i = *it, c = 0, last = 0;
                for (int w = 0; w < W; ++w) {
                    U x = (cand[i][w] &= pool[w]);
                    if (x) {
                        c += __builtin_popcountll(x);
                        last = w * 64 + __builtin_ctzll(x);
                    }
                }
                if (c == 1) {
                    finishDoor(i, last);
                    it = active.erase(it);
                    changed = true;
                } else {
                    if (ids.size() < 64) {
                        ids.push_back(i);
                        counts.push_back(c);
                    }
                    ++it;
                }
            }
            if (!changed) break;
        }

        if (solved == n) break;
        int m = int(ids.size());
        vector<int> target(m);
        // Balance the possible replies, taking into account that opening
        // an earlier door permits the same test to inform later doors.
        double v = seen < n ? 2.0 * (1.0 - ldexp(1.0, -min(50, n - seen))) : 0.0;
        for (int j = m - 1; j >= 0; --j) {
            int c = counts[j];
            double opt = c / (1.0 + exp2(v));
            int k = int(opt);
            auto score = [&](int a) {
                return entropy(a, c) + (1.0 - double(a) / c) * v;
            };
            if (k < c && score(k + 1) >= score(k)) ++k;
            target[j] = k;
            v = score(k);
        }

        for (int w = 0; w < W; ++w) {
            q[w] = (rng() & pool[w]) | fixedBits[w];
            locked[w] = 0;
        }

        bool informative = false, reachable = true;
        for (int j = 0; j < m && reachable; ++j) {
            int i = ids[j], bad = 0, avail = 0;
            for (int w = 0; w < W; ++w) {
                U x = cand[i][w];
                bad += __builtin_popcountll(x & locked[w] & (q[w] ^ good[i][w]));
                avail += __builtin_popcountll(x & ~locked[w]);
            }
            int take = clamp(target[j] - bad, 0, avail);
            int total = bad + take;
            if (total && total < counts[j]) informative = true;
            if (total == counts[j]) reachable = false;

            bool rev = rng() & 1;
            for (int t = 0; t < W; ++t) {
                int w = rev ? W - 1 - t : t;
                U x = cand[i][w] & ~locked[w];
                if (!x) continue;
                U flip = 0;
                if (take) {
                    int pc = __builtin_popcountll(x);
                    if (pc <= take) {
                        flip = x;
                        take -= pc;
                    } else {
                        U rest = x;
                        while (take > 0) {
                            rest &= rest - 1;
                            --take;
                        }
                        flip = x ^ rest;
                    }
                }
                q[w] = (q[w] & ~x) | ((good[i][w] ^ flip) & x);
                locked[w] |= x;
            }
        }

        // If overlapping candidate sets frustrated the simultaneous
        // tests, guarantee progress by bisecting the first unresolved door.
        if (!informative && !(reachable && int(active.size()) == m && seen < n)) {
            int i = ids.front();
            int take = counts.front() / 2;
            for (int w = 0; w < W; ++w) {
                U x = cand[i][w], rest = x;
                int pc = __builtin_popcountll(x);
                if (pc <= take) {
                    rest = 0;
                    take -= pc;
                } else {
                    while (take > 0) {
                        rest &= rest - 1;
                        --take;
                    }
                }
                q[w] = (q[w] & ~x) | ((good[i][w] ^ x ^ rest) & x);
            }
        }

        string s(n, '0');
        for (int k = 0; k < n; ++k)
            s[k] += (q[k / 64] >> (k % 64)) & 1;
        cout << "? " << s << '\n' << flush;
        int r;
        if (!(cin >> r) || r < -1 || r >= n) return 0;
        int last = r == -1 ? n - 1 : r;

        for (int i : active) {
            if (i > last) break;
            for (int w = 0; w < W; ++w) {
                U mismatch = q[w] ^ good[i][w];
                cand[i][w] &= (i == r ? mismatch : ~mismatch);
            }
        }
        while (seen <= last) {
            int i = seen++;
            cand[i] = pool;
            good[i] = q;
            if (i == r)
                for (U &x : good[i]) x = ~x;
            active.insert(active.end(), i);
        }
    }

    string s(n, '0');
    for (int i = 0; i < n; ++i)
        s[i] += (fixedBits[i / 64] >> (i % 64)) & 1;
    cout << "! " << s;
    for (int d : door) cout << ' ' << d;
    cout << '\n' << flush;
    return 0;
}
