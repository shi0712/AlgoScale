#include <bits/stdc++.h>
using namespace std;

int qcnt = 0;
int ask(int p, int e, int &qid) {
    printf("? %d %d\n", p, e);
    fflush(stdout);
    int r;
    if (scanf("%d", &r) != 1) exit(0);
    if (r == -1) exit(0);
    qid = ++qcnt;
    return r;
}

int main() {
    int m;
    if (scanf("%d", &m) != 1) return 0;
    vector<int> prefixNode;
    prefixNode.push_back(0);
    vector<int> classCnt(1, 0); // 1-indexed
    vector<int> leafOf(m + 1, 0);
    const int C = 16;
    vector<double> hz(C + 1, 0.0), ex(C + 1, 0.0);
    double hBig = 0, exBig = 0;
    vector<int> hist(C + 1, 0);
    long long bigSum = 0;
    double nb = 0.3, nj = 0.3, nn = 0.3;
    double sameC = 0.3, totC = 1.0;
    const double dec = 0.99;
    int prevLeaf = -1;

    for (int f = 1; f <= m; f++) {
        int nc = (int)classCnt.size() - 1;
        int K = nc + 1;
        // exposure update
        for (int c = 1; c < C; c++) ex[c] += hist[c];
        exBig += (double)bigSum;
        vector<double> rate(C + 1);
        for (int c = 1; c < C; c++) rate[c] = (hz[c] + 0.5) / (ex[c] + 25.0);
        double rBig = (hBig + 0.5) / (exBig + 25.0);
        vector<double> w(K + 1);
        double sumR = 0;
        for (int i = 1; i <= nc; i++) {
            int c = classCnt[i];
            double r = (c < C) ? rate[c] : rBig * c;
            w[i] = r;
            sumR += r;
        }
        double tb = nb, tn = nn, tj = (nc > 0 ? nj : 0);
        double tot = tb + tn + tj;
        w[0] = tb / tot;
        w[K] = tn / tot;
        for (int i = 1; i <= nc; i++) w[i] = (tj / tot) * w[i] / sumR;
        if (prevLeaf >= 0) {
            double lam = sameC / totC;
            if (lam > 0.98) lam = 0.98;
            for (int i = 0; i <= K; i++) w[i] *= (1 - lam);
            w[prevLeaf] += lam;
        }
        for (int i = 0; i <= K; i++) if (w[i] < 1e-12) w[i] = 1e-12;
        vector<double> P(K + 1);
        P[0] = w[0];
        for (int i = 1; i <= K; i++) P[i] = P[i - 1] + w[i];
        int lo = 0, hi = K;
        int fullQid = -1;
        while (lo < hi) {
            double base = lo > 0 ? P[lo - 1] : 0.0;
            double target = (base + P[hi]) / 2;
            int t = (int)(lower_bound(P.begin() + lo, P.begin() + hi, target) - P.begin());
            if (t > hi - 1) t = hi - 1;
            if (t - 1 >= lo) {
                double d1 = fabs(2 * P[t] - base - P[hi]);
                double d2 = fabs(2 * P[t - 1] - base - P[hi]);
                if (d2 < d1) t = t - 1;
            }
            int qid;
            int r = ask(prefixNode[t], f, qid);
            if (r == 1) {
                lo = t + 1;
                if (t == nc) fullQid = qid;
            } else hi = t;
        }
        int L = lo;
        // decay counts
        nb *= dec; nj *= dec; nn *= dec;
        sameC *= dec; totC *= dec;
        if (prevLeaf >= 0) {
            totC += 1;
            if (L == prevLeaf) sameC += 1;
        }
        if (L == 0) {
            nb += 1;
            leafOf[f] = 0;
            prevLeaf = 0;
        } else if (L == K) {
            nn += 1;
            classCnt.push_back(1);
            prefixNode.push_back(fullQid);
            leafOf[f] = K;
            hist[1]++;
            prevLeaf = K;
        } else {
            nj += 1;
            int c = classCnt[L];
            if (c < C) {
                hz[c] += 1;
                hist[c]--;
            } else {
                hBig += 1;
                bigSum -= c;
            }
            c++;
            classCnt[L] = c;
            if (c < C) hist[c]++;
            else bigSum += c;
            leafOf[f] = L;
            prevLeaf = L;
        }
    }
    string out = "!";
    for (int e = 1; e <= m; e++) {
        int L = leafOf[e];
        int v;
        if (L == 0) v = -1;
        else {
            int s = classCnt[L];
            v = (s <= 14) ? s : 0;
        }
        out += " " + to_string(v);
    }
    printf("%s\n", out.c_str());
    fflush(stdout);
    int conf;
    scanf("%d", &conf);
    return 0;
}
