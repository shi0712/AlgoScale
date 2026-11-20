#include <algorithm>
#include <array>
#include <bit>
#include <cassert>
#include <cstdlib>
#include <iostream>
#include <map>
#include <string>
#include <vector>
using namespace std;
using I = long long;

constexpr I max_width = 399999999;
constexpr array<I,6> offsets{164455670,122512630,69254700,18898705,1204020,2135};
I n,x,y;
int ceiling_log(I values) { return bit_width(static_cast<unsigned long long>(values-1)); }
bool examine(I a,I b) {
    if(a<1 || b<1 || a>n || b>n) return false;
    cout<<"? "<<a<<' '<<b<<endl;
    string reply;
    if(!(cin>>reply)) exit(0);
    return reply=="true";
}

// An offset from the other edge with the parity of first.  'index' encodes
// distance first%2 + 2*index.  Conservative bounds also work before first is known.
int second_budget(I last,I width_lower,I first) {
    for(int h=ceiling_log(last+1);;h++) {
        I index=0;
        for(int rem=h-1;rem>=0 && index<last;rem--)
            index+=min(max(width_lower,first+2*index+1)/2,1LL<<rem);
        if(index>=last) return h;
    }
}

bool plan_first(I upper,I lower,I cap1,I cap2,int available,vector<I>& probes) {
    probes.clear();
    I a=0;
    while(a<cap1) {
        int other=second_budget(min(cap2,upper-1-a)/2,lower,a);
        int refinement=available-int(probes.size())-1-other;
        if(refinement<0) return false;
        a+=min({cap1-a,max(lower,a+1),1LL<<min(refinement,40)});
        probes.push_back(a);
    }
    return int(probes.size())+second_budget(min(cap2,upper-1-a)/2,lower,a)<=available;
}

pair<I,I> horizontal(I upper,I lower,array<I,2> caps,int h,bool certificate=false) {
    vector<I> probes;
    int side=0;
    for(;side<2;side++)
        if(plan_first(upper,lower,min(caps[side],upper-1),min(caps[1-side],upper-1),h,probes)) break;
    assert(side<2);
    if(certificate) return {0,0};
    int sign=side==0?-1:1;
    I first=0;
    for(I q:probes) {
        if(examine(x+sign*q,y)) { first=q;continue; }
        I hi=q-1;
        while(first<hi) {
            I mid=(first+hi+1)/2;
            if(examine(x+sign*mid,y)) first=mid;else hi=mid-1;
        }
        break;
    }
    I parity=first%2;
    I limit=(min(caps[1-side],upper-1-first)-parity)/2;
    int remaining=second_budget(limit,lower,first);
    I a=0;
    while(a<limit) {
        --remaining;
        I q=a+min({limit-a,max(lower,first+2*a+1)/2,1LL<<remaining});
        if(examine(x-sign*(parity+2*q),y)) { a=q;continue; }
        limit=q-1;
        while(a<limit) {
            I mid=(a+limit+1)/2;
            if(examine(x-sign*(parity+2*mid),y)) a=mid;else limit=mid-1;
        }
        break;
    }
    I second=parity+2*a;
    return side==0?make_pair(first,second):make_pair(second,first);
}

struct Test { int dx,dy,mask; };
vector<pair<int,int>> cells;
vector<Test> tests;
map<int,pair<int,int>> decisions;
pair<int,int> tree(int mask) {
    if(has_single_bit(unsigned(mask))) return {0,-1};
    if(decisions.count(mask)) return decisions.at(mask);
    pair<int,int> best{100,-1};
    for(int i=0;i<int(tests.size());i++) {
        int yes=mask&tests[i].mask;
        if(yes && yes!=mask) best=min(best,{1+max(tree(yes).first,tree(mask^yes).first),i});
    }
    return decisions[mask]=best;
}
void build_tree() {
    for(int a=0;a<5;a++) for(int b=0;b<5;b++)
        if((a+b)%2==0) cells.emplace_back(a,b);
    for(int dx=-4;dx<=4;dx++) for(int dy=-4;dy<=4;dy++) if((dx+dy)%2==0) {
        int mask=0;
        for(int j=0;j<int(cells.size());j++) {
            auto [a,b]=cells[j];
            if(0<=a+dx && a+dx<5 && 0<=b+dy && b+dy<5) mask|=1<<j;
        }
        tests.push_back({dx,dy,mask});
    }
    assert(tree((1<<13)-1).first==4);
}
int main(int argc,char** argv) {
    build_tree();
    bool certificate=argc==2 && string(argv[1])=="--certificate";
    if(!certificate && !(cin>>n>>x>>y)) return 0;
    array<I,2> caps{max_width-1,max_width-1};
    I upper=max_width,lower=3;
    int used=0;
    for(I d:offsets) {
        int side=used%2;
        ++used;
        I found_lower=max(3LL,d/5+1);
        if(certificate) {
            horizontal(upper,found_lower,caps,92-used-ceiling_log(upper)-4,true);
            cout<<"positive probe "<<used<<" verified\n";
        } else if(examine(x+(side==0?-d:d),y)) {
            lower=found_lower;break;
        }
        caps[side]=min(caps[side],d-1);
        upper=min(upper,caps[0]+caps[1]+1);
    }
    auto [left,right]=horizontal(upper,lower,caps,92-used-ceiling_log(upper)-4,certificate);
    if(certificate) { cout<<"all-negative branch verified\n";return 0; }
    I m=left+right+1,lo=0,hi=m-1;
    while(lo<hi) {
        I mid=(lo+hi+1)/2;
        if(examine(x,y-mid)) lo=mid;else hi=mid-1;
    }
    I cx=x-left+m/2,cy=y-lo+m/2;
    int mask=(1<<13)-1;
    while(!has_single_bit(unsigned(mask))) {
        Test t=tests[tree(mask).second];
        mask&=examine(cx+t.dx*m,cy+t.dy*m)?t.mask:~t.mask;
    }
    auto [a,b]=cells[countr_zero(unsigned(mask))];
    cout<<"! "<<cx+(2-a)*m<<' '<<cy+(2-b)*m<<endl;
}
