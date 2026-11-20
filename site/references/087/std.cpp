#include <algorithm>
#include <array>
#include <cstdlib>
#include <iostream>
#include <numeric>
#include <utility>
#include <vector>
using namespace std;

int ask(int a,int b,int c) {
    cout<<"? "<<a<<' '<<b<<' '<<c<<endl;
    int r;if(!(cin>>r)||r<0)exit(0);return r;
}

struct Four {
    int width;
    vector<int> uniqueEnds, inside;
};
Four inspect(const array<int,4>& p) {
    array<int,4> ranges;
    for (int k=0;k<4;++k) {
        ranges[k]=ask(p[(k+1)%4],p[(k+2)%4],p[(k+3)%4]);
    }
    Four f{*max_element(ranges.begin(),ranges.end()),{},{}};
    for(int k=0;k<4;++k) {
        if(ranges[k]<f.width)f.uniqueEnds.push_back(p[k]);
        else f.inside.push_back(p[k]);
    }
    return f;
}

pair<int,int> extend(const Four& f,const vector<int>& todo) {
    int a=f.uniqueEnds[0],b=f.uniqueEnds[1],middle=f.inside[0],width=f.width;
    for(size_t i=0;i<todo.size();) {
        int x=todo[i++];
        int rx=ask(a,b,x);
        if(i==todo.size()) {
            if(rx>width) {
                int far=ask(a,middle,x)==rx ? a : b;
                a=far;b=x;
            }
            break;
        }
        int y=todo[i++],ry=ask(a,b,y);
        if(rx==width && ry==width)continue;
        if(rx==width || ry==width) {
            if(rx==width) {swap(x,y);swap(rx,ry);}
            int far=ask(a,middle,x)==rx ? a : b;
            a=far;b=x;width=rx;
        } else {
            int joint=ask(a,x,y);
            if(joint>max(rx,ry)) {
                a=x;b=y;width=joint;
            } else {
                int far=joint==max(rx,ry) ? a : b;
                a=far;b=rx>=ry ? x : y;width=max(rx,ry);
            }
        }
    }
    return {a,b};
}

pair<int,int> locate(int n) {
    vector<int> live(n);
    iota(live.begin(),live.end(),1);
    while(live.size()>2) {
        if(live.size()==3) {
            // All removed positions are known not to contain zero.
            int spare=1;
            while(find(live.begin(),live.end(),spare)!=live.end())++spare;
            auto f=inspect({live[0],live[1],live[2],spare});
            return {f.uniqueEnds.front(),f.uniqueEnds.back()};
        }
        vector<int> next;
        size_t i=0;
        while(i+4<=live.size()) {
            Four f=inspect({live[i],live[i+1],live[i+2],live[i+3]});
            i+=4;
            if(f.uniqueEnds.size()==2) {
                next.insert(next.end(),live.begin()+i,live.end());
                return extend(f,next);
            }
            if(f.uniqueEnds.size()==1)next.push_back(f.uniqueEnds[0]);
        }
        next.insert(next.end(),live.begin()+i,live.end());
        live.swap(next);
    }
    return {live.front(),live.back()};
}

int main(){
    ios::sync_with_stdio(false);cin.tie(nullptr);
    {
        int n;cin>>n;
        auto [a,b]=locate(n);
        cout<<"! "<<a<<' '<<b<<endl;
    }
}
