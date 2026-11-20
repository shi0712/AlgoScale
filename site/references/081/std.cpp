#include <algorithm>
#include <array>
#include <cstdlib>
#include <iostream>
#include <queue>
#include <random>
#include <vector>
using namespace std;
using Group=array<vector<int>,2>;
struct Answer {bool bipartite; vector<int> certificate;};
template<class Ask> Answer solve(int n,Ask ask){
    vector<Group> components(n);
    vector<int> owner(n,-1);
    vector<vector<int>> tree(n);
    for(int v=0;v<n;++v){
        vector<int> active;
        for(int id=0;id<v;++id) if(!components[id][0].empty())active.push_back(id);
        Group merged;merged[0].push_back(v);owner[v]=v;
        auto count=[&](vector<int> side){
            if(side.empty())return 0;
            side.push_back(v);return ask(side);
        };
        auto locate=[&](vector<int> side,int degree){
            while(side.size()>1){
                int mid=(side.size()+1)/2;
                vector<int> half(side.begin(),side.begin()+mid);
                int found=count(half);
                if(found){side.swap(half);degree=found;}
                else side.erase(side.begin(),side.begin()+mid);
            }
            return side[0];
        };
        while(!active.empty()){
            vector<int> set;
            int side=0,degree=0;
            for(;side<2;++side){
                set.clear();
                for(int id:active)set.insert(set.end(),components[id][side].begin(),components[id][side].end());
                degree=count(set);
                if(degree)break;
            }
            if(!degree)break;
            int x=locate(set,degree), id=owner[x];
            Group &part=components[id];
            int opposite=count(part[1-side]);
            if(opposite){
                int y=locate(part[1-side],opposite);
                vector<int> pred(n,-1);queue<int> q;q.push(x);pred[x]=x;
                while(pred[y]<0){int z=q.front();q.pop();for(int w:tree[z])if(pred[w]<0){pred[w]=z;q.push(w);}}
                vector<int> cycle={v,y};while(cycle.back()!=x)cycle.push_back(pred[cycle.back()]);
                return {false,cycle};
            }
            tree[v].push_back(x);tree[x].push_back(v);
            for(int s=0;s<2;++s)for(int z:part[s]){merged[s==side?1:0].push_back(z);owner[z]=v;}
            part={};active.erase(find(active.begin(),active.end(),id));
        }
        components[v]=move(merged);
    }
    vector<int> partition;
    for(auto &component:components)partition.insert(partition.end(),component[0].begin(),component[0].end());
    return {true,partition};
}
#ifdef VERIFY
int main(){
    mt19937 gen(387);int cases=0,peak=0;
    auto check=[&](const vector<vector<bool>> &g){
        int n=g.size(),calls=0;
        auto ans=solve(n,[&](const vector<int>& vs){
            ++calls;int total=0;
            for(int i=0;i<int(vs.size());++i)for(int j=0;j<i;++j){if(vs[i]==vs[j])abort();total+=g[vs[i]][vs[j]];}
            return total;
        });
        vector<int> seen(n);
        for(int i:ans.certificate)if(seen[i]++)abort();
        if(ans.bipartite){for(int i=0;i<n;++i)for(int j=0;j<i;++j)if(g[i][j]&&seen[i]==seen[j])abort();}
        else {auto &v=ans.certificate;if(v.size()%2!=1||v.size()<3)abort();for(int i=0;i<int(v.size());++i)if(!g[v[i]][v[(i+1)%v.size()]])abort();}
        if(calls>15*(n-1)+10)abort();++cases;peak=max(peak,calls);
    };
    for(int n=1;n<=6;++n){int m=n*(n-1)/2;for(int mask=0;mask<1<<m;++mask){
        vector<vector<bool>> g(n,vector<bool>(n));int bit=0;
        for(int i=0;i<n;++i)for(int j=0;j<i;++j)g[i][j]=g[j][i]=(mask>>bit++)&1;
        check(g);
    }}
    for(int repetition=0;repetition<64;++repetition){int n=1+gen()%600;vector<vector<bool>>g(n,vector<bool>(n));for(int i=0;i<n;++i)for(int j=0;j<i;++j)g[i][j]=g[j][i]=(i%2!=j%2||repetition%3==0)&&gen()%100<4;check(g);}
    cout<<cases<<" graph certificates, observed max "<<peak<<'\n';
}
#else
int main(){
    int n;if(!(cin>>n))return 0;
    auto answer=solve(n,[](const vector<int>&s){cout<<"? "<<s.size()<<'\n';for(int v:s)cout<<v+1<<' ';cout<<endl;int ans;if(!(cin>>ans)||ans<0)exit(0);return ans;});
    cout<<(answer.bipartite?'Y':'N')<<' '<<answer.certificate.size()<<'\n';for(int v:answer.certificate)cout<<v+1<<' ';cout<<endl;
}
#endif
