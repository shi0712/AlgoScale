#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <map>
#include <vector>
using namespace std;
using ll=long long;using Triple=array<int,3>;
ll pairs(ll x){return x*(x-1)/2;}
int lengthOf(ll value) {
    ll a=(1+ll(sqrtl(1+8.0L*value)))/2;
    while(pairs(a)>value)--a;
    while(pairs(a)<value)++a;
    return int(a);
}
ll ask(int l,int r) {
    cout<<"? "<<l<<' '<<r<<endl;ll v;
    if(!(cin>>v)||v<0)exit(0);return v;
}
int localValue(Triple t,int l,int r) {
    auto count=[&](int x,int y){return pairs(max(0,min(y,r)-max(x,l)+1));};
    return int(count(t[0],t[1]-1)+count(t[1],t[2]));
}
// Choose a legal local query whose every reply can be finished on time.
bool choose(const vector<Triple>& set,int n,int budget,pair<int,int>& query) {
    if(set.size()<=1)return true;
    if(!budget)return false;
    for(int l=1;l<n;++l)for(int r=l+1;r<=n;++r) {
        map<int,vector<Triple>> groups;
        for(auto x:set)groups[localValue(x,l,r)].push_back(x);
        if(groups.size()<2)continue;
        bool feasible=true;pair<int,int> child;
        for(auto& [value,block]:groups)if(!choose(block,n,budget-1,child)){feasible=false;break;}
        if(feasible){query={l,r};return true;}
    }
    return false;
}
Triple small(int L,int R,ll total) {
    int n=R-L+1;vector<Triple> options;
    for(int i=1;i<=n;++i)for(int j=i+2;j<n;++j)for(int k=j+1;k<=n;++k)
        if(pairs(j-i)+pairs(k-j+1)==total)options.push_back({i,j,k});
    for(int budget=2;options.size()>1;--budget) {
        pair<int,int> q;bool feasible=choose(options,n,budget,q);assert(feasible);if(!feasible)exit(1);
        int value=int(ask(L+q.first-1,L+q.second-1));
        options.erase(remove_if(options.begin(),options.end(),[&](Triple x){return localValue(x,q.first,q.second)!=value;}),options.end());
    }
    auto ans=options[0];for(int& x:ans)x+=L-1;return ans;
}
Triple solve(int n) {
    if(n==4)return {1,3,4};
    ll total=ask(1,n);int L=1,R=n;
    while(R-L+1>10) {
        int mid=L+(R-L)/2;ll value=ask(1,mid);
        if(value==0){L=mid;continue;}
        if(value==total){R=mid;continue;}
        ll delta=value-ask(1,mid-1);
        if(pairs(delta+1)==value) {
            int i=mid-int(delta);
            int a=int(total-ask(i+1,n)+1),j=i+a;
            return {i,j,j+lengthOf(total-pairs(a))-1};
        }
        int j=mid-int(delta),a=lengthOf(value-pairs(delta+1));
        return {j-a,j,j+lengthOf(total-pairs(a))-1};
    }
    return small(L,R,total);
}
int main() {
    {int n;cin>>n;auto t=solve(n);cout<<"! "<<t[0]<<' '<<t[1]<<' '<<t[2]<<endl;}
}
