#include <bits/stdc++.h>
using namespace std;

int n;
int w[20][20];
int adjMask[20];
double ansValue = 0.0;

void solveClique(const vector<int>& verts) {
    int k = (int)verts.size();
    if (k < 2) return;

    double a[20][21];

    for (int i = 0; i < k; ++i) {
        int vi = verts[i];
        for (int j = 0; j < k; ++j) {
            a[i][j] = (double)w[vi][verts[j]];
        }
        a[i][k] = 1.0;
    }

    // Gaussian elimination
    for (int col = 0; col < k; ++col) {
        int piv = col;
        double best = fabs(a[col][col]);

        for (int r = col + 1; r < k; ++r) {
            double v = fabs(a[r][col]);
            if (v > best) {
                best = v;
                piv = r;
            }
        }

        if (best < 1e-12) return;

        if (piv != col) {
            for (int j = col; j <= k; ++j) {
                swap(a[col][j], a[piv][j]);
            }
        }

        double pivot = a[col][col];

        for (int r = col + 1; r < k; ++r) {
            double factor = a[r][col] / pivot;
            if (fabs(factor) < 1e-18) continue;
            a[r][col] = 0.0;
            for (int j = col + 1; j <= k; ++j) {
                a[r][j] -= factor * a[col][j];
            }
        }
    }

    double y[20];
    double sum = 0.0;

    for (int i = k - 1; i >= 0; --i) {
        double val = a[i][k];
        for (int j = i + 1; j < k; ++j) {
            val -= a[i][j] * y[j];
        }
        val /= a[i][i];

        if (val < -1e-9) return;
        if (val < 0.0) val = 0.0;

        y[i] = val;
        sum += val;
    }

    if (sum <= 1e-15) return;

    double cur = 0.5 / sum;
    if (cur > ansValue) ansValue = cur;
}

void dfs(int mask, int allowed, vector<int>& verts) {
    int U = mask | allowed;
    int m = __builtin_popcount((unsigned)U);

    if (m < 2) return;

    int arr[20], cnt = 0;
    int tmp = U;
    while (tmp) {
        int v = __builtin_ctz((unsigned)tmp);
        tmp &= tmp - 1;
        arr[cnt++] = v;
    }

    int maxW = 0;
    for (int i = 0; i < cnt; ++i) {
        int vi = arr[i];
        for (int j = i + 1; j < cnt; ++j) {
            maxW = max(maxW, w[vi][arr[j]]);
        }
    }

    if (maxW == 0) return;

    double upperBound = 1.0 * maxW * (1.0 - 1.0 / m) * 0.5;
    if (upperBound <= ansValue + 1e-12) return;

    if ((int)verts.size() >= 2) {
        solveClique(verts);
    }

    int curAllowed = allowed;

    while (curAllowed) {
        int v = __builtin_ctz((unsigned)curAllowed);
        curAllowed &= curAllowed - 1;

        int newAllowed = curAllowed & adjMask[v];

        verts.push_back(v);
        dfs(mask | (1 << v), newAllowed, verts);
        verts.pop_back();
    }
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    cin >> n;

    int maxEdge = 0;

    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            cin >> w[i][j];
            if (i < j) maxEdge = max(maxEdge, w[i][j]);
        }
    }

    for (int i = 0; i < n; ++i) {
        adjMask[i] = 0;
        for (int j = 0; j < n; ++j) {
            if (i != j && w[i][j] > 0) {
                adjMask[i] |= (1 << j);
            }
        }
    }

    ansValue = maxEdge * 0.25;

    vector<int> verts;
    verts.reserve(n);

    dfs(0, (1 << n) - 1, verts);

    cout << fixed << setprecision(12) << ansValue << '\n';

    return 0;
}
