#include <bits/stdc++.h>
using namespace std;

static bool query(int c) {
    printf("? %d\n", c);
    fflush(stdout);
    char ch;
    if (scanf(" %c", &ch) != 1) exit(0);
    if (ch == 'E') exit(0);
    return ch == 'Y';
}

static void resetMem() {
    printf("R\n");
    fflush(stdout);
}

int main() {
    int n, k;
    if (scanf("%d %d", &n, &k) != 2) return 0;
    int s = max(1, k / 2);
    vector<int> prevv;
    for (int st = 1; st <= n; st += s) {
        int en = min(n, st + s - 1);
        resetMem();
        vector<int> R;
        for (int c = st; c <= en; c++) {
            if (!query(c)) R.push_back(c);
        }
        size_t idx = 0;
        while (idx < prevv.size() && !R.empty()) {
            int cap = k - (int)R.size();
            if (cap < 1) cap = 1;
            size_t e = min(prevv.size(), idx + (size_t)cap);
            resetMem();
            for (size_t i = idx; i < e; i++) query(prevv[i]);
            vector<int> NR;
            for (int c : R) {
                if (!query(c)) NR.push_back(c);
            }
            R = NR;
            idx = e;
        }
        for (int c : R) prevv.push_back(c);
    }
    printf("! %d\n", (int)prevv.size());
    fflush(stdout);
    return 0;
}
