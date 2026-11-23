#include <bits/stdc++.h>
using namespace std;

int R, C, N;
vector<string> g;
vector<int> dd, stp, Lt, Rt, cls;
int dr[4] = {-1, 0, 1, 0}, dc[4] = {0, 1, 0, -1};

int nxt(int s, int act) {
    if (act == 0) return Lt[s];
    if (act == 1) return Rt[s];
    return stp[s];
}

int main() {
    scanf("%d %d", &R, &C);
    g.resize(R);
    for (int i = 0; i < R; i++) {
        char buf[205];
        scanf("%s", buf);
        g[i] = buf;
    }
    N = R * C * 4;
    dd.assign(N, -1);
    stp.assign(N, -1);
    Lt.assign(N, -1);
    Rt.assign(N, -1);
    vector<int> open;
    for (int i = 0; i < R; i++)
        for (int j = 0; j < C; j++) {
            if (g[i][j] != '.') continue;
            for (int d = 0; d < 4; d++) {
                int s = (i * C + j) * 4 + d;
                open.push_back(s);
                int k = 0, x = i + dr[d], y = j + dc[d];
                while (x >= 0 && x < R && y >= 0 && y < C && g[x][y] == '.') {
                    k++;
                    x += dr[d];
                    y += dc[d];
                }
                dd[s] = k;
                Lt[s] = (i * C + j) * 4 + (d + 3) % 4;
                Rt[s] = (i * C + j) * 4 + (d + 1) % 4;
                if (k > 0) stp[s] = ((i + dr[d]) * C + (j + dc[d])) * 4 + d;
            }
        }
    // Moore partition refinement
    cls.assign(N, -1);
    int ncls = 0;
    {
        map<int, int> m;
        for (int s : open) {
            if (!m.count(dd[s])) {
                int id = m.size();
                m[dd[s]] = id;
            }
            cls[s] = m[dd[s]];
        }
        ncls = m.size();
        while (true) {
            vector<pair<array<int, 4>, int>> v;
            v.reserve(open.size());
            for (int s : open) {
                array<int, 4> k = {cls[s], cls[Lt[s]], cls[Rt[s]], stp[s] >= 0 ? cls[stp[s]] : -1};
                v.push_back({k, s});
            }
            sort(v.begin(), v.end());
            vector<int> nc(N, -1);
            int id = 0;
            for (size_t i = 0; i < v.size(); i++) {
                if (i > 0 && v[i].first != v[i - 1].first) id++;
                nc[v[i].second] = id;
            }
            id++;
            cls = nc;
            if (id == ncls) break;
            ncls = id;
        }
    }
    mt19937 rng(12345);
    vector<int> S = open;
    deque<int> plan;
    int pa = -1, pb = -1;
    vector<char> inS(N, 0);
    bool first = true;
    while (true) {
        int d;
        if (scanf("%d", &d) != 1) return 0;
        {
            vector<int> T;
            for (int s : S)
                if (dd[s] == d) T.push_back(s);
            S = T;
        }
        (void)first;
        first = false;
        // yes check
        bool same = true;
        for (int s : S)
            if ((s >> 2) != (S[0] >> 2)) { same = false; break; }
        if (same) {
            int p = S[0] >> 2;
            printf("yes %d %d\n", p / C + 1, p % C + 1);
            fflush(stdout);
            return 0;
        }
        // no check
        {
            unordered_map<int, int> mp;
            bool no = false;
            for (int s : S) {
                auto it = mp.find(cls[s]);
                if (it == mp.end()) mp[cls[s]] = s >> 2;
                else if (it->second != (s >> 2)) { no = true; break; }
            }
            if (no) {
                printf("no\n");
                fflush(stdout);
                return 0;
            }
        }
        for (int s : S) inS[s] = 1;
        bool valid = !plan.empty() && pa >= 0 && inS[pa] && inS[pb];
        if (!valid) {
            plan.clear();
            vector<int> best;
            bool have = false;
            int bestA = -1, bestB = -1;
            int tries = 12;
            for (int t = 0; t < tries; t++) {
                int a = S[rng() % S.size()], b = S[rng() % S.size()];
                int guard = 0;
                while ((a >> 2) == (b >> 2) && guard < 50) {
                    b = S[rng() % S.size()];
                    guard++;
                }
                if ((a >> 2) == (b >> 2)) {
                    // find any with different position
                    bool f = false;
                    for (int s : S)
                        if ((s >> 2) != (a >> 2)) { b = s; f = true; break; }
                    if (!f) continue;
                }
                int maxDepth = have ? (int)best.size() - 1 : 1000000;
                // BFS
                struct Nd { int a, b, par, act, dep; };
                vector<Nd> nodes;
                unordered_set<long long> vis;
                nodes.push_back({a, b, -1, -1, 0});
                vis.insert((long long)a * N + b);
                size_t h = 0;
                int foundIdx = -1, foundAct = -1;
                while (h < nodes.size() && foundIdx < 0) {
                    Nd cur = nodes[h];
                    int idx = h;
                    h++;
                    if (cur.dep + 1 > maxDepth) break;
                    for (int act = 0; act < 3; act++) {
                        if (act == 2 && dd[cur.a] == 0) continue;
                        int na = nxt(cur.a, act), nb = nxt(cur.b, act);
                        if (dd[na] != dd[nb]) {
                            foundIdx = idx;
                            foundAct = act;
                            break;
                        }
                        long long key = (long long)na * N + nb;
                        if (vis.insert(key).second) nodes.push_back({na, nb, idx, act, cur.dep + 1});
                    }
                }
                if (foundIdx >= 0) {
                    vector<int> seq;
                    seq.push_back(foundAct);
                    int i = foundIdx;
                    while (nodes[i].par >= 0) {
                        seq.push_back(nodes[i].act);
                        i = nodes[i].par;
                    }
                    reverse(seq.begin(), seq.end());
                    if (!have || seq.size() < best.size()) {
                        best = seq;
                        have = true;
                        bestA = a;
                        bestB = b;
                    }
                }
            }
            if (!have) {
                // fallback: should not happen
                best = {0};
                bestA = S[0];
                bestB = S[0];
            }
            for (int x : best) plan.push_back(x);
            pa = bestA;
            pb = bestB;
        }
        for (int s : S) inS[s] = 0;
        int act = plan.front();
        plan.pop_front();
        if (act == 2 && d == 0) act = 0;  // safety
        const char* names[3] = {"left", "right", "step"};
        printf("%s\n", names[act]);
        fflush(stdout);
        vector<int> T;
        T.reserve(S.size());
        for (int s : S) {
            int n = nxt(s, act);
            if (n >= 0) T.push_back(n);
        }
        S = T;
        if (pa >= 0) {
            pa = nxt(pa, act);
            pb = nxt(pb, act);
        }
    }
}
