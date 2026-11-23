#include <bits/stdc++.h>
using namespace std;

int ask(int a,int b,int c){
    printf("? %d %d %d\n",a,b,c);
    fflush(stdout);
    int r;
    if(scanf("%d",&r)!=1) exit(0);
    if(r==-1) exit(0);
    return r;
}

int main(){
    int n;
    if(scanf("%d",&n)!=1) return 0;
    deque<int> cur;
    for(int i=1;i<=n;i++) cur.push_back(i);
    int dummy=-1;
    while(cur.size()>=3){
        vector<int> g;
        if(cur.size()>=4){
            for(int t=0;t<4;t++){g.push_back(cur.front());cur.pop_front();}
        } else {
            for(int t=0;t<3;t++){g.push_back(cur.front());cur.pop_front();}
            g.push_back(dummy);
        }
        int q[3];
        for(int x=0;x<3;x++){
            vector<int> o;
            for(int y=0;y<4;y++) if(y!=x) o.push_back(g[y]);
            q[x]=ask(o[0],o[1],o[2]);
        }
        int mx=max({q[0],q[1],q[2]});
        vector<int> tops;
        for(int x=0;x<3;x++) if(q[x]==mx) tops.push_back(x);
        bool rem[4]={false,false,false,false};
        if(tops.size()>=2){
            rem[tops[0]]=true; rem[tops[1]]=true;
        } else {
            rem[tops[0]]=true; rem[3]=true;
        }
        for(int y=0;y<4;y++){
            if(rem[y]){ if(g[y]!=dummy || dummy==-1) dummy=g[y]; }
        }
        for(int y=0;y<4;y++) if(!rem[y]) cur.push_back(g[y]);
    }
    int a=cur[0];
    int b=cur.size()>1?cur[1]:cur[0];
    printf("! %d %d\n",a,b);
    fflush(stdout);
    return 0;
}
