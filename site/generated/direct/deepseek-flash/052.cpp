#include <bits/stdc++.h>
using namespace std;

using ll = long long;
const ll NEG = -(1LL << 60);

struct Item {
    int full;
    int b[11];
};

/*
    dp2[s] = max value for exact total weight s after adding one weight class.

    For this class, if we take c items, we take the c largest full values,
    so pref[c] is concave.  The transition is

        ndp[s] = max_c dp[s - c*w] + pref[c].

    For every remainder modulo w, this is a max-plus convolution with a
    concave sequence.  Its best previous index is monotone, so divide and
    conquer optimization computes one class transition in O(K log K).
*/
void add_class(const vector<ll>& dp, vector<ll>& ndp, int w,
               const vector<ll>& pref, int K) {
    if (w > K || (int)pref.size() <= 1) {
        ndp = dp;
        return;
    }

    int m = (int)pref.size() - 1;
    ndp.assign(K + 1, NEG);

    for (int rem = 0; rem < w && rem <= K; ++rem) {
        int L = (K - rem) / w + 1; // number of states with this remainder

        vector<ll> A(L);
        for (int j = 0; j < L; ++j) {
            A[j] = dp[rem + j * w];
        }

        auto solve = [&](auto&& self, int l, int r, int optL, int optR) -> void {
            if (l > r) return;

            int mid = (l + r) >> 1;

            int lo = max(optL, mid - m);
            if (lo < 0) lo = 0;
            int hi = min(optR, mid);

            ll bestVal = NEG;
            int bestJ = -1;

            if (lo <= hi) {
                for (int j = lo; j <= hi; ++j) {
                    if (A[j] == NEG) continue;

                    ll val = A[j] + pref[mid - j];
                    if (val > bestVal || (val == bestVal && j > bestJ)) {
                        bestVal = val;
                        bestJ = j;
                    }
                }
            }

            if (bestJ != -1) {
                ndp[rem + mid * w] = bestVal;
            } else {
                // No reachable predecessor in this interval.
                if (optL > mid) bestJ = optL;
                else bestJ = min(mid, optR);
            }

            if (bestJ < optL) bestJ = optL;
            if (bestJ > optR) bestJ = optR;

            self(self, l, mid - 1, optL, bestJ);
            self(self, mid + 1, r, bestJ, optR);
        };

        solve(solve, 0, L - 1, 0, L - 1);
    }
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, k;
    cin >> n >> k;

    vector<vector<Item>> groups(11);

    int totalP = 0;
    ll totalFull = 0;

    for (int i = 0; i < n; ++i) {
        int p;
        cin >> p;

        Item it;
        for (int j = 1; j <= p; ++j) {
            cin >> it.b[j];
        }

        it.full = it.b[p];
        groups[p].push_back(it);

        totalP += p;
        totalFull += it.full;
    }

    // If all items can be fully upgraded, the answer is simply the sum
    // of all full bonuses.
    if (totalP <= k) {
        cout << totalFull << '\n';
        return 0;
    }

    // Sort full bonuses inside every weight class in decreasing order.
    for (int w = 1; w <= 10; ++w) {
        sort(groups[w].begin(), groups[w].end(),
             [](const Item& a, const Item& b) {
                 return a.full > b.full;
             });
    }

    // Prefix sums of full bonuses for each weight class.
    vector<vector<ll>> prefAll(11);
    for (int w = 1; w <= 10; ++w) {
        int m = (int)groups[w].size();
        prefAll[w].assign(m + 1, 0);

        for (int i = 0; i < m; ++i) {
            prefAll[w][i + 1] = prefAll[w][i] + groups[w][i].full;
        }
    }

    int K = k;

    // prefix[w][s] = best full bonus using weights 1..w with exact weight s.
    vector<vector<ll>> prefix(11);
    prefix[0].assign(K + 1, NEG);
    prefix[0][0] = 0;

    for (int w = 1; w <= 10; ++w) {
        add_class(prefix[w - 1], prefix[w], w, prefAll[w], K);
    }

    // Case 1: no partial item; prefix sum exactly reaches k.
    ll ans = prefix[10][k];

    // Case 2: one item is partially upgraded while crossing k.
    for (int p = 2; p <= 10; ++p) {
        int m = (int)groups[p].size();
        if (m == 0) continue;

        int maxR = min(p - 1, k);
        if (maxR < 1) continue;

        /*
            work = DP using all weight classes except p.
            We start from classes 1..p-1 and add p+1..10.
        */
        vector<ll> work = prefix[p - 1];
        vector<ll> tmp;

        for (int w = p + 1; w <= 10; ++w) {
            add_class(work, tmp, w, prefAll[w], K);
            work.swap(tmp);
        }

        for (int r = 1; r <= maxR; ++r) {
            int t = k - r; // total weight of fully upgraded items

            int L = min(t / p, m - 1);

            vector<ll> G(L + 1);
            vector<ll> prefA(L + 1, NEG);
            vector<ll> suffixB(L + 2, NEG);

            for (int c = 0; c <= L; ++c) {
                int s = t - c * p;
                ll g = work[s];
                G[c] = g;

                if (g != NEG) {
                    ll cur = g + prefAll[p][c];
                    ll prev = (c == 0 ? NEG : prefA[c - 1]);
                    prefA[c] = max(prev, cur);
                } else {
                    if (c > 0) prefA[c] = prefA[c - 1];
                }
            }

            for (int c = L; c >= 0; --c) {
                ll val = NEG;
                if (G[c] != NEG) {
                    val = G[c] + prefAll[p][c + 1];
                }
                suffixB[c] = max(suffixB[c + 1], val);
            }

            for (int idx = 0; idx < m; ++idx) {
                int rank = idx + 1; // 1-based rank by full bonus
                ll fullValue = groups[p][idx].full;

                ll bestFull = NEG;

                // c < rank: item idx is not among the selected top c.
                int ca = min(rank - 1, L);
                if (ca >= 0) bestFull = max(bestFull, prefA[ca]);

                // c >= rank: selected top c excluding this item can be
                // represented as pref[c + 1] - fullValue.
                if (rank <= L && suffixB[rank] != NEG) {
                    bestFull = max(bestFull, suffixB[rank] - fullValue);
                }

                if (bestFull != NEG) {
                    ll cand = bestFull + groups[p][idx].b[r];
                    ans = max(ans, cand);
                }
            }
        }
    }

    if (ans == NEG) ans = 0;
    cout << ans << '\n';

    return 0;
}
