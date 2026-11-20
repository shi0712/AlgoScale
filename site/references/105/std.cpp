#include <bits/stdc++.h>
using namespace std;

using Perm=vector<int>;
struct Node {
    int a=-1,b=-1,left=-1,right=-1;
    Perm result;
};
vector<Node> nodes;
int build(const vector<Perm>& possible,int depth) {
    if(possible.size()==1) {
        Node n;n.result=possible[0];nodes.push_back(n);return int(nodes.size())-1;
    }
    if(int(possible.size())>(1<<depth))return -1;
    int len=possible[0].size();
    for(int a=0;a<len;++a)for(int b=a+1;b<len;++b) {
        vector<Perm> l,r;
        for(const auto& p:possible)
            (find(p.begin(),p.end(),a)<find(p.begin(),p.end(),b)?l:r).push_back(p);
        if(l.empty() || r.empty())continue;
        int left=build(l,depth-1);if(left<0)continue;
        int right=build(r,depth-1);if(right<0)continue;
        nodes.push_back({a,b,left,right,{}});return int(nodes.size())-1;
    }
    return -1;
}
int decisionTree(int k) {
    Perm p(k+2);iota(p.begin(),p.end(),0);vector<Perm> possible;
    do {
        if(find(p.begin(),p.end(),0)>find(p.begin(),p.end(),1))continue;
        bool ok=true;
        for(int i=0;i<k;++i)if(p[k+1-i]>i+2)ok=false;
        if(ok)possible.push_back(p);
    } while(next_permutation(p.begin(),p.end()));
    return build(possible,(k==3?5:2*k));
}
bool lessAt(int a,int b) {
    cout<<"? "<<a<<' '<<b<<endl;char c;
    if(!(cin>>c) || (c!='<' && c!='>'))exit(0);
    return c=='<';
}
int main() {
    int n;cin>>n;if(n==1){cout<<"! 1"<<endl;return 0;}
    vector<int> root(4);for(int i=1;i<=3;++i)root[i]=decisionTree(i);
    vector<int> remaining={n-1,n}, answer(n+1);
    if(!lessAt(n-1,n))swap(remaining[0],remaining[1]);
    int next=n-2,value=n;
    while(next>=1) {
        int k=min(3,next);
        for(int i=0;i<k;++i)remaining.push_back(next--);
        int at=root[k];
        while(nodes[at].a>=0) {
            auto& nd=nodes[at];
            at=lessAt(remaining[nd.a],remaining[nd.b])?nd.left:nd.right;
        }
        auto& order=nodes[at].result;
        for(int j=k+1;j>=2;--j)answer[remaining[order[j]]]=value--;
        remaining={remaining[order[0]],remaining[order[1]]};
    }
    answer[remaining[0]]=1;answer[remaining[1]]=2;
    cout<<"!";for(int i=1;i<=n;++i)cout<<' '<<answer[i];cout<<endl;
}
