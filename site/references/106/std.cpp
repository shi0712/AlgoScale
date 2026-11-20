#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <iostream>
using namespace std;

constexpr int side=100000;
long double probe(int x1,int y1,int x2,int y2) {
    cout<<"query "<<x1<<' '<<y1<<' '<<x2<<' '<<y2<<endl;
    long double length;
    if (!(cin>>length)) exit(0);
    return length;
}

int main() {
    int center_x=99700, radius=100;
    int s=99800;
    for(int i=0;i<498;++i) {
        int scan=199+200*i;
        if(probe(scan,0,scan+1,side)<0.01L) continue;
        s=scan;break;
    }
    long double left=probe(s,0,s,side),right=probe(s+1,0,s+1,side);
    if(left>0.01L||right>0.01L) {
        long long a=llroundl(left*left/4), b=llroundl(right*right/4);
        center_x=int((b-a+2*s+1)/2);
        long long offset=center_x-s;
        radius=int(llroundl(sqrtl(a+offset*offset)));
    }
    int start=radius/3, end=2*radius/3, slope=end-start;
    long double measured=probe(center_x+start,0,center_x+end,side);
    auto chord=[&](int y) {
        long double distance=(1.L*start*side+1.L*slope*y)/hypotl(side,slope);
        return 2*sqrtl(1.L*radius*radius-distance*distance);
    };
    int lo=radius, hi=side-radius;
    while(lo<hi) {
        int mid=(lo+hi)/2;
        if(chord(mid)>measured) lo=mid+1;
        else hi=mid;
    }
    if(lo>radius && fabsl(chord(lo-1)-measured)<fabsl(chord(lo)-measured)) --lo;
    cout<<"answer "<<center_x<<' '<<lo<<' '<<radius<<endl;
}
