#include <bits/stdc++.h>
using namespace std;
static char buf[1<<25];int bl,bp;
inline int gc(){if(bp==bl){bl=fread(buf,1,sizeof buf,stdin);bp=0;if(bl<=0)return -1;}return buf[bp++];}
inline int rd(){int c=gc();while(c!='-'&&(c<'0'||c>'9'))c=gc();int x=0;while(c>='0'&&c<='9'){x=x*10+c-'0';c=gc();}return x;}
int main(){
    int n=rd(),k=rd();
    vector<array<int,11>> w(n);vector<int> p(n);
    long long tot=0,full=0;
    for(int i=0;i<n;i++){p[i]=rd();for(int j=1;j<=p[i];j++)w[i][j]=rd();tot+=p[i];full+=w[i][p[i]];}
    if(tot<=k){printf("%lld\n",full);return 0;}
    vector<int> ord(n);iota(ord.begin(),ord.end(),0);
    sort(ord.begin(),ord.end(),[&](int a,int b){return (long long)w[a][p[a]]*p[b]>(long long)w[b][p[b]]*p[a];});
    const long long NEG=LLONG_MIN/4;const int C=100;
    vector<long long> d0(k+1,NEG),d1(k+1,NEG);d0[0]=0;
    long long P=0;
    for(int idx:ord){
        int pi=p[idx];auto&ww=w[idx];long long wp=ww[pi];
        P+=pi;
        auto proc=[&](int hi,int lo){
            for(int s=hi;s>=lo;s--){
                long long b1=d1[s],b0=d0[s];
                if(s>=pi){long long v=d0[s-pi];if(v>NEG){b0=max(b0,v+wp);} long long u=d1[s-pi]; if(u>NEG)b1=max(b1,u+wp);}
                int tm=min(pi-1,s);
                for(int t=1;t<=tm;t++){long long v=d0[s-t];if(v>NEG)b1=max(b1,v+ww[t]);}
                d1[s]=b1;d0[s]=b0;
            }
        };
        int hi1=k,lo1=max(0,k-C);
        long long hi2=min<long long>(P,k),lo2=max<long long>(0,P-C);
        if(hi2>=lo1){proc(hi1,(int)min<long long>(lo1,lo2));}
        else{proc(hi1,lo1);proc((int)hi2,(int)lo2);}
    }
    printf("%lld\n",max(d0[k],d1[k]));
}
