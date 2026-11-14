#include <bits/stdc++.h>
using namespace std;

class FastInput {
    static constexpr int S = 1 << 16;
    char buf[S]; int pos = 0, len = 0;
public:
    inline char get() {
        if (pos == len) { len = (int)fread(buf, 1, S, stdin); pos = 0; if (!len) return 0; }
        return buf[pos++];
    }
    template<class T> bool read(T &x) {
        char c; do { c = get(); if (!c) return false; } while (c <= ' ');
        x = 0; do { x = x * 10 + (c - '0'); c = get(); } while (c >= '0' && c <= '9');
        return true;
    }
};

int main() {
    FastInput in;
    int n, q; if (!in.read(n)) return 0; in.read(q);
    const int LIMIT = 1000000;
    vector<int> suffix(LIMIT + 2, 0);
    int mx = 0; long long total = 0;
    for (int i = 0, a; i < n; ++i) {
        in.read(a); ++suffix[a]; mx = max(mx, a); total += a;
    }
    for (int i = mx - 1; i >= 1; --i) suffix[i] += suffix[i + 1];
    vector<int> diff(mx + 2, 0);
    for (int t = 1; t <= mx; ++t) {
        int groups = 0;
        for (int x = t; x <= mx; x += t) {
            groups += suffix[x];
            if (groups >= mx) { groups = mx; break; }
        }
        int lo = (mx + t - 1) / t;
        if (lo <= groups) { ++diff[lo]; --diff[groups + 1]; }
    }
    vector<unsigned char> possible(mx + 1);
    int active = 0;
    for (int k = 1; k <= mx; ++k) { active += diff[k]; possible[k] = active > 0; }
    string out; out.reserve((size_t)q * 4);
    while (q--) {
        long long k; in.read(k);
        bool ok = k <= mx ? possible[(int)k] : k <= total;
        out += ok ? "YES\n" : "NO\n";
    }
    fwrite(out.data(), 1, out.size(), stdout);
}
