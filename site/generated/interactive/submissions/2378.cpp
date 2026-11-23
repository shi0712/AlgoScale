#include <bits/stdc++.h>
using namespace std;

static char q(int c) {
    printf("? %d\n", c);
    fflush(stdout);
    char ch;
    if (scanf(" %c", &ch) != 1) exit(0);
    if (ch == 'E') exit(0);
    return ch;
}
static void rst() {
    printf("R\n");
    fflush(stdout);
}

int main() {
    int n, k;
    if (scanf("%d %d", &n, &k) != 2) return 0;
    int s = max(1, k / 2);
    vector<int> D;
    int nxt = 1;
    while (nxt <= n) {
        int e = min(n, nxt + s - 1);
        rst();
        vector<int> alive;
        for (int i = nxt; i <= e; i++) {
            if (q(i) == 'N') alive.push_back(i);
        }
        nxt = e + 1;
        size_t pos = 0;
        while (pos < D.size() && !alive.empty()) {
            int a = alive.size();
            int c = max(1, k - a);
            c = min<int>(c, D.size() - pos);
            rst();
            for (int j = 0; j < c; j++) q(D[pos + j]);
            pos += c;
            vector<int> na;
            for (int x : alive) {
                if (q(x) == 'N') na.push_back(x);
            }
            alive = na;
        }
        for (int x : alive) D.push_back(x);
    }
    printf("! %d\n", (int)D.size());
    fflush(stdout);
    return 0;
}
