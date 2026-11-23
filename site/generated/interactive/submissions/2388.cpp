#include <bits/stdc++.h>
using namespace std;

int n, m;
vector<int> eu, ev;
vector<int> st;

int ask(const vector<int>& t) {
    printf("?");
    for (int x : t) printf(" %d", x);
    printf("\n");
    fflush(stdout);
    int r;
    if (scanf("%d", &r) != 1) exit(0);
    if (r < 0) exit(0);
    return r;
}

int par[505];
int fnd(int x) {
    while (par[x] != x) {
        par[x] = par[par[x]];
        x = par[x];
    }
    return x;
}

int main() {
    if (scanf("%d %d", &n, &m) != 2) return 0;
    eu.resize(m);
    ev.resize(m);
    vector<vector<int>> adj(n);
    for (int i = 0; i < m; i++) {
        scanf("%d %d", &eu[i], &ev[i]);
        adj[eu[i]].push_back(i);
        adj[ev[i]].push_back(i);
    }
    st.assign(m, -1);
    int royalCount = 0;
    auto setst = [&](int e, int v) {
        if (st[e] == -1) {
            st[e] = v;
            if (v == 1) royalCount++;
        }
    };
    for (int w = 0; w < n; w++) {
        if (royalCount == n - 1) break;
        bool anyUnknown = false;
        for (int e : adj[w])
            if (st[e] == -1) anyUnknown = true;
        if (!anyUnknown) continue;
        for (int i = 0; i < n; i++) par[i] = i;
        vector<int> treeEdges;
        for (int e = 0; e < m; e++) {
            if (eu[e] == w || ev[e] == w) continue;
            int a = fnd(eu[e]), b = fnd(ev[e]);
            if (a != b) {
                par[a] = b;
                treeEdges.push_back(e);
            }
        }
        map<int, vector<int>> groups;
        for (int e : adj[w]) {
            int o = eu[e] == w ? ev[e] : eu[e];
            groups[fnd(o)].push_back(e);
        }
        vector<vector<int>> G;
        vector<int> bases;
        for (auto& kv : groups) {
            auto& vec = kv.second;
            int base = -1;
            for (int e : vec)
                if (st[e] != -1) {
                    base = e;
                    break;
                }
            if (base == -1) base = vec[0];
            G.push_back(vec);
            bases.push_back(base);
        }
        vector<int> tree = treeEdges;
        int off = tree.size();
        for (int b : bases) tree.push_back(b);
        bool haveQ0 = false;
        int q0 = 0;
        for (size_t j = 0; j < G.size(); j++) {
            int base = bases[j];
            vector<int> others;
            for (int e : G[j])
                if (e != base && st[e] == -1) others.push_back(e);
            bool baseKnown = st[base] != -1;
            if (!baseKnown && others.empty()) {
                setst(base, 1);
                continue;
            }
            if (others.empty()) continue;
            if (!haveQ0) {
                q0 = ask(tree);
                haveQ0 = true;
            }
            vector<int> deltas;
            for (int e : others) {
                tree[off + j] = e;
                int q = ask(tree);
                tree[off + j] = base;
                deltas.push_back(q - q0);
            }
            if (baseKnown) {
                for (size_t k = 0; k < others.size(); k++) {
                    int r = st[base] + deltas[k];
                    setst(others[k], r);
                }
            } else {
                int rb = -1;
                for (size_t k = 0; k < others.size(); k++) {
                    if (deltas[k] == -1) rb = 1;
                    else if (deltas[k] == 1) rb = 0;
                }
                if (rb == -1) rb = 1;
                setst(base, rb);
                for (size_t k = 0; k < others.size(); k++) {
                    setst(others[k], rb + deltas[k]);
                }
            }
        }
    }
    printf("!");
    for (int e = 0; e < m; e++)
        if (st[e] == 1) printf(" %d", e);
    printf("\n");
    fflush(stdout);
    return 0;
}
