#include <bits/stdc++.h>
using namespace std;
typedef long long ll; typedef unsigned long long ull;
const ull MOD=1000000007;
struct Tr{unsigned a,b,c;};
struct Ed{int to; unsigned a,b,c;};
struct RawE{int a,b,w;};
static inline unsigned neg(unsigned x){return x?MOD-x:0;}
int main(){
    int T; if(scanf("%d",&T)!=1) return 0;
    vector<array<int,3>> qs(T); int MX=1;
    for(auto&q:qs){if(scanf("%d %d %d",&q[0],&q[1],&q[2])!=3) return 0; MX=max({MX,q[0],q[1],q[2]});}
    int N=MX; vector<signed char> mu(N+1,0); vector<int> lpf(N+1,0);
    { vector<int> pr; mu[1]=1;
      for(int i=2;i<=N;i++){ if(!lpf[i]){lpf[i]=i;pr.push_back(i);mu[i]=-1;}
        for(int p:pr){ if(p>lpf[i]||(ll)p*i>N)break; lpf[p*i]=p; mu[p*i]=(p==lpf[i])?0:-mu[i];}} }
    vector<int> omg(N+1,0); for(int i=2;i<=N;i++) omg[i]=omg[i/lpf[i]]+(lpf[i/lpf[i]]!=lpf[i]);
    // global work arrays
    vector<int> deg(N+2,0), st(N+3,0), pos(N+2,0);
    vector<unsigned short> mark(N+1,0);
    vector<RawE> raw; vector<Ed> ed; vector<Tr> loc; vector<int> hj, hk;
    vector<int> gp; // primes of g
    for(auto&q:qs){
        int A=q[0],B=q[1],C=q[2]; int M=max({A,B,C});
        vector<Tr> F(M+1,Tr{0,0,0});
        auto mkF=[&](int X,int which){ for(int n=1;n<=X;n++){ ll s=0; for(int x=n;x<=X;x+=n) s+=X/x; unsigned val=s%MOD; if(which==0)F[n].a=val; else if(which==1)F[n].b=val; else F[n].c=val;} };
        mkF(A,0);mkF(B,1);mkF(C,2);
        ull ans=0;
        for(int g=1;g<=M;g++){ if(!mu[g]) continue;
            int n=M/g;
            ull dg=1ull<<omg[g];
            ull acc=0; // sum without d(g) factor, mod MOD
            // a=b=c=1
            { const Tr&f=F[g]; acc=(ull)f.a*f.b%MOD*f.c%MOD; }
            gp.clear(); { int t=g; while(t>1){gp.push_back(lpf[t]); t/=lpf[t];} }
            raw.clear();
            int maxv=1;
            for(int w=2;w<=n;w++){ if(!mu[w]) continue;
                bool ok=true; for(int p:gp) if(w%p==0){ok=false;break;}
                if(!ok) continue;
                // degenerate {1,1,w}
                { const Tr&f1=F[g]; const Tr&f2=F[(ll)g*w];
                  ull s=((ull)f2.a*f2.b%MOD*f1.c + (ull)f2.a*f1.b%MOD*f2.c + (ull)f1.a*f2.b%MOD*f2.c)%MOD;
                  acc+= mu[w]>0? s: MOD-s; }
                int ps[8],k=0; int t=w; while(t>1){ps[k++]=lpf[t];t/=lpf[t];}
                int half=1<<(k-1);
                for(int m=0;m<half;m++){ int a=1; for(int i=0;i<k-1;i++) if(m>>i&1) a*=ps[i]; int b=w/a; if(a>b) swap(a,b);
                    raw.push_back(RawE{a,b,w}); deg[a]++; deg[b]++; if(b>maxv)maxv=b; }
            }
            acc%=MOD;
            size_t E=raw.size();
            if(E){
                auto lessv=[&](int x,int y){return deg[x]<deg[y]||(deg[x]==deg[y]&&x<y);};
                for(int i=0;i<=maxv+1;i++) st[i]=0;
                for(auto&e:raw){ int s=lessv(e.a,e.b)?e.a:e.b; st[s+1]++; }
                for(int i=1;i<=maxv+1;i++) st[i]+=st[i-1];
                for(int i=0;i<=maxv;i++) pos[i]=st[i];
                if(ed.size()<E) ed.resize(E);
                for(auto&e:raw){ int s=e.a,t2=e.b; if(!lessv(s,t2)) swap(s,t2); const Tr&f=F[(ll)g*e.w]; ed[pos[s]++]=Ed{t2,f.a,f.b,f.c}; }
                int maxd=0; for(int i=1;i<=maxv;i++) maxd=max(maxd,st[i+1]-st[i]);
                for(int i=1;i<=maxv;i++) if(st[i+1]-st[i]>1) sort(ed.begin()+st[i],ed.begin()+st[i+1],[](const Ed&x,const Ed&y){return x.to<y.to;});
                if((int)loc.size()<maxd+1){loc.resize(maxd+1);hj.resize(maxd+1);hk.resize(maxd+1);}
                loc[0]=Tr{0,0,0};
                ull tot=0;
                Ed* EP=ed.data(); int* hjp=hj.data(); int* hkp=hk.data(); unsigned short* mk=mark.data(); Tr* L=loc.data();
                for(int u=1;u<=maxv;u++){ int s0=st[u],s1=st[u+1]; if(s1-s0<2) continue;
                    for(int i=s0;i<s1;i++){ int w=EP[i].to; mk[w]=i-s0+1; Tr t{EP[i].a,EP[i].b,EP[i].c}; if(mu[w]<0){t.a=neg(t.a);t.b=neg(t.b);t.c=neg(t.c);} L[i-s0+1]=t; }
                    ull su=0;
                    for(int i=s0;i<s1;i++){ int v=EP[i].to; int jb=st[v],je=st[v+1]; if(jb==je) continue;
                        int cnt=0;
                        for(int j=jb;j<je;j++){ int kk=mk[EP[j].to]; hjp[cnt]=j; hkp[cnt]=kk; cnt+=(kk!=0); }
                        if(!cnt) continue;
                        unsigned __int128 X=0,Y=0,Z=0;
                        for(int h0=0;h0<cnt;h0+=8){ int he=min(cnt,h0+8); ull x=0,y=0,z=0;
                            for(int h=h0;h<he;h++){ const Tr &e3=L[hkp[h]]; const Ed &e2=EP[hjp[h]];
                                x+=(ull)e2.b*e3.c+(ull)e3.b*e2.c;
                                y+=(ull)e2.a*e3.c+(ull)e3.a*e2.c;
                                z+=(ull)e2.a*e3.b+(ull)e3.a*e2.b; }
                            X+=x;Y+=y;Z+=z; }
                        ull x=X%MOD,y=Y%MOD,z=Z%MOD;
                        const Ed &e1=EP[i];
                        ull s=(e1.a*x+e1.b*y+e1.c*z)%MOD;
                        if(mu[v]<0) s=neg(s);
                        su+=s;
                    }
                    su%=MOD; if(mu[u]<0) su=neg(su);
                    tot+=su;
                    for(int i=s0;i<s1;i++) mk[EP[i].to]=0;
                }
                tot%=MOD;
                acc=(acc+tot)%MOD;
                for(auto&e:raw){deg[e.a]=0;deg[e.b]=0;}
            }
            ans=(ans+dg%MOD*acc)%MOD;
        }
        printf("%llu\n",ans);
    }
}
