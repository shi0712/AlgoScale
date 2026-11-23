#include <bits/stdc++.h>
using namespace std;

long long ask(char c, long long a){
    printf("%c %lld\n", c, a);
    fflush(stdout);
    long long r;
    if(scanf("%lld",&r)!=1) exit(0);
    if(r<0) exit(0);
    return r;
}

int main(){
    long long n;
    if(scanf("%lld",&n)!=1) return 0;
    if(n==1){ printf("C 1\n"); fflush(stdout); return 0; }
    vector<int> spf(n+1,0);
    vector<int> primes;
    for(int i=2;i<=n;i++){
        if(spf[i]==0){
            for(long long j=i;j<=n;j+=i) if(spf[j]==0) spf[j]=i;
            primes.push_back(i);
        }
    }
    int M=primes.size();
    vector<int> idx(n+1,0);
    for(int i=0;i<M;i++) idx[primes[i]]=i+1;
    vector<double> cnt(M+2,0);
    for(int i=2;i<=n;i++) cnt[idx[spf[i]]]+=1;
    vector<double> w(M+2,0);
    for(int k=1;k<=M;k++) w[k]=0.5*cnt[k]/n+0.5/(M+1);
    double wone=0.5/n+0.5/(M+1);
    vector<double> Ws(M+3,0), S1(M+2,0), S2(M+2,0);
    Ws[M+1]=wone;
    for(int k=M;k>=1;k--) Ws[k]=Ws[k+1]+w[k];
    for(int k=1;k<=M;k++){ S1[k]=S1[k-1]+w[k]; S2[k]=S2[k-1]+w[k]*k; }
    vector<double> g(M+2,0);
    vector<int> ch(M+2,0);
    g[M+1]=0;
    for(int s=M;s>=1;s--){
        double best=1e300; int be=s;
        double ws=Ws[s];
        for(int e=s;e<=M;e++){
            double inb=(S2[e-1]-S2[s-1])-(double)(s-1)*(S1[e-1]-S1[s-1])+w[e]*(e-s);
            double v=ws*(e-s+2)+inb+g[e+1];
            if(v<best){best=v;be=e;}
        }
        g[s]=best; ch[s]=be;
    }
    vector<char> alive(n+1,1);
    long long simcount=n;
    int s=1;
    while(s<=M){
        int e=ch[s];
        for(int k=s;k<=e;k++){
            int p=primes[k-1];
            ask('B',p);
            for(long long j=p;j<=n;j+=p) if(alive[j]){alive[j]=0;simcount--;}
        }
        long long r=ask('A',1);
        if(r==simcount){ s=e+1; continue; }
        // detected
        int qi=-1;
        for(int k=s;k<e;k++){
            long long rr=ask('A',primes[k-1]);
            if(rr>0){qi=k;break;}
        }
        if(qi==-1) qi=e;
        long long q=primes[qi-1];
        long long y=q;
        while(y*q<=n){
            long long rr=ask('A',y*q);
            if(rr>0) y*=q; else break;
        }
        for(int k=qi+1;k<=M;k++){
            long long p=primes[k-1];
            if(y*p>n) break;
            long long rr=ask('A',y*p);
            if(rr>0){
                y*=p;
                while(y*p<=n){
                    long long r2=ask('A',y*p);
                    if(r2>0) y*=p; else break;
                }
            }
        }
        printf("C %lld\n",y); fflush(stdout);
        return 0;
    }
    printf("C 1\n"); fflush(stdout);
    return 0;
}
