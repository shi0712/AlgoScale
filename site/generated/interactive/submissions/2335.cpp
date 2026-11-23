#include <bits/stdc++.h>
using namespace std;

double ask(long long x1,long long y1,long long x2,long long y2){
    printf("query %lld %lld %lld %lld\n",x1,y1,x2,y2);
    fflush(stdout);
    char buf[256];
    if(scanf("%255s",buf)!=1) exit(0);
    string s(buf);
    if(s=="-1") exit(0);
    return atof(buf);
}

int main(){
    const long long W=100000;
    long long y0=-1,yprev=0; double L=0;
    for(long long k=1;k<=502;k++){
        long long y=199*k;
        if(y>W) break;
        double r=ask(0,y,W,y);
        if(r>0.5){y0=y;L=r;yprev=199*(k-1);break;}
    }
    if(y0<0){ // should not happen
        printf("answer 50000 50000 100\n");fflush(stdout);return 0;
    }
    long long N=llround(L*L/4.0);
    // candidates (cy, r)
    struct C{long long cy,r,s;};
    vector<C> cand;
    long long dmax=y0-yprev;
    for(long long d=1;d<=dmax;d++){
        if(N%d) continue;
        long long e=N/d;
        if((d+e)%2) continue;
        long long r=(d+e)/2, u=(e-d)/2;
        long long cy=y0+u;
        if(r<100) continue;
        if(cy-r<0||cy+r>W) continue;
        cand.push_back({cy,r,e});
    }
    long long rmin=100;
    if(!cand.empty()){
        rmin=LLONG_MAX;
        for(auto&c:cand) rmin=min(rmin,c.r);
    }
    rmin=max(rmin,(long long)ceil(L/2-1e-3));
    rmin=max(rmin,100LL);
    long long lo=rmin,hi=W-rmin;
    long long cx=-1;
    const double eps=1e-4;
    while(lo<hi){
        long long m=(lo+hi)/2;
        if(m<1) m=1;
        double res=ask(0,y0,m,y0);
        if(res<eps){
            long long nl=(long long)ceil(m+L/2-eps);
            lo=max(lo,nl);
        } else if(res>L-eps){
            long long nh=(long long)floor(m-L/2+eps);
            hi=min(hi,nh);
        } else {
            cx=llround(m-res+L/2);
            break;
        }
    }
    if(cx<0) cx=lo;
    // filter candidates by cx
    vector<C> c2;
    for(auto&c:cand){
        if(cx-c.r<0||cx+c.r>W) continue;
        c2.push_back(c);
    }
    if(c2.empty()) c2=cand;
    long long cy,r;
    if(c2.size()==1){
        cy=c2[0].cy;r=c2[0].r;
    } else if(c2.empty()){
        // fallback: measure directly
        double s=ask(cx,y0,cx,W);
        long long S=llround(s);
        long long D=llround(N/(double)max(S,1LL));
        r=(S+D)/2; cy=y0+(S-D)/2;
    } else {
        double s=ask(cx,y0,cx,W);
        long long best=0;double bd=1e18;
        for(size_t i=0;i<c2.size();i++){
            double df=fabs(c2[i].s-s);
            if(df<bd){bd=df;best=i;}
        }
        cy=c2[best].cy;r=c2[best].r;
    }
    printf("answer %lld %lld %lld\n",cx,cy,r);
    fflush(stdout);
    return 0;
}
