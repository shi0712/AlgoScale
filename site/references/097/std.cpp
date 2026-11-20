#include <algorithm>
#include <array>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>
using namespace std;
string ask(const vector<int>& radii) {
    cout<<"?";for(int r:radii)cout<<' '<<r;cout<<endl;
    string s;if(!(cin>>s)||s.size()!=radii.size())exit(0);return s;
}
int widthBits(int size) {
    int b=0;while((1<<b)<size)++b;return b;
}
int main() {
    int n;if(!(cin>>n))return 0;
    if(n==2){cout<<"!\n1 2"<<endl;return 0;}
    vector<int> block(n),depth(n),parent(n),label(n);
    array<string,3> last;
    for(int width=1<<widthBits(n);width>1;width/=2) {
        vector<int> next=block;int mid=width/2;
        for(int r=0;r<3;++r) {
            vector<int> radii(n);
            bool needed=width==2;
            for(int v=1;v<n;++v) {
                if(block[v]%3==r)radii[v]=mid;
                if(block[v]%3==(r+1)%3)needed=true;
            }
            if(!needed)continue;
            if(r==2)radii[0]=mid;
            string lit=ask(radii);
            if(width==2)last[r]=lit;
            for(int v=1;v<n;++v)if(block[v]%3==(r+1)%3)
                next[v]=2*block[v]+(lit[v]=='0');
        }
        block.swap(next);
    }
    for(int v=1;v<n;++v)depth[v]=block[v]+1;
    int deepest=*max_element(depth.begin(),depth.end());
    vector<vector<int>> candidates(deepest+1);
    candidates[0].push_back(0);
    for(int v=1;v<n;++v) {
        int d=depth[v];
        // The width-2 block containing children is indexed by floor(d/2).
        // Its colour differs from the grandparent-side neighbour's colour.
        if(last[(d/2)%3][v]=='1') {
            label[v]=candidates[d].size();
            candidates[d].push_back(v);
        }
    }
    vector<int> bits(deepest+3);
    for(int d=0;d<deepest;++d)bits[d]=widthBits(candidates[d].size());
    int rounds=0;
    for(int d=0;d<deepest;++d)rounds=max(rounds,bits[d]+bits[d+2]);
    // On the path d,d+2,... of interfering source layers, alternate a
    // prefix and suffix of the rounds as disjoint transmission slots.
    auto start=[&](int d){return (d/2)%2 ? rounds-bits[d] : 0;};
    for(int t=0;t<rounds;++t) {
        vector<int> radii(n);
        for(int v=0;v<n;++v) {
            int d=depth[v],b=t-start(d);
            if(0<=b && b<bits[d])radii[v]=(label[v]>>b)&1;
        }
        string lit=ask(radii);
        for(int v=1;v<n;++v) {
            int d=depth[v]-1,b=t-start(d);
            if(0<=b && b<bits[d] && lit[v]=='1')parent[v]|=1<<b;
        }
    }
    cout<<"!\n";
    for(int v=1;v<n;++v)cout<<candidates[depth[v]-1][parent[v]]+1<<' '<<v+1<<'\n';
    cout<<flush;
}
