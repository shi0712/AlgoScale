#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <numeric>
#include <vector>
using namespace std;
int query(const vector<int>& v){
    cout<<"? 1 "<<v.size();
    for(int x:v)cout<<" "<<x+1;
    cout<<endl;
    int z;if(!(cin>>z))exit(0);return z;
}
void flip(int v){cout<<"? 2 "<<v+1<<endl;}
struct Branch {
    int representative,length=0,code=0;
    vector<int> members;
};
void solve(int n){
    vector<vector<int>> graph(n);
    for(int i=1;i<n;++i){
        int a,b;cin>>a>>b;--a;--b;
        graph[a].push_back(b);graph[b].push_back(a);
    }
    vector<int> sums(n),active(n);
    iota(active.begin(),active.end(),0);
    for(int i=0;i<n;++i)sums[i]=query({i});
    while(active.size()>1){
        int size=active.size();
        vector<bool> live(n);for(int v:active)live[v]=true;
        vector<int> order{active.front()},parent(n,-1),subtree(n,1);
        for(size_t i=0;i<order.size();++i)
            for(int w:graph[order[i]])if(live[w]&&w!=parent[order[i]]){
                parent[w]=order[i];order.push_back(w);
            }
        int center=-1;
        for(auto it=order.rbegin();it!=order.rend();++it){
            int v=*it,largest=0;
            for(int w:graph[v])if(live[w]&&parent[w]==v){
                subtree[v]+=subtree[w];largest=max(largest,subtree[w]);
            }
            if(max(largest,size-subtree[v])<=size/2)center=v;
        }
        vector<Branch> parts{{-1,0,0,{center}}};
        for(int w:graph[center])if(live[w]){
            vector<int> branch{w};parent[w]=center;
            for(size_t i=0;i<branch.size();++i)
                for(int u:graph[branch[i]])if(live[u]&&u!=parent[branch[i]]){
                    parent[u]=branch[i];branch.push_back(u);
                }
            parts.push_back({w,0,0,move(branch)});
        }
        // Shannon prefix lengths telescope over centroid levels.
        for(auto &b:parts)while((int(b.members.size())<<b.length)<size)++b.length;
        sort(parts.begin(),parts.end(),[](auto &a,auto &b){return a.length<b.length;});
        for(int i=1;i<int(parts.size());++i)
            parts[i].code=(parts[i-1].code+1)<<(parts[i].length-parts[i-1].length);
        vector<int> choices(parts.size());iota(choices.begin(),choices.end(),0);
        flip(center);
        for(int depth=0;choices.size()>1;++depth){
            vector<int> half[2];int test=0;
            for(int i:choices){
                int bit=(parts[i].code>>(parts[i].length-depth-1))&1;
                half[bit].push_back(i);
                if(parts[i].representative<0)test=1-bit;
            }
            if(half[0].empty()||half[1].empty()){
                choices.swap(half[half[0].empty()?1:0]);continue;
            }
            vector<int> nodes;int before=0;
            for(int i:half[test]){
                int v=parts[i].representative;nodes.push_back(v);before+=sums[v];
            }
            int changed=abs(query(nodes)-before)/2;
            choices.swap(half[changed<int(nodes.size())?test:1-test]);
        }
        flip(center);
        active=move(parts[choices[0]].members);
    }
    int root=active.front();
    vector<int> result(n),order{root},parent(n,-1);
    result[root]=sums[root];
    for(size_t i=0;i<order.size();++i)
        for(int v:graph[order[i]])if(v!=parent[order[i]]){
            parent[v]=order[i];result[v]=sums[v]-sums[order[i]];order.push_back(v);
        }
    cout<<"!";for(int x:result)cout<<" "<<x;cout<<endl;
}
int main(){{int n;cin>>n;solve(n);}}
