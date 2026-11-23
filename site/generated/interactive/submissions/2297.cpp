#include <bits/stdc++.h>
using namespace std;
static int n; static vector<vector<int>> adj;
static long long doQuery(const vector<pair<int,int>>&S){
    int k=0; for(auto&p:S)k+=p.second; string out="? 1 "; out+=to_string(k);
    for(auto&p:S) for(int i=0;i<p.second;i++){out+=' ';out+=to_string(p.first);} out+='\n';
    fwrite(out.data(),1,out.size(),stdout); fflush(stdout);
    long long r; if(scanf("%lld",&r)!=1) exit(0); return r;
}
static void doToggle(int u){printf("? 2 %d\n",u);fflush(stdout);}
int main(){
    if(scanf("%d",&n)!=1)return 0; adj.assign(n+1,{});
    for(int i=0;i<n-1;i++){int u,v;if(scanf("%d %d",&u,&v)!=2)return 0;adj[u].push_back(v);adj[v].push_back(u);}
    vector<int> par1(n+1,0),tin(n+1,0),tout(n+1,0),sz1(n+1,1),ord; vector<vector<int>> ch1(n+1); ord.reserve(n);
    {vector<int> it(n+1,0),st;st.reserve(n);int T=0;st.push_back(1);tin[1]=T++;ord.push_back(1);
     while(!st.empty()){int v=st.back(); if(it[v]<(int)adj[v].size()){int u=adj[v][it[v]++]; if(u==par1[v])continue;
        par1[u]=v;ch1[v].push_back(u);tin[u]=T++;ord.push_back(u);st.push_back(u);} else{tout[v]=T-1;st.pop_back();if(!st.empty())sz1[st.back()]+=sz1[v];}}}
    vector<char> cand(n+1,1); int cc=n; bool haveQV=false; long long QV=0; vector<int> cs(n+1,0),rp(n+1,0);
    auto tryM=[&](int M,const vector<pair<int,int>>&items,int bud,vector<int>&sig)->bool{
        int d=items.size(); sig.assign(d,-1); if(M<1)return false; int g=0,cur=1,i=0; long long cost=0;
        while(i<d){ if(items[i].first>M)return false;
            if(cur+items[i].first<=M){cur+=items[i].first;sig[i]=g;cost+=(long long)g*items[i].first;if(cost>bud)return false;i++;}
            else{g++;cur=0;if(g>d)return false;}}
        return cost>0&&cost<=bud;};
    while(cc>1){
        for(int idx=n-1;idx>=0;idx--){int v=ord[idx];cs[v]=cand[v]?1:0;rp[v]=cand[v]?v:0;
            for(int c:ch1[v]){cs[v]+=cs[c];if(!rp[v])rp[v]=rp[c];}}
        double bR=1e18; int bW=-1,bO=-1; vector<array<int,4>> bBr; vector<int> bS;
        for(int w=1;w<=n;w++) if(cand[w]){
            vector<array<int,4>> br;
            for(int c:ch1[w]) if(cs[c]>0) br.push_back({sz1[c],cs[c],rp[c],c});
            int up=cc-cs[w];
            if(w!=1&&up>0){int rep=0;for(int idx=0;idx<n;idx++){int v=ord[idx];if(cand[v]&&(tin[v]<tin[w]||tin[v]>tout[w])){rep=v;break;}}br.push_back({n-sz1[w],up,rep,0});}
            {vector<pair<int,int>> t;for(auto&b:br)t.push_back({b[0],b[1]});sort(t.begin(),t.end());
             int mx=1,cu=0,pv=-1;for(auto&p:t){if(p.first!=pv){cu=p.second;pv=p.first;}else cu+=p.second;mx=max(mx,cu);}
             if(mx<cc){double g=log2((double)cc/mx),co=haveQV?2.0:3.0,ra=co/g;if(ra<bR-1e-12){bR=ra;bW=w;bO=0;bBr=br;bS.clear();}}}
            if(!br.empty()){
                vector<pair<int,int>> it2;for(int i2=0;i2<(int)br.size();i2++)it2.push_back({br[i2][1],i2});
                sort(it2.begin(),it2.end(),greater<pair<int,int>>());
                int lo=1;for(auto&x:it2)lo=max(lo,x.first); int hi=cc-1,bM=-1; vector<int> bs2;
                while(lo<=hi){int mid=(lo+hi)/2;vector<int> s;if(tryM(mid,it2,n,s)){bM=mid;bs2=s;hi=mid-1;}else lo=mid+1;}
                if(bM>0&&bM<cc){double g=log2((double)cc/bM),ra=3.0/g;
                    if(ra<bR-1e-12){bR=ra;bW=w;bO=1;bBr=br;bS.assign(br.size(),0);for(int i2=0;i2<(int)it2.size();i2++)bS[it2[i2].second]=bs2[i2];}}}}
        if(bW<0)break; int w=bW; vector<pair<int,int>> S; int tW=0; bool isV=false;
        if(bO==0){isV=true;tW=n;for(int v=1;v<=n;v++)S.push_back({v,1});}
        else for(int i2=0;i2<(int)bBr.size();i2++) if(bS[i2]>0){S.push_back({bBr[i2][2],bS[i2]});tW+=bS[i2];}
        long long base; if(isV&&haveQV)base=QV; else base=doQuery(S);
        doToggle(w); long long aft=doQuery(S); long long dl=aft-base; int cnt=(int)(llabs(dl)/2); int To=tW-cnt;
        vector<int> bw(n+1),bs(n+1); int uW=(bO==0?(n-sz1[w]):0),uS=n-sz1[w];
        for(int i2=0;i2<(int)bBr.size();i2++) if(bBr[i2][3]==0){uS=bBr[i2][0];uW=(bO==0?bBr[i2][0]:bS[i2]);}
        for(int v=1;v<=n;v++){bw[v]=uW;bs[v]=uS;}
        for(int i2=0;i2<(int)bBr.size();i2++) if(bBr[i2][3]!=0){int c=bBr[i2][3];int W=(bO==0?bBr[i2][0]:bS[i2]);
            for(int idx=tin[c];idx<=tout[c];idx++){int v=ord[idx];bw[v]=W;bs[v]=bBr[i2][0];}}
        bw[w]=0;bs[w]=0; int nc=0;
        for(int v=1;v<=n;v++) if(cand[v]){char kp=(bw[v]==To)?1:0;cand[v]=kp;if(kp)nc++;}
        if(isV){QV=aft;haveQV=true;}
        else{bool uq=true;int Z=-1;for(int v=1;v<=n;v++)if(cand[v]){int z=(v==w?n:n-bs[v]);if(Z<0)Z=z;else if(Z!=z){uq=false;break;}}
            if(uq&&cnt>0&&haveQV&&Z>=0){long long aw=-dl/(2LL*cnt);QV-=2*aw*Z;}else haveQV=false;}
        cc=nc;
    }
    int r=-1; for(int v=1;v<=n;v++) if(cand[v]) r=v; if(r<0) r=1;
    vector<int> parR(n+1,0),oR,pos(n+1,-1); oR.reserve(n);
    {vector<char> vis(n+1,0);vector<int> st;st.reserve(n);st.push_back(r);vis[r]=1;
     while(!st.empty()){int v=st.back();st.pop_back();pos[v]=(int)oR.size();oR.push_back(v);
        for(int u:adj[v])if(!vis[u]){vis[u]=1;parR[u]=v;st.push_back(u);}}}
    vector<int> a(n+1,0),cv(n+1,0),mv(n+1,0); vector<long long> acc(n+1,0); int i=0;
    while(i<n){
        int mxT=min(n-i,20),t=1;
        for(int tt=1;tt<=mxT;tt++){long long K=0;
            for(int j=0;j<tt;j++){int v=oR[i+j];if(j==0||pos[parR[v]]<i)K+=(1LL<<(tt-1-j));}
            if(K<=n)t=tt;else break;}
        for(int j=0;j<t;j++){int v=oR[i+j];cv[v]=1<<(t-1-j);mv[v]=cv[v];}
        for(int j=0;j<t;j++){int v=oR[i+j];int p=parR[v];if(p!=0&&pos[p]>=i)mv[p]-=cv[v];}
        vector<pair<int,int>> S; for(int j=0;j<t;j++){int v=oR[i+j];if(mv[v]>0)S.push_back({v,mv[v]});}
        long long V=doQuery(S);
        for(int v=1;v<=n;v++)acc[v]=0;
        for(int j=0;j<t;j++){int v=oR[i+j];acc[v]=mv[v];}
        for(int idx=n-1;idx>=0;idx--){int v=oR[idx];if(v!=r)acc[parR[v]]+=acc[v];}
        long long kn=0; for(int idx=0;idx<i;idx++){int v=oR[idx];kn+=acc[v]*(long long)a[v];}
        long long tg=V-kn, tot=(1LL<<t)-1; long long dec=(tg+tot)/2;
        if(dec<0)dec=0; if(dec>tot)dec=tot;
        for(int j=0;j<t;j++){int v=oR[i+j];int b=(int)((dec>>(t-1-j))&1LL);a[v]=2*b-1;}
        for(int j=0;j<t;j++){int v=oR[i+j];cv[v]=0;mv[v]=0;}
        i+=t;
    }
    string out="!"; for(int v=1;v<=n;v++){out+=' ';out+=to_string(a[v]);} out+='\n';
    fwrite(out.data(),1,out.size(),stdout); fflush(stdout);
    return 0;
}
