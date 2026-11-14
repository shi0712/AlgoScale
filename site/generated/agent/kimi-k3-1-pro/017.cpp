#include <bits/stdc++.h>
using namespace std;

using int64 = long long;
constexpr int64 NEG = numeric_limits<int64>::min() / 4;

struct Value {
    int64 deficiency;
};

struct SuitData {
    vector<Value> lowNon, highNon, lowAll, highAll;
    vector<int64> highA, highDNon, highDAll;
};

static Value combine(const Value& low, const Value& high, int64 highTotal) {
    int64 fromLow = low.deficiency == NEG ? NEG : low.deficiency + highTotal;
    if (high.deficiency >= fromLow) return high;
    return {fromLow};
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, m;
    cin >> n >> m;
    vector<int> trump(n);
    for (int& x : trump) cin >> x;

    vector<vector<int64>> attacker(n, vector<int64>(m + 1));
    vector<vector<int64>> defender(n, vector<int64>(m + 1));
    for (int s = 0; s < n; ++s)
        for (int r = 1; r <= m; ++r) cin >> attacker[s][r];
    for (int s = 0; s < n; ++s)
        for (int r = 1; r <= m; ++r) cin >> defender[s][r];

    const int left = m / 2;
    const int right = m - left;
    const int lowSize = 1 << left;
    const int highSize = 1 << right;
    const int lowMask = lowSize - 1;

    vector<SuitData> data(n);
    for (int s = 0; s < n; ++s) {
        SuitData& d = data[s];
        d.lowNon.resize(lowSize);
        d.lowAll.resize(lowSize);
        d.highNon.resize(highSize);
        d.highAll.resize(highSize);
        d.highA.assign(highSize, 0);
        d.highDNon.assign(highSize, 0);
        d.highDAll.assign(highSize, 0);

        for (int mask = 0; mask < lowSize; ++mask) {
            Value non{NEG}, all{NEG};
            for (int k = 1; k <= left; ++k) {
                int64 a = 0, dn = 0, da = 0;
                for (int r = k; r <= left; ++r) {
                    if (!(mask & (1 << (r - 1)))) continue;
                    a += attacker[s][r];
                    if (r > k) {
                        if (!trump[s]) dn += defender[s][r];
                        da += defender[s][r];
                    }
                }
                int64 hn = a - dn;
                int64 ha = a - da;
                if (hn >= non.deficiency) non = {hn};
                if (ha >= all.deficiency) all = {ha};
            }
            d.lowNon[mask] = non;
            d.lowAll[mask] = all;
        }

        for (int mask = 0; mask < highSize; ++mask) {
            Value non{NEG}, all{NEG};
            for (int k = left + 1; k <= m; ++k) {
                int64 a = 0, dn = 0, da = 0;
                for (int r = k; r <= m; ++r) {
                    if (!(mask & (1 << (r - left - 1)))) continue;
                    a += attacker[s][r];
                    if (r > k) {
                        if (!trump[s]) dn += defender[s][r];
                        da += defender[s][r];
                    }
                }
                int64 hn = a - dn;
                int64 ha = a - da;
                if (hn >= non.deficiency) non = {hn};
                if (ha >= all.deficiency) all = {ha};
            }
            d.highNon[mask] = non;
            d.highAll[mask] = all;

            if (mask) {
                int bit = __builtin_ctz(mask);
                int previous = mask & (mask - 1);
                int r = left + bit + 1;
                d.highA[mask] = d.highA[previous] + attacker[s][r];
                d.highDNon[mask] = d.highDNon[previous] + (trump[s] ? 0 : defender[s][r]);
                d.highDAll[mask] = d.highDAll[previous] + defender[s][r];
            }
        }
    }

    const int subsets = 1 << m;
    vector<int64> trumpDefenders(subsets, 0);
    for (int mask = 1; mask < subsets; ++mask) {
        int bit = __builtin_ctz(mask);
        int previous = mask & (mask - 1);
        int64 add = 0;
        for (int s = 0; s < n; ++s)
            if (trump[s]) add += defender[s][bit + 1];
        trumpDefenders[mask] = trumpDefenders[previous] + add;
    }

    int defendedRanks = 0;
    vector<Value> non(n), all(n);
    for (int mask = 0; mask < subsets; ++mask) {
        int lm = mask & lowMask;
        int hm = mask >> left;
        int64 availableTrump = trumpDefenders[mask];
        int64 regularUnmatched = 0;
        int64 trumpDeficiency = 0;

        for (int s = 0; s < n; ++s) {
            const SuitData& d = data[s];
            non[s] = combine(d.lowNon[lm], d.highNon[hm],
                             d.highA[hm] - d.highDNon[hm]);
            if (trump[s]) {
                all[s] = combine(d.lowAll[lm], d.highAll[hm],
                                 d.highA[hm] - d.highDAll[hm]);
                int64 beforeTrump = max<int64>(0, non[s].deficiency);
                int64 afterTrump = max<int64>(0, all[s].deficiency);
                availableTrump -= beforeTrump - afterTrump;
                trumpDeficiency += afterTrump;
            } else {
                regularUnmatched += max<int64>(0, non[s].deficiency);
            }
        }

        int64 regularDeficiency = max<int64>(0, regularUnmatched - availableTrump);
        int64 totalDeficiency = trumpDeficiency + regularDeficiency;
        if (totalDeficiency == 0) defendedRanks |= mask;
    }

    int64 answer = 0;
    for (int s = 0; s < n; ++s)
        for (int r = 1; r <= m; ++r)
            if (!(defendedRanks & (1 << (r - 1)))) answer += attacker[s][r];
    cout << answer << '\n';
    return 0;
}
