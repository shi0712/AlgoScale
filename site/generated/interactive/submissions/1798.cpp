#include <bits/stdc++.h>
using namespace std;

static const int MAXN = 1000000000;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int k;
    if (!(cin >> k)) return 0;

    int W = MAXN / 32 + 2;
    vector<uint32_t> bits(W, 0);

    int countC = 0;
    auto setBit = [&](uint32_t v) {
        bits[v >> 5] |= 1u << (v & 31);
    };

    if (k >= 0 && k <= 30) {
        uint64_t x = (k == 0 ? 0 : ((1ULL << k) - 1));
        uint64_t limit = (1ULL << 30) - 1;

        while (x <= limit) {
            if (x >= 1 && x <= MAXN) {
                setBit((uint32_t)x);
                ++countC;
            }

            if (k == 0) break;

            uint64_t u = x & (~x + 1);
            if (u == 0) break;

            uint64_t v = x + u;
            int sh = __builtin_ctzll(u);
            x = v + (((x ^ v) >> sh) >> 2);
        }
    }

    if (countC == 0) {
        return 0;
    }

    auto findMin = [&]() -> uint32_t {
        for (int i = 0; i < W; ++i) {
            if (bits[i]) {
                return (uint32_t(i) << 5) + __builtin_ctz(bits[i]);
            }
        }
        return 0;
    };

    auto findMax = [&](uint32_t upper) -> uint32_t {
        int w = int(upper >> 5);
        if (w >= W) w = W - 1;

        uint32_t mask;
        if ((upper & 31) == 31) mask = 0xFFFFFFFFu;
        else mask = (1u << (((upper & 31) + 1))) - 1u;

        uint32_t val = bits[w] & mask;
        while (val == 0) {
            if (w == 0) return 0;
            --w;
            val = bits[w];
        }

        return (uint32_t(w) << 5) + (31 - __builtin_clz(val));
    };

    auto applyFilter = [&](uint32_t x, int obs, uint32_t oldMax) {
        int oldMaxWord = int(oldMax >> 5);

        for (int w = 0; w <= oldMaxWord; ++w) {
            uint32_t old = bits[w];
            if (old == 0) continue;

            bits[w] = 0;
            uint32_t temp = old;

            while (temp) {
                int b = __builtin_ctz(temp);
                temp &= temp - 1;

                uint32_t r = (uint32_t(w) << 5) + uint32_t(b);
                if (r > oldMax) continue;

                uint32_t nr = r - x;
                if (__builtin_popcount(nr) == obs) {
                    bits[nr >> 5] |= 1u << (nr & 31);
                }
            }
        }
    };

    uint32_t minR = findMin();
    uint32_t maxR = findMax(MAXN);

    long long subtracted = 0;

    if (countC == 1) {
        cout << "! " << minR << '\n';
        cout.flush();
        return 0;
    }

    mt19937 rng(712367);

    bool firstQuery = true;
    bool forceMin = false;

    while (true) {
        if (countC == 1) {
            long long answer = subtracted + minR;
            cout << "! " << answer << '\n';
            cout.flush();
            return 0;
        }

        vector<uint32_t> xs;
        auto addx = [&](uint32_t v) {
            if (v < 1 || v > minR) return;
            for (uint32_t y : xs) {
                if (y == v) return;
            }
            xs.push_back(v);
        };

        auto addRandom = [&]() {
            if (minR == 0) return;
            uint32_t v = 1 + (uint32_t)(rng() % minR);
            addx(v);
        };

        if (forceMin) {
            addx(minR);
        } else if (minR <= 64 && countC <= 10000000) {
            for (uint32_t v = 1; v <= minR; ++v) {
                xs.push_back(v);
            }
        } else {
            long long opsLimit = (firstQuery ? 350000000LL : 60000000LL);
            int limit = int(opsLimit / max(1, countC));
            if (limit < 3) limit = 3;
            if (limit > 300) limit = 300;

            for (int i = 0; i < limit / 2; ++i) {
                addRandom();
            }

            addx(minR / 2);
            addx(minR / 3);
            addx(1);

            if (countC <= 1000000 || int(xs.size()) < limit) {
                addx(minR);
            }

            for (int b = 0; b < 30 && int(xs.size()) < limit; ++b) {
                uint32_t v = (1u << b);
                if (v <= minR) addx(v);
                if (minR > v) addx(minR - v);
            }

            for (uint32_t v = 3; v <= min(minR, 100u) && int(xs.size()) < limit; ++v) {
                addx(v);
            }

            while (int(xs.size()) < limit && xs.size() < minR) {
                addRandom();
            }

            if (xs.empty()) addx(1);
        }

        if (xs.empty()) {
            addx(minR);
        }

        int m = int(xs.size());
        vector<array<int, 32>> cnts(m);

        for (int i = 0; i < m; ++i) {
            cnts[i].fill(0);
        }

        int maxWord = int(maxR >> 5);

        for (int w = 0; w <= maxWord; ++w) {
            uint32_t temp = bits[w];

            while (temp) {
                int b = __builtin_ctz(temp);
                temp &= temp - 1;

                uint32_t r = (uint32_t(w) << 5) + uint32_t(b);
                if (r > maxR) continue;

                for (int i = 0; i < m; ++i) {
                    uint32_t nr = r - xs[i];
                    int p = __builtin_popcount(nr);
                    ++cnts[i][p];
                }
            }
        }

        int best = 0;
        long long bestMax = LLONG_MAX;
        long long bestSq = LLONG_MAX;

        for (int i = 0; i < m; ++i) {
            int mx = 0;
            long long sq = 0;

            for (int p = 0; p < 32; ++p) {
                int c = cnts[i][p];
                if (c > mx) mx = c;
                sq += 1LL * c * c;
            }

            if (mx < bestMax || (mx == bestMax && sq < bestSq)) {
                bestMax = mx;
                bestSq = sq;
                best = i;
            }
        }

        uint32_t chosen = xs[best];
        int before = countC;

        cout << "- " << chosen << '\n';
        cout.flush();

        int obs;
        cin >> obs;

        if (obs < 0) return 0;

        int after = 0;
        if (obs >= 0 && obs < 32) {
            after = cnts[best][obs];
        }

        uint32_t oldMax = maxR;

        applyFilter(chosen, obs, oldMax);

        subtracted += chosen;
        countC = after;

        minR = findMin();

        if (countC > 0) {
            uint32_t upper = oldMax - chosen;
            maxR = findMax(upper);
        }

        if (countC == 1) {
            long long answer = subtracted + minR;
            cout << "! " << answer << '\n';
            cout.flush();
            return 0;
        }

        if (after == before && before > 1) {
            forceMin = true;
        } else {
            forceMin = false;
        }

        firstQuery = false;
    }

    return 0;
}
