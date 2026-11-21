#include <bits/stdc++.h>
using namespace std;

int N, K;
string ans;

const int B = 20;
static unsigned char pc[1 << B];

uint64_t rngState = 123456789123456789ULL;

uint32_t rngNext() {
    rngState ^= rngState << 7;
    rngState ^= rngState >> 9;
    rngState ^= rngState << 8;
    return (uint32_t)(rngState >> 32);
}

int askSubset(uint32_t q, int start, int L) {
    string s(N, 'T');
    for (int i = 0; i < L; ++i) {
        if ((q >> i) & 1u) s[start + i] = 'F';
    }

    cout << s << '\n' << flush;
    int resp;
    if (!(cin >> resp)) exit(0);
    if (resp == -1) exit(0);
    if (resp == N) exit(0);

    int ones = __builtin_popcount(q);
    return (K + ones - resp) / 2;
}

int askInterval(int l, int r) {
    int len = r - l;
    if (len <= 0) return 0;

    string s(N, 'T');
    for (int i = l; i < r; ++i) s[i] = 'F';

    cout << s << '\n' << flush;
    int resp;
    if (!(cin >> resp)) exit(0);
    if (resp == -1) exit(0);
    if (resp == N) exit(0);

    return (K + len - resp) / 2;
}

uint32_t chooseQuery(const vector<uint32_t>& cand, int L) {
    int M = (int)cand.size();
    if (M <= 1) return 0;

    if (M == 2) {
        uint32_t diff = cand[0] ^ cand[1];
        return diff & (~diff + 1u);
    }

    uint32_t all = (1u << L) - 1u;

    int rndCount;
    if (M > 100000) rndCount = 32;
    else if (M > 20000) rndCount = 48;
    else if (M > 2000) rndCount = 64;
    else rndCount = 128;

    vector<uint32_t> qs;
    qs.reserve(rndCount + 4 + (M <= 2000 ? L : 0) + 32);

    uint32_t half = (1u << (L / 2)) - 1u;
    qs.push_back(half);
    qs.push_back(all ^ half);

    for (int i = 0; i < rndCount; ++i) {
        qs.push_back(rngNext() & all);
    }

    int sample = min(16, M);
    for (int i = 0; i < sample; ++i) {
        uint32_t q = cand[rngNext() % M];
        qs.push_back(q);
        qs.push_back(all ^ q);
    }

    if (M <= 2000) {
        for (int i = 0; i < L; ++i) qs.push_back(1u << i);
    }

    int bestMax = M + 1;
    long long bestSq = LLONG_MAX;
    uint32_t bestQ = 0;

    int freq[32];

    for (uint32_t q : qs) {
        if (q == 0 || q == all) continue;

        for (int r = 0; r <= L; ++r) freq[r] = 0;

        for (uint32_t mask : cand) {
            int r = pc[mask & q];
            ++freq[r];
        }

        int mx = 0;
        long long sq = 0;
        for (int r = 0; r <= L; ++r) {
            int f = freq[r];
            if (f > mx) mx = f;
            sq += 1LL * f * f;
        }

        if (mx == M) continue;

        if (mx < bestMax || (mx == bestMax && sq < bestSq)) {
            bestMax = mx;
            bestSq = sq;
            bestQ = q;
        }
    }

    if (bestQ == 0) {
        int bestDiff = INT_MAX;
        for (int i = 0; i < L; ++i) {
            uint32_t bit = 1u << i;
            int cnt = 0;
            for (uint32_t mask : cand) {
                if (mask & bit) ++cnt;
            }
            int other = M - cnt;
            int diff = abs(cnt - other);
            if (diff < bestDiff) {
                bestDiff = diff;
                bestQ = bit;
            }
        }
    }

    return bestQ;
}

void processBlock(int start, int L, int& remT) {
    int remAfter = N - (start + L);

    int low = max(0, remT - remAfter);
    int high = min(L, remT);

    int c;

    if (low == high) {
        c = low;
    } else {
        c = askSubset((1u << L) - 1u, start, L);
    }

    uint32_t finalMask;

    if (c == 0) {
        finalMask = 0;
    } else if (c == L) {
        finalMask = (1u << L) - 1u;
    } else {
        vector<uint32_t> cand;
        uint32_t lim = (1u << L);

        for (uint32_t mask = 0; mask < lim; ++mask) {
            if (pc[mask] == c) cand.push_back(mask);
        }

        if (cand.size() > 1) {
            int halfLen = L / 2;
            uint32_t q = (1u << halfLen) - 1u;
            if (q == 0) q = 1;

            int t = askSubset(q, start, L);

            vector<uint32_t> nxt;
            nxt.reserve(cand.size());

            for (uint32_t mask : cand) {
                if (pc[mask & q] == t) nxt.push_back(mask);
            }

            cand.swap(nxt);
        }

        while (cand.size() > 1) {
            uint32_t q = chooseQuery(cand, L);
            int t = askSubset(q, start, L);

            vector<uint32_t> nxt;
            nxt.reserve(cand.size());

            for (uint32_t mask : cand) {
                if (pc[mask & q] == t) nxt.push_back(mask);
            }

            cand.swap(nxt);
        }

        finalMask = cand[0];
    }

    for (int i = 0; i < L; ++i) {
        ans[start + i] = ((finalMask >> i) & 1u) ? 'T' : 'F';
    }

    remT -= c;
}

void findMinority(int l, int r, int c, char type) {
    int len = r - l;

    if (c == 0) return;

    if (c == len) {
        for (int i = l; i < r; ++i) ans[i] = type;
        return;
    }

    if (len == 1) {
        ans[l] = type;
        return;
    }

    int mid = (l + r) / 2;
    int t = askInterval(l, mid);
    int lenL = mid - l;

    int cL;
    if (type == 'T') {
        cL = t;
    } else {
        cL = lenL - t;
    }

    findMinority(l, mid, cL, type);
    findMinority(mid, r, c - cL, type);
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    cin >> N;

    int LIM = (1 << B);
    pc[0] = 0;
    for (int i = 1; i < LIM; ++i) {
        pc[i] = pc[i >> 1] + (i & 1);
    }

    string initial(N, 'T');
    cout << initial << '\n' << flush;

    int resp;
    if (!(cin >> resp)) return 0;
    if (resp == -1) return 0;
    if (resp == N) return 0;

    K = resp;

    int minority = min(K, N - K);
    int threshold = max(1, N / 25);

    if (minority <= threshold) {
        char type;
        if (K <= N - K) type = 'T';
        else type = 'F';

        ans.assign(N, type == 'T' ? 'F' : 'T');
        findMinority(0, N, minority, type);
    } else {
        ans.assign(N, '?');

        int remT = K;
        int start = 0;

        while (start < N) {
            int L = min(B, N - start);
            processBlock(start, L, remT);
            start += L;
        }
    }

    cout << ans << '\n' << flush;
    cin >> resp;

    return 0;
}
