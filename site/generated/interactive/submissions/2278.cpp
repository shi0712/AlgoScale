#include <bits/stdc++.h>
using namespace std;
typedef long long ll;
static const ll MAXV=1000000000LL;
static int n;
static vector<array<int,3>> gT;
static vector<ll> gA;
static ll ask(int i,int j,int k){
    printf("? %d %d %d\n",i,j,k); fflush(stdout);
    ll r; if(scanf("%lld",&r)!=1) exit(0); if(r<0) exit(0); return r;
}
static ll fdiv(ll a,ll b){ll q=a/b;if((a%b!=0)&&((a<0)!=(b<0)))--q;return q;}
static ll cdiv(ll a,ll b){return -fdiv(-a,b);}
struct Res{int status;array<ll,5> v;};
static Res analyze(const array<int,5>& pm){
    int rk[5];for(int i=0;i<5;i++)rk[pm[i]]=i;
    vector<vector<pair<int,ll>>> adj(5);
    for(size_t q=0;q<gT.size();q++){int mn=gT[q][0],mx=gT[q][0];
        for(int x:gT[q]){if(rk[x]<rk[mn])mn=x;if(rk[x]>rk[mx])mx=x;}
        adj[mn].push_back(make_pair(mx,gA[q]));adj[mx].push_back(make_pair(mn,gA[q]));}
    int s[5],cp[5];ll o[5];for(int i=0;i<5;i++){cp[i]=-1;s[i]=0;o[i]=0;}
    int nc=0;vector<vector<int>> mem(5);ll tlo[5],thi[5];
    bool bad[5]={false,false,false,false,false};
    for(int rt=0;rt<5;rt++){ if(cp[rt]!=-1)continue; int cid=nc++;
        cp[rt]=cid;s[rt]=1;o[rt]=0;vector<int> st;st.push_back(rt);bool det=false;ll tv0=0;
        while(!st.empty()){int u=st.back();st.pop_back();mem[cid].push_back(u);
            for(size_t e=0;e<adj[u].size();e++){int v=adj[u][e].first;ll w=adj[u][e].second;
                if(cp[v]==-1){cp[v]=cid;s[v]=-s[u];o[v]=w-o[u];st.push_back(v);}
                else{ll ss=s[u]+s[v],oo=o[u]+o[v];
                    if(ss==0){if(oo!=w)bad[cid]=true;}
                    else{ll num=w-oo;if(num%ss!=0)bad[cid]=true;else{ll tv=num/ss;
                        if(det){if(tv!=tv0)bad[cid]=true;}else{det=true;tv0=tv;}}}}}}
        if(det){tlo[cid]=tv0;thi[cid]=tv0;}else{tlo[cid]=0;thi[cid]=MAXV;}}
    for(int c=0;c<nc;c++){ if(bad[c])return Res{0,{}};
        for(int v:mem[c]){ll L,H;if(s[v]==1){L=-o[v];H=MAXV-o[v];}else{L=o[v]-MAXV;H=o[v];}
            tlo[c]=max(tlo[c],L);thi[c]=min(thi[c],H);}}
    for(int i=0;i+1<5;i++){int u=pm[i],v=pm[i+1];
        if(cp[u]==cp[v]){int c=cp[u],d=s[u]-s[v];ll D=o[v]-o[u];
            if(d==0){if(o[u]>o[v])bad[c]=true;}
            else if(d==2)thi[c]=min(thi[c],fdiv(D,2));
            else tlo[c]=max(tlo[c],cdiv(-D,2));}}
    for(int c=0;c<nc;c++) if(bad[c]||tlo[c]>thi[c]) return Res{0,{}};
    ll lo[5],hi[5];
    for(int c=0;c<nc;c++)for(int v:mem[c]){if(s[v]==1){lo[v]=tlo[c]+o[v];hi[v]=thi[c]+o[v];}else{lo[v]=o[v]-thi[c];hi[v]=o[v]-tlo[c];}}
    for(int it=0;it<100;it++){bool ch=false;
        for(int i=0;i+1<5;i++){int u=pm[i],v=pm[i+1];
            if(lo[v]<lo[u]){lo[v]=lo[u];ch=true;} if(hi[u]>hi[v]){hi[u]=hi[v];ch=true;}}
        for(int c=0;c<nc;c++)for(int v:mem[c]){ll L,H;
            if(s[v]==1){L=lo[v]-o[v];H=hi[v]-o[v];}else{L=o[v]-hi[v];H=o[v]-lo[v];}
            if(L>tlo[c]){tlo[c]=L;ch=true;} if(H<thi[c]){thi[c]=H;ch=true;}}
        for(int c=0;c<nc;c++)for(int v:mem[c]){ll L,H;
            if(s[v]==1){L=tlo[c]+o[v];H=thi[c]+o[v];}else{L=o[v]-thi[c];H=o[v]-tlo[c];}
            if(L>lo[v]){lo[v]=L;ch=true;} if(H<hi[v]){hi[v]=H;ch=true;}}
        for(int i=0;i<5;i++) if(lo[i]>hi[i]) return Res{0,{}};
        for(int c=0;c<nc;c++) if(tlo[c]>thi[c]) return Res{0,{}};
        if(!ch)break;}
    for(int c=0;c<nc;c++) if(tlo[c]!=thi[c]) return Res{2,{}};
    array<ll,5> val{};for(int v=0;v<5;v++)val[v]=s[v]*tlo[cp[v]]+o[v];
    for(int i=0;i<5;i++) if(val[i]<0||val[i]>MAXV) return Res{0,{}};
    for(int i=0;i+1<5;i++) if(val[pm[i]]>val[pm[i+1]]) return Res{0,{}};
    for(size_t q=0;q<gT.size();q++){int mn=gT[q][0],mx=gT[q][0];
        for(int x:gT[q]){if(rk[x]<rk[mn])mn=x;if(rk[x]>rk[mx])mx=x;}
        if(val[mn]+val[mx]!=gA[q]) return Res{0,{}};}
    return Res{1,val};
}
static int deficiency(const array<int,5>& pm,const vector<array<int,3>>& T){
    int rk[5];for(int i=0;i<5;i++)rk[pm[i]]=i;
    vector<vector<int>> adj(5);
    for(size_t q=0;q<T.size();q++){int mn=T[q][0],mx=T[q][0];
        for(int x:T[q]){if(rk[x]<rk[mn])mn=x;if(rk[x]>rk[mx])mx=x;}
        adj[mn].push_back(mx);adj[mx].push_back(mn);}
    int col[5]={-1,-1,-1,-1,-1};int def=0;
    for(int r=0;r<5;r++){ if(col[r]!=-1)continue; bool odd=false;
        vector<int> st;st.push_back(r);col[r]=0;
        while(!st.empty()){int u=st.back();st.pop_back();
            for(int v:adj[u]){if(col[v]==-1){col[v]=col[u]^1;st.push_back(v);}else if(col[v]==col[u])odd=true;}}
        if(!odd)def++;}
    return def;
}
static array<ll,5> bootstrap(){
    vector<array<int,5>> perms;array<int,5> p={0,1,2,3,4};
    do{perms.push_back(p);}while(next_permutation(p.begin(),p.end()));
    bool used[5][5][5]={};
    while(true){
        vector<array<ll,5>> cands;vector<char> feas(perms.size(),1);int fc=0;
        for(size_t i=0;i<perms.size();i++){Res r=analyze(perms[i]);
            if(r.status==0)feas[i]=0;else if(r.status==1)cands.push_back(r.v);else fc++;}
        sort(cands.begin(),cands.end());
        cands.erase(unique(cands.begin(),cands.end()),cands.end());
        if(cands.size()==1&&fc==0) return cands[0];
        pair<int,int> best=make_pair(INT_MAX,INT_MAX);array<int,3> bT{};bool found=false;
        for(int i=0;i<5;i++)for(int j=i+1;j<5;j++)for(int k=j+1;k<5;k++){
            if(used[i][j][k])continue;
            array<int,3> t={i,j,k};vector<array<int,3>> T2=gT;T2.push_back(t);
            int fa=0;
            for(size_t pi=0;pi<perms.size();pi++) if(feas[pi]&&deficiency(perms[pi],T2)>0) fa++;
            map<ll,int> grp;
            for(size_t z=0;z<cands.size();z++){const array<ll,5>&v=cands[z];
                ll mn=min(min(v[i],v[j]),v[k]),mx=max(max(v[i],v[j]),v[k]);grp[mn+mx]++;}
            int mg=0;for(map<ll,int>::iterator it=grp.begin();it!=grp.end();++it) mg=max(mg,it->second);
            pair<int,int> sc=make_pair(mg,fa);
            if(!found||sc<best){best=sc;bT=t;found=true;}}
        if(!found){ if(!cands.empty())return cands[0]; return array<ll,5>{0,0,0,0,0}; }
        used[bT[0]][bT[1]][bT[2]]=true;
        ll r=ask(bT[0]+1,bT[1]+1,bT[2]+1);
        gT.push_back(bT);gA.push_back(r);
    }
}
int main(){
    if(scanf("%d",&n)!=1) return 0;
    array<ll,5> bv=bootstrap();
    vector<ll> c(n+1,-1);
    for(int i=0;i<5;i++) c[i+1]=bv[i];
    vector<int> known;for(int i=1;i<=5;i++)known.push_back(i);
    sort(known.begin(),known.end(),[&](int x,int y){return c[x]<c[y];});
    for(int k=6;k<=n;k++){
        int bi=0;ll bg=LLONG_MAX;
        for(int i=0;i+1<(int)known.size();i++){
            ll g=c[known[i+1]]-c[known[i]];
            if(g<bg){bg=g;bi=i;}}
        int u=known[bi],v=known[bi+1];
        ll a=c[u],b=c[v];
        ll r=ask(u,v,k);
        ll val;
        if(r>a+b) val=r-a;
        else if(r<a+b) val=r-b;
        else if(a==b) val=a;
        else if(bi>0){int w=known[bi-1];ll r2=ask(w,u,k);val=r2-c[w];}
        else{int w=known[bi+2];ll r2=ask(v,w,k);val=r2-c[w];}
        c[k]=val;
        vector<int>::iterator it=upper_bound(known.begin(),known.end(),k,[&](int x,int y){return c[x]<c[y];});
        known.insert(it,k);
    }
    printf("!");
    for(int i=1;i<=n;i++) printf(" %lld",c[i]);
    printf("\n");
    fflush(stdout);
    return 0;
}
