#include <bits/stdc++.h>
using namespace std;
typedef long long ll; typedef __int128 lll;
struct Cycle{int l,S,Q;}; struct SC{int N;vector<int> xs,ys,ws;};
static vector<SC> scs; static const ll LINF=(ll)4e18;
static ll dpk[105][105]; static int prd[105][105];
static bool evalMu(ll p,ll q,Cycle&best){
    bool found=false; lll bnum=0; ll bden=1; ll cost[21];
    for(int w=0;w<=20;w++){ll t=(ll)w*q-p;cost[w]=t*t;}
    for(size_t si=0;si<scs.size();si++){ const SC&sc=scs[si]; int N=sc.N; if(N<2)continue;
        int M=(int)sc.xs.size(); if(M==0)continue;
        for(int v=0;v<N;v++)dpk[0][v]=0;
        for(int k=1;k<=N;k++){ ll*prev=dpk[k-1];ll*cur=dpk[k];int*pr=prd[k];
            for(int v=0;v<N;v++)cur[v]=LINF;
            for(int i=0;i<M;i++){ ll b=prev[sc.xs[i]]; if(b==LINF)continue; ll val=b+cost[sc.ws[i]]; int y=sc.ys[i];
                if(val<cur[y]){cur[y]=val;pr[y]=i;} } }
        int bv=-1; ll vnum=0,vden=1;
        for(int v=0;v<N;v++){ if(dpk[N][v]==LINF)continue; bool has=false; ll mn=0,md=1;
            for(int k=0;k<N;k++){ if(dpk[k][v]==LINF)continue; ll nu=dpk[N][v]-dpk[k][v]; ll de=N-k;
                if(!has||(lll)nu*md>(lll)mn*de){mn=nu;md=de;has=true;} }
            if(!has)continue;
            if(bv<0||(lll)mn*vden<(lll)vnum*md){vnum=mn;vden=md;bv=v;} }
        if(bv<0)continue;
        int verts[105]; verts[N]=bv; int cv=bv;
        for(int k=N;k>=1;k--){int e=prd[k][cv];verts[k-1]=sc.xs[e];cv=verts[k-1];}
        int pos[105]; for(int v=0;v<N;v++)pos[v]=-1; int i0=-1,i1=-1;
        for(int i=0;i<=N;i++){int v=verts[i]; if(pos[v]>=0){i0=pos[v];i1=i;break;} pos[v]=i;}
        if(i0<0)continue; int l=0,S=0,Q=0;
        for(int k=i0+1;k<=i1;k++){int e=prd[k][verts[k]];int w=sc.ws[e];l++;S+=w;Q+=w*w;}
        if(l<=0)continue;
        lll num=(lll)Q*q*q-(lll)2*S*p*q+(lll)l*p*p; ll den=l;
        if(!found||num*bden<bnum*den){found=true;bnum=num;bden=den;best.l=l;best.S=S;best.Q=Q;} }
    return found; }
static bool haveB=false; static ll Bnum=0,Bden=1; static long double bestVarL=1e18L;
static void consider(const Cycle&c){ ll nu=(ll)c.Q*c.l-(ll)c.S*c.S,de=(ll)c.l*c.l;
    if(!haveB||(lll)nu*Bden<(lll)Bnum*de){haveB=true;Bnum=nu;Bden=de;bestVarL=(long double)nu/(long double)de;} }
struct Task{ll p1,q1;Cycle c1;ll p2,q2;Cycle c2;};
int main(){ int n,m; if(scanf("%d %d",&n,&m)!=2)return 0;
    vector<int> ex(m),ey(m),ew(m); vector<vector<int>> adj(n),radj(n);
    for(int i=0;i<m;i++){int x,y,w;scanf("%d %d %d",&x,&y,&w);--x;--y;ex[i]=x;ey[i]=y;ew[i]=w;adj[x].push_back(y);radj[y].push_back(x);}
    vector<char> reach(n,0),coreach(n,0);
    {vector<int> st{0};reach[0]=1;while(!st.empty()){int v=st.back();st.pop_back();for(int u:adj[v])if(!reach[u]){reach[u]=1;st.push_back(u);}}}
    {vector<int> st{n-1};coreach[n-1]=1;while(!st.empty()){int v=st.back();st.pop_back();for(int u:radj[v])if(!coreach[u]){coreach[u]=1;st.push_back(u);}}}
    if(!reach[n-1]){printf("-1\n");return 0;}
    vector<int> id(n,-1); int N=0; for(int v=0;v<n;v++)if(reach[v]&&coreach[v])id[v]=N++;
    vector<int> gx,gy,gw;
    for(int i=0;i<m;i++)if(id[ex[i]]>=0&&id[ey[i]]>=0){gx.push_back(id[ex[i]]);gy.push_back(id[ey[i]]);gw.push_back(ew[i]);}
    int GM=gx.size(),src=id[0],tgt=id[n-1];
    int MAXS=20*(n-1)+1; const int INF=INT_MAX;
    vector<int> dp((size_t)N*MAXS,INF),ndp((size_t)N*MAXS,INF); dp[(size_t)src*MAXS]=0;
    vector<char> alive(N,0),nalive(N,0); alive[src]=1;
    bool haveA=false; ll Anum=0,Aden=1; int maxSum=0;
    for(int l=1;l<=n-1;l++){ int newMax=20*l;
        for(int v=0;v<N;v++)fill(ndp.begin()+(size_t)v*MAXS,ndp.begin()+(size_t)v*MAXS+newMax+1,INF);
        fill(nalive.begin(),nalive.end(),0);
        for(int i=0;i<GM;i++){ int x=gx[i]; if(!alive[x])continue; const int*s=dp.data()+(size_t)x*MAXS;
            int*d=ndp.data()+(size_t)gy[i]*MAXS; int w=gw[i],w2=w*w; char fl=0;
            for(int S=0;S<=maxSum;S++){int cur=s[S];if(cur==INF)continue;int val=cur+w2;if(val<d[S+w]){d[S+w]=val;fl=1;}}
            if(fl)nalive[gy[i]]=1; }
        dp.swap(ndp); alive.swap(nalive); maxSum=newMax;
        const int*t=dp.data()+(size_t)tgt*MAXS;
        for(int S=0;S<=maxSum;S++){int Q=t[S];if(Q==INF)continue; ll nu=(ll)Q*l-(ll)S*S,de=(ll)l*l;
            if(!haveA||(lll)nu*Aden<(lll)Anum*de){haveA=true;Anum=nu;Aden=de;}} }
    vector<vector<pair<int,int>>> sadj(N);
    for(int i=0;i<GM;i++)sadj[gx[i]].push_back(make_pair(gy[i],i));
    vector<int> comp(N,-1),low(N,0),num(N,0),stk; vector<char> onstk(N,0); int idx=0,nc=0;
    function<void(int)> tarjan=[&](int v){ num[v]=low[v]=++idx; stk.push_back(v); onstk[v]=1;
        for(auto&pr:sadj[v]){int u=pr.first; if(!num[u]){tarjan(u);low[v]=min(low[v],low[u]);} else if(onstk[u])low[v]=min(low[v],num[u]);}
        if(low[v]==num[v]){while(true){int u=stk.back();stk.pop_back();onstk[u]=0;comp[u]=nc;if(u==v)break;}nc++;} };
    for(int v=0;v<N;v++)if(!num[v])tarjan(v);
    scs.assign(nc,SC()); vector<int> cnt(nc,0),lid(N,-1);
    for(int v=0;v<N;v++)lid[v]=cnt[comp[v]]++;
    for(int c=0;c<nc;c++)scs[c].N=cnt[c];
    for(int i=0;i<GM;i++)if(comp[gx[i]]==comp[gy[i]]){int c=comp[gx[i]];scs[c].xs.push_back(lid[gx[i]]);scs[c].ys.push_back(lid[gy[i]]);scs[c].ws.push_back(gw[i]);}
    Cycle c0,c20;
    if(evalMu(0,1,c0)&&evalMu(20,1,c20)){ consider(c0); consider(c20);
        vector<Task> ts; ts.push_back(Task{0,1,c0,20,1,c20});
        while(!ts.empty()){ Task t=ts.back(); ts.pop_back();
            ll nu=(ll)t.c1.Q*t.c2.l-(ll)t.c2.Q*t.c1.l;
            ll de=2*((ll)t.c1.S*t.c2.l-(ll)t.c2.S*t.c1.l);
            if(de==0)continue; if(de<0){de=-de;nu=-nu;}
            if(!((lll)t.p1*de<(lll)nu*t.q1))continue;
            if(!((lll)nu*t.q2<(lll)t.p2*de))continue;
            if(haveB){ long double u1=(long double)t.p1/(long double)t.q1,u2=(long double)t.p2/(long double)t.q2;
                if(u2-u1>1e-12L){ long double G1=(long double)t.c1.Q/t.c1.l-2.0L*((long double)t.c1.S/t.c1.l)*u1;
                    long double G2=(long double)t.c2.Q/t.c2.l-2.0L*((long double)t.c2.S/t.c2.l)*u2;
                    long double B=(G2-G1)/(u2-u1),A=G1-B*u1,um=-B/2;
                    if(um<u1)um=u1; if(um>u2)um=u2;
                    long double lb=um*um+A+B*um;
                    if(lb>bestVarL+1e-11L)continue; } }
            ll g=__gcd(nu<0?-nu:nu,de); if(g>1){nu/=g;de/=g;}
            Cycle c; if(!evalMu(nu,de,c))continue; consider(c);
            ll na=(ll)t.c1.Q*de-2LL*t.c1.S*nu,da=(ll)t.c1.l*de;
            ll nb=(ll)t.c2.Q*de-2LL*t.c2.S*nu,db=(ll)t.c2.l*de;
            ll ncc=(ll)c.Q*de-2LL*c.S*nu,dc=(ll)c.l*de;
            bool first=((lll)na*db<=(lll)nb*da);
            ll nm=first?na:nb,dm=first?da:db;
            if((lll)ncc*dm>=(lll)nm*dc)continue;
            ts.push_back(Task{t.p1,t.q1,t.c1,nu,de,c});
            ts.push_back(Task{nu,de,c,t.p2,t.q2,t.c2}); } }
    double ans;
    if(haveA&&haveB){ if((lll)Bnum*Aden<(lll)Anum*Bden)ans=(double)Bnum/(double)Bden; else ans=(double)Anum/(double)Aden; }
    else if(haveA)ans=(double)Anum/(double)Aden;
    else if(haveB)ans=(double)Bnum/(double)Bden;
    else {printf("-1\n");return 0;}
    if(ans<0)ans=0;
    printf("%.15f\n",ans);
    return 0; }
