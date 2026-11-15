#include <bits/stdc++.h>
using namespace std;

static char buf[1 << 25];

int main() {
    size_t len = fread(buf, 1, sizeof(buf), stdin);
    // may need more than buffer size; handle by reading all
    string extra;
    if (len == sizeof(buf)) {
        // read the rest
        char tmp[1 << 16];
        size_t r;
        while ((r = fread(tmp, 1, sizeof(tmp), stdin)) > 0) extra.append(tmp, r);
    }
    string all;
    all.reserve(len + extra.size());
    all.append(buf, len);
    all += extra;
    size_t pos = 0;
    auto readLL = [&]() -> long long {
        while (pos < all.size() && (all[pos] < '0' || all[pos] > '9')) pos++;
        long long x = 0;
        while (pos < all.size() && all[pos] >= '0' && all[pos] <= '9') {
            x = x * 10 + (all[pos] - '0');
            pos++;
        }
        return x;
    };
    int n = (int)readLL();
    int q = (int)readLL();
    vector<int> ge;
    int M = 0;
    vector<int> a(n);
    for (int i = 0; i < n; i++) {
        a[i] = (int)readLL();
        M = max(M, a[i]);
    }
    ge.assign(M + 2, 0);
    for (int i = 0; i < n; i++) ge[a[i]]++;
    for (int v = M - 1; v >= 1; v--) ge[v] += ge[v + 1];
    vector<pair<long long, long long>> iv;
    iv.reserve(M);
    for (int T = 1; T <= M; T++) {
        long long hi = 0;
        for (int j = T; j <= M; j += T) hi += ge[j];
        long long lo = (M + T - 1) / T;
        if (lo <= hi) iv.push_back({lo, hi});
    }
    sort(iv.begin(), iv.end());
    vector<pair<long long, long long>> mg;
    for (auto &p : iv) {
        if (!mg.empty() && p.first <= mg.back().second + 1) {
            mg.back().second = max(mg.back().second, p.second);
        } else {
            mg.push_back(p);
        }
    }
    vector<long long> starts(mg.size());
    for (size_t i = 0; i < mg.size(); i++) starts[i] = mg[i].first;
    string out;
    out.reserve((size_t)q * 4);
    for (int i = 0; i < q; i++) {
        long long k = readLL();
        auto it = upper_bound(starts.begin(), starts.end(), k);
        bool ok = false;
        if (it != starts.begin()) {
            size_t idx = (it - starts.begin()) - 1;
            if (k <= mg[idx].second) ok = true;
        }
        out += ok ? "YES\n" : "NO\n";
    }
    fwrite(out.data(), 1, out.size(), stdout);
    return 0;
}
