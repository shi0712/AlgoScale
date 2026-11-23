#include <bits/stdc++.h>
using namespace std;

int n, T;
map<int,int> fc, gc;

int ask(int l, int r, int k){
    printf("? %d %d %d\n", l, r, k);
    fflush(stdout);
    int x;
    if(scanf("%d",&x)!=1) exit(0);
    if(x==-1) exit(0);
    return x;
}
int fget(int m){ // Q(1,m,2)
    if(m==0) return 0;
    if(m==1) return 1;
    if(m==n) return T;
    auto it=fc.find(m);
    if(it!=fc.end()) return it->second;
    int v=ask(1,m,2);
    fc[m]=v;
    return v;
}
int gget(int m){ // Q(m+1,n,2)
    if(m==n) return 0;
    if(m==n-1) return 1;
    if(m==0) return T;
    auto it=gc.find(m);
    if(it!=gc.end()) return it->second;
    int v=ask(m+1,n,2);
    gc[m]=v;
    return v;
}

int main(){
    if(scanf("%d",&n)!=1) return 0;
    T=n/2+1;
    // DP
    vector<int> M(n+1,0), A(n+1,0), bM(n+1,1), bA(n+1,1);
    for(int len=2;len<=n;len++){
        int best=INT_MAX,ba=1;
        for(int a=1;a<len;a++){
            int c=2+max(M[a],M[len-a]);
            if(c<best){best=c;ba=a;}
        }
        M[len]=best;bM[len]=ba;
    }
    for(int len=2;len<n;len++){
        int best=INT_MAX,ba=1;
        for(int a=1;a<len;a++){
            int c=(2-(a==1))+max(A[a],M[len-a]);
            if(c<best){best=c;ba=a;}
        }
        A[len]=best;bA[len]=ba;
    }
    int bF=1;
    {
        int best=INT_MAX;
        for(int a=1;a<n;a++){
            int c=(2-(a==1)-(a==n-1))+max(A[a],A[n-a]);
            if(c<best){best=c;bF=a;}
        }
    }
    int l=1,r=n;
    bool nodd=(n%2==1);
    int nstate=0; // 0 unknown, 1 n in prefix (< l), 2 n beyond (> r)
    while(l<r){
        int len=r-l+1;
        int mid;
        if(l==1&&r==n) mid=bF;
        else if(l==1) mid=bA[len];
        else if(r==n) mid=r-bA[len];
        else mid=l-1+bM[len];
        int F=fget(mid), G=gget(mid);
        int s=F-G+T-mid;
        if(nodd){
            if(s==1) r=mid; else l=mid+1;
        } else {
            if(nstate!=0){
                if(nstate==1) s-=1;
                if(s==1) r=mid; else l=mid+1;
            } else {
                if(s==2) r=mid;
                else if(s==0) l=mid+1;
                else {
                    bool nleft;
                    if(mid>=2){
                        int v=ask(1,mid,n);
                        nleft=(v==2);
                    } else {
                        int v=ask(mid+1,n,n);
                        nleft=!(v==2);
                    }
                    if(nleft){ l=mid+1; nstate=1; }
                    else { r=mid; nstate=2; }
                }
            }
        }
    }
    printf("! %d\n", l);
    fflush(stdout);
    return 0;
}
