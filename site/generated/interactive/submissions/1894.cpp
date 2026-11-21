#include <bits/stdc++.h>
using namespace std;

#if defined(__x86_64__) && defined(__GNUC__)
#pragma GCC target("popcnt")
#endif

using U = uint64_t;
constexpr int N = 2000;
constexpr int W = (N + 63) / 64;

static U maskAt[N + 1][W];
static int p[N], suffixParity[N + 1];

static int ask(int l, int r) {
    printf("? %d %d\n", l, r);
    fflush(stdout);
    int x;
    if (scanf("%d", &x) != 1 || (x != 0 && x != 1)) exit(0);
    return x;
}

int main() {
    int n;
    if (scanf("%d", &n) != 1 || n < 1 || n > N) return 0;

    p[0] = 1;
    bool increasing = true, decreasing = true;

    for (int k = 1; k < n; ++k) {
        // p[0..k-1] are the relative ranks in the known prefix.
        // Candidate t means exactly t prefix elements are below p[k].
        int words = (k + 64) / 64;

        // Bit t of row l is the parity contributed by pairs (j,k),
        // l <= j < k. Including position l flips exactly the bits t < p[l].
        // Row k is still zero, since earlier iterations wrote only lower rows.
        for (int l = k - 1; l >= 0; --l) {
            int w = p[l] >> 6;
            U *dst = maskAt[l];
            const U *src = maskAt[l + 1];
            for (int j = 0; j < w; ++j) dst[j] = ~src[j];
            dst[w] = src[w] ^ ((U(1) << (p[l] & 63)) - 1);
            for (int j = w + 1; j < words; ++j) dst[j] = src[j];
        }

        U candidates[W];
        int live[W], used = words;
        for (int j = 0; j < words; ++j) {
            candidates[j] = ~U(0);
            live[j] = j;
        }
        if ((k + 1) & 63)
            candidates[words - 1] = (U(1) << ((k + 1) & 63)) - 1;

        int count = k + 1;
        while (count > 1) {
            int l = 0;
            if (count == k + 1) {
                // Row 0 alternates with rank, hence always splits evenly.
                // For a monotone prefix, first test continuation: a single
                // adjacent comparison then certifies the new extreme.
                if (k >= 2 && (increasing || decreasing)) l = k - 1;
            } else {
                int best = -1;
                for (int s = k - 1; s >= 0; --s) {
                    int ones = 0;
                    for (int h = 0; h < used; ++h) {
                        int j = live[h];
                        ones += __builtin_popcountll(candidates[j] & maskAt[s][j]);
                    }
                    int score = min(ones, count - ones);
                    if (score > best) {
                        best = score;
                        l = s;
                        if (best == count / 2) break;
                    }
                }
                // Distinct ranks always differ in some row: take the
                // rightmost position whose value lies between the two cuts.
            }

            int bit = ask(l + 1, k + 1) ^ suffixParity[l];
            int nextUsed = 0;
            count = 0;
            for (int h = 0; h < used; ++h) {
                int j = live[h];
                candidates[j] &= bit ? maskAt[l][j] : ~maskAt[l][j];
                if (candidates[j]) {
                    live[nextUsed++] = j;
                    count += __builtin_popcountll(candidates[j]);
                }
            }
            used = nextUsed;
            if (!count) return 0;
        }

        int w = live[0];
        int t = 64 * w + __builtin_ctzll(candidates[w]);
        increasing = increasing && (t == k);
        decreasing = decreasing && (t == 0);

        for (int l = 0; l < k; ++l) {
            suffixParity[l] ^= (maskAt[l][w] >> (t & 63)) & 1;
            if (p[l] > t) ++p[l];
        }
        p[k] = t + 1;
    }

    printf("!");
    for (int i = 0; i < n; ++i) printf(" %d", p[i]);
    printf("\n");
    fflush(stdout);
    return 0;
}
