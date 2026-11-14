#include <bits/stdc++.h>
#include <ext/pb_ds/assoc_container.hpp>
#include <ext/pb_ds/tree_policy.hpp>
using namespace std;
using ll=long long;
using i128=__int128_t;
using ld=long double;
struct Point { int x,y; };
ll cross(Point a,Point b,Point c){return (ll)(b.x-a.x)*(c.y-a.y)-(ll)(b.y-a.y)*(c.x-a.x);}
// Divide-and-conquer Delaunay triangulation, using exact integer predicates.
struct Delaunay {
    vector<Point> p;
    vector<int> nxt,org,freeEdges;
    vector<unsigned char> alive;
    static int rot(int e){return (e&~3)|((e+1)&3);}
    static int rev(int e){return e^2;}
    static int invrot(int e){return (e&~3)|((e+3)&3);}
    int dest(int e)const{return org[e^2];}
    int lnext(int e)const{return rot(nxt[invrot(e)]);}
    int oprev(int e)const{return rot(nxt[rot(e)]);}
    int edge(int a,int b){
        int e;
        if(freeEdges.empty()) {e=nxt.size();nxt.resize(e+4);org.resize(e+4);alive.push_back(1);}
        else {e=freeEdges.back();freeEdges.pop_back();alive[e/4]=1;}
        nxt[e]=e;nxt[e+1]=e+3;nxt[e+2]=e+2;nxt[e+3]=e+1;
        org[e]=a;org[e+2]=b;org[e+1]=org[e+3]=-1;return e;
    }
    void splice(int a,int b){int x=rot(nxt[a]),y=rot(nxt[b]);swap(nxt[a],nxt[b]);swap(nxt[x],nxt[y]);}
    void erase(int e){splice(e,oprev(e));splice(e^2,oprev(e^2));alive[e/4]=0;freeEdges.push_back(e&~3);}
    int connect(int a,int b){int e=edge(dest(a),org[b]);splice(e,lnext(a));splice(e^2,b);return e;}
    bool left(int v,int e)const{return cross(p[org[e]],p[dest(e)],p[v])>0;}
    bool right(int v,int e)const{return cross(p[org[e]],p[dest(e)],p[v])<0;}
    bool inCircle(int a,int b,int c,int d)const{
        ll ax=p[a].x-p[d].x, ay=p[a].y-p[d].y;
        ll bx=p[b].x-p[d].x, by=p[b].y-p[d].y;
        ll cx=p[c].x-p[d].x, cy=p[c].y-p[d].y;
        return (i128)(ax*ax+ay*ay)*(bx*cy-by*cx)
             +(i128)(bx*bx+by*by)*(cx*ay-cy*ax)
             +(i128)(cx*cx+cy*cy)*(ax*by-ay*bx)>0;
    }
    pair<int,int> build(int l,int r){
        if(r-l==1){int a=edge(l,r);return {a,a^2};}
        if(r-l==2){
            int a=edge(l,l+1),b=edge(l+1,r);splice(a^2,b);
            ll s=cross(p[l],p[l+1],p[r]);
            if(s==0)return {a,b^2};
            int c=connect(b,a);
            if(s>0)return {a,b^2};
            return {c^2,c};
        }
        int m=(l+r)/2;
        auto [ldo,ldi]=build(l,m);auto [rdi,rdo]=build(m+1,r);
        while(true){
            if(left(org[rdi],ldi))ldi=lnext(ldi);
            else if(right(org[ldi],rdi))rdi=nxt[rdi^2];
            else break;
        }
        int base=connect(rdi^2,ldi);
        if(org[ldi]==org[ldo])ldo=base^2;
        if(org[rdi]==org[rdo])rdo=base;
        while(true){
            int lc=nxt[base^2];
            if(right(dest(lc),base)){
                while(inCircle(dest(base),org[base],dest(lc),dest(nxt[lc]))){int t=nxt[lc];erase(lc);lc=t;}
            }
            int rc=oprev(base);
            if(right(dest(rc),base)){
                while(inCircle(dest(base),org[base],dest(rc),dest(oprev(rc)))){int t=oprev(rc);erase(rc);rc=t;}
            }
            bool lv=right(dest(lc),base),rv=right(dest(rc),base);
            if(!lv&&!rv)break;
            if(!lv||(rv&&inCircle(dest(lc),org[lc],org[rc],dest(rc))))base=connect(rc,base^2);
            else base=connect(base^2,lc^2);
        }
        return {ldo,rdo};
    }
    Delaunay(vector<Point> points):p(move(points)){
        sort(p.begin(),p.end(),[](Point a,Point b){return tie(a.x,a.y)<tie(b.x,b.y);});
        nxt.reserve(p.size()*12);org.reserve(p.size()*12);alive.reserve(p.size()*3);
        build(0,(int)p.size()-1);
    }
};
// Polygon-edge hierarchy. The additional oriented boxes keep parallel,
// diagonal edges separated even when their axis-aligned boxes overlap.
struct BVH {
    struct Node {
        int xmin,xmax,ymin,ymax,left=-1,right=-1,start=0,count=0;
        int ux=0,uy=0;ll umin=0,umax=0,vmin=0,vmax=0;
    };
    vector<Point> p; vector<int> ids;vector<Node> nodes;
    BVH(vector<Point> a):p(move(a)),ids(p.size()) {iota(ids.begin(),ids.end(),0);nodes.reserve(p.size()/3);build(0,ids.size());}
    Point end(int i)const{return p[i+1==(int)p.size()?0:i+1];}
    int build(int l,int r){
        int v=nodes.size();nodes.push_back({10001,-10001,10001,-10001});
        int cxmin=40001,cxmax=-40001,cymin=40001,cymax=-40001;
        ll longest=-1;int ux=0,uy=0;
        for(int j=l;j<r;j++){Point a=p[ids[j]],b=end(ids[j]);auto &q=nodes[v];q.xmin=min({q.xmin,a.x,b.x});q.xmax=max({q.xmax,a.x,b.x});q.ymin=min({q.ymin,a.y,b.y});q.ymax=max({q.ymax,a.y,b.y});cxmin=min(cxmin,a.x+b.x);cxmax=max(cxmax,a.x+b.x);cymin=min(cymin,a.y+b.y);cymax=max(cymax,a.y+b.y);ll len=(ll)(b.x-a.x)*(b.x-a.x)+(ll)(b.y-a.y)*(b.y-a.y);if(len>longest){longest=len;ux=b.x-a.x;uy=b.y-a.y;}}
        if(ux!=0&&uy!=0){
            auto &q=nodes[v];q.ux=ux;q.uy=uy;q.umin=q.vmin=LLONG_MAX;q.umax=q.vmax=LLONG_MIN;
            for(int j=l;j<r;j++)for(Point a:{p[ids[j]],end(ids[j])}){
                ll u=(ll)a.x*ux+(ll)a.y*uy,w=-(ll)a.x*uy+(ll)a.y*ux;
                q.umin=min(q.umin,u);q.umax=max(q.umax,u);q.vmin=min(q.vmin,w);q.vmax=max(q.vmax,w);
            }
        }
        if(r-l<=8){nodes[v].start=l;nodes[v].count=r-l;return v;}
        bool x=cxmax-cxmin>=cymax-cymin;
        int m=(l+r)/2;
        nth_element(ids.begin()+l,ids.begin()+m,ids.begin()+r,[&](int i,int j){Point a=end(i),b=end(j);return x?p[i].x+a.x<p[j].x+b.x:p[i].y+a.y<p[j].y+b.y;});
        int a=build(l,m),b=build(m,r);nodes[v].left=a;nodes[v].right=b;return v;
    }
    struct Line {ld mx,my,dx,dy,len2;};
    // Clip a parameter interval to an expanded node box.
    bool clip(const Line&z,int v,ld &lo,ld &hi)const{
        const Node&q=nodes[v];
        constexpr ld eps=1e-9L;
        if(z.dx==0){if(z.mx<q.xmin-eps||z.mx>q.xmax+eps)return false;}
        else {ld a=(q.xmin-eps-z.mx)/z.dx,b=(q.xmax+eps-z.mx)/z.dx;if(a>b)swap(a,b);lo=max(lo,a);hi=min(hi,b);if(lo>hi)return false;}
        if(z.dy==0){if(z.my<q.ymin-eps||z.my>q.ymax+eps)return false;}
        else {ld a=(q.ymin-eps-z.my)/z.dy,b=(q.ymax+eps-z.my)/z.dy;if(a>b)swap(a,b);lo=max(lo,a);hi=min(hi,b);if(lo>hi)return false;}
        if(q.ux!=0){
            ld origin=z.mx*q.ux+z.my*q.uy,dir=z.dx*q.ux+z.dy*q.uy;
            ld margin=eps*(abs(q.ux)+abs(q.uy));
            if(dir==0){if(origin<q.umin-margin||origin>q.umax+margin)return false;}
            else{ld a=(q.umin-margin-origin)/dir,b=(q.umax+margin-origin)/dir;if(a>b)swap(a,b);lo=max(lo,a);hi=min(hi,b);if(lo>hi)return false;}
            origin=-z.mx*q.uy+z.my*q.ux;dir=-z.dx*q.uy+z.dy*q.ux;
            if(dir==0){if(origin<q.vmin-margin||origin>q.vmax+margin)return false;}
            else{ld a=(q.vmin-margin-origin)/dir,b=(q.vmax+margin-origin)/dir;if(a>b)swap(a,b);lo=max(lo,a);hi=min(hi,b);if(lo>hi)return false;}
        }
        return true;
    }
    void extreme(const Line&z,int v,ld lo,ld hi,bool positive,ld &hit,bool &found)const{
        if(!clip(z,v,lo,hi))return;
        if(found&&(positive?hi<=hit:lo>=hit))return;
        const Node&q=nodes[v];
        if(q.count){
            for(int j=q.start;j<q.start+q.count;j++){
                Point a=p[ids[j]],b=end(ids[j]);
                ll ex=b.x-a.x,ey=b.y-a.y;
                ld px=a.x-z.mx,py=a.y-z.my;
                ld den=z.dx*ey-z.dy*ex;
                ld t;
                if(den!=0){
                    ld s=(px*z.dy-py*z.dx)/den;
                    if(s < -1e-16L||s > 1+1e-16L)continue;
                    t=(px*ey-py*ex)/den;
                    if(t<lo-1e-14L||t>hi+1e-14L)continue;
                    t=max(lo,min(hi,t));
                }else{
                    if(px*z.dy-py*z.dx!=0)continue;
                    ld t1=(px*z.dx+py*z.dy)/z.len2;
                    ld t2=((b.x-z.mx)*z.dx+(b.y-z.my)*z.dy)/z.len2;
                    if(t1>t2)swap(t1,t2);
                    t1=max(t1,lo);t2=min(t2,hi);if(t1>t2)continue;
                    t=positive?t2:t1;
                }
                if(!found||(positive?t>hit:t<hit)){found=true;hit=t;}
            }
        }else{
            ld al=lo,ah=hi,bl=lo,bh=hi;
            bool ca=clip(z,q.left,al,ah),cb=clip(z,q.right,bl,bh);
            if(!ca&&!cb)return;
            if(ca&&cb){
                bool first=positive?ah>bh:al<bl;
                if(first){extreme(z,q.left,al,ah,positive,hit,found);extreme(z,q.right,bl,bh,positive,hit,found);}
                else {extreme(z,q.right,bl,bh,positive,hit,found);extreme(z,q.left,al,ah,positive,hit,found);}
            }else if(ca)extreme(z,q.left,al,ah,positive,hit,found);
            else extreme(z,q.right,bl,bh,positive,hit,found);
        }
    }
};
struct Center {ld x,y,r2;};
struct VEdge {BVH::Line z;ld lo,hi,upper;};

// The active polygon edges never cross, so their vertical order is fixed
// between vertex events. An order-statistics sweep classifies all centers.
ld interiorMaximum(const vector<Point>&p,vector<Center>&centers){
    struct Segment {ll x,y,dx,dy;};
    struct Event {int x,e,type;bool operator<(const Event&b)const{return tie(x,type)<tie(b.x,b.type);}};
    vector<Segment> seg;vector<Event> events;
    for(int i=0;i<(int)p.size();i++){
        Point a=p[i],b=p[(i+1)%p.size()];if(a.x==b.x)continue;
        if(a.x>b.x)swap(a,b);int e=seg.size();
        seg.push_back({a.x,a.y,b.x-a.x,b.y-a.y});events.push_back({a.x,e,1});events.push_back({b.x,e,-1});
    }
    ld sweepX=0,queryY=0;
    struct Cmp {
        const vector<Segment>*s;const ld*x;const ld*y;
        bool operator()(int a,int b)const{
            if(a==b)return false;
            auto value=[&](int e)->ld{if(e<0)return *y;const auto&q=(*s)[e];return q.y+(*x-q.x)*q.dy/q.dx;};
            ld ya=value(a),yb=value(b);if(ya!=yb)return ya<yb;
            if(a<0||b<0)return a<b;
            const auto&u=(*s)[a];const auto&v=(*s)[b];
            ll lhs=u.dy*v.dx,rhs=v.dy*u.dx;
            return lhs!=rhs?lhs<rhs:a<b;
        }
    };
    using namespace __gnu_pbds;
    using Tree=tree<int,null_type,Cmp,rb_tree_tag,tree_order_statistics_node_update>;
    Tree active(Cmp{&seg,&sweepX,&queryY});
    vector<Tree::point_iterator> handles(seg.size());
    sort(events.begin(),events.end());
    sort(centers.begin(),centers.end(),[](const Center&a,const Center&b){return a.x<b.x;});
    size_t pos=0;ld best=0;
    for(const auto&c:centers){
        while(pos<events.size()&&events[pos].x<=c.x){
            sweepX=events[pos].x;
            int x=events[pos].x;
            while(pos<events.size()&&events[pos].x==x&&events[pos].type<0){active.erase(handles[events[pos].e]);pos++;}
            while(pos<events.size()&&events[pos].x==x){int e=events[pos++].e;handles[e]=active.insert(e).first;}
        }
        sweepX=c.x;queryY=c.y;
        if((active.order_of_key(-1)&1)!=0)best=max(best,c.r2);
    }
    return best;
}

// Within a Voronoi cell the squared nearest-site distance is convex.
// Thus its maximum over the polygon occurs at a Voronoi vertex inside the
// polygon or where a Voronoi edge meets the polygon boundary (polygon
// vertices themselves have distance zero).
// On the bisector M+t*D, that distance is |D|^2*(1/4+t^2), so only the
// first/last boundary intersection of each Voronoi edge can be optimal.
int main(){
    ios::sync_with_stdio(false);cin.tie(nullptr);
    int n;if(!(cin>>n))return 0;vector<Point> p(n);for(auto &a:p)cin>>a.x>>a.y;
    BVH bvh(p);Delaunay dt(p);
    vector<Center> centers;vector<VEdge> edges;centers.reserve(2*n);edges.reserve(3*n);
    for(int e=0;e<(int)dt.nxt.size();e+=4)if(dt.alive[e/4]){
        Point a=dt.p[dt.org[e]],b=dt.p[dt.dest(e)];
        BVH::Line z{(a.x+b.x)/2.L,(a.y+b.y)/2.L,(ld)a.y-b.y,(ld)b.x-a.x,0};
        z.len2=z.dx*z.dx+z.dy*z.dy;
        ld lo=-1e30L,hi=1e30L;
        for(int side=0;side<2;side++){
            int h=e^(side*2),f=dt.lnext(h),g=dt.lnext(f);
            if(dt.lnext(g)!=h)continue;
            Point c=dt.p[dt.dest(f)];
            if(cross(dt.p[dt.org[h]],dt.p[dt.dest(h)],c)<=0)continue;
            ll ux=c.x-a.x,uy=c.y-a.y,vx=b.x-a.x,vy=b.y-a.y;
            ld t=(ld)(ux*ux+uy*uy-ux*vx-uy*vy)/(2.L*(ux*z.dx+uy*z.dy));
            if(side==0)hi=t;else lo=t;
            if(h<f&&h<g){
                ld x=z.mx+t*z.dx,y=z.my+t*z.dy;
                const auto&q=bvh.nodes[0];
                if(x>=q.xmin&&x<=q.xmax&&y>=q.ymin&&y<=q.ymax)centers.push_back({x,y,z.len2*(.25L+t*t)});
            }
        }
        if(lo>hi)swap(lo,hi); // only possible for roundoff at cocircular sites
        if(bvh.clip(z,0,lo,hi))edges.push_back({z,lo,hi,z.len2*(.25L+max(lo*lo,hi*hi))});
    }
    ld best=interiorMaximum(p,centers);
    sort(edges.begin(),edges.end(),[](const VEdge&a,const VEdge&b){return a.upper>b.upper;});
    for(const auto&e:edges){
        if(e.upper<=best)break;
        ld threshold=sqrtl(max(0.L,best/e.z.len2-.25L));
        if(e.hi>=threshold){ld hit=0;bool found=false;bvh.extreme(e.z,0,max(e.lo,threshold),e.hi,true,hit,found);if(found)best=max(best,e.z.len2*(.25L+hit*hit));}
        threshold=sqrtl(max(0.L,best/e.z.len2-.25L));
        if(e.lo<=-threshold){ld hit=0;bool found=false;bvh.extreme(e.z,0,e.lo,min(e.hi,-threshold),false,hit,found);if(found)best=max(best,e.z.len2*(.25L+hit*hit));}
    }
    cout<<fixed<<setprecision(12)<<sqrtl(best)<<'\n';
}
