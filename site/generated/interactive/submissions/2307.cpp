#include <bits/stdc++.h>
using namespace std;

int n, t;
vector<int> zc;
vector<char> kn;

int ask(int l, int r) {
    printf("? %d %d\n", l, r);
    fflush(stdout);
    int s;
    if (scanf("%d", &s) != 1) exit(0);
    if (s == -1) exit(0);
    return (r - l + 1) - s;
}

int descend(int k) {
    int v = 1, l = 1, r = n;
    while (l < r) {
        if (kn[v] && zc[v] == r - l + 1) return l + k - 1;
        int m = (l + r) / 2;
        int L = 2 * v, R = 2 * v + 1;
        int zl;
        if (kn[L]) zl = zc[L];
        else if (kn[v] && kn[R]) { zl = zc[v] - zc[R]; }
        else zl = ask(l, m);
        zc[L] = zl; kn[L] = 1;
        if (kn[v]) { zc[R] = zc[v] - zl; kn[R] = 1; }
        if (k <= zl) { v = L; r = m; }
        else { k -= zl; v = R; l = m + 1; }
    }
    return l;
}

void update(int x) {
    int v = 1, l = 1, r = n;
    while (true) {
        if (kn[v]) zc[v]--;
        if (l == r) break;
        int m = (l + r) / 2;
        if (x <= m) { v = 2 * v; r = m; }
        else { v = 2 * v + 1; l = m + 1; }
    }
}

int main() {
    if (scanf("%d %d", &n, &t) != 2) return 0;
    zc.assign(4 * n + 4, 0);
    kn.assign(4 * n + 4, 0);
    for (int i = 0; i < t; i++) {
        int k;
        if (scanf("%d", &k) != 1) return 0;
        if (k == -1) return 0;
        int x = descend(k);
        printf("! %d\n", x);
        fflush(stdout);
        update(x);
    }
    return 0;
}
