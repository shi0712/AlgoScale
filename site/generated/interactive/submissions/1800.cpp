#include <bits/stdc++.h>
using namespace std;

int n;
vector<int> deg;
vector<char> inC;

void answer(int x) {
    cout << "! " << x << '\n';
    cout.flush();
    exit(0);
}

int askString(string &s) {
    cout << "? " << s << '\n';
    cout.flush();
    int x;
    cin >> x;
    if (x < 0) exit(0);
    return x;
}

int askVertex(int v) {
    string s(n, '0');
    s[v] = '1';
    return askString(s);
}

int askPair(int u, int v) {
    string s(n, '0');
    s[u] = '1';
    s[v] = '1';
    return askString(s);
}

int askSet(const vector<char> &a) {
    string s(n, '0');
    for (int i = 0; i < n; ++i)
        if (a[i]) s[i] = '1';
    return askString(s);
}

int getDeg(int v) {
    if (deg[v] != -1) return deg[v];

    int x = askVertex(v);
    deg[v] = x;

    if (x == 0) answer(0);
    if (x == n - 1) answer(1);

    return x;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    cin >> n;

    if (n == 1) answer(1);

    deg.assign(n, -1);
    inC.assign(n, 0);

    mt19937 rng(chrono::steady_clock::now().time_since_epoch().count());

    vector<int> all(n);
    iota(all.begin(), all.end(), 0);
    shuffle(all.begin(), all.end(), rng);

    int sampleCnt = min(n, 12);
    vector<int> sampled;
    for (int i = 0; i < sampleCnt; ++i) {
        int v = all[i];
        getDeg(v);
        sampled.push_back(v);
    }

    int start = sampled[0];
    for (int v : sampled) {
        if (deg[v] < deg[start]) start = v;
    }

    inC[start] = 1;

    vector<int> comp;
    comp.push_back(start);

    vector<int> order(n);
    iota(order.begin(), order.end(), 0);
    shuffle(order.begin(), order.end(), rng);

    vector<int> ptr(n, 0);
    vector<int> st;
    st.push_back(start);

    while (!st.empty()) {
        int u = st.back();
        bool advanced = false;

        while (ptr[u] < n) {
            int v = order[ptr[u]++];
            if (inC[v]) continue;

            int du = getDeg(u);
            int dv = getDeg(v);

            int q = askPair(u, v);
            int val = du + dv - q;

            if (val > 0) {
                inC[v] = 1;
                comp.push_back(v);

                if ((int)comp.size() == n) answer(1);

                int ext = askSet(inC);

                if (ext == 0) answer(0);
                if (ext == n - (int)comp.size()) answer(1);

                st.push_back(v);
                advanced = true;
                break;
            }
        }

        if (!advanced) st.pop_back();
    }

    answer(0);
    return 0;
}
