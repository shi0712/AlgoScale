#include <bits/stdc++.h>
using namespace std;

static bool lessq(int a, int b) {
    printf("? %d %d\n", a, b);
    fflush(stdout);
    int r;
    if (scanf("%d", &r) != 1) exit(0);
    if (r < 0) exit(0);
    return r == 1;
}

static void msort(vector<int>& v, int l, int r) {
    if (r - l <= 1) return;
    int m = (l + r) / 2;
    msort(v, l, m);
    msort(v, m, r);
    vector<int> t;
    t.reserve(r - l);
    int i = l, j = m;
    while (i < m && j < r) {
        if (lessq(v[j], v[i])) t.push_back(v[j++]);
        else t.push_back(v[i++]);
    }
    while (i < m) t.push_back(v[i++]);
    while (j < r) t.push_back(v[j++]);
    for (int k = 0; k < (int)t.size(); k++) v[l + k] = t[k];
}

int main() {
    int n;
    if (scanf("%d", &n) != 1) return 0;
    vector<int> v(n);
    for (int i = 0; i < n; i++) v[i] = i + 1;
    msort(v, 0, n);
    deque<int> d(v.begin(), v.end());
    while (d.size() > 1) {
        int a = d.front();
        d.pop_front();
        int b = d.back();
        d.pop_back();
        printf("+ %d %d\n", a, b);
        fflush(stdout);
        int c;
        if (scanf("%d", &c) != 1) return 0;
        if (c < 0) return 0;
        if (d.size() <= 1) {
            d.push_back(c);
        } else {
            int lo = 0, hi = d.size();
            while (lo < hi) {
                int mid = (lo + hi) / 2;
                if (lessq(c, d[mid])) hi = mid;
                else lo = mid + 1;
            }
            d.insert(d.begin() + lo, c);
        }
    }
    printf("!\n");
    fflush(stdout);
    return 0;
}
