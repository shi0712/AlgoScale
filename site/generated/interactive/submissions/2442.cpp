#include <bits/stdc++.h>
using namespace std;
int n;
pair<int,int> ask(int d){
    printf("? %d\n",d); fflush(stdout);
    int a,b; if(scanf("%d %d",&a,&b)!=2) exit(0);
    if(a<0) exit(0);
    return {a,b};
}
int main(){
    if(scanf("%d",&n)!=1) return 0;
    vector<char> alive(n+1,1);
    // ops: type 0: (v,u) attach v to end != u ; type 1: (v,w): push v front then w front
    vector<array<int,3>> ops;
    int k=n;
    bool complete=false;
    while(k>2 && !complete){
        auto [v,u]=ask(k-2);
        alive[v]=0;
        if(u!=0){ ops.push_back({0,v,u}); k--; continue; }
        k--;
        if(k<=2){ ops.push_back({0,v,0}); break; }
        auto [w,u2]=ask(0);
        alive[w]=0; k--;
        ops.push_back({1,v,w});
        if(u2==0) complete=true;
    }
    deque<int> P;
    for(int i=1;i<=n;i++) if(alive[i]) P.push_back(i);
    for(int i=(int)ops.size()-1;i>=0;i--){
        auto [t,v,x]=ops[i];
        if(t==0){
            if(P.empty()) P.push_back(v);
            else if(P.back()!=x) P.push_back(v);
            else P.push_front(v);
        } else {
            P.push_front(v); P.push_front(x);
        }
    }
    printf("!");
    for(int x:P) printf(" %d",x);
    printf("\n"); fflush(stdout);
}
