#include <bits/stdc++.h>
using namespace std;

int n;

int ask(int i, int j) {
    printf("? %d %d\n", i, j);
    fflush(stdout);
    int x;
    if (scanf("%d", &x) != 1 || x == -1) exit(0);
    return x;
}

int main() {
    if (scanf("%d", &n) != 1) return 0;

    if (n == 1) {
        printf("!\n1\n0\n");
        fflush(stdout);
        return 0;
    }
    if (n == 2) {
        printf("!\n2\n0 1\n");
        fflush(stdout);
        return 0;
    }

    vector<int> A(n);
    for (int i = 0; i < n; ++i) A[i] = ask(i, 0);

    bool power2 = ((n & (n - 1)) == 0);
    vector<int> p(n);
    int cnt;

    if (!power2) {
        int found = -1;
        for (int x = 0; x < n; ++x) {
            vector<char> used(n, 0);
            bool ok = true;
            for (int i = 0; i < n; ++i) {
                int v = A[i] ^ x;
                if (v < 0 || v >= n || used[v]) {
                    ok = false;
                    break;
                }
                used[v] = 1;
            }
            if (ok) {
                found = x;
                break;
            }
        }
        for (int i = 0; i < n; ++i) p[i] = A[i] ^ found;
        cnt = 1;
    } else {
        vector<int> invA(n);
        for (int i = 0; i < n; ++i) invA[A[i]] = i;

        vector<char> H(n, 1);

        while (true) {
            int d0 = -1;
            for (int d = 0; d < n; ++d) {
                if (H[d]) {
                    d0 = d;
                    break;
                }
            }

            vector<int> pp(n);
            for (int i = 0; i < n; ++i) pp[i] = A[i] ^ d0;

            vector<int> b(n);
            for (int i = 0; i < n; ++i) b[pp[i]] = i;

            bool allEq = true;
            for (int x = 0; x < n; ++x) {
                if (!H[x]) continue;
                int d = x ^ d0;
                bool ok = true;
                for (int j = 0; j < n; ++j) {
                    if (b[j ^ d] != (b[j] ^ d)) {
                        ok = false;
                        break;
                    }
                }
                if (!ok) {
                    allEq = false;
                    break;
                }
            }
            if (allEq) break;

            int chosenJ = -1;
            for (int j = 0; j < n && chosenJ == -1; ++j) {
                int first = -1;
                for (int x = 0; x < n; ++x) {
                    if (!H[x]) continue;
                    int val = A[0] ^ x ^ invA[j ^ x];
                    if (first == -1) first = val;
                    else if (val != first) {
                        chosenJ = j;
                        break;
                    }
                }
            }

            int val = ask(0, chosenJ);

            vector<char> newH(n, 0);
            for (int x = 0; x < n; ++x) {
                if (!H[x]) continue;
                int pred = A[0] ^ x ^ invA[chosenJ ^ x];
                if (pred == val) newH[x] = 1;
            }
            H.swap(newH);
        }

        int xfinal = -1;
        for (int d = 0; d < n; ++d) {
            if (H[d]) {
                xfinal = d;
                break;
            }
        }

        for (int i = 0; i < n; ++i) p[i] = A[i] ^ xfinal;

        vector<int> b(n);
        for (int i = 0; i < n; ++i) b[p[i]] = i;

        cnt = 0;
        for (int t = 0; t < n; ++t) {
            bool ok = true;
            for (int j = 0; j < n; ++j) {
                if (b[j ^ t] != (b[j] ^ t)) {
                    ok = false;
                    break;
                }
            }
            if (ok) ++cnt;
        }
    }

    printf("!\n%d\n", cnt);
    for (int i = 0; i < n; ++i) {
        if (i) printf(" ");
        printf("%d", p[i]);
    }
    printf("\n");
    fflush(stdout);
    return 0;
}
