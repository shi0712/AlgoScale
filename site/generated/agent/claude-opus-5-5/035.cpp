#include <bits/stdc++.h>
using namespace std;
static const int INF=INT_MAX;
static int n,m;
static vector<int> st,to_,rv,pre_,nxt_,dsu,sz,fst,lst,dg,par,inSlot;
static vector<int> parent_,tin,tout,heavy,top_,subsz,upSlot,heavySlot,bnd,childOfSlot,lb,keyv;
static vector<set<pair<int,int>>> S;
static vector<int> seg; static int SN;
static int find(int x){while(dsu[x]!=x){dsu[x]=dsu[dsu[x]];x=dsu[x];}return x;}
static void segSet(int p,int v){p+=SN;seg[p]=v;for(p>>=1;p;p>>=1)seg[p]=min(seg[2*p],seg[2*p+1]);}
static int segQ(int l,int r){int res=INF;for(l+=SN,r+=SN;l<r;l>>=1,r>>=1){if(l&1)res=min(res,seg[l++]);if(r&1)res=min(res,seg[--r]);}return res;}
static inline int boundOf(int v,int k){
    int w=to_[k];
    if(w==parent_[v]) return max(lb[k],min(segQ(0,tin[v]),segQ(tout[v],n)));
    return max(lb[k],segQ(tin[w],tout[w]));
}
static inline bool isLightSlot(int v,int k){int c=childOfSlot[k];return c!=-1 && c!=heavy[v];}
struct Frame{
    int w,ein,re,fv,lv,rl,k,acc; bool fin;
    set<pair<int,int>>::iterator it,ed;
    pair<int,int> ex[2]; int ne,ei;
};
int main(){
    int T; if(scanf("%d",&T)!=1) return 0;
    while(T--){
        if(scanf("%d",&n)!=1) return 0;
        vector<int> pos(n+1);
        for(int i=1;i<=n;i++) if(scanf("%d",&pos[i])!=1) return 0;
        vector<int> ex(max(n-1,0)),ey(max(n-1,0));
        dg.assign(n+2,0);
        for(int i=0;i<n-1;i++){if(scanf("%d %d",&ex[i],&ey[i])!=2) return 0;dg[ex[i]]++;dg[ey[i]]++;}
        if(n==1){puts("1");continue;}
        st.assign(n+2,0);
        for(int u=1;u<=n;u++) st[u+1]=st[u]+dg[u];
        m=2*(n-1);
        to_.assign(m,0); rv.assign(m,0);
        {vector<int> fl(st.begin(),st.end());
        for(int i=0;i<n-1;i++){
            int a=fl[ex[i]]++, b=fl[ey[i]]++;
            to_[a]=ey[i]; to_[b]=ex[i]; rv[a]=b; rv[b]=a;
        }}
        // root at 1, HLD
        parent_.assign(n+1,0); upSlot.assign(n+1,-1); childOfSlot.assign(m,-1);
        vector<int> order; order.reserve(n);
        {vector<int> stk={1}; parent_[1]=0;
        while(!stk.empty()){int v=stk.back();stk.pop_back();order.push_back(v);
            for(int k=st[v];k<st[v+1];k++){int w=to_[k];if(w==parent_[v]){upSlot[v]=k;continue;}parent_[w]=v;childOfSlot[k]=w;stk.push_back(w);}}}
        subsz.assign(n+1,1); heavy.assign(n+1,-1); heavySlot.assign(n+1,-1);
        for(int i=n-1;i>=0;i--){int v=order[i];int b=-1;
            for(int k=st[v];k<st[v+1];k++){int w=childOfSlot[k];if(w==-1)continue;subsz[v]+=subsz[w];if(b==-1||subsz[w]>subsz[heavy[v]]){heavy[v]=w;heavySlot[v]=k;b=w;}}}
        tin.assign(n+1,0); tout.assign(n+1,0); top_.assign(n+1,0);
        {int timer=0; vector<pair<int,int>> stk; stk.push_back({1,-1}); top_[1]=1;
        // iterative dfs: heavy first
        vector<int> itk(n+1);
        tin[1]=timer++; itk[1]=-2;
        vector<int> s2={1};
        while(!s2.empty()){int v=s2.back();
            if(itk[v]==-2){ itk[v]=st[v];
                if(heavy[v]!=-1){int h=heavy[v];top_[h]=top_[v];tin[h]=timer++;itk[h]=-2;s2.push_back(h);continue;}
            }
            bool pushed=false;
            while(itk[v]<st[v+1]){int k=itk[v]++;int w=childOfSlot[k];if(w==-1||w==heavy[v])continue;
                top_[w]=w;tin[w]=timer++;itk[w]=-2;s2.push_back(w);pushed=true;break;}
            if(!pushed){tout[v]=timer;s2.pop_back();}
        }}
        SN=1; while(SN<n) SN<<=1;
        seg.assign(2*SN,INF);
        for(int v=1;v<=n;v++) seg[SN+tin[v]]=v;
        for(int p=SN-1;p>=1;p--) seg[p]=min(seg[2*p],seg[2*p+1]);
        bnd.assign(n+1,INF);
        S.assign(n+1,{});
        for(int v=1;v<=n;v++){
            bnd[v]=segQ(tin[v],tout[v]);
        }
        lb.assign(m,0); keyv.assign(m,0);
        for(int v=1;v<=n;v++) for(int k=st[v];k<st[v+1];k++) if(isLightSlot(v,k)){keyv[k]=bnd[childOfSlot[k]]; S[v].insert({keyv[k],k});}
        vector<int> dirty; dirty.reserve(n);
        auto refresh=[&](int k){ // k slot at v=to_[rv[k]]
            int v=to_[rv[k]];
            if(!isLightSlot(v,k)||pre_[k]!=-1) return;
            int nk=max(lb[k],bnd[childOfSlot[k]]);
            if(nk!=keyv[k]){S[v].erase({keyv[k],k});keyv[k]=nk;S[v].insert({nk,k});}
        };
        auto raiseLb=[&](int k,int L){ if(L>lb[k]){lb[k]=L;dirty.push_back(k);} };
        pre_.assign(m,-1); nxt_.assign(m,-1); dsu.resize(m); sz.assign(m,1);
        for(int i=0;i<m;i++) dsu[i]=i;
        fst.assign(n+1,-1); lst.assign(n+1,-1); par.assign(n+1,-1); inSlot.assign(n+1,-1);
        vector<int> ans(n+1);
        vector<Frame> fr; fr.reserve(n+1);
        auto initFrame=[&](Frame&f,int w,int ein){
            f.w=w;f.ein=ein;f.k=-1;f.acc=INF;f.fv=fst[w];f.lv=lst[w];
            if(ein>=0){f.re=find(ein);f.fin=f.fv!=-1&&find(f.fv)==f.re;}else{f.re=-1;f.fin=false;}
            f.rl=f.lv!=-1?find(f.lv):-1;
            f.it=S[w].begin();f.ed=S[w].end();
            f.ne=0;f.ei=0;
            int k=heavySlot[w];
            if(k!=-1&&pre_[k]==-1&&k!=ein&&k!=f.fv) f.ex[f.ne++]={boundOf(w,k),k};
            k=upSlot[w];
            if(k!=-1&&pre_[k]==-1&&k!=ein&&k!=f.fv) f.ex[f.ne++]={boundOf(w,k),k};
            if(f.ne==2&&f.ex[1]<f.ex[0]) swap(f.ex[0],f.ex[1]);
        };
        for(int i=1;i<=n;i++){
            int s=pos[i];
            int best=INF;
            fr.clear();
            fr.emplace_back(); initFrame(fr.back(),s,-1);
            while(!fr.empty()){
                Frame &f=fr.back();
                // next candidate
                pair<int,int> c; bool have=false;
                bool useSet = f.it!=f.ed; bool useEx = f.ei<f.ne;
                if(useSet && (!useEx || *f.it < f.ex[f.ei])){c=*f.it;++f.it;have=true;}
                else if(useEx){c=f.ex[f.ei++];have=true;}
                if(have && c.first>=best){f.acc=min(f.acc,c.first);have=false;}
                if(!have){
                    int L=f.acc,kk=f.k; fr.pop_back();
                    if(kk>=0){raiseLb(kk,L); fr.back().acc=min(fr.back().acc,L);}
                    continue;
                }
                int k=c.second; int w=f.w;
                if(f.ein<0){
                    if(f.lv!=-1){int r=find(k); if(r==f.rl && sz[r]<dg[w]) continue;}
                }else{
                    if(k==f.ein||k==f.fv) continue;
                    int rk=find(k);
                    if(rk==f.re) continue;
                    if(f.fin && rk==f.rl && sz[f.re]+sz[rk]!=dg[w]) continue;
                }
                int v=to_[k]; int e=rv[k];
                if(nxt_[e]!=-1){raiseLb(k,INF);continue;}
                par[v]=k;
                int fv=fst[v], lv=lst[v];
                int re=find(e);
                bool fin= fv!=-1&&find(fv)==re;
                int endv=(lv==-1 && !(fin&&sz[re]<dg[v]))?v:INF;
                if(endv<best) best=endv;
                if(lv==e){raiseLb(k,endv);f.acc=min(f.acc,endv);continue;}
                Frame nf; initFrame(nf,v,e); nf.k=k; nf.acc=endv;
                fr.push_back(nf);
            }
            int t=best; ans[i]=t;
            for(int d:dirty) refresh(d); dirty.clear();
            int k=par[t]; lst[t]=rv[k];
            // eligibility update
            segSet(tin[t],INF);
            {int x=t; while(true){x=top_[x]; if(x==1)break; int p=parent_[x]; int nb=segQ(tin[x],tout[x]);
                if(nb!=bnd[x]){bnd[x]=nb; refresh(rv[upSlot[x]]);}
                x=p;}}
            int v=to_[rv[k]];
            int eo=k;
            while(v!=s){
                int kin=par[v]; int ei=rv[kin];
                nxt_[ei]=eo; pre_[eo]=ei;
                if(isLightSlot(v,eo)) S[v].erase({keyv[eo],eo});
                int a=find(ei), b=find(eo); dsu[b]=a; sz[a]+=sz[b];
                eo=kin; v=to_[rv[kin]];
            }
            fst[s]=eo;
        }
        for(int i=1;i<=n;i++) printf("%d%c",ans[i],i==n?'\n':' ');
    }
}
