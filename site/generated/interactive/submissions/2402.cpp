#include <bits/stdc++.h>
using namespace std;

int n;
int ask(int l, int r, int k) {
    printf("? %d %d %d\n", l, r, k);
    fflush(stdout);
    int x;
    if (scanf("%d", &x) != 1) exit(0);
    if (x < 0) exit(0);
    return x;
}

int main() {
    if (scanf("%d", &n) != 1) return 0;
    int pn = -1;
    if (n % 2 == 0) {
        int lo = 1, hi = n;
        while (lo < hi) {
            int mid = (lo + hi) / 2;
            bool inLeft;
            if (mid >= 2) {
                inLeft = (ask(1, mid, n) == 2);
            } else {
                // lo=1, hi=2
                inLeft = !(ask(2, 3, n) == 2);
            }
            if (inLeft) hi = mid; else lo = mid + 1;
        }
        pn = lo;
    }
    int Dall = n / 2 + 1;
    int lo = 1, hi = n;
    while (lo < hi) {
        int mid = (lo + hi) / 2;
        int a = ask(1, mid, 2);
        int b = ask(mid + 1, n, 2);
        int s = Dall - mid + a - b;
        if (pn != -1 && pn <= mid) s -= 1;
        if (s >= 1) hi = mid; else lo = mid + 1;
    }
    printf("! %d\n", lo);
    fflush(stdout);
    return 0;
}
