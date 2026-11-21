#include <bits/stdc++.h>
using namespace std;

const int MAXN = 200005;

int L[4 * MAXN], R[4 * MAXN], parentNode[4 * MAXN];
int sumv[4 * MAXN];
bool knownSum[4 * MAXN];
queue<int> q;

void setKnown(int v, int val) {
    if (knownSum[v]) return;
    knownSum[v] = true;
    sumv[v] = val;
    q.push(v);
}

void processQueue() {
    while (!q.empty()) {
        int v = q.front();
        q.pop();

        if (L[v] < R[v]) {
            int lc = v << 1;
            int rc = lc | 1;

            if (knownSum[lc] && !knownSum[rc]) {
                setKnown(rc, sumv[v] - sumv[lc]);
            } else if (knownSum[rc] && !knownSum[lc]) {
                setKnown(lc, sumv[v] - sumv[rc]);
            }
        }

        int p = parentNode[v];
        if (p) {
            int lc = p << 1;
            int sib = (v == lc ? (lc | 1) : lc);

            if (knownSum[p]) {
                if (!knownSum[sib]) {
                    setKnown(sib, sumv[p] - sumv[v]);
                }
            } else {
                if (knownSum[sib]) {
                    setKnown(p, sumv[v] + sumv[sib]);
                }
            }
        }
    }
}

void build(int v, int l, int r) {
    L[v] = l;
    R[v] = r;

    if (l == r) return;

    int m = (l + r) >> 1;
    int lc = v << 1;
    int rc = lc | 1;

    build(lc, l, m);
    build(rc, m + 1, r);

    parentNode[lc] = v;
    parentNode[rc] = v;
}

int ask(int l, int r) {
    cout << "? " << l << " " << r << '\n';
    cout.flush();

    int x;
    cin >> x;

    if (!cin || x == -1) {
        exit(0);
    }

    return x;
}

int findKthZero(int v, int k) {
    if (L[v] == R[v]) {
        return v;
    }

    int lc = v << 1;
    int rc = lc | 1;

    if (!knownSum[lc]) {
        int res = ask(L[lc], R[lc]);
        setKnown(lc, res);
        processQueue();
    }

    int zerosLeft = (R[lc] - L[lc] + 1) - sumv[lc];

    if (k <= zerosLeft) {
        return findKthZero(lc, k);
    } else {
        return findKthZero(rc, k - zerosLeft);
    }
}

void turnZeroIntoOne(int leaf) {
    int v = leaf;

    while (v) {
        if (knownSum[v]) {
            ++sumv[v];
        }
        v = parentNode[v];
    }

    if (!knownSum[leaf]) {
        setKnown(leaf, 1);
    }

    processQueue();
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, t;
    if (!(cin >> n >> t)) return 0;

    build(1, 1, n);

    for (int round = 0; round < t; ++round) {
        int k;
        if (!(cin >> k)) return 0;

        if (k == -1) return 0;

        int leaf = findKthZero(1, k);
        int answer = L[leaf];

        cout << "! " << answer << '\n';
        cout.flush();

        turnZeroIntoOne(leaf);
    }

    return 0;
}
