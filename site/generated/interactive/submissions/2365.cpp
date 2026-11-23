#include <bits/stdc++.h>
using namespace std;

int ask(int i, int j) {
    printf("? %d %d\n", i, j);
    fflush(stdout);
    int r;
    if (scanf("%d", &r) != 1) exit(0);
    if (r == -1) exit(0);
    return r;
}

int main() {
    int n;
    if (scanf("%d", &n) != 1) return 0;
    int a = 1, b = 2;
    int known = -1;
    for (int c = 3; c <= n; c++) {
        int u = known >= 0 ? known : ask(a, b);
        int w = ask(b, c);
        if (u < w) {
            // eliminate a
            a = b;
            b = c;
            known = w;
        } else if (u > w) {
            // eliminate c
            known = u;
        } else {
            // eliminate b
            b = c;
            known = -1;
        }
    }
    printf("! %d %d\n", a, b);
    fflush(stdout);
    int r;
    if (scanf("%d", &r) != 1) return 0;
    return 0;
}
