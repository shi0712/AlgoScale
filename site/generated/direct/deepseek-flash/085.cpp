#include <bits/stdc++.h>
using namespace std;

static const int MAXN = 2005;

struct FenwickParity {
    int n;
    vector<int> bit;
    void init(int n_) {
        n = n_;
        bit.assign(n + 2, 0);
    }
    void add(int i) {
        for (; i <= n; i += i & -i) bit[i] ^= 1;
    }
    int sum(int i) const {
        int s = 0;
        for (; i > 0; i -= i & -i) s ^= bit[i];
        return s;
    }
};

int cntAct[4 * MAXN], onesAct[4 * MAXN];
bool lazyFlip[4 * MAXN];

void buildSeg(int idx, int l, int r, const vector<char>& active) {
    lazyFlip[idx] = false;
    if (l == r) {
        cntAct[idx] = active[l];
        onesAct[idx] = 0;
        return;
    }
    int mid = (l + r) >> 1;
    buildSeg(idx << 1, l, mid, active);
    buildSeg(idx << 1 | 1, mid + 1, r, active);
    cntAct[idx] = cntAct[idx << 1] + cntAct[idx << 1 | 1];
    onesAct[idx] = 0;
}

inline void applyFlip(int idx) {
    lazyFlip[idx] = !lazyFlip[idx];
    onesAct[idx] = cntAct[idx] - onesAct[idx];
}

inline void push(int idx) {
    if (lazyFlip[idx]) {
        applyFlip(idx << 1);
        applyFlip(idx << 1 | 1);
        lazyFlip[idx] = false;
    }
}

void flipPrefix(int idx, int l, int r, int qr) {
    if (qr < l) return;
    if (r <= qr) {
        applyFlip(idx);
        return;
    }
    push(idx);
    int mid = (l + r) >> 1;
    flipPrefix(idx << 1, l, mid, qr);
    if (qr > mid) flipPrefix(idx << 1 | 1, mid + 1, r, qr);
    onesAct[idx] = onesAct[idx << 1] + onesAct[idx << 1 | 1];
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    cin >> n;

    vector<int> rankAt(n + 1, 0);

    for (int k = 1; k <= n; ++k) {
        if (k == 1) {
            rankAt[1] = 1;
            continue;
        }

        // suffixInv[l] = parity of inversions in rankAt[l..k-1]
        vector<int> suffixInv(k, 0);
        FenwickParity fw;
        fw.init(k);

        int invParity = 0;
        for (int i = k - 1; i >= 1; --i) {
            int r = rankAt[i];
            int smallerToRight = fw.sum(r - 1);
            invParity ^= smallerToRight;
            suffixInv[i] = invParity;
            fw.add(r);
        }

        vector<char> active(k, 1);
        int activeCnt = k;
        bool firstQuery = true;

        while (activeCnt > 1) {
            int bestL = -1;

            if (activeCnt == 2) {
                int d1 = -1, d2 = -1;
                for (int d = 0; d < k; ++d) {
                    if (active[d]) {
                        if (d1 == -1) d1 = d;
                        else {
                            d2 = d;
                            break;
                        }
                    }
                }
                int low = min(d1, d2);
                int high = max(d1, d2);

                int parity = 0;
                for (int l = k - 1; l >= 1; --l) {
                    int r = rankAt[l];
                    if (r > low && r <= high) parity ^= 1;
                    if (parity) {
                        bestL = l;
                        break;
                    }
                }
            } else if (firstQuery) {
                // Querying l = 1 splits candidates by parity, which is optimal.
                bestL = 1;
            } else {
                buildSeg(1, 0, k - 1, active);

                int bestScore = INT_MAX;
                for (int l = k - 1; l >= 1; --l) {
                    int r = rankAt[l]; // rank in 1..k-1
                    flipPrefix(1, 0, k - 1, r - 1);

                    int ones = onesAct[1];
                    int score = max(ones, activeCnt - ones);

                    if (score < bestScore) {
                        bestScore = score;
                        bestL = l;
                    }
                }
            }

            firstQuery = false;

            cout << "? " << bestL << ' ' << k << endl;
            int ans;
            cin >> ans;
            if (ans == -1) return 0;

            int realR = ans ^ suffixInv[bestL];

            // present[r] = parity of occurrences of rank r in positions bestL..k-1
            vector<char> present(k + 1, 0);
            for (int i = bestL; i <= k - 1; ++i) {
                present[rankAt[i]] ^= 1;
            }

            // greater[d] = parity of count of ranks > d in positions bestL..k-1
            vector<char> greater(k, 0);
            int cur = 0;
            for (int d = k - 1; d >= 0; --d) {
                greater[d] = cur;
                if (d >= 1 && present[d]) cur ^= 1;
            }

            vector<char> newActive(k, 0);
            int newCnt = 0;
            for (int d = 0; d < k; ++d) {
                if (active[d] && greater[d] == realR) {
                    newActive[d] = 1;
                    ++newCnt;
                }
            }

            active.swap(newActive);
            activeCnt = newCnt;
        }

        int d = -1;
        for (int i = 0; i < k; ++i) {
            if (active[i]) {
                d = i;
                break;
            }
        }

        int newRank = d + 1; // rank among first k elements, 1-indexed

        for (int pos = 1; pos < k; ++pos) {
            if (rankAt[pos] >= newRank) ++rankAt[pos];
        }
        rankAt[k] = newRank;
    }

    cout << "!";
    for (int i = 1; i <= n; ++i) {
        cout << ' ' << rankAt[i];
    }
    cout << endl;

    return 0;
}
