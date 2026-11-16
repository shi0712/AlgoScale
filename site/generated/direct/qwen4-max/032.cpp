#include <bits/stdc++.h>
using namespace std;

struct Cp { double x, y; };
static inline Cp operator+(Cp a, Cp b){ return Cp{a.x+b.x, a.y+b.y}; }
static inline Cp operator-(Cp a, Cp b){ return Cp{a.x-b.x, a.y-b.y}; }
static inline Cp operator*(Cp a, Cp b){ return Cp{a.x*b.x-a.y*b.y, a.x*b.y+a.y*b.x}; }

static vector<Cp> rt; static vector<int> rev;

static void init_fft(int n){
    rt.assign(n, Cp{1.0,0.0}); rev.assign(n,0);
    if(n<=1) return;
    int L=__builtin_ctz((unsigned)n);
    for(int i=1;i<n;i++) rev[i]=(rev[i>>1]|((i&1)<<L))>>1;
    vector<long double> Rr(n,0.0L), Ri(n,0.0L); Rr[1]=1.0L;
    const long double PI=acosl(-1.0L);
    for(int k=2;k<n;k<<=1){
        long double ang=PI/(long double)k, xr=cosl(ang), xi=sinl(ang);
        for(int i=k;i<2*k;i++){
            int h=i>>1; long double pr,pi;
            if(i&1){ pr=Rr[h]*xr-Ri[h]*xi; pi=Rr[h]*xi+Ri[h]*xr; }
            else { pr=Rr[h]; pi=Ri[h]; }
            Rr[i]=pr; Ri[i]=pi; rt[i]=Cp{(double)pr,(double)pi};
        }
    }
}
static void fft(vector<Cp>& a){
    int n=(int)a.size(); Cp* A=a.data();
    for(int i=0;i<n;i++){ int j=rev[i]; if(i<j) swap(A[i],A[j]); }
    for(int k=1;k<n;k<<=1){
        const Cp* w=rt.data()+k;
        for(int i=0;i<n;i+=2*k){
            Cp* p=A+i;
            for(int j=0;j<k;j++){
                Cp z=w[j]*p[j+k];
                p[j+k]=p[j]-z; p[j]=p[j]+z;
            }
        }
    }
}
static vector<Cp> dft_neg(const vector<double>& f){
    int n=(int)f.size();
    if((n&(n-1))==0){
        init_fft(n); vector<Cp> a(n);
        for(int i=0;i<n;i++) a[i]=Cp{f[i],0.0};
        fft(a); for(int i=0;i<n;i++) a[i].y=-a[i].y; return a;
    }
    int m=1; while(m<2*n-1) m<<=1;
    init_fft(m);
    const long double PI=acosl(-1.0L);
    vector<Cp> w(n);
    for(int j=0;j<n;j++){
        long long j2=((long long)j*j)%(2LL*n);
        long double ang=PI*(long double)j2/(long double)n;
        w[j]=Cp{(double)cosl(ang), -(double)sinl(ang)};
    }
    vector<Cp> a(m,Cp{0,0}), b(m,Cp{0,0});
    for(int j=0;j<n;j++){
        a[j]=Cp{f[j]*w[j].x, f[j]*w[j].y};
        b[j]=Cp{w[j].x, -w[j].y};
    }
    for(int j=1;j<n;j++) b[m-j]=b[j];
    fft(a); fft(b);
    for(int i=0;i<m;i++) a[i]=a[i]*b[i];
    for(int i=0;i<m;i++) a[i].y=-a[i].y;
    fft(a);
    double inv=1.0/(double)m;
    vector<Cp> res(n);
    for(int t=0;t<n;t++){ Cp c{a[t].x*inv, -a[t].y*inv}; res[t]=w[t]*c; }
    return res;
}
static inline long long readInt(){
    static char buf[1<<16]; static size_t len=0,pos=0;
    auto gc=[&]()->int{ if(pos==len){ len=fread(buf,1,sizeof(buf),stdin); pos=0; if(!len) return -1;} return (unsigned char)buf[pos++]; };
    int c=gc(); while(c!=-1&&c!='-'&&(c<'0'||c>'9')) c=gc();
    bool neg=false; if(c=='-'){neg=true;c=gc();}
    long long v=0; while(c>='0'&&c<='9'){ v=v*10+(c-'0'); c=gc(); }
    return neg?-v:v;
}
int main(){
    int n=(int)readInt(); int k=(int)readInt();
    vector<double> f(n);
    for(int i=0;i<n;i++) f[i]=(double)readInt();
    if(n==1){ printf("%.10f\n",0.0); return 0; }
    f[k]=0.0;
    vector<Cp> A=dft_neg(f);
    long double maxA2=0.0L;
    for(int t=0;t<n;t++){
        long double v=(long double)A[t].x*A[t].x+(long double)A[t].y*A[t].y;
        if(v>maxA2) maxA2=v;
    }
    long long R=(long long)ceill(sqrtl(maxA2))+2;
    int cnt=n/2+1;
    vector<long double> bx(cnt), v2(cnt);
    const long double PI=acosl(-1.0L);
    for(int t=0;t<cnt;t++){
        long long r=((long long)k*t)%n;
        long double ang=2.0L*PI*(long double)r/(long double)n;
        long double c=cosl(ang), s=sinl(ang);
        long double re=(long double)A[t].x*c-(long double)A[t].y*s;
        long double im=(long double)A[t].x*s+(long double)A[t].y*c;
        bx[t]=re; v2[t]=im*im;
    }
    const long double* BX=bx.data(); const long double* V2=v2.data();
    auto H2=[&](long long x)->long double{
        long double xx=(long double)x, best=0.0L;
        for(int t=0;t<cnt;t++){
            long double re=BX[t]+xx;
            long double val=re*re+V2[t];
            if(val>best) best=val;
        }
        return best;
    };
    long long lo=-R, hi=R;
    while(lo<hi){
        long long mid=lo+(hi-lo)/2;
        if(H2(mid)<=H2(mid+1)) hi=mid; else lo=mid+1;
    }
    long double best=H2(lo);
    for(long long x=lo-2;x<=lo+2;x++){ long double v=H2(x); if(v<best) best=v; }
    long double ans=sqrtl(best);
    printf("%.10f\n",(double)ans);
    return 0;
}
