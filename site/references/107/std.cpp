#include <algorithm>
#include <array>
#include <cstdlib>
#include <iostream>
#include <set>
#include <utility>
using namespace std;

constexpr int side = 32, startup = 5;
int rx, ry;
set<pair<int,int>> black;

void paint(int x, int y) {
    if (!black.emplace(x,y).second) return;
    cout << x << ' ' << y << endl;
    if (!(cin >> rx >> ry) || rx <= 0 || ry <= 0) exit(0);
}

// Within h turns, all marks relevant to the final three possible crossing
// points lie in the interval [-h,h] around the current projection.
constexpr int no_strategy=20, covered=21, uncomputed=22;
constexpr int masks=1<<(2*startup+1);
int memo[startup+1][masks];
int position_bit(int d) { return 1<<(startup+d); }

int guard_action(int h, int mask) {
    const int window=((1<<(2*h+1))-1)<<(startup-h);
    mask &= window;
    int& answer=memo[h][mask];
    if (answer!=uncomputed) return answer;
    const int near=position_bit(-1)|position_bit(0)|position_bit(1);
    if ((mask&near)==near) return answer=covered;
    for (int d=-1;d<=1;d++) {
        if (((mask|position_bit(d))&near)==near) return answer=d;
    }
    if (h==1) return answer=no_strategy;
    for (int dist=0;dist<=h;dist++) {
        for (int d : {-dist,dist}) {
            int bit=position_bit(d);
            if (mask&bit) continue;
            int after=mask|bit;
            if (guard_action(h-1,after<<1)!=no_strategy &&
                guard_action(h-1,after)!=no_strategy &&
                guard_action(h-1,after>>1)!=no_strategy) return answer=d;
        }
    }
    return answer=no_strategy;
}

pair<int,int> guard(bool vertical, int h) {
    int z=vertical?ry:rx;
    int mask=0;
    for (int d=-h;d<=h;d++) {
        int t=z+d;
        auto cell=vertical?make_pair(side,t):make_pair(t,side);
        if (t<1 || t>side || black.count(cell)) mask|=position_bit(d);
    }
    int d=guard_action(h,mask);
    if (d==covered) return {0,0};
    if (d==no_strategy) abort();
    return vertical?make_pair(side,z+d):make_pair(z+d,side);
}

int main() {
    if (!(cin >> rx >> ry)) return 0;
    for (auto& row:memo) fill(begin(row),end(row),uncomputed);
    for (int t=side-startup-1;t<=side;t++) paint(side,t);

    set<pair<int,int>> missing;
    for (int t=1;t<=side;t++) {
        for (auto a : {make_pair(t,side), make_pair(side,t)}) {
            if (!black.count(a)) missing.insert(a);
        }
    }
    int vertical_age=0, horizontal_age=0;
    while (!missing.empty()) {
        vertical_age=rx>=side-startup?min(startup,vertical_age+1):0;
        horizontal_age=ry>=side-startup?min(startup,horizontal_age+1):0;
        pair<int,int> chosen={0,0};
        if (vertical_age) chosen=guard(true,startup+1-vertical_age);
        if (chosen.first==0 && horizontal_age) chosen=guard(false,startup+1-horizontal_age);
        if (chosen.first==0) chosen=*missing.begin();
        if (missing.erase(chosen)!=1) abort();
        paint(chosen.first,chosen.second);
    }

    int x0=1, x1=side-1, y0=1, y1=side-1;
    while (max(x1-x0,y1-y0)>1) {
        if (x1-x0 >= y1-y0) {
            int mid=(x0+x1)/2;
            for (int y=y0;y<=y1;y++) paint(mid,y);
            if (rx<mid) x1=mid-1; else x0=mid+1;
        } else {
            int mid=(y0+y1)/2;
            for (int x=x0;x<=x1;x++) paint(x,mid);
            if (ry<mid) y1=mid-1; else y0=mid+1;
        }
    }
    // The at most four white squares are pairwise adjacent. With at least
    // two white squares, paint one other than the robot's current location.
    while (true) {
        pair<int,int> choice={rx,ry};
        for (int x=x0;x<=x1;x++) for (int y=y0;y<=y1;y++) {
            if (make_pair(x,y)!=make_pair(rx,ry) && !black.count({x,y})) choice={x,y};
        }
        paint(choice.first,choice.second);
    }
}
