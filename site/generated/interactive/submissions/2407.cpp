#include <bits/stdc++.h>
using namespace std;
typedef unsigned long long u64;

int cls_[4][4][4];
map<int,int> respIdx;

struct Q {
    int type;
    int x[3];
    int pos[3];
    int tab[4];
};

int main() {
    int n;
    if (!(cin >> n)) return 0;
    // build tables
    {
        set<int> vals;
        int r[4][4][4];
        for (int a = 1; a <= 4; a++)
            for (int b = 1; b <= 4; b++)
                for (int c = 1; c <= 4; c++) {
                    int s = a + b + c;
                    int v = 0;
                    if (s - 2 * a > 0 && s - 2 * b > 0 && s - 2 * c > 0) v = s * (s - 2 * a) * (s - 2 * b) * (s - 2 * c);
                    r[a - 1][b - 1][c - 1] = v;
                    vals.insert(v);
                }
        int id = 0;
        for (int v : vals) respIdx[v] = id++;
        for (int a = 0; a < 4; a++)
            for (int b = 0; b < 4; b++)
                for (int c = 0; c < 4; c++) cls_[a][b][c] = respIdx[r[a][b][c]];
    }
    const int B = 6;
    vector<int> val(n + 1, 0);
    int cnt[4] = {0, 0, 0, 0};
    vector<int> rep[4];
    vector<int> C;
    vector<u64> SC = {0};
    int ptr = 1;
    while (ptr <= n) {
        int cs = C.size();
        int nc = min(n - ptr + 1, max(1, B - cs));
        while (nc > 1 && SC.size() * (size_t(1) << (2 * nc)) > 400000) nc--;
        vector<int> P = C;
        for (int i = 0; i < nc; i++) P.push_back(ptr + i);
        ptr += nc;
        int m = P.size();
        vector<u64> S;
        S.reserve(SC.size() << (2 * nc));
        for (u64 s : SC)
            for (u64 t = 0; t < (1ULL << (2 * nc)); t++) S.push_back(s | (t << (2 * cs)));
        double tot = cnt[0] + cnt[1] + cnt[2] + cnt[3] + 4.0;
        double p[4];
        for (int v = 0; v < 4; v++) p[v] = (cnt[v] + 1.0) / tot;
        vector<double> w(S.size());
        for (size_t i = 0; i < S.size(); i++) {
            double ww = 1;
            for (int j = 0; j < m; j++) ww *= p[(S[i] >> (2 * j)) & 3];
            w[i] = ww;
        }
        // build queries
        vector<Q> qs;
        vector<int> rp;
        for (int v = 0; v < 4; v++)
            for (int pos : rep[v]) rp.push_back(pos);
        bool seen[4][4];
        memset(seen, 0, sizeof(seen));
        for (size_t i = 0; i < rp.size(); i++)
            for (size_t j = i + 1; j < rp.size(); j++) {
                int vi = val[rp[i]] - 1, vj = val[rp[j]] - 1;
                int a = min(vi, vj), b = max(vi, vj);
                if (seen[a][b]) continue;
                seen[a][b] = true;
                for (int u = 0; u < m; u++) {
                    Q q;
                    q.type = 1;
                    q.x[0] = u;
                    q.pos[0] = rp[i];
                    q.pos[1] = rp[j];
                    q.pos[2] = P[u];
                    for (int v = 0; v < 4; v++) q.tab[v] = cls_[vi][vj][v];
                    qs.push_back(q);
                }
            }
        for (int v = 0; v < 4; v++) {
            if (rep[v].empty()) continue;
            for (int u = 0; u < m; u++)
                for (int ww = u + 1; ww < m; ww++) {
                    Q q;
                    q.type = 2;
                    q.x[0] = v;
                    q.x[1] = u;
                    q.x[2] = ww;
                    q.pos[0] = rep[v][0];
                    q.pos[1] = P[u];
                    q.pos[2] = P[ww];
                    qs.push_back(q);
                }
        }
        for (int u = 0; u < m; u++)
            for (int ww = u + 1; ww < m; ww++)
                for (int z = ww + 1; z < m; z++) {
                    Q q;
                    q.type = 3;
                    q.x[0] = u;
                    q.x[1] = ww;
                    q.x[2] = z;
                    q.pos[0] = P[u];
                    q.pos[1] = P[ww];
                    q.pos[2] = P[z];
                    qs.push_back(q);
                }
        auto evalCls = [&](const Q &q, u64 s) -> int {
            if (q.type == 1) return q.tab[(s >> (2 * q.x[0])) & 3];
            if (q.type == 2) return cls_[q.x[0]][(s >> (2 * q.x[1])) & 3][(s >> (2 * q.x[2])) & 3];
            return cls_[(s >> (2 * q.x[0])) & 3][(s >> (2 * q.x[1])) & 3][(s >> (2 * q.x[2])) & 3];
        };
        while (S.size() > 1) {
            double bestH = -1;
            int bi = -1;
            for (size_t qi = 0; qi < qs.size(); qi++) {
                const Q &q = qs[qi];
                double ws[16];
                int cn[16];
                for (int c = 0; c < 16; c++) { ws[c] = 0; cn[c] = 0; }
                size_t sz = S.size();
                if (q.type == 1) {
                    int sh = 2 * q.x[0];
                    for (size_t i = 0; i < sz; i++) {
                        int c = q.tab[(S[i] >> sh) & 3];
                        ws[c] += w[i];
                        cn[c]++;
                    }
                } else if (q.type == 2) {
                    const int (*t)[4] = cls_[q.x[0]];
                    int s1 = 2 * q.x[1], s2 = 2 * q.x[2];
                    for (size_t i = 0; i < sz; i++) {
                        int c = t[(S[i] >> s1) & 3][(S[i] >> s2) & 3];
                        ws[c] += w[i];
                        cn[c]++;
                    }
                } else {
                    int s0 = 2 * q.x[0], s1 = 2 * q.x[1], s2 = 2 * q.x[2];
                    for (size_t i = 0; i < sz; i++) {
                        int c = cls_[(S[i] >> s0) & 3][(S[i] >> s1) & 3][(S[i] >> s2) & 3];
                        ws[c] += w[i];
                        cn[c]++;
                    }
                }
                int nz = 0;
                double W = 0;
                for (int c = 0; c < 16; c++)
                    if (cn[c]) { nz++; W += ws[c]; }
                if (nz < 2) continue;
                double H = 0;
                for (int c = 0; c < 16; c++)
                    if (cn[c] && ws[c] > 0) {
                        double pr = ws[c] / W;
                        H -= pr * log(pr);
                    }
                if (H > bestH) { bestH = H; bi = qi; }
            }
            if (bi < 0) break;
            Q &q = qs[bi];
            int ps[3] = {q.pos[0], q.pos[1], q.pos[2]};
            sort(ps, ps + 3);
            cout << "? " << ps[0] << " " << ps[1] << " " << ps[2] << endl;
            long long r;
            if (!(cin >> r)) return 0;
            if (r < 0) return 0;
            auto it = respIdx.find((int)r);
            if (it == respIdx.end()) return 0;
            int idx = it->second;
            size_t k = 0;
            for (size_t i = 0; i < S.size(); i++) {
                if (evalCls(q, S[i]) == idx) {
                    S[k] = S[i];
                    w[k] = w[i];
                    k++;
                }
            }
            S.resize(k);
            w.resize(k);
            if (S.empty()) return 0;
        }
        u64 first = S[0];
        u64 diff = 0;
        for (u64 s : S) diff |= (s ^ first);
        vector<int> newC;
        vector<int> undet;
        for (int j = 0; j < m; j++) {
            if (((diff >> (2 * j)) & 3) == 0) {
                int v = ((first >> (2 * j)) & 3);
                val[P[j]] = v + 1;
                cnt[v]++;
                if (rep[v].size() < 2) rep[v].push_back(P[j]);
            } else {
                newC.push_back(P[j]);
                undet.push_back(j);
            }
        }
        vector<u64> nsc;
        if (undet.empty()) {
            nsc.push_back(0);
        } else {
            for (u64 s : S) {
                u64 code = 0;
                int k = 0;
                for (int j : undet) {
                    code |= ((s >> (2 * j)) & 3) << (2 * k);
                    k++;
                }
                nsc.push_back(code);
            }
            sort(nsc.begin(), nsc.end());
            nsc.erase(unique(nsc.begin(), nsc.end()), nsc.end());
        }
        C = newC;
        SC = nsc;
    }
    if (!C.empty()) {
        cout << "! -1" << endl;
    } else {
        cout << "!";
        for (int i = 1; i <= n; i++) cout << " " << val[i];
        cout << endl;
    }
    return 0;
}
