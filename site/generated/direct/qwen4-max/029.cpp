#include <bits/stdc++.h>
using namespace std;
typedef unsigned long long u64; typedef unsigned int u32;
static const u64 MOD=1000000007ULL;
static char ibuf[1<<16]; static int ipos=0,ilen=0;
static inline int gc(){ if(ipos==ilen){ilen=(int)fread(ibuf,1,sizeof(ibuf),stdin);ipos=0;if(ilen<=0)return -1;} return (unsigned char)ibuf[ipos++]; }
static inline long long readInt(){ int c=gc(); while(c!=-1&&(c<'0'||c>'9')&&c!='-')c=gc(); long long s=1; if(c=='-'){s=-1;c=gc();} long long x=0; while(c>='0'&&c<='9'){x=x*10+(c-'0');c=gc();} return x*s; }
struct Mat{u32 a[9];}; static vector<Mat> tr; static int SZ;
static inline void pull(int k){ const Mat&A=tr[2*k],&B=tr[2*k+1]; Mat&C=tr[k];
  for(int i=0;i<3;i++){ u64 a0=A.a[3*i],a1=A.a[3*i+1],a2=A.a[3*i+2];
    C.a[3*i]=(u32)((a0*B.a[0]+a1*B.a[3]+a2*B.a[6])%MOD);
    C.a[3*i+1]=(u32)((a0*B.a[1]+a1*B.a[4]+a2*B.a[7])%MOD);
    C.a[3*i+2]=(u32)((a0*B.a[2]+a1*B.a[5]+a2*B.a[8])%MOD);} }
static u64 fact[2105],invfact[2105];
int main(){ const int MF=2105; fact[0]=1; for(int i=1;i<MF;i++)fact[i]=fact[i-1]*(u64)i%MOD;
  {u64 b=fact[MF-1],e=MOD-2,rr=1;while(e){if(e&1)rr=rr*b%MOD;b=b*b%MOD;e>>=1;}invfact[MF-1]=rr;}
  for(int i=MF-1;i>0;i--)invfact[i-1]=invfact[i]*(u64)i%MOD;
  int T=(int)readInt();
  while(T--){ int n=(int)readInt(); vector<long long> l(n),r(n),L(n);
    for(int i=0;i<n;i++)l[i]=readInt(); for(int i=0;i<n;i++)r[i]=readInt();
    u64 Tot=1; long long R=0;
    for(int i=0;i<n;i++){L[i]=r[i]-l[i]+1;Tot=Tot*(u64)(L[i]%MOD)%MOD;if(r[i]>R)R=r[i];}
    vector<long long> vs; vs.reserve(2*n);
    for(int i=0;i<n;i++){vs.push_back(l[i]);vs.push_back(r[i]+1);}
    sort(vs.begin(),vs.end()); vs.erase(unique(vs.begin(),vs.end()),vs.end());
    int m=(int)vs.size(),nb=m-1;
    vector<int> st(n,-1),dcnt(nb,0),st0; vector<vector<pair<int,int>>> ch(nb); int k0=-1;
    for(int k=0;k<nb;k++){ long long p=vs[k],q1=vs[k+1]; int cnt=0;
      for(int i=0;i<n;i++){ int s; if(l[i]>=q1)s=0; else if(r[i]+1<=p)s=2; else {s=1;cnt++;}
        if(s!=st[i]){ch[k].push_back(make_pair(i,s));st[i]=s;} }
      dcnt[k]=cnt;
      if(k0<0){ bool ok=true; int last=-10;
        for(int i=0;i<n;i++) if(st[i]==0){ if(i-last<=2){ok=false;break;} last=i; }
        if(ok){k0=k;st0=st;} } }
    if(k0<0){ printf("%llu\n",(u64)(R%MOD)*Tot%MOD); continue; }
    int J=n+2; vector<int> lastNeed(J,-1);
    for(int k=k0;k<nb;k++){int j=dcnt[k]+1; if(j>J-1)j=J-1; if(k>lastNeed[j])lastNeed[j]=k;}
    for(int j=J-2;j>=0;j--) if(lastNeed[j+1]>lastNeed[j])lastNeed[j]=lastNeed[j+1];
    vector<u32> F((size_t)(nb-k0)*(size_t)J,0u);
    SZ=1; while(SZ<n)SZ<<=1; tr.assign((size_t)2*SZ,Mat());
    for(int i=n;i<SZ;i++){Mat&M=tr[SZ+i];M.a[0]=1;M.a[4]=1;M.a[8]=1;}
    auto setleaf=[&](int i,int s,long long j){ u64 c,d;
      if(s==0){c=(u64)(L[i]%MOD);d=0;} else if(s==2){c=0;d=(u64)(L[i]%MOD);}
      else{ long long cc=r[i]+1-j,dd=j-l[i];
        c=(u64)((cc%(long long)MOD+(long long)MOD)%(long long)MOD);
        d=(u64)((dd%(long long)MOD+(long long)MOD)%(long long)MOD);}
      Mat&M=tr[SZ+i]; M.a[0]=(u32)d;M.a[1]=(u32)c;M.a[2]=0;M.a[3]=0;M.a[4]=0;M.a[5]=(u32)d;M.a[6]=(u32)d;M.a[7]=0;M.a[8]=0; };
    for(int j=0;j<J;j++){ int ln=lastNeed[j]; if(ln<k0)continue;
      for(int i=0;i<n;i++)setleaf(i,st0[i],(long long)j);
      for(int k=SZ-1;k>=1;k--)pull(k);
      F[(size_t)j]=(u32)(((u64)tr[1].a[0]+tr[1].a[1]+tr[1].a[2])%MOD);
      for(int k=k0+1;k<=ln;k++){
        for(size_t e=0;e<ch[k].size();e++){ int pos=ch[k][e].first,s=ch[k][e].second;
          setleaf(pos,s,(long long)j);
          for(int t=(SZ+pos)>>1;t>=1;t>>=1)pull(t); }
        F[(size_t)(k-k0)*J+j]=(u32)(((u64)tr[1].a[0]+tr[1].a[1]+tr[1].a[2])%MOD); } }
    u64 sumF=0; vector<u64> y,pre,suf;
    for(int k=k0;k<nb;k++){ int mm=dcnt[k]+1; y.assign(mm+1,0);
      const u32*Fk=&F[(size_t)(k-k0)*J]; u64 s=0;
      for(int t=0;t<=mm;t++){s+=Fk[t];if(s>=MOD)s-=MOD;y[t]=s;}
      pre.assign(mm+2,1); suf.assign(mm+2,1);
      auto lag=[&](u64 Tv)->u64{ if(Tv<=(u64)mm)return y[(size_t)Tv];
        for(int i=0;i<=mm;i++)pre[i+1]=pre[i]*((Tv-(u64)i)%MOD)%MOD;
        suf[mm+1]=1;
        for(int i=mm;i>=0;i--)suf[i]=suf[i+1]*((Tv-(u64)i)%MOD)%MOD;
        u64 res=0;
        for(int i=0;i<=mm;i++){ u64 t=y[i]*pre[i]%MOD; t=t*suf[i+1]%MOD; t=t*invfact[i]%MOD; t=t*invfact[mm-i]%MOD;
          if((mm-i)&1){res=(res>=t)?res-t:res+MOD-t;} else {res+=t;if(res>=MOD)res-=MOD;} }
        return res; };
      u64 g1=lag((u64)(vs[k+1]-1)),g2=lag((u64)(vs[k]-1));
      u64 val=(g1>=g2)?g1-g2:g1+MOD-g2; sumF+=val; if(sumF>=MOD)sumF-=MOD; }
    u64 ans=(u64)(R%MOD)*Tot%MOD; ans=(ans>=sumF)?ans-sumF:ans+MOD-sumF;
    printf("%llu\n",ans); }
  return 0; }
