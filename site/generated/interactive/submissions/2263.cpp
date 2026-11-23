#include <bits/stdc++.h>
using namespace std;

static long long ask(const vector<pair<int,int>>& w) {
    long long k = 0;
    for (const auto &p : w) k += p.second;
    string s = "? 1 " + to_string(k);
    for (const auto &p : w)
        for (int t = 0; t < p.second; ++t) { s += ' '; s += to_string(p.first); }
    s += '\n';
    fwrite(s.data(), 1, s.size(), stdout);
    fflush(stdout);
    long long v;
    if (scanf("%lld", &v) != 1) exit(0);
    return v;
}

static void flipNode(int u) {
    printf("? 2 %d\n", u);
    fflush(stdout);
}

int main() {
    int n;
    if (scanf("%d", &n) != 1) return 0;
    for (int i = 0; i < n - 1; ++i) {
        int a, b;
        if (scanf("%d %d", &a, &b) != 2) return 0;
    }
    vector<int> val(n + 1, 0);
    long long S = ask({{1,1}});
    int mode_;
    if (S == 1 || S == -1) { mode_ = 0; val[1] = (int)S; }
    else if (S == 2 || S == -2) { flipNode(1); val[1] = (int)(-S/2); mode_ = 1; }
    else { mode_ = 2; }

    int m = 0;
    while (((1LL << (m+1)) - 1) <= (long long)n) ++m;
    long long M = (1LL << m) - 1;
    int W = (int)((long long)n - M);

    vector<int> order;
    for (int i = 2; i <= n; ++i) order.push_back(i);
    mt19937 rng((unsigned)chrono::steady_clock::now().time_since_epoch().count());
    shuffle(order.begin(), order.end(), rng);

    int ptr = 0;
    int pending = -1;
    vector<pair<long long, vector<int>>> stored;

    auto decode = [&](long long B, const vector<int>& nodes) {
        for (int j = 0; j < (int)nodes.size(); ++j)
            val[nodes[j]] = ((B >> j) & 1LL) ? 1 : -1;
    };
    auto resolvePending = [&](int b) {
        for (auto &st : stored) decode(st.first - (long long)W * b, st.second);
        stored.clear();
        pending = -1;
    };

    while (true) {
        int R = (int)order.size() - ptr;
        int tot = R + (pending != -1 ? 1 : 0);
        if (tot <= 0) break;
        if (tot <= m || W == 0) {
            int cnt = min(tot, m);
            vector<int> nodes;
            if (pending != -1) nodes.push_back(pending);
            while ((int)nodes.size() < cnt) nodes.push_back(order[ptr++]);
            vector<pair<int,int>> w;
            for (int j = 0; j < cnt; ++j) w.push_back({nodes[j], 1 << j});
            long long K = (1LL << cnt) - 1;
            long long V = ask(w);
            long long Va = (mode_ == 0) ? V - K * (long long)val[1] : V;
            long long B = (Va + K) / 2;
            decode(B, nodes);
            if (pending != -1) resolvePending(val[pending] == 1 ? 1 : 0);
            continue;
        }
        vector<int> lows;
        for (int j = 0; j < m; ++j) lows.push_back(order[ptr++]);
        int high;
        if (pending != -1) high = pending; else high = order[ptr++];
        vector<pair<int,int>> w;
        for (int j = 0; j < m; ++j) w.push_back({lows[j], 1 << j});
        w.push_back({high, W});
        long long K = M + W;
        long long V = ask(w);
        long long Va = (mode_ == 0) ? V - K * (long long)val[1] : V;
        long long B = (Va + K) / 2;
        if (B < (long long)W) {
            decode(B, lows); val[high] = -1;
            if (pending != -1) resolvePending(0);
        } else if (B > M) {
            decode(B - W, lows); val[high] = 1;
            if (pending != -1) resolvePending(1);
        } else {
            stored.push_back({B, lows});
            pending = high;
        }
    }

    if (mode_ == 2) {
        flipNode(1);
        long long S2 = ask({{1,1}});
        val[1] = (int)(S2 / 2);
    }

    string out = "!";
    for (int i = 1; i <= n; ++i) { out += ' '; out += to_string(val[i]); }
    out += '\n';
    fwrite(out.data(), 1, out.size(), stdout);
    fflush(stdout);
    return 0;
}
