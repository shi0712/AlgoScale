#include <bits/stdc++.h>
using namespace std;

static int qcnt = 0;

static int ask(int p, int e){
    ++qcnt;
    printf("? %d %d\n", p, e);
    fflush(stdout);
    int r;
    if(scanf("%d", &r) != 1) exit(0);
    if(r != 0 && r != 1) exit(0);
    return r;
}

int main(){
    int m;
    if(scanf("%d", &m) != 1) return 0;
    vector<int> prefixQ; prefixQ.reserve(m+2); prefixQ.push_back(0);
    vector<int> cnt; cnt.reserve(m+2); cnt.push_back(0);
    vector<int> cyc(m+1, 0);
    vector<double> pre(m+3, 0.0);
    int tip = 0;
    double cycRej = 0, locS = 0, locF = 0;
    for(int e = 1; e <= m; ++e){
        int id = qcnt + 1;
        int r = ask(tip, e);
        if(r == 1){
            tip = id;
            prefixQ.push_back(id);
            cnt.push_back(0);
            cyc[e] = (int)cnt.size() - 1;
        } else {
            int j = (int)prefixQ.size() - 1;
            int idx = 0;
            if(j > 0){
                double bridges = (double)cnt[0];
                double p0 = (bridges + 0.5) / (bridges + cycRej + 1.0);
                double gamma = (locS + 0.5) / (locS + locF + 1.0);
                double B = (double)(j + 1);
                double base = B * (1.0 - p0);
                double cc = base * (1.0 - gamma) / (cycRej + (double)j);
                pre[0] = 0.0;
                pre[1] = 1.0 + B * p0;
                for(int i = 1; i <= j; ++i){
                    double w = 1.0 + cc * ((double)cnt[i] + 1.0);
                    if(i == j) w += base * gamma;
                    pre[i+1] = pre[i] + w;
                }
                int lo = 0, hi = j;
                while(lo < hi){
                    double tot = pre[hi+1] - pre[lo];
                    double target = pre[lo] + 0.5 * tot;
                    int t = (int)(lower_bound(pre.begin()+lo+1, pre.begin()+hi+1, target) - pre.begin());
                    if(t > hi) t = hi;
                    int best = t;
                    if(t-1 >= lo+1 && fabs(pre[t-1]-target) <= fabs(pre[t]-target)) best = t-1;
                    int x = best - 1;
                    if(x < lo) x = lo;
                    if(x > hi-1) x = hi-1;
                    int rr = ask(prefixQ[x], e);
                    if(rr == 0) hi = x; else lo = x + 1;
                }
                idx = lo;
            }
            cyc[e] = idx;
            cnt[idx]++;
            if(idx > 0){
                cycRej += 1.0;
                if(j >= 2){ if(idx == j) locS += 1.0; else locF += 1.0; }
            }
        }
    }
    int k = (int)cnt.size() - 1;
    vector<int> val(k+1, -1);
    for(int i = 1; i <= k; ++i){
        int L = cnt[i] + 1;
        val[i] = (L <= 14 ? L : 0);
    }
    string out;
    out.reserve((size_t)6*m + 8);
    out += '!';
    for(int e = 1; e <= m; ++e){
        int v = val[cyc[e]];
        out += ' ';
        if(v < 0){ out += '-'; v = -v; }
        char tmp[8]; int len = 0;
        if(v == 0) tmp[len++] = '0';
        while(v){ tmp[len++] = char('0' + v % 10); v /= 10; }
        while(len) out += tmp[--len];
    }
    out += '\n';
    fwrite(out.data(), 1, out.size(), stdout);
    fflush(stdout);
    int r;
    if(scanf("%d", &r) != 1) return 0;
    return 0;
}
