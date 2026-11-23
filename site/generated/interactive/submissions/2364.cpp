#include <bits/stdc++.h>
using namespace std;

int main() {
    int n;
    if (scanf("%d", &n) != 1) return 0;
    set<int> xs;
    for (int i = 0; i < n; i++) {
        int x, y;
        if (scanf("%d %d", &x, &y) != 2) return 0;
        xs.insert(x);
    }
    vector<int> v(xs.begin(), xs.end());
    long double area = 0;
    for (size_t i = 0; i + 1 < v.size(); i++) {
        int a = v[i], b = v[i + 1];
        printf("? %d %d\n", a + b, 2);
        fflush(stdout);
        long long r, s;
        if (scanf("%lld %lld", &r, &s) != 2) return 0;
        if (r < 0 || s <= 0) return 0;
        area += (long double)(b - a) * (long double)r / (long double)s;
    }
    long long u = llroundl(area * 2);
    long long d = 2;
    long long g = __gcd(u, d);
    if (g == 0) g = 1;
    u /= g;
    d /= g;
    if (u == 0) d = 1;
    printf("! %lld %lld\n", u, d);
    fflush(stdout);
    return 0;
}
