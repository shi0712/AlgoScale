#include <bits/stdc++.h>
using namespace std;

using int64 = long long;

struct FastInput {
    static constexpr int BUFSIZE = 1 << 20;
    char buf[BUFSIZE];
    int pos = 0, len = 0;

    inline char getChar() {
        if (pos == len) {
            len = (int)fread(buf, 1, BUFSIZE, stdin);
            pos = 0;
            if (len == 0) return 0;
        }
        return buf[pos++];
    }

    int readInt() {
        char c;
        do c = getChar(); while (c < '0' || c > '9');
        int x = 0;
        while (c >= '0' && c <= '9') {
            x = x * 10 + (c - '0');
            c = getChar();
        }
        return x;
    }
};

struct Evaluation {
    int64 dual;
    int64 q;
};

class Solver {
    static constexpr int MAX_M = 1000000;
    static constexpr int SQRT_LIMIT = 1414;

    int P, Q, Hmax;
    vector<uint32_t> freq;
    vector<uint32_t> sqrtGroup;
    vector<uint16_t> isqrt;
    vector<uint32_t> cnt;
    vector<int32_t> dqAtGain;

    static inline int bitLength(unsigned x) {
        return 32 - __builtin_clz(x);
    }

    // C(a,m) = min_q (floor(a/2^q) + m*q).
    // q is the number of halving operations whose gain is strictly greater than m.
    static inline void adjustedCost(int a, int m, int &cost, int &q) {
        unsigned threshold = 2u * (unsigned)m + 1u;
        if ((unsigned)a < threshold) {
            cost = a;
            q = 0;
            return;
        }
        q = bitLength((unsigned)a / threshold);
        cost = (a >> q) + m * q;
    }

public:
    Solver(int p, int q, vector<uint32_t> &&f, int hmax)
        : P(p), Q(q), Hmax(hmax), freq(std::move(f)) {

        isqrt.resize(Hmax + 1);
        int r = 0;
        for (int x = 0; x <= Hmax; ++x) {
            while ((r + 1) * (r + 1) <= x) ++r;
            isqrt[x] = (uint16_t)r;
        }

        sqrtGroup.assign(SQRT_LIMIT + 1, 0);
        for (int h = 0; h <= Hmax; ++h) {
            if (freq[h]) sqrtGroup[isqrt[h]] += freq[h];
        }

        cnt.resize(Hmax + 1);
        dqAtGain.resize(Hmax + 1);
    }

    Evaluation evaluate(int m) {
        fill(cnt.begin(), cnt.end(), 0);
        fill(dqAtGain.begin(), dqAtGain.end(), 0);

        int C[SQRT_LIMIT + 1];
        int8_t Qs[SQRT_LIMIT + 1];

        for (int a = 0; a <= SQRT_LIMIT; ++a) {
            int q;
            adjustedCost(a, m, C[a], q);
            Qs[a] = (int8_t)q;
        }

        int64 baseCost = 0;
        int64 baseQ = 0;
        int maxGain = 0;

        auto addOperations = [&](int gain, int deltaQ, uint32_t weight) {
            if (gain <= 0 || weight == 0) return;
            cnt[gain] += weight;
            dqAtGain[gain] += (int32_t)((int64_t)weight * deltaQ);
            if (gain > maxGain) maxGain = gain;
        };

        // The first square-root operation depends on the original height.
        for (int h = 0; h <= Hmax; ++h) {
            uint32_t weight = freq[h];
            if (!weight) continue;

            int ch, qh;
            adjustedCost(h, m, ch, qh);

            baseCost += (int64)weight * ch;
            baseQ += (int64)weight * qh;

            int r = isqrt[h];
            int gain = ch - C[r];
            int deltaQ = (int)Qs[r] - qh;
            addOperations(gain, deltaQ, weight);
        }

        // All later square-root operations depend only on floor(sqrt(h)).
        for (int r = 0; r <= SQRT_LIMIT; ++r) {
            uint32_t weight = sqrtGroup[r];
            if (!weight) continue;

            int cur = r;
            for (int p = 2; p <= 5; ++p) {
                int nxt = isqrt[cur];
                int gain = C[cur] - C[nxt];
                int deltaQ = (int)Qs[nxt] - (int)Qs[cur];
                addOperations(gain, deltaQ, weight);
                cur = nxt;
            }
        }

        int64 operationsLeft = P;
        int64 selectedGain = 0;
        int64 selectedDeltaQ = 0;

        bool partialGainGroup = false;
        int partialGain = 0;
        int64 tiesNeeded = 0;

        if (operationsLeft > 0) {
            for (int g = maxGain; g >= 1; --g) {
                int64 amount = cnt[g];
                if (amount == 0) continue;

                if (operationsLeft >= amount) {
                    selectedGain += (int64)g * amount;
                    selectedDeltaQ += dqAtGain[g];
                    operationsLeft -= amount;
                    if (operationsLeft == 0) break;
                } else {
                    selectedGain += (int64)g * operationsLeft;
                    tiesNeeded = operationsLeft;
                    partialGain = g;
                    partialGainGroup = true;
                    operationsLeft = 0;
                    break;
                }
            }
        }

        int64 tieDeltaQ = 0;

        // Select the remaining operations of one tied gain value, respecting
        // the order of square-root operations on each mountain.
        if (partialGainGroup && tiesNeeded > 0) {
            for (int h = 0; h <= Hmax && tiesNeeded > 0; ++h) {
                uint32_t weight = freq[h];
                if (!weight) continue;

                int ch, qh;
                adjustedCost(h, m, ch, qh);

                int gains[5], deltas[5];
                int r = isqrt[h];
                gains[0] = ch - C[r];
                deltas[0] = (int)Qs[r] - qh;

                int cur = r;
                for (int j = 1; j < 5; ++j) {
                    int nxt = isqrt[cur];
                    gains[j] = C[cur] - C[nxt];
                    deltas[j] = (int)Qs[nxt] - (int)Qs[cur];
                    cur = nxt;
                }

                int pos = 0;
                while (pos < 5 && gains[pos] > partialGain) ++pos;
                int start = pos;
                while (pos < 5 && gains[pos] == partialGain) ++pos;

                int tieCount = pos - start;
                if (tieCount == 0) continue;

                int prefix[6] = {0, 0, 0, 0, 0, 0};
                for (int j = 0; j < tieCount; ++j) {
                    prefix[j + 1] = prefix[j] + deltas[start + j];
                }
                int sumDelta = prefix[tieCount];

                int64 available = (int64)weight * tieCount;
                if (tiesNeeded >= available) {
                    tieDeltaQ += (int64)weight * sumDelta;
                    tiesNeeded -= available;
                } else {
                    int64 wholeMountains = tiesNeeded / tieCount;
                    int remainder = (int)(tiesNeeded % tieCount);
                    tieDeltaQ += wholeMountains * sumDelta + prefix[remainder];
                    tiesNeeded = 0;
                }
            }
        }

        int64 adjustedMinimum = baseCost - selectedGain;
        int64 qTotal = baseQ + selectedDeltaQ + tieDeltaQ;
        int64 dual = adjustedMinimum - (int64)m * Q;

        return {dual, qTotal};
    }

    int64 solve() {
        if (Q == 0) {
            // At this penalty every halving operation is too expensive.
            return evaluate(MAX_M).dual;
        }

        int lo = 0, hi = MAX_M;
        int64 answer = 0;

        while (lo < hi) {
            int mid = lo + (hi - lo) / 2;
            Evaluation ev = evaluate(mid);
            answer = max(answer, ev.dual);

            if (ev.q == Q) {
                // The relaxed solution is feasible, so it is optimal.
                return ev.dual;
            }

            // qTotal-Q is a supergradient of the concave dual function.
            if (ev.q > Q)
                lo = mid + 1;
            else
                hi = mid;
        }

        Evaluation ev = evaluate(lo);
        answer = max(answer, ev.dual);
        return answer;
    }
};

int main() {
    FastInput in;

    int n = in.readInt();
    int P = in.readInt();
    int Q = in.readInt();

    vector<uint32_t> freq(2000001, 0);
    int Hmax = 0;

    for (int i = 0; i < n; ++i) {
        int h = in.readInt();
        ++freq[h];
        Hmax = max(Hmax, h);
    }

    Solver solver(P, Q, std::move(freq), Hmax);
    printf("%lld\n", solver.solve());
    return 0;
}
