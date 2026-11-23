#include <bits/stdc++.h>
using namespace std;

static long long ask(int i,int j,int k){
    printf("? %d %d %d\n",i,j,k); fflush(stdout);
    long long r;
    if(scanf("%lld",&r)!=1) exit(0);
    if(r<0) exit(0);
    return r;
}
static void answer(int i,int j){
    printf("! %d %d\n",i,j); fflush(stdout); exit(0);
}

int main(){
    int n;
    if(scanf("%d",&n)!=1) return 0;
    vector<int> idx(n);
    for(int i=0;i<n;i++) idx[i]=i+1;
    mt19937 rng((unsigned)(chrono::steady_clock::now().time_since_epoch().count()&0xffffffffu));
    shuffle(idx.begin(),idx.end(),rng);

    int b[4]={idx[0],idx[1],idx[2],idx[3]};
    long long Rv[4];
    for(int i=0;i<4;i++){
        int x[3],c=0;
        for(int j=0;j<4;j++) if(j!=i) x[c++]=b[j];
        Rv[i]=ask(x[0],x[1],x[2]);
    }
    long long R=*max_element(Rv,Rv+4);
    vector<int> M,nonM;
    for(int i=0;i<4;i++){ if(Rv[i]==R) M.push_back(i); else nonM.push_back(i); }

    int P=b[0],Q=b[1],wit=-1,start=4; long long B=0;

    if(M.size()==2){
        P=b[nonM[0]]; Q=b[nonM[1]]; B=R; wit=b[M[0]];
    } else if(M.size()==3 && Rv[nonM[0]]==0){
        P=b[nonM[0]]; Q=b[M[0]]; B=R;
    } else if(M.size()==4 && R==0){
        P=b[0]; Q=b[1]; B=0;
    } else {
        int pp; int u[3];
        if(M.size()==3){ pp=b[nonM[0]]; for(int i=0;i<3;i++) u[i]=b[M[i]]; }
        else { pp=b[0]; u[0]=b[1]; u[1]=b[2]; u[2]=b[3]; }
        if(n<5){
            if(M.size()==3) answer(pp,pp);
            answer(b[0],b[1]);
        }
        int e=idx[4];
        long long r[3];
        for(int i=0;i<3;i++) r[i]=ask(pp,u[i],e);
        long long mx=*max_element(r,r+3);
        if(r[0]==r[1]&&r[1]==r[2]){
            P=pp; Q=e; B=mx; wit=-1;
        } else {
            int imax=int(max_element(r,r+3)-r);
            int imin=int(min_element(r,r+3)-r);
            if(mx>R){ P=e; Q=u[imax]; B=mx; wit=pp; }
            else {
                P=pp; Q=u[imax]; B=R;
                if(M.size()==3) wit=u[imin]; else wit=-1;
            }
        }
        start=5;
    }

    for(int k=start;k<n;k++){
        int e=idx[k];
        long long r=ask(P,Q,e);
        if(r<=B) continue;
        if(B==0){ P=e; B=r; wit=-1; continue; }
        if(wit!=-1){
            long long r1=ask(e,P,wit);
            if(r1==r){ int near=Q; Q=P; P=e; wit=near; }
            else { int near=P; P=e; wit=near; }
            B=r;
        } else {
            int t0=-1;
            for(int j=0;j<k;j++){
                int c=idx[j];
                if(c!=P&&c!=Q){ t0=c; break; }
            }
            if(t0==-1){ P=e; B=r; wit=-1; continue; }
            long long r1=ask(e,P,t0);
            if(r1<r){ int near=P; P=e; wit=near; B=r; }
            else {
                long long r2=ask(e,Q,t0);
                if(r2<r){ int near=Q; Q=P; P=e; wit=near; B=r; }
                else { P=e; Q=t0; B=r; wit=-1; }
            }
        }
    }
    answer(P,Q);
    return 0;
}
