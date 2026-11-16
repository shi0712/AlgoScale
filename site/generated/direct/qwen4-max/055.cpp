#include <bits/stdc++.h>
using namespace std;
typedef long long ll;
static const int SH=17; static const ll BASE=1LL<<SH, NEG=-(1LL<<50);
static int n; static unsigned char inf_[200005][4]; static char sbuf[200006];
static pair<ll,int> solveDP(int lam){
  ll addv[3]; for(int t=0;t<3;t++) addv[t]=(((ll)(t+3-lam))<<SH)-1;
  ll buf[6][3]; for(int a=0;a<6;a++){buf[a][0]=buf[a][1]=buf[a][2]=NEG;}
  buf[0][0]=BASE-1; int cur=0;
  for(int i=0;i<n;i++){
    ll v0=buf[cur][0],v1=buf[cur][1],v2=buf[cur][2];
    ll b01=v0>v1?v0:v1, ball=b01>v2?b01:v2;
    int nx=cur+1; if(nx==6)nx=0;
    if(b01>buf[nx][0])buf[nx][0]=b01;
    const unsigned char*pp=inf_[i]; int cnt=pp[0];
    for(int p=1;p<=cnt;p++){
      unsigned char code=pp[p]; int t=code&3, ds=((code>>2)&1)+1;
      ll src=(code&8)?v1:ball; ll nv=src+addv[t];
      int j=cur+t+3; if(j>=6)j-=6;
      if(nv>buf[j][ds])buf[j][ds]=nv;
    }
    buf[cur][0]=buf[cur][1]=buf[cur][2]=NEG; cur=nx;
  }
  ll res=buf[cur][0]>buf[cur][1]?buf[cur][0]:buf[cur][1];
  ll g=res>>SH; int c=(int)((BASE-1)-(res&(BASE-1)));
  return make_pair(g,c);
}
static char ibuf[1<<16]; static int ip=0,il=0;
static inline int gc(){ if(ip==il){il=(int)fread(ibuf,1,sizeof(ibuf),stdin);ip=0;if(il<=0)return -1;} return (unsigned char)ibuf[ip++]; }
static inline int readInt(){int c=gc();while(c!=-1&&(c<'0'||c>'9'))c=gc();int x=0;while(c>='0'&&c<='9'){x=x*10+(c-'0');c=gc();}return x;}
static inline int readStr(char*d){int c=gc();while(c!=-1&&c<=' ')c=gc();int L=0;while(c>' '){d[L++]=(char)c;c=gc();}d[L]=0;return L;}
int main(){
  int T=readInt();
  while(T-->0){
    n=readStr(sbuf); int K=readInt(); int W1=0;
    for(int i=0;i<n;i++){
      int cnt=0; char c1=sbuf[i];
      if(i+1<n&&sbuf[i+1]==c1){
        if(i+2<n&&sbuf[i+2]!=c1){
          int j=i+3,ds=(j<n&&sbuf[j]==c1)?1:0;
          inf_[i][++cnt]=(unsigned char)(0|(ds<<2)); if(!ds&&W1<3)W1=3;
          if(i+3<n&&sbuf[i+3]==c1){
            j=i+4;ds=(j<n&&sbuf[j]==c1)?1:0;
            inf_[i][++cnt]=(unsigned char)(1|(ds<<2)); if(!ds&&W1<4)W1=4;
            if(i+4<n&&sbuf[i+4]==c1){
              j=i+5;ds=(j<n&&sbuf[j]==c1)?1:0;
              inf_[i][++cnt]=(unsigned char)(2|(ds<<2)); if(!ds&&W1<5)W1=5;
            }
          }
        }
      }else{
        if(i+2<n&&sbuf[i+1]==sbuf[i+2]){
          char c=sbuf[i+1]; int ns=(i>0&&sbuf[i-1]==c)?1:0;
          int j=i+3,ds=(j<n&&sbuf[j]==c)?1:0;
          inf_[i][++cnt]=(unsigned char)(0|(ds<<2)|(ns<<3)); if(!ds&&!ns&&W1<3)W1=3;
        }else if(i+2<n&&sbuf[i+2]==c1){
          int ns=(i>0&&sbuf[i-1]==c1)?1:0;
          int j=i+3,ds=(j<n&&sbuf[j]==c1)?1:0;
          inf_[i][++cnt]=(unsigned char)(0|(ds<<2)|(ns<<3)); if(!ds&&!ns&&W1<3)W1=3;
          if(i+3<n&&sbuf[i+3]==c1){
            j=i+4;ds=(j<n&&sbuf[j]==c1)?1:0;
            inf_[i][++cnt]=(unsigned char)(1|(ds<<2)|(ns<<3)); if(!ds&&!ns&&W1<4)W1=4;
          }
        }
      }
      inf_[i][0]=(unsigned char)cnt;
    }
    int t5=0; {int i=0;while(i<n){if(inf_[i][0]==3){t5++;i+=5;}else i++;}}
    ll ans;
    if(K<=1) ans=W1;
    else if(K<=t5) ans=5LL*K;
    else if(3LL*K>=n) ans=solveDP(0).first;
    else{
      ll gv[6]; int cv[6]; bool hv[6]={false,false,false,false,false,false};
      int lo=0,hi=4;
      while(lo<hi){
        int mid=(lo+hi)>>1;
        if(!hv[mid]){pair<ll,int> r=solveDP(mid);gv[mid]=r.first;cv[mid]=r.second;hv[mid]=true;}
        if(cv[mid]<=K)hi=mid;else lo=mid+1;
      }
      if(!hv[lo]){pair<ll,int> r=solveDP(lo);gv[lo]=r.first;cv[lo]=r.second;hv[lo]=true;}
      ans=gv[lo]+(ll)lo*K;
    }
    printf("%lld\n",ans);
  }
  return 0;
}
