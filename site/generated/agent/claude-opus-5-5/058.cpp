#include <bits/stdc++.h>
using namespace std;
static char buf[1<<25];
int bl,bp;
inline int gc(){if(bp==bl){bl=fread(buf,1,sizeof(buf),stdin);bp=0;if(bl<=0)return -1;}return buf[bp++];}
inline unsigned long long rd(){int c=gc();while(c<'0')c=gc();unsigned long long x=0;while(c>='0'){x=x*10+(c-'0');c=gc();}return x;}
int main(){
    int n=rd(),q=rd();
    const int MX=1000001;
    vector<int> cnt(MX+1,0);
    int M=0; long long S=0;
    for(int i=0;i<n;i++){int a=rd();cnt[a]++;M=max(M,a);S+=a;}
    // suffix: ge[v] = count of a >= v
    vector<int> ge(M+2,0);
    for(int v=M;v>=1;v--)ge[v]=ge[v+1]+cnt[v];
    vector<long long> f(M+1,0);
    for(int T=1;T<=M;T++){long long s=0;for(int v=T;v<=M;v+=T)s+=ge[v];f[T]=s;}
    string out;out.reserve((size_t)q*4);
    for(int j=0;j<q;j++){
        unsigned long long K=rd();
        bool ok;
        if(K>=(unsigned long long)M) ok = K<=(unsigned long long)S;
        else{ long long T=(M+K-1)/K; ok = (unsigned long long)f[T]>=K; }
        out+= ok?"YES\n":"NO\n";
    }
    fwrite(out.data(),1,out.size(),stdout);
}
