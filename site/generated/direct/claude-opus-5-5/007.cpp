#include <bits/stdc++.h>
using namespace std;
typedef long long ll;
typedef __int128 LL;

static char buf[1<<25];
size_t bl=0,bp=0;
inline int gc(){
    if(bp==bl){ bl=fread(buf,1,sizeof(buf),stdin); bp=0; if(bl==0) return -1; }
    return buf[bp++];
}
inline ll rd(){
    int c=gc(); while(c!='-' && (c<'0'||c>'9')) c=gc();
    int s=1; if(c=='-'){s=-1;c=gc();}
    ll x=0; while(c>='0'&&c<='9'){x=x*10+(c-'0');c=gc();}
    return x*s;
}

int n;
vector<ll> X,Y,R,V;

struct Arc{int id; bool up;};
int curId=-1; bool curUp=false;

// sign of point vs arc: -1 below, 0 on, +1 above
static int pointVsArc(ll px,ll py,const Arc&a){
    int j=a.id;
    ll dx=px-X[j];
    LL h2=(LL)R[j]*R[j]-(LL)dx*dx;
    if(a.up){
        ll dy=py-Y[j];
        if(dy<0) return -1;
        LL d2=(LL)dy*dy;
        if(d2>h2) return 1;
        if(d2==h2) return 0;
        return -1;
    } else {
        ll dy=Y[j]-py;
        if(dy<0) return 1;
        LL d2=(LL)dy*dy;
        if(d2>h2) return -1;
        if(d2==h2) return 0;
        return 1;
    }
}
static int f(const Arc&nw,const Arc&o){
    int i=nw.id;
    int s=pointVsArc(X[i]-R[i],Y[i],o);
    if(s!=0) return s;
    if(nw.up!=o.up) return nw.up?1:-1;
    bool bigger=R[i]>R[o.id];
    if(nw.up) return bigger?1:-1;
    return bigger?-1:1;
}
struct Cmp{
    bool operator()(const Arc&a,const Arc&b)const{
        bool an=(a.id==curId&&a.up==curUp);
        bool bn=(b.id==curId&&b.up==curUp);
        if(an) return f(a,b)<0;
        if(bn) return f(b,a)>0;
        return false;
    }
};

static bool isTangent(int i,int j){
    ll dx=X[i]-X[j],dy=Y[i]-Y[j];
    LL d2=(LL)dx*dx+(LL)dy*dy;
    LL s=R[i]+R[j];
    LL t=R[i]-R[j];
    return d2==s*s || d2==t*t;
}

struct Rec{ll kx,ky,kd;int c;};
static bool recLess(const Rec&a,const Rec&b){
    if(a.kx!=b.kx) return a.kx<b.kx;
    if(a.ky!=b.ky) return a.ky<b.ky;
    if(a.kd!=b.kd) return a.kd<b.kd;
    return a.c<b.c;
}
static bool sameKey(const Rec&a,const Rec&b){return a.kx==b.kx&&a.ky==b.ky&&a.kd==b.kd;}

static void normalize(ll&nx,ll&ny,ll&d){
    if(d<0){nx=-nx;ny=-ny;d=-d;}
    ll g=std::gcd(std::gcd(llabs(nx),llabs(ny)),d);
    if(g>1){nx/=g;ny/=g;d/=g;}
}

int main(){
    n=(int)rd();
    ll Sx=rd(),Sy=rd(),Tx=rd(),Ty=rd();
    X.resize(n);Y.resize(n);R.resize(n);V.resize(n);
    for(int i=0;i<n;i++){X[i]=rd();Y[i]=rd();R[i]=rd();V[i]=rd();}

    vector<pair<int,int>> pairs;
    {
        vector<pair<ll,int>> ev;
        ev.reserve(2*(size_t)n);
        for(int i=0;i<n;i++){
            ev.push_back({X[i]-R[i],(1<<20)+i});
            ev.push_back({X[i]+R[i],i});
        }
        sort(ev.begin(),ev.end());
        set<Arc,Cmp> st;
        vector<set<Arc,Cmp>::iterator> its(2*(size_t)n);
        auto addPair=[&](int a,int b){
            if(a==b) return;
            if(isTangent(a,b)){
                if(a>b) swap(a,b);
                pairs.push_back({a,b});
            }
        };
        for(auto&e:ev){
            int id=e.second&((1<<20)-1);
            bool ins=(e.second>>20)!=0;
            if(!ins){
                for(int u=0;u<2;u++){
                    auto it=its[2*(size_t)id+u];
                    auto nx=next(it);
                    bool hasP=(it!=st.begin());
                    bool hasN=(nx!=st.end());
                    if(hasP&&hasN){
                        auto pv=prev(it);
                        addPair(pv->id,nx->id);
                    }
                    st.erase(it);
                }
            } else {
                for(int u=0;u<2;u++){
                    curId=id;curUp=(u==0);
                    auto it=st.insert(Arc{id,u==0}).first;
                    its[2*(size_t)id+u]=it;
                    if(it!=st.begin()) addPair(prev(it)->id,id);
                    auto nx=next(it);
                    if(nx!=st.end()) addPair(id,nx->id);
                }
            }
        }
    }
    sort(pairs.begin(),pairs.end());
    pairs.erase(unique(pairs.begin(),pairs.end()),pairs.end());

    vector<Rec> recs;
    recs.reserve(pairs.size()*2+2*(size_t)n/2+16);
    for(auto&p:pairs){
        int i=p.first,j=p.second;
        ll nx,ny,d;
        ll dx=X[i]-X[j],dy=Y[i]-Y[j];
        LL d2=(LL)dx*dx+(LL)dy*dy;
        LL s=R[i]+R[j];
        if(d2==s*s){
            nx=X[i]*R[j]+X[j]*R[i];
            ny=Y[i]*R[j]+Y[j]*R[i];
            d=R[i]+R[j];
        } else {
            nx=X[j]*R[i]-X[i]*R[j];
            ny=Y[j]*R[i]-Y[i]*R[j];
            d=R[i]-R[j];
        }
        normalize(nx,ny,d);
        recs.push_back({nx,ny,d,i});
        recs.push_back({nx,ny,d,j});
    }
    {
        vector<pair<pair<ll,ll>,int>> ex;
        ex.reserve(2*(size_t)n);
        for(int i=0;i<n;i++){
            ex.push_back({{X[i]-R[i],Y[i]},i});
            ex.push_back({{X[i]+R[i],Y[i]},i});
        }
        sort(ex.begin(),ex.end());
        size_t a=0;
        while(a<ex.size()){
            size_t b=a;
            while(b<ex.size()&&ex[b].first==ex[a].first) b++;
            if(b-a>=2){
                for(size_t k=a;k<b;k++)
                    recs.push_back({ex[k].first.first,ex[k].first.second,1,ex[k].second});
            }
            a=b;
        }
    }
    for(int i=0;i<n;i++){
        ll dx=Sx-X[i],dy=Sy-Y[i];
        if((LL)dx*dx+(LL)dy*dy==(LL)R[i]*R[i]) recs.push_back({Sx,Sy,1,i});
        dx=Tx-X[i];dy=Ty-Y[i];
        if((LL)dx*dx+(LL)dy*dy==(LL)R[i]*R[i]) recs.push_back({Tx,Ty,1,i});
    }
    sort(recs.begin(),recs.end(),recLess);
    recs.erase(unique(recs.begin(),recs.end(),[](const Rec&a,const Rec&b){return sameKey(a,b)&&a.c==b.c;}),recs.end());
    size_t P=recs.size();
    vector<int> pidOf(P);
    int M=0;
    for(size_t k=0;k<P;k++){
        if(k>0&&!sameKey(recs[k],recs[k-1])) M++;
        else if(k==0) M=0;
        pidOf[k]=M;
    }
    M=P?M+1:0;
    auto findPid=[&](ll x,ll y)->int{
        Rec key{x,y,1,-1};
        size_t lo=0,hi=P;
        while(lo<hi){
            size_t mid=(lo+hi)/2;
            const Rec&r=recs[mid];
            bool less;
            if(r.kx!=key.kx) less=r.kx<key.kx;
            else if(r.ky!=key.ky) less=r.ky<key.ky;
            else if(r.kd!=key.kd) less=r.kd<key.kd;
            else less=false;
            if(less) lo=mid+1; else hi=mid;
        }
        if(lo<P&&recs[lo].kx==x&&recs[lo].ky==y&&recs[lo].kd==1) return pidOf[lo];
        return -1;
    };
    int sp=findPid(Sx,Sy),tp=findPid(Tx,Ty);
    if(sp<0||tp<0){ printf("-1\n"); return 0; }

    vector<int> off(n+1,0);
    for(size_t k=0;k<P;k++) off[recs[k].c+1]++;
    for(int i=0;i<n;i++) off[i+1]+=off[i];
    vector<int> ordRec(P);
    {
        vector<int> fillp(off.begin(),off.end()-1);
        for(size_t k=0;k<P;k++) ordRec[fillp[recs[k].c]++]=(int)k;
    }
    vector<double> ang(P);
    vector<int> pidAt(P),circAt(P);
    {
        vector<pair<long double,int>> tmp;
        for(int c=0;c<n;c++){
            int L=off[c],Rr=off[c+1];
            if(L==Rr) continue;
            tmp.clear();
            for(int k=L;k<Rr;k++){
                const Rec&r=recs[ordRec[k]];
                ll vx=r.kx-X[c]*r.kd, vy=r.ky-Y[c]*r.kd;
                long double a=atan2l((long double)vy,(long double)vx);
                tmp.push_back({a,ordRec[k]});
            }
            sort(tmp.begin(),tmp.end());
            for(int k=L;k<Rr;k++){
                ang[k]=(double)tmp[k-L].first;
                pidAt[k]=pidOf[tmp[k-L].second];
                circAt[k]=c;
            }
        }
    }
    vector<int>().swap(ordRec);
    vector<Rec>().swap(recs);
    vector<int> pidOff(M+2,0);
    for(size_t k=0;k<P;k++) pidOff[pidAt[k]+1]++;
    for(int i=0;i<M;i++) pidOff[i+1]+=pidOff[i];
    vector<int> lst(P);
    {
        vector<int> fp(pidOff.begin(),pidOff.begin()+M);
        for(size_t k=0;k<P;k++) lst[fp[pidAt[k]]++]=(int)k;
    }
    const double INF=1e300;
    const double TWO_PI=2*acos(-1.0);
    vector<double> dist(M,INF);
    priority_queue<pair<double,int>,vector<pair<double,int>>,greater<pair<double,int>>> pq;
    dist[sp]=0;pq.push({0.0,sp});
    while(!pq.empty()){
        auto [d,u]=pq.top();pq.pop();
        if(d>dist[u]) continue;
        if(u==tp) break;
        for(int q=pidOff[u];q<pidOff[u+1];q++){
            int pos=lst[q];
            int c=circAt[pos];
            int L=off[c],Rr=off[c+1];
            if(Rr-L<2) continue;
            double fac=(double)V[c]*(double)R[c];
            int nx=(pos+1<Rr)?pos+1:L;
            double dn=(nx>pos)?ang[nx]-ang[pos]:ang[nx]+TWO_PI-ang[pos];
            int pv=(pos-1>=L)?pos-1:Rr-1;
            double dp=(pv<pos)?ang[pos]-ang[pv]:ang[pos]+TWO_PI-ang[pv];
            double w1=d+fac*dn;
            int v1=pidAt[nx];
            if(w1<dist[v1]){dist[v1]=w1;pq.push({w1,v1});}
            double w2=d+fac*dp;
            int v2=pidAt[pv];
            if(w2<dist[v2]){dist[v2]=w2;pq.push({w2,v2});}
        }
    }
    if(dist[tp]>=INF/2) printf("-1\n");
    else printf("%.10f\n",dist[tp]);
    return 0;
}
