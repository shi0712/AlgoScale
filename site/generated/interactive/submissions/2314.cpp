#include <bits/stdc++.h>
using namespace std;
int P;
vector<int> tr;
bool dead=false;
int ask(int a,int b){
    printf("? %d %d\n",a,b);
    fflush(stdout);
    int w;
    if(scanf("%d",&w)!=1){exit(0);}
    if(w==-1)exit(0);
    return w;
}
int comb(int a,int b){
    if(!a)return b;
    if(!b)return a;
    return ask(a,b);
}
int main(){
    int n,k;
    if(scanf("%d %d",&n,&k)!=2)return 0;
    vector<int> res;
    if(k==n){
        printf("!");
        for(int i=1;i<=n;i++)printf(" %d",i);
        printf("\n");fflush(stdout);return 0;
    }
    int m = (k==1)? n : n-k+2;
    P=1;while(P<m)P<<=1;
    tr.assign(2*P,0);
    for(int i=0;i<m;i++)tr[P+i]=i+1;
    int nxt=m+1;
    for(int v=P-1;v>=1;v--)tr[v]=comb(tr[2*v],tr[2*v+1]);
    // map label->leaf position
    vector<int> pos(n+2,-1);
    for(int i=0;i<m;i++)pos[i+1]=P+i;
    for(int r=0;r<k-1;r++){
        int w=tr[1];
        res.push_back(w);
        int leaf=pos[w];
        int nv=0;
        if(nxt<=n){nv=nxt;pos[nxt]=leaf;nxt++;}
        tr[leaf]=nv;
        for(int v=leaf>>1;v>=1;v>>=1)tr[v]=comb(tr[2*v],tr[2*v+1]);
    }
    res.push_back(tr[1]);
    printf("!");
    for(int x:res)printf(" %d",x);
    printf("\n");
    fflush(stdout);
    return 0;
}
