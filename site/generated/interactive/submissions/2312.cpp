#include <bits/stdc++.h>
using namespace std;

struct Cand { int type, a, b; int w; bool alive; };

int main(){
    vector<Cand> cs;
    // const
    for(int c=0;c<3;c++) cs.push_back({0,c,0,10,true});
    // cycles
    for(int s=0;s<3;s++) for(int d=1;d<=2;d++){
        int w = (s==0 && d==1)?10:1;
        cs.push_back({1,s,d,w,true});
    }
    // reactive: first move s, offset k
    for(int s=0;s<3;s++) for(int k=0;k<3;k++){
        int w = (s==0 && (k==0||k==1))?10:1;
        cs.push_back({2,s,k,w,true});
    }
    vector<int> h;
    auto pred=[&](const Cand&c,int t)->int{
        if(c.type==0) return c.a;
        if(c.type==1) return (c.a + c.b*t)%3;
        if(t==0) return c.a;
        return (h[t-1]+c.b)%3;
    };
    const char* M="RPS";
    int streak=0;
    for(int t=0;;t++){
        long long sc[3]={0,0,0};
        bool any=false;
        for(auto&c:cs) if(c.alive){
            any=true;
            int p=pred(c,t);
            sc[(p+1)%3]+=c.w;
        }
        int m;
        if(!any){ m=t%3; }
        else {
            m=0;
            for(int i=1;i<3;i++) if(sc[i]>sc[m]) m=i;
        }
        printf("%c\n",M[m]);
        fflush(stdout);
        string r;
        if(!(cin>>r)) return 0;
        bool win;
        if(r=="player") win=true;
        else if(r=="ai") win=false;
        else return 0;
        for(auto&c:cs) if(c.alive){
            int p=pred(c,t);
            bool w2 = (m==(p+1)%3);
            if(w2!=win) c.alive=false;
        }
        h.push_back(m);
        if(win){ streak++; if(streak>=10) return 0; }
        else streak=0;
    }
}
