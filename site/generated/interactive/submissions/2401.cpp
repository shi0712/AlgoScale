#include <bits/stdc++.h>
using namespace std;

static char ask(int i, int j) {
    printf("? %d %d\n", i, j);
    fflush(stdout);
    char c;
    if (scanf(" %c", &c) != 1) exit(0);
    if (c != '<' && c != '>') exit(0);
    return c;
}

int main() {
    int n;
    if (scanf("%d", &n) != 1) return 0;
    if (n == 1) {
        printf("! 1\n");
        fflush(stdout);
        return 0;
    }
    int m1, m2;
    vector<int> dropped;
    {
        char c = ask(n - 1, n);
        if (c == '<') {
            m1 = n - 1;
            m2 = n;
        } else {
            m1 = n;
            m2 = n - 1;
        }
    }
    for (int i = n - 2; i >= 1; i--) {
        char c = ask(i, m1);
        if (c == '<') {
            dropped.push_back(m2);
            m2 = m1;
            m1 = i;
        } else {
            char d = ask(i, m2);
            if (d == '<') {
                dropped.push_back(m2);
                m2 = i;
            } else {
                dropped.push_back(i);
            }
        }
    }
    vector<int> order;
    order.push_back(m1);
    order.push_back(m2);
    for (int k = (int)dropped.size() - 1; k >= 0; k--) order.push_back(dropped[k]);
    vector<int> b(n + 1);
    for (int r = 0; r < (int)order.size(); r++) b[order[r]] = r + 1;
    printf("!");
    for (int i = 1; i <= n; i++) printf(" %d", b[i]);
    printf("\n");
    fflush(stdout);
    return 0;
}
