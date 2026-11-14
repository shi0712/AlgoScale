#include <bits/stdc++.h>
using namespace std;
typedef unsigned long long ull; typedef long long ll; typedef long double ld;
struct Solver{
    ull h,w,s,Pmax;
    ull T(ull P){ // P in [1,Pmax]
        ull a=(h+P-1)/P; ull Q=min(w,s/P); ull b=(w+Q-1)/Q; return a+b;
    }
    ld L(ull P){ ld x=(ld)h/P; ld y=(ld)w*(ld)P/(ld)s; if(y<1)y=1; return x+y;}
    void init(ull H,ull W,ull S){h=H;w=W;s=S;Pmax=min(h,s);}
    ull Pm;
    ull startU(){
        ull lo=1,hi=Pmax;
        while(hi-lo>2){ ull m1=lo+(hi-lo)/3,m2=hi-(hi-lo)/3; if(L(m1)<L(m2))hi=m2; else lo=m1;}
        Pm=lo; for(ull p=lo;p<=hi;p++) if(L(p)<L(Pm))Pm=p;
        ull U=min(T(1),T(Pmax));
        for(ll d=-3;d<=3;d++){ ll p=(ll)Pm+d; if(p>=1&&(ull)p<=Pmax) U=min(U,T(p)); }
        return U;
    }
    ull P1,P2,A1,A2;
    void window(ull U){
        ld lim=(ld)U+2;
        // P1: smallest in [1,Pm] with L<=lim
        ull lo=1,hi=Pm; while(lo<hi){ull m=lo+(hi-lo)/2; if(L(m)<=lim)hi=m; else lo=m+1;} P1=lo;
        lo=Pm,hi=Pmax; while(lo<hi){ull m=hi-(hi-lo)/2; if(L(m)<=lim)lo=m; else hi=m-1;} P2=lo;
        A1=(h+P2-1)/P2; A2=(h+P1-1)/P1;
    }
    ull sizeP(){return P2-P1+1;} ull sizeA(){return A2-A1+1;}
    ull runP(ull U){ for(ull p=P1;p<=P2;p++) U=min(U,T(p)); return U;}
    ull runA(ull U){ for(ull a=A1;a<=A2;a++){ ull p=(h+a-1)/a; if(p<=Pmax) U=min(U,T(p)); } return U;}
};
int main(){
    int t; scanf("%d",&t);
    while(t--){
        ull h,w,s; scanf("%llu %llu %llu",&h,&w,&s);
        Solver X,Y; X.init(h,w,s); Y.init(w,h,s);
        ull U=min(X.startU(),Y.startU());
        X.window(U); Y.window(U);
        ull c[4]={X.sizeP(),X.sizeA(),Y.sizeP(),Y.sizeA()};
        int k=min_element(c,c+4)-c;
        if(k==0)U=X.runP(U); else if(k==1)U=X.runA(U); else if(k==2)U=Y.runP(U); else U=Y.runA(U);
        printf("%llu\n",U-2);
    }
}
