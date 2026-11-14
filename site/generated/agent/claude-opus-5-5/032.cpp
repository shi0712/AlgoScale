#include <bits/stdc++.h>
using namespace std;
typedef complex<double> cd;
const long double PI=acosl(-1.0L);
void fft(vector<cd>&a,bool inv,const vector<cd>&rt){
    int n=a.size();
    for(int i=1,j=0;i<n;i++){int bit=n>>1;for(;j&bit;bit>>=1)j^=bit;j^=bit;if(i<j)swap(a[i],a[j]);}
    for(int len=2;len<=n;len<<=1){
        int step=n/len;
        for(int i=0;i<n;i+=len)for(int j=0;j<len/2;j++){
            cd w=rt[j*step]; if(inv) w=conj(w);
            cd u=a[i+j],v=a[i+j+len/2]*w;
            a[i+j]=u+v;a[i+j+len/2]=u-v;
        }
    }
    if(inv)for(auto&x:a)x/=n;
}
int main(){
    int n,k;scanf("%d %d",&n,&k);
    vector<long long>f(n);for(auto&x:f)scanf("%lld",&x);
    f[k]=0;
    vector<cd> c(n);
    for(long long j=0;j<n;j++){long long m=(j*j)%(2LL*n);long double ang=-PI*m/n;c[j]=cd((double)cosl(ang),(double)sinl(ang));}
    int sz=1;while(sz<3*n)sz<<=1;
    vector<cd> rt(sz/2);
    for(int i=0;i<sz/2;i++){long double ang=2*PI*i/sz;rt[i]=cd((double)cosl(ang),(double)sinl(ang));}
    vector<cd>A(sz),B(sz);
    for(int s=0;s<n;s++)A[s]=(double)f[s]*c[s];
    for(int j=-(n-1);j<=n-1;j++)B[j+n-1]=conj(c[abs(j)]);
    fft(A,false,rt);fft(B,false,rt);
    for(int i=0;i<sz;i++)A[i]*=B[i];
    fft(A,true,rt);
    vector<double> re(n),im(n);
    for(int t=0;t<n;t++){
        cd G=A[t+n-1]*c[t];
        long long m=(1LL*k*t)%n;long double ang=2*PI*m/n;
        cd H=G*cd((double)cosl(ang),(double)sinl(ang));
        re[t]=H.real();im[t]=H.imag();
    }
    auto g=[&](long long x){double mx=0;for(int t=0;t<n;t++){double a=x+re[t];double v=a*a+im[t]*im[t];if(v>mx)mx=v;}return mx;};
    long long lo=-400000000LL,hi=400000000LL; // find smallest x with g(x+1)-g(x)>=0
    while(lo<hi){long long m=lo+(hi-lo)/2;if(g(m+1)>=g(m))hi=m;else lo=m+1;}
    double best=g(lo);
    for(long long d=-2;d<=2;d++)best=min(best,g(lo+d));
    printf("%.12f\n",sqrt(best));
}
