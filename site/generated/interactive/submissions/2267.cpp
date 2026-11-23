#include <bits/stdc++.h>
using namespace std;
typedef __int128 lll;
static const int DEAD=5;
static const int TR[5][2]={{1,3},{2,3},{5,3},{1,4},{1,5}};
static lll FIB[170];
static vector<int> st, alive;
static int qrem;
static vector<int8_t> memo;
static const int KS=625000;
static int readInt(){int x;if(scanf("%d",&x)!=1)exit(0);if(x==-1)exit(0);return x;}
static int ask(int l,int r){printf("? %d %d\n",l,r);fflush(stdout);int c=readInt();return (c==r-l)?1:0;}
static void applyAnswer(int l,int r,int a){
    for(int p:alive){int t=(p>=l&&p<=r)?1:0;int d=(a!=t)?1:0;st[p]=TR[st[p]][d];}
    vector<int> na;na.reserve(alive.size());
    for(int p:alive)if(st[p]!=DEAD)na.push_back(p);
    alive.swap(na);
}
static void greedyStep(){
    int m=(int)alive.size(); if(m==0)return;
    vector<lll> P(m+1); vector<int> h2(m+1,0),d2(m+1,0);
    lll Up=FIB[qrem+1],c=FIB[qrem-1]; P[0]=0;
    for(int i=0;i<m;i++){int s=st[alive[i]];lll dl=0;
        if(s==1)dl=-c;else if(s==2)dl=-Up;else if(s==3)dl=c;else if(s==4)dl=Up;
        P[i+1]=P[i]+dl;h2[i+1]=h2[i]+(s==2?1:0);d2[i+1]=d2[i]+(s==4?1:0);}
    lll D=P[m];int tH=h2[m],tD=d2[m];
    set<pair<lll,int>> S;S.insert(make_pair((lll)0,0));
    lll bestI=-1;int bi=0,bj=1,bN=INT_MAX,bB=INT_MAX;
    for(int j=1;j<=m;j++){
        lll target=2*P[j]-D;
        auto it=S.lower_bound(make_pair(target,-1));
        int cand[2];int nc=0;
        if(it!=S.end())cand[nc++]=it->second;
        if(it!=S.begin()){auto it2=it;--it2;cand[nc++]=it2->second;}
        for(int k=0;k<nc;k++){int i=cand[k];
            lll imb=2*(P[j]-P[i])-D; if(imb<0)imb=-imb;
            int inH=h2[j]-h2[i],inD=d2[j]-d2[i];
            int N1=m-inH-(tD-inD), N0=m-(tH-inH)-inD;
            int mx=max(N0,N1), bal=abs(2*(j-i)-m);
            if(bestI<0||imb<bestI||(imb==bestI&&(mx<bN||(mx==bN&&bal<bB)))){bestI=imb;bi=i;bj=j;bN=mx;bB=bal;}
        }
        S.insert(make_pair(2*P[j],j));
    }
    int l=alive[bi],r=alive[bj-1];
    int a=ask(l,r); applyAnswer(l,r,a); if(qrem>2)qrem--;
}
static bool feas(int code,int len,int depth){
    if(len<=2)return true; if(depth<=0)return false;
    int idx=depth*KS+code*8+len; int8_t v=memo[idx]; if(v>=0)return v!=0;
    int s[8];int tmp=code;for(int i=0;i<len;i++){s[i]=tmp%5;tmp/=5;}
    bool ok=false;
    for(int i=0;i<len&&!ok;i++)for(int j=i;j<len&&!ok;j++){
        int c0=0,c1=0,l0=0,l1=0,p0=1,p1=1;
        for(int k=0;k<len;k++){int t=(k>=i&&k<=j)?1:0;
            int n0=TR[s[k]][t],n1=TR[s[k]][1-t];
            if(n0!=DEAD){c0+=n0*p0;p0*=5;l0++;}
            if(n1!=DEAD){c1+=n1*p1;p1*=5;l1++;}}
        if(feas(c0,l0,depth-1)&&feas(c1,l1,depth-1))ok=true;
    }
    memo[idx]=ok?1:0; return ok;
}
static void endgame(){
    while((int)alive.size()>2){
        int len=(int)alive.size();
        if(len>7){greedyStep();continue;}
        int s[8];int code=0,p=1;
        for(int i=0;i<len;i++){s[i]=st[alive[i]];code+=s[i]*p;p*=5;}
        int D=0; while(D<=12&&!feas(code,len,D))D++;
        int ci=-1,cj=-1;
        if(D<=12){
            for(int i=0;i<len&&ci<0;i++)for(int j=i;j<len&&ci<0;j++){
                int c0=0,c1=0,l0=0,l1=0,p0=1,p1=1;
                for(int k=0;k<len;k++){int t=(k>=i&&k<=j)?1:0;
                    int n0=TR[s[k]][t],n1=TR[s[k]][1-t];
                    if(n0!=DEAD){c0+=n0*p0;p0*=5;l0++;}
                    if(n1!=DEAD){c1+=n1*p1;p1*=5;l1++;}}
                if(feas(c0,l0,D-1)&&feas(c1,l1,D-1)){ci=i;cj=j;}
            }
        }
        if(ci<0){greedyStep();continue;}
        int l=alive[ci],r=alive[cj];
        int a=ask(l,r); applyAnswer(l,r,a); if(qrem>2)qrem--;
    }
}
int main(){
    int n; if(scanf("%d",&n)!=1)return 0;
    FIB[0]=0;FIB[1]=1;for(int i=2;i<170;i++)FIB[i]=FIB[i-1]+FIB[i-2];
    st.assign(n+1,0); alive.resize(n);
    for(int i=0;i<n;i++)alive[i]=i+1;
    qrem=120; memo.assign(13*KS,-1);
    while((int)alive.size()>7)greedyStep();
    endgame();
    if(alive.empty()){printf("#\n");fflush(stdout);return 0;}
    printf("! %d\n",alive[0]);fflush(stdout);
    int r1=readInt();
    if(r1==1){printf("#\n");fflush(stdout);return 0;}
    if((int)alive.size()>=2){printf("! %d\n",alive[1]);fflush(stdout);readInt();}
    printf("#\n");fflush(stdout);
    return 0;
}
