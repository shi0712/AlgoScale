#include <bits/stdc++.h>
using namespace std;

long long ask(int a, int b, int c) {
    printf("? %d %d %d\n", a, b, c);
    fflush(stdout);
    long long r;
    if (scanf("%lld", &r) != 1) exit(0);
    if (r < 0) exit(0);
    return r;
}

int main() {
    int n;
    if (scanf("%d", &n) != 1) return 0;
    int p = 1, q = 2;
    int i = 3;
    while (i <= n) {
        int c = i, d;
        if (i + 1 <= n) d = i + 1;
        else {
            d = 1;
            while (d == p || d == q || d == c) d++;
        }
        i += 2;
        int a = p;
        int e[3] = {q, c, d};
        long long v[3];
        for (int k = 0; k < 3; k++) {
            int o1 = e[(k + 1) % 3], o2 = e[(k + 2) % 3];
            v[k] = ask(a, o1, o2);
        }
        long long M = max({v[0], v[1], v[2]});
        int cnt = 0;
        for (int k = 0; k < 3; k++) if (v[k] == M) cnt++;
        if (cnt == 3) {
            p = a; q = e[0];
        } else if (cnt == 2) {
            int k = 0;
            for (int t = 0; t < 3; t++) if (v[t] < M) k = t;
            p = a; q = e[k];
        } else {
            vector<int> r;
            for (int t = 0; t < 3; t++) if (v[t] < M) r.push_back(e[t]);
            p = r[0]; q = r[1];
        }
    }
    printf("! %d %d\n", p, q);
    fflush(stdout);
    return 0;
}
