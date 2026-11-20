#include <bits/stdc++.h>
#ifndef towns_h
#define towns_h

int getDistance(int i, int j);
int hubDistance(int N, int sub);

#endif

#include <algorithm>
#include <map>
#include <vector>
using namespace std;
int hubDistance(int n,int sub) {
    vector<vector<int>> cache(n,vector<int>(n,-1));
    auto distance=[&](int a,int b) {
        if(a==b) return 0;
        if(cache[a][b]<0) cache[a][b]=cache[b][a]=getDistance(a,b);
        return cache[a][b];
    };
    int a=0,b=0;
    for(int i=1;i<n;++i) if(distance(0,i)>distance(0,a)) a=i;
    for(int i=0;i<n;++i) if(distance(a,i)>distance(a,b)) b=i;
    int diameter=distance(a,b);
    auto foot=[&](int i) { return (distance(a,i)+distance(a,0)-distance(0,i))/2; };
    int junction=foot(b);
    map<int,vector<int>> attached;
    for(int i=0;i<n;++i) attached[foot(i)].push_back(i);
    int radius=diameter;
    for(const auto &entry:attached) {
        int h=entry.first;
        if(h>0 && h<distance(a,0) && h<=junction) radius=min(radius,max(h,diameter-h));
    }
    if(sub<=2) return radius;
    int left=0;
    for(const auto &entry:attached) {
        int h=entry.first;
        const auto &group=entry.second;
        int size=group.size(),right=n-left-size;
        bool candidate=h>0 && h<distance(a,0) && h<=junction && max(h,diameter-h)==radius;
        if(candidate && max(left,right)<=n/2) {
            if(size<=n/2) return radius;
            auto equal=[&](int i,int j) {
                return distance(i,j)<distance(a,i)+distance(a,j)-2*h;
            };
            struct Block { int leader, count; vector<int> different; };
            vector<Block> blocks;
            int balance=0;
            for(int i:group) {
                if(balance==0) { blocks.push_back({i,1,{}}); balance=1; }
                else if(equal(i,blocks.back().leader)) { ++blocks.back().count; ++balance; }
                else { blocks.back().different.push_back(i); --balance; }
            }
            // Each cancelled pair belongs to different branches. Even its
            // best possible contribution to a single branch is one town.
            if((size+balance)/2<=n/2) return radius;
            int winner=blocks.back().leader, count=blocks.back().count;
            for(int k=0;k+1<int(blocks.size());++k) {
                const auto &blk=blocks[k];
                if(equal(winner,blk.leader)) count+=blk.count;
                else for(int i:blk.different) count+=equal(winner,i);
            }
            if(count<=n/2) return radius;
        }
        left+=size;
    }
    return -radius;
}

int getDistance(int a,int b){
    std::cout<<"? "<<a<<' '<<b<<std::endl;int r;
    if(!(std::cin>>r)||r<0)std::exit(0);
    return r;
}
int main(){
    std::ios::sync_with_stdio(false);std::cin.tie(nullptr);
    int n;if(!(std::cin>>n))return 0;
    int d=hubDistance(n,6);std::cout<<"! "<<d<<std::endl;
}
