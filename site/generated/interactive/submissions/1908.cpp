#include <bits/stdc++.h>
using namespace std;

#if defined(__GNUC__) && (defined(__i386__) || defined(__x86_64__))
#pragma GCC target("popcnt")
#endif

constexpr int MAXN = 1024;
using Mask = bitset<MAXN>;

struct Entry {
    int single;
    Mask possible;
};

int n, k;
Mask g[MAXN]; // Edges are pairs of surviving indices not yet proved different.
int degv[MAXN];
deque<Entry> mem;
deque<int> plan;

void separate(int a, int b) {
    if (g[a][b]) {
        g[a].reset(b);
        g[b].reset(a);
        --degv[a];
        --degv[b];
    }
}

// With a one-cup memory, a trail tests one new pair at every step.
// Join odd vertices through a dummy vertex to obtain the trails.
void planTrail(int last) {
    int s = -1;
    bool useLast = last >= 0 && degv[last] > 0;
    if (useLast) s = last;
    else {
        for (int i = 0; i < n; ++i) {
            if (degv[i]) s = i;
            if (degv[i] & 1) break;
        }
    }

    int dummy = n;
    vector<vector<unsigned char>> adj(n + 1, vector<unsigned char>(n + 1));
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) adj[i][j] = g[i][j];
        adj[i][dummy] = adj[dummy][i] = degv[i] & 1;
    }

    vector<int> st, tour, ptr(n + 1);
    if (degv[s] & 1) {
        st.push_back(dummy);
        adj[s][dummy] = adj[dummy][s] = 0;
    }
    st.push_back(s);
    while (!st.empty()) {
        int v = st.back();
        int &j = ptr[v];
        while (j <= n && !adj[v][j]) ++j;
        if (j > n) {
            tour.push_back(v);
            st.pop_back();
        } else {
            adj[v][j] = adj[j][v] = 0;
            st.push_back(j);
        }
    }
    reverse(tour.begin(), tour.end());
    int p = (tour.front() == dummy);
    if (useLast) ++p;
    while (p < (int)tour.size() && tour[p] != dummy) {
        plan.push_back(tour[p++]);
    }
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    if (!(cin >> n >> k)) return 0;
    int survivors = n;
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) if (i != j) g[i].set(j);
        degv[i] = n - 1;
    }

    while (*max_element(degv, degv + n) > 0) {
        Mask singles, blocked;
        for (const Entry &e : mem) {
            if (e.single >= 0) singles.set(e.single);
            else blocked |= e.possible;
        }
        blocked |= singles;

        int x = -1;
        if (k == 1) {
            if (plan.empty())
                planTrail(mem.empty() ? -1 : mem.back().single);
            x = plan.front();
            plan.pop_front();
        } else {
            // Prefer tests that resolve the most outstanding edges on N.
            int best = 0;
            for (int i = 0; i < n; ++i) {
                if (blocked[i] || !degv[i]) continue;
                int gain = (g[i] & singles).count();
                if (gain > best ||
                    (gain == best && gain && degv[i] > degv[x])) {
                    best = gain;
                    x = i;
                }
            }
            if (x == -1) {
                // Seed the next run with a vertex that can serve as a
                // reference for many of the currently available vertices.
                Mask available = ~blocked;
                for (int i = 0; i < n; ++i) {
                    if (blocked[i] || degv[i] <= best) continue;
                    int score = (g[i] & available).count();
                    if (score > best) {
                        best = score;
                        x = i;
                    }
                }
            }
            if (x == -1) {
                // With singleton-only memory its entries are pairwise
                // different, so this obstruction needs an ambiguous Y.
                cout << "R\n" << flush;
                if (!cout) return 0;
                mem.clear();
                continue;
            }
        }

        cout << "? " << x + 1 << '\n' << flush;
        if (!cout) return 0;
        string reply;
        if (!(cin >> reply) || (reply != "Y" && reply != "N")) return 0;

        Entry next;
        if (reply == "N") {
            for (const Entry &e : mem)
                if (e.single >= 0) separate(x, e.single);
            next.single = x;
            next.possible.set(x);
        } else {
            plan.clear();
            // The cup has a surviving representative in this set. We
            // deliberately never test an index in the current blocked set,
            // so deleting x cannot invalidate any cup's representative.
            next.possible = blocked & g[x];
            next.single = -1;
            if (next.possible.count() == 1) {
                int r = 0;
                while (!next.possible[r]) ++r;
                next.single = r;
                for (int j = 0; j < n; ++j)
                    if (!g[x][j]) separate(r, j);
            }
            for (int j = 0; j < n; ++j) separate(x, j);
            --survivors;
        }

        mem.push_back(next);
        if ((int)mem.size() > k) mem.pop_front();
    }

    cout << "! " << survivors << '\n' << flush;
    return 0;
}
