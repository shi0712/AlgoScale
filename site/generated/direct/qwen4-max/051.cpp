#include <bits/stdc++.h>
using namespace std;
typedef unsigned int u32;
static const u32 MOD=1000000007u;
static inline u32 addm(u32 a,u32 b){u32 s=a+b;if(s>=MOD)s-=MOD;return s;}
static inline u32 subm(u32 a,u32 b){return a>=b?a-b:a+MOD-b;}
static char ibuf[1<<16]; static int ipos=0,ilen=0;
static inline int gc(){if(ipos==ilen){ilen=(int)fread(ibuf,1,sizeof(ibuf),stdin);ipos=0;if(ilen<=0)return -1;}return (unsigned char)ibuf[ipos++];}
static inline int readInt(){int c=gc();while(c!=-1&&(c<'0'||c>'9'))c=gc();int x=0;while(c>='0'&&c<='9'){x=x*10+(c-'0');c=gc();}return x;}
int main(){
    int n=readInt(),Q=readInt(); string s;
    {int c=gc();while(c==' '||c=='\n'||c=='\r'||c=='\t')c=gc();
     while(c!=-1&&c!=' '&&c!='\n'&&c!='\r'&&c!='\t'){s.push_back((char)c);c=gc();}}
    vector<int> A(n+1),B(n+1),C(n+1),T(n+1);
    {int a=0,b=0,c=0,t=0,trun=0,left=-1;
     for(int i=0;i<n;i++){char ch=s[i];
        if(ch=='?'){++trun;++t;int odd=trun&1;
            a+=(left==0)?(1-odd):odd;b+=(left==1)?(1-odd):odd;c+=(left==2)?(1-odd):odd;
        }else{left=ch-'a';trun=0;}
        A[i+1]=a;B[i+1]=b;C[i+1]=c;T[i+1]=t;}}
    const int maxA=A[n],maxB=B[n],maxC=C[n],m=T[n];
    const int W=maxB+2, SZ=(maxA+3)*W;
    vector<u32> vA(SZ,0),vB(SZ,0),vC(SZ,0),vD(SZ,0),vE(SZ,0),vF(SZ,0);
    u32 *pa=vA.data()+W,*pb=vB.data()+W,*pc=vC.data()+W;
    u32 *qa=vD.data()+W,*qb=vE.data()+W,*qc=vF.data()+W;
    {char ch=s[0];
     if(ch=='?'){pa[W]=1;pb[1]=1;pc[0]=1;}
     else if(ch=='a')pa[0]=1; else if(ch=='b')pb[0]=1; else pc[0]=1;}
    for(int i=1;i<n;i++){
        const int A2=A[i+1],B2=B[i+1],C2=C[i+1],t2=T[i+1];
        int L2=t2-C2; if(L2<0)L2=0; const int U2=t2;
        const int type=(s[i]=='?')?3:(s[i]-'a');
        for(int na=0;na<=A2+1;++na){
            const int off=na*W;
            u32 *ra=qa+off,*rb=qb+off,*rc=qc+off;
            int hlo=L2-1-na; if(hlo<0)hlo=0;
            int hhi=min(B2+1,U2+1-na);
            int lo,hi;
            if(na>A2){lo=hlo;hi=hlo-1;}
            else{lo=L2-na;if(lo<0)lo=0;hi=min(B2,U2-na);if(lo>hi){lo=hlo;hi=hlo-1;}}
            for(int nb=hlo;nb<lo;++nb){ra[nb]=0;rb[nb]=0;rc[nb]=0;}
            if(lo<=hi){
                if(type==3){
                    const u32 *sa=pa+off,*sb=pb+off,*sc=pc+off;
                    const u32 *sbu=pb+off-W,*scu=pc+off-W;
                    u32 aprev=(lo>=1)?sa[lo-1]:0u, cprev=(lo>=1)?sc[lo-1]:0u;
                    for(int nb=lo;nb<=hi;++nb){
                        u32 x=sa[nb],y=sb[nb],z=sc[nb];
                        rc[nb]=addm(x,y); rb[nb]=addm(aprev,cprev); ra[nb]=addm(sbu[nb],scu[nb]);
                        aprev=x;cprev=z;}
                }else if(type==0){
                    const u32 *sb=pb+off,*sc=pc+off;
                    for(int nb=lo;nb<=hi;++nb){ra[nb]=addm(sb[nb],sc[nb]);rb[nb]=0;rc[nb]=0;}
                }else if(type==1){
                    const u32 *sa=pa+off,*sc=pc+off;
                    for(int nb=lo;nb<=hi;++nb){rb[nb]=addm(sa[nb],sc[nb]);ra[nb]=0;rc[nb]=0;}
                }else{
                    const u32 *sa=pa+off,*sb=pb+off;
                    for(int nb=lo;nb<=hi;++nb){rc[nb]=addm(sa[nb],sb[nb]);ra[nb]=0;rb[nb]=0;}
                }
            }
            for(int nb=hi+1;nb<=hhi;++nb){ra[nb]=0;rb[nb]=0;rc[nb]=0;}
        }
        swap(pa,qa);swap(pb,qb);swap(pc,qc);
    }
    const int WA=maxB+2, WC=maxC+2;
    vector<u32> SAB((size_t)(maxA+2)*WA,0),SAC((size_t)(maxA+2)*WC,0),SBC((size_t)(maxB+2)*WC,0);
    {int L=m-maxC; if(L<0)L=0;
     for(int na=0;na<=maxA;++na){
        int lo=max(0,L-na),hi=min(maxB,m-na);
        for(int nb=lo;nb<=hi;++nb){
            int idx=na*W+nb; u32 val=addm(addm(pa[idx],pb[idx]),pc[idx]);
            if(val){int nc=m-na-nb;
                SAB[(size_t)na*WA+nb]=val;
                SAC[(size_t)na*WC+nc]=val;
                SBC[(size_t)nb*WC+nc]=val;}}}}
    for(int na=maxA;na>=0;--na)for(int nb=maxB;nb>=0;--nb){
        size_t id=(size_t)na*WA+nb; u32 v=addm(SAB[id],SAB[id+WA]);
        v=addm(v,SAB[id+1]);v=subm(v,SAB[id+WA+1]);SAB[id]=v;}
    for(int na=maxA;na>=0;--na)for(int nc=maxC;nc>=0;--nc){
        size_t id=(size_t)na*WC+nc; u32 v=addm(SAC[id],SAC[id+WC]);
        v=addm(v,SAC[id+1]);v=subm(v,SAC[id+WC+1]);SAC[id]=v;}
    for(int nb=maxB;nb>=0;--nb)for(int nc=maxC;nc>=0;--nc){
        size_t id=(size_t)nb*WC+nc; u32 v=addm(SBC[id],SBC[id+WC]);
        v=addm(v,SBC[id+1]);v=subm(v,SBC[id+WC+1]);SBC[id]=v;}
    const u32 total=SAB[0];
    string out; out.reserve((size_t)Q*11); char tmp[16];
    for(int q=0;q<Q;q++){
        int x=readInt(),y=readInt(),z=readInt(); u32 ans=0;
        if(x+y+z>=m){
            int xa=min(x+1,maxA+1),yb=min(y+1,maxB+1),zc=min(z+1,maxC+1);
            ans=subm(total,SAB[(size_t)xa*WA]);
            ans=subm(ans,SAB[yb]);
            ans=subm(ans,SAC[zc]);
            ans=addm(ans,SAB[(size_t)xa*WA+yb]);
            ans=addm(ans,SAC[(size_t)xa*WC+zc]);
            ans=addm(ans,SBC[(size_t)yb*WC+zc]);
        }
        int len=0; if(ans==0)tmp[len++]='0'; else while(ans){tmp[len++]=(char)('0'+ans%10);ans/=10;}
        while(len)out.push_back(tmp[--len]); out.push_back('\n');
    }
    fwrite(out.data(),1,out.size(),stdout);
    return 0;
}
