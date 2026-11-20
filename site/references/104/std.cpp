#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <numeric>
#include <vector>
using namespace std;
using Classes=vector<vector<int>>;

int move_item(int item) {
    if(item) cout<<"+ "<<item<<endl; else cout<<"-"<<endl;
    int result;
    if(!(cin>>result)||result<0) exit(0);
    return result;
}

Classes partition(int first,int n) {
    if(n==1) return {{first}};
    int a=n/2;
    Classes l=partition(first,a),r=partition(first+a,n-a);
    vector<int> lm,rm;
    int count=0;
    for(auto& g:l) count=move_item(g[0]);
    for(int i=0;i<int(r.size());++i) {
        int next=move_item(r[i][0]);
        if(next==count) rm.push_back(i);
        count=next;
    }
    for(int i=0;i<int(l.size());++i) {
        int next=move_item(0);
        if(next==count) lm.push_back(i);
        count=next;
    }
    for(size_t i=0;i<r.size();++i) count=move_item(0);

    int t=int(lm.size());
    vector<int> mate(r.size(),-1);
    if(t>0) {
        int bits=0;
        while((1<<bits)<t) bits+=2;
        vector<int> codes(1<<bits);
        iota(codes.begin(),codes.end(),0);
        sort(codes.begin(),codes.end(),[](int x,int y){
            int a=__builtin_popcount(unsigned(x)),b=__builtin_popcount(unsigned(y));
            return a==b ? x<y : a<b;
        });
        vector<int> decoded(1<<bits,-1),ids(t-1);
        for(int i=0;i<t;++i) decoded[codes[i]]=lm[i];
        for(int j=0;j<bits;j+=2) {
            int na=0,nb=0;
            for(int i=0;i<t;++i) if(codes[i]&(1<<j)) {count=move_item(l[lm[i]][0]);++na;}
            for(int i=0;i<t-1;++i) {
                int next=move_item(r[rm[i]][0]);
                if(next==count) ids[i]|=1<<j;
                count=next;
            }
            for(int i=0;i<t;++i) if(codes[i]&(2<<j)) {count=move_item(l[lm[i]][0]);++nb;}
            while(na--) count=move_item(0);
            for(int i=0;i<t-1;++i) {
                int next=move_item(0);
                if(next==count) ids[i]|=2<<j;
                count=next;
            }
            while(nb--) count=move_item(0);
        }
        int missing=0;
        for(int x:lm) missing^=x;
        for(int i=0;i<t-1;++i) {
            mate[rm[i]]=decoded[ids[i]];
            missing^=mate[rm[i]];
        }
        mate[rm.back()]=missing;
    }
    for(int i=0;i<int(r.size());++i) {
        if(mate[i]<0) l.push_back(std::move(r[i]));
        else l[mate[i]].insert(l[mate[i]].end(),r[i].begin(),r[i].end());
    }
    return l;
}

int main() {
    int n;if(!(cin>>n))return 0;
    Classes result=partition(1,n);
    cout<<"! "<<result.size()<<'\n';
    for(auto& c:result) {
        cout<<c.size();
        for(int x:c) cout<<' '<<x;
        cout<<'\n';
    }
    cout<<flush;
}
