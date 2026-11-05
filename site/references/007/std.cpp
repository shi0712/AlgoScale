// Arc Paths, n <= 1,000,000. Deterministic O(n log n), O(n) memory.
// C++17. Geometry is exact; only arc weights and shortest-path sums are floating point.
#include <bits/stdc++.h>
using namespace std;
using i64 = long long;
using i128 = __int128_t;
using u64 = unsigned long long;
using ld = long double;
struct FastInput {
    static constexpr int SZ = 1 << 16;
    char b[SZ]; int p=0,q=0;
    int get() { if(p==q){q=(int)fread(b,1,SZ,stdin);p=0;if(!q)return EOF;}return b[p++]; }
    int next(){int c=get(),s=1,x=0;while(c<=32&&c!=EOF)c=get();if(c=='-')s=-1,c=get();while(c>='0'&&c<='9'){x=x*10+c-'0';c=get();}return x*s;}
};
struct Circle { int x,y,r,v; };
static vector<Circle> cs;
struct ArcLess {
    bool operator()(int a,int b)const {
        int i=a>>1,j=b>>1;
        if(i==j)return a<b;
        const auto &A=cs[i],&B=cs[j];
        i128 dx=(i64)A.x-B.x,dy=(i64)A.y-B.y,dr=(i64)A.r-B.r;
        if(dx*dx+dy*dy<=dr*dr){
            if(A.r>B.r)return !(a&1);
            return b&1;
        }
        if(A.y!=B.y)return A.y<B.y;
        return A.x<B.x;
    }
};
struct Event { int x,code; bool operator<(const Event&b)const{
    if(x!=b.x)return x<b.x;
    // The highest bit used here separates all insertions from all deletions.
    return code<b.code;
}};
// Node number before angle sorting identifies its free transfer partner: id ^ 1.
// Additional S/T occurrences have no such partner and are handled as sources/targets.
struct Mark { int x,y,c,id; };
static int half(const Mark&a){return a.y>0||(a.y==0&&a.x>0)?0:1;}
static i64 cross(const Mark&a,const Mark&b){return (i64)a.x*b.y-(i64)a.y*b.x;}
static i64 dot(const Mark&a,const Mark&b){return (i64)a.x*b.x+(i64)a.y*b.y;}
struct AngleLess {
    bool operator()(const Mark&a,const Mark&b)const{
        if(a.c!=b.c)return a.c<b.c;
        int x=half(a),y=half(b);if(x!=y)return x<y;
        i64 z=cross(a,b);if(z)return z>0;
        return a.id<b.id;
    }
};
// Indexed binary heap: at most one heap entry per vertex; no unbounded stale copies.
struct Heap {
    vector<int> a,pos;
    const vector<double>&d;
    Heap(int n,const vector<double>&D):pos(n,-1),d(D){a.reserve(n);}
    void up(int p){while(p){int q=(p-1)/2;if(d[a[q]]<=d[a[p]])break;swap(a[p],a[q]);pos[a[p]]=p;pos[a[q]]=q;p=q;}}
    void push_or_decrease(int v){if(pos[v]==-2)return;if(pos[v]==-1){pos[v]=(int)a.size();a.push_back(v);}up(pos[v]);}
    int pop(){int v=a[0];pos[v]=-2;int w=a.back();a.pop_back();if(!a.empty()){
        a[0]=w;pos[w]=0;int p=0;
        for(;;){int l=p*2+1;if(l>=(int)a.size())break;int r=l+1,q=l;if(r<(int)a.size()&&d[a[r]]<d[a[l]])q=r;
            if(d[a[p]]<=d[a[q]])break;swap(a[p],a[q]);pos[a[p]]=p;pos[a[q]]=q;p=q;}}
        return v;
    }
};
int main(){
    FastInput in;int n=in.next(),sx=in.next(),sy=in.next(),tx=in.next(),ty=in.next();
    cs.resize(n);for(auto &c:cs){c.x=in.next();c.y=in.next();c.r=in.next();c.v=in.next();}
    if(sx==tx&&sy==ty){puts("0");return 0;}
    vector<u64> pairs;
    pairs.reserve((size_t)6*n);
    auto check=[&](int i,int j){
        if(i==j)return;
        i128 dx=(i64)cs[i].x-cs[j].x,dy=(i64)cs[i].y-cs[j].y;
        i128 s=(i64)cs[i].r+cs[j].r,d=(i64)cs[i].r-cs[j].r;
        i128 q=dx*dx+dy*dy;
        if(q==s*s||q==d*d){if(i>j)swap(i,j);pairs.push_back((u64)(unsigned)i<<32|(unsigned)j);}
    };
    {
        const int DEL=1<<21; // 2*n < 2^21; code actually stores the circle id.
        vector<Event> ev;ev.reserve((size_t)2*n);
        for(int i=0;i<n;i++){ev.push_back({cs[i].x-cs[i].r,i});ev.push_back({cs[i].x+cs[i].r,DEL+i});}
        sort(ev.begin(),ev.end());
        set<int,ArcLess> st;
        vector<array<set<int,ArcLess>::iterator,2>> where(n);
        for(auto e:ev){
            bool del=e.code>=DEL;int i=del?e.code-DEL:e.code;
            if(!del){for(int s=0;s<2;s++){
                auto p=st.insert(2*i+s).first;where[i][s]=p;
                if(p!=st.begin())check(*prev(p)>>1,i);
                auto q=next(p);if(q!=st.end())check(i,*q>>1);
            }}else{for(int s=0;s<2;s++){
                auto p=where[i][s],q=next(p);
                if(p!=st.begin()&&q!=st.end())check(*prev(p)>>1,*q>>1);
                st.erase(p);
            }}
        }
    }
    sort(pairs.begin(),pairs.end());pairs.erase(unique(pairs.begin(),pairs.end()),pairs.end());
    // Release spare pair storage before allocating marks.
    vector<u64>(pairs).swap(pairs);
    const int paired=(int)pairs.size()*2;
    vector<int> sources,targets;
    auto on=[&](int i,int x,int y){i128 dx=(i64)x-cs[i].x,dy=(i64)y-cs[i].y;return dx*dx+dy*dy==(i128)cs[i].r*cs[i].r;};
    size_t extra=0;for(int i=0;i<n;i++)extra+=on(i,sx,sy)+on(i,tx,ty);
    const int N=paired+(int)extra;
    vector<Mark> mark;mark.reserve(N);
    for(auto p:pairs){
        int i=(int)(p>>32),j=(int)(unsigned)p;
        int dx=cs[j].x-cs[i].x,dy=cs[j].y-cs[i].y;
        i128 sum=(i64)cs[i].r+cs[j].r;
        bool ext=(i128)dx*dx+(i128)dy*dy==sum*sum;
        int k=(int)mark.size();
        if(ext){mark.push_back({dx,dy,i,k});mark.push_back({-dx,-dy,j,k+1});}
        else{if(cs[i].r<cs[j].r)dx=-dx,dy=-dy;mark.push_back({dx,dy,i,k});mark.push_back({dx,dy,j,k+1});}
    }
    vector<u64>().swap(pairs);
    for(int i=0;i<n;i++){
        if(on(i,sx,sy)){int k=(int)mark.size();sources.push_back(k);mark.push_back({sx-cs[i].x,sy-cs[i].y,i,k});}
        if(on(i,tx,ty)){int k=(int)mark.size();targets.push_back(k);mark.push_back({tx-cs[i].x,ty-cs[i].y,i,k});}
    }
    sort(mark.begin(),mark.end(),AngleLess{});
    vector<int> pre(N),nxt(N);
    vector<double> cw(N);
    const ld TAU=2*acosl(-1.L);
    for(int l=0;l<N;){
        int r=l+1;while(r<N&&mark[r].c==mark[l].c)r++;
        const Circle &c=cs[mark[l].c];
        for(int k=l;k<r;k++){
            const auto&a=mark[k];const auto&b=mark[k+1==r?l:k+1];
            nxt[a.id]=b.id;pre[b.id]=a.id;
            ld theta=atan2l((ld)cross(a,b),(ld)dot(a,b));if(theta<0)theta+=TAU;
            cw[a.id]=(double)(theta*(ld)c.r*(ld)c.v);
        }
        l=r;
    }
    vector<Mark>().swap(mark);vector<Circle>().swap(cs);
    vector<double> dist(N,numeric_limits<double>::infinity());
    Heap heap(N,dist);
    for(int u:sources){dist[u]=0;heap.push_or_decrease(u);}
    vector<unsigned char> is_target(N,0);for(int u:targets)is_target[u]=1;
    auto relax=[&](int u,int v,double w){double nd=dist[u]+w;if(nd<dist[v]){dist[v]=nd;heap.push_or_decrease(v);}};
    while(!heap.a.empty()){
        int u=heap.pop();
        if(is_target[u]){printf("%.18f\n",dist[u]);return 0;}
        if(u<paired)relax(u,u^1,0);
        relax(u,nxt[u],cw[u]);relax(u,pre[u],cw[pre[u]]);
    }
    puts("-1");
}
