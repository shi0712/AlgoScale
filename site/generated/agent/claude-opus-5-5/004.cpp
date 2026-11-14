#include <bits/stdc++.h>
using namespace std;
typedef long long ll;
static char ibuf[1<<25];
int ipos=0,ilen=0;
inline int gc(){if(ipos==ilen){ilen=fread(ibuf,1,sizeof(ibuf),stdin);ipos=0;if(ilen<=0)return -1;}return ibuf[ipos++];}
inline int rd(){int c=gc();while(c!='-'&&(c<'0'||c>'9'))c=gc();int x=0;while(c>='0'&&c<='9'){x=x*10+(c-'0');c=gc();}return x;}
struct Info{int par,pth,hgt,dep,ls,le;};
int main(){
    int t=rd();
    while(t--){
        int n=rd(),m=rd(); (void)m;
        vector<int> par0(n,-1);
        for(int i=1;i<n;i++) par0[i]=rd()-1;
        vector<int> h0(n,0), dep0(n,0);
        for(int i=1;i<n;i++) dep0[i]=dep0[par0[i]]+1;
        for(int i=n-1;i>0;i--) h0[par0[i]]=max(h0[par0[i]],h0[i]+1);
        vector<int> deep0(n,-1);
        for(int i=1;i<n;i++){int p=par0[i]; if(deep0[p]<0||h0[i]>h0[deep0[p]]) deep0[p]=i;}
        vector<int> cs(n+1,0);
        for(int i=1;i<n;i++) cs[par0[i]+1]++;
        for(int i=0;i<n;i++) cs[i+1]+=cs[i];
        vector<int> ch(max(1,n-1));
        {vector<int> pp(cs.begin(),cs.end()-1); for(int i=1;i<n;i++) ch[pp[par0[i]]++]=i;}
        // preorder, deep child first
        vector<int> id(n), ord(n); int cnt=0;
        {vector<int> st; st.reserve(n); st.push_back(0);
         while(!st.empty()){int v=st.back(); st.pop_back(); id[v]=cnt; ord[cnt++]=v;
            for(int j=cs[v];j<cs[v+1];j++) if(ch[j]!=deep0[v]) st.push_back(ch[j]);
            if(deep0[v]>=0) st.push_back(deep0[v]);}}
        vector<Info> I(n);
        vector<int> lch; lch.reserve(n);
        vector<int> sz(n,1);
        int D=0;
        for(int k=0;k<n;k++){
            int v=ord[k]; Info &f=I[k];
            f.par = v==0?-1:id[par0[v]];
            f.hgt=h0[v]; f.dep=dep0[v]; D=max(D,f.dep);
            f.pth = (v!=0 && deep0[par0[v]]==v)? I[f.par].pth : k;
            f.ls=lch.size();
            for(int j=cs[v];j<cs[v+1];j++) if(ch[j]!=deep0[v]) lch.push_back(id[ch[j]]);
            f.le=lch.size();
        }
        for(int k=n-1;k>0;k--) sz[I[k].par]+=sz[k];
        // vertices by depth
        vector<int> dcnt(D+2,0);
        for(int k=0;k<n;k++) dcnt[I[k].dep+1]++;
        for(int d=0;d<=D;d++) dcnt[d+1]+=dcnt[d];
        vector<int> byd(n); {vector<int> pp(dcnt.begin(),dcnt.end()-1); for(int k=0;k<n;k++) byd[pp[I[k].dep]++]=k;}
        vector<ll> buf(n,0), A(n,0);
        vector<int> S(n,0), best(n,0), g(n,0), Hp(n,0), Hc(n,0);
        for(int L=D+1;L>=1;L--){
            if(L-1>D) continue;
            for(int q=dcnt[L-1];q<dcnt[L];q++){
                int u=byd[q];
                for(int v=u+sz[u]-1;v>=u;v--){
                    const Info &f=I[v];
                    int a=L+S[v];
                    int P=f.pth;
                    ll AP;
                    if(f.hgt==0) AP=0; else AP=A[P]-g[v+1];
                    for(int j=f.ls;j<f.le;j++){
                        int c=lch[j]; const Info &fc=I[c];
                        int lim=min(L,fc.hgt+1);
                        ll Ac=A[c]; int gc_=g[c];
                        const ll* bc=&buf[c-1];
                        ll* bv=&buf[v];
                        ll off=-(ll)gc_-AP, flo=-AP;
                        // y = max(bc[r]+Ac,0) - gc - AP ; only if >  -AP  (val>0) ; always fine to apply max
                        for(int r=1;r<=lim;r++){
                            ll x=bc[r]+Ac; if(x<0) x=0;
                            ll y=x+off; if(y>bv[r]) bv[r]=y;
                        }
                        (void)flo;
                    }
                    A[P]=AP;
                    int e0=0;
                    if(f.dep>=L){e0=Hp[v]-a; if(e0<0) e0=0;}
                    buf[v]=e0-AP;
                    int gv=0;
                    if(L<=f.hgt){ll x=buf[v+L]+AP; gv = x>0? (int)x:0;}
                    g[v]=gv;
                    int dp=a+gv;
                    Hc[v]=a+best[v];
                    S[v]=0; best[v]=0;
                    if(v!=u){int p=f.par; S[p]+=dp; int d=Hp[v]-dp; if(d>best[p]) best[p]=d;}
                }
            }
            swap(Hp,Hc);
        }
        printf("%d\n",Hp[0]);
    }
}
