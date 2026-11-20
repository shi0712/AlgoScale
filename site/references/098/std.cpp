#include <bits/stdc++.h>
using namespace std;

// The oracle scores an n-bit guess and may terminate the interaction on a hit.
template<class Oracle> string read_orient(int n, Oracle ask) {
    struct Column { int mask, bit, position; };
    vector<Column> columns;
    int rows = 1;
    while (int(columns.size()) + 1 < n) {
        int mask = rows++;
        for (int j = 0; j < __builtin_popcount(unsigned(mask)) &&
             int(columns.size()) + 1 < n; ++j)
            columns.push_back({mask, j, int(columns.size())});
    }
    vector<vector<int>> odd(rows, vector<int>(rows, INT_MAX));
    for (int mask=1; mask<rows; ++mask) {
        int rank=0;
        for (int t=0; t<=mask; ++t)
            if ((t & mask)==t && __builtin_parity(unsigned(t))) odd[mask][t]=rank++;
    }
    vector<vector<int>> active(rows);
    for (int x=1; x<rows; ++x)
        for (auto c:columns)
            if (odd[c.mask][x & c.mask] < (1<<c.bit)) active[x].push_back(c.position);
    vector<int> response(rows);
    int weight=0;
    for (int x=0; x<rows; ++x) {
        string g(n,'F');
        for (int i:active[x]) g[i]='T';
        int score=ask(g);
        if (score==n) return g;
        if (!x) weight=n-score;
        response[x]=(score-(n-weight)+int(active[x].size()))/2;
    }
    string answer(n,'F');
    for (int s=rows-1; s>0; --s) {
        int coefficient=0;
        for (int t=s;; t=(t-1)&s) {
            coefficient+=(__builtin_parity(unsigned(s^t))?-1:1)*response[t];
            if (t==0) break;
        }
        coefficient=abs(coefficient);
        for (auto c:columns) if (c.mask==s && (coefficient & (1<<c.bit))) {
            answer[c.position]='T'; --weight;
            for (int x=0; x<rows; ++x)
                if (odd[s][x&s] < (1<<c.bit)) --response[x];
        }
    }
    if (weight) answer.back()='T';
    return answer;
}
int ask(int v,const string& s){
    cout<<"? "<<v+1<<' '<<s<<endl;int d;if(!(cin>>d)||d<0)exit(0);return d;
}
struct Node{int x,y,child[3];};
// A negative child -(4*x+y+1) returns the two quaternary digits.
const Node decision[]={
    {1,3,{1,2,3}},{2,4,{4,5,-7}},{3,13,{6,7,8}},{0,1,{-2,-1,0}},
    {4,0,{-16,-12,0}},{4,4,{-8,-15,-11}},{4,0,{-14,-10,0}},
    {4,1,{-6,-13,-9}},{1,4,{-5,-4,-3}}
};
int main(){
    int n;if(!(cin>>n))return 0;int m=n-1;
    string orientation=read_orient(m,[&](string s){
        for(char& c:s)c=c=='T'?'1':'0';
        return ask(0,s);
    });
    for(char& c:orientation)c=c=='T'?'1':'0';
    int maxbits=0;while((1<<maxbits)<n)++maxbits;
    vector<vector<int>> cost(n+1,vector<int>(n+1));
    for(int a=1;a<=n;++a)for(int b=1;b<=n;++b){
        auto height=[](int a){int d=0;while((1<<d)<a)++d;return d;};
        cost[a][b]=height(a)+height(b);
        if(min(a,b)>1)cost[a][b]=min(cost[a][b],3+cost[(a+3)/4][(b+3)/4]);
    }
    vector<vector<int>> layer(n);layer[0]={0};vector<int> depth(n);
    for(int v=1;v<n;++v){depth[v]=m-ask(v,orientation);layer[depth[v]].push_back(v);}
    vector<int> incoming(n,-1),from(m),to(m),unassigned(m);iota(unassigned.begin(),unassigned.end(),0);
    for(int d=1;d<n;++d)for(int v:layer[d]){
        vector<int> edges=unassigned,parents=layer[d-1];
        auto probe=[&](const vector<int>& es,const vector<int>& ps){
            string q=orientation;int flipped=0;
            for(int e:es){q[e]^=1;++flipped;}
            for(int u:ps){q[incoming[u]]^=1;++flipped;}
            return (ask(v,q)-(m-depth[v])+flipped)/2;
        };
        while(edges.size()>1 || parents.size()>1){
            int a=edges.size(),b=parents.size();
            if(min(a,b)>1 && 3+cost[(a+3)/4][(b+3)/4]==cost[a][b]){
                array<vector<int>,4> ex,py;
                for(int i=0;i<a;++i)ex[i%4].push_back(edges[i]);
                for(int i=0;i<b;++i)py[i%4].push_back(parents[i]);
                int node=0;
                do{
                    vector<int> se,sp;
                    for(int k=0;k<4;++k){
                        if(decision[node].x>>k&1)se.insert(se.end(),ex[k].begin(),ex[k].end());
                        if(decision[node].y>>k&1)sp.insert(sp.end(),py[k].begin(),py[k].end());
                    }
                    node=decision[node].child[probe(se,sp)];
                }while(node>=0);
                int digit=-node-1;edges=ex[digit/4];parents=py[digit%4];
            }else{
                bool is_edge=a>1;auto& pool=is_edge?edges:parents;
                int mid=pool.size()/2;
                vector<int> part(pool.begin(),pool.begin()+mid),empty;
                if(probe(is_edge?part:empty,is_edge?empty:part))pool=move(part);
                else pool.erase(pool.begin(),pool.begin()+mid);
            }
        }
        int e=edges[0],p=parents[0];incoming[v]=e;
        from[e]=v;to[e]=p;if(orientation[e]=='0')swap(from[e],to[e]);
        unassigned.erase(find(unassigned.begin(),unassigned.end(),e));
    }
    cout<<"!";for(int e=0;e<m;++e)cout<<' '<<from[e]+1<<' '<<to[e]+1;cout<<endl;
}
