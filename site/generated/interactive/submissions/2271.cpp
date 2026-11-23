#include <bits/stdc++.h>
using namespace std;

static const long long B = 100000;

static double ask(long long x1, long long y1, long long x2, long long y2){
    printf("query %lld %lld %lld %lld\n", x1, y1, x2, y2);
    fflush(stdout);
    double v;
    if(scanf("%lf", &v) != 1) exit(0);
    if(v < 0.0) exit(0);
    return v;
}

int main(){
    const int NK = 502;                 // lines y = 199*k, k=1..502 cover cy in [100,99900]
    priority_queue<pair<int,int>> pq;   // (length, -start) of unqueried runs
    pq.push(make_pair(NK, -1));
    long long Y = -1; double L1 = 0.0;
    while(!pq.empty()){
        pair<int,int> t = pq.top(); pq.pop();
        int len = t.first, l = -t.second, r = l + len - 1;
        int m = l + (len - 1)/2;
        long long y = 199LL * m;
        double v = ask(0, y, B, y);
        if(v > 0.5){ Y = y; L1 = v; break; }
        if(m > l) pq.push(make_pair(m - l, -l));
        if(m < r) pq.push(make_pair(r - m, -(m + 1)));
    }
    if(Y < 0){                          // safety fallback (theoretically unreachable)
        for(int k = 0; k <= 502; ++k){
            long long y = 199LL*k + 100;
            if(y > B) break;
            double v = ask(0, y, B, y);
            if(v > 0.5){ Y = y; L1 = v; break; }
        }
    }
    if(Y < 0) return 0;

    // chord at Y+1 : (L2^2-L1^2)/4 = 2d-1, d = cy - Y
    double L2 = ask(0, Y + 1, B, Y + 1);
    long long d = llround((L2*L2 - L1*L1 + 4.0) / 8.0);
    long long cy = Y + d;
    long long R = llround(sqrt(L1*L1*0.25 + (double)d*(double)d));
    if(R < 100) R = 100;
    if(R > 50000) R = 50000;
    if(cy < R) cy = R;
    if(cy > B - R) cy = B - R;

    // slanted line (0,cy)-(B,cy+R): distance = R*cx/sqrt(R^2+1e10) < R always
    double C = ask(0, cy, B, cy + R);
    double RR = (double)R;
    double KK = sqrt(RR*RR + 1e10);
    auto f = [&](double c) -> double {
        double t = RR*RR - c*c*0.25;
        if(t < 0) t = 0;
        return sqrt(t) * KK / RR;
    };
    double eps = 2e-6;                  // judge error <= 1e-6 (+ printing rounding)
    double lo = f(C + eps);
    double hi = f(C > eps ? C - eps : 0.0);
    long long cx;
    if(llround(lo) == llround(hi)){
        cx = llround(hi);               // unique integer in the certified interval
    } else {
        long long X = llround(0.5*(lo + hi));
        if(X < 1) X = 1;
        if(X > B - 1) X = B - 1;
        // X is strictly inside the disk at height cy -> response = X - (cx - R)
        double resp = ask(0, cy, X, cy);
        cx = llround((double)X + RR - resp);
    }
    if(cx < R) cx = R;
    if(cx > B - R) cx = B - R;

    printf("answer %lld %lld %lld\n", cx, cy, R);
    fflush(stdout);
    return 0;
}
