#include <bits/stdc++.h>
// Nearest-Delaunay quad-edge divide-and-conquer adapted from cp-algorithms:
// https://cp-algorithms.com/geometry/delaunay.html (CC BY-SA 4.0).
// Changes: indexed vertices, exact predicates, edge-pool recycling, adjacency output.
// This adapted source is distributed under CC BY-SA 4.0.
using namespace std;
using ll=long long;using I=__int128_t;using ld=long double;
struct P{ll x,y;};
ll orient(P a,P b,P c){return (b.x-a.x)*(c.y-a.y)-(b.y-a.y)*(c.x-a.x);}
I incircle(P a,P b,P c,P d){
    ll ax=a.x-d.x,ay=a.y-d.y,bx=b.x-d.x,by=b.y-d.y,cx=c.x-d.x,cy=c.y-d.y;
    return ((I)ax*ax+(I)ay*ay)*((I)bx*cy-(I)by*cx)+((I)bx*bx+(I)by*by)*((I)cx*ay-(I)cy*ax)+((I)cx*cx+(I)cy*cy)*((I)ax*by-(I)ay*bx);
}
struct NearestDelaunay{
    struct Block;
    struct Q{int from=-1;Q*rot=nullptr,*next=nullptr;Block*owner=nullptr;Q*rev(){return rot->rot;}Q*lnext(){return rot->rev()->next->rot;}Q*oprev(){return rot->next->rot;}int to(){return rev()->from;}};
    struct Block{Q e[4];bool active=false;};
    const vector<P>&p;vector<int>order;vector<unique_ptr<Block>>storage;vector<Block*>free_blocks;
    NearestDelaunay(const vector<P>&points):p(points){}
    Q*make(int a,int b){
        Block*z;
        if(free_blocks.empty()){storage.push_back(make_unique<Block>());z=storage.back().get();}
        else{z=free_blocks.back();free_blocks.pop_back();}
        z->active=true;for(auto&e:z->e){e=Q();e.owner=z;}Q*x=z->e;
        x[0].from=a;x[1].from=b;x[0].rot=x+2;x[1].rot=x+3;x[2].rot=x+1;x[3].rot=x;
        x[0].next=x;x[1].next=x+1;x[2].next=x+3;x[3].next=x+2;return x;
    }
    void splice(Q*a,Q*b){swap(a->next->rot->next,b->next->rot->next);swap(a->next,b->next);}
    void erase(Q*e){splice(e,e->oprev());splice(e->rev(),e->rev()->oprev());auto z=e->owner;z->active=false;free_blocks.push_back(z);}
    Q*connect(Q*a,Q*b){Q*e=make(a->to(),b->from);splice(e,a->lnext());splice(e->rev(),b);return e;}
    bool left(int v,Q*e){return orient(p[e->from],p[e->to()],p[v])>0;}
    bool right(int v,Q*e){return orient(p[e->from],p[e->to()],p[v])<0;}
    bool circle(int a,int b,int c,int d){return incircle(p[a],p[b],p[c],p[d])>0;}
    pair<Q*,Q*> build(int l,int r){
        if(r-l==1){Q*e=make(order[l],order[r]);return {e,e->rev()};}
        if(r-l==2){
            Q*a=make(order[l],order[l+1]),*b=make(order[l+1],order[r]);splice(a->rev(),b);
            ll s=orient(p[order[l]],p[order[l+1]],p[order[r]]);if(!s)return {a,b->rev()};
            Q*c=connect(b,a);return s>0?make_pair(a,b->rev()):make_pair(c->rev(),c);
        }
        int mid=(l+r)/2;auto [ldo,ldi]=build(l,mid);auto [rdi,rdo]=build(mid+1,r);
        while(true){if(left(rdi->from,ldi))ldi=ldi->lnext();else if(right(ldi->from,rdi))rdi=rdi->rev()->next;else break;}
        Q*base=connect(rdi->rev(),ldi);if(ldi->from==ldo->from)ldo=base->rev();if(rdi->from==rdo->from)rdo=base;
        auto valid=[&](Q*e){return right(e->to(),base);};
        while(true){
            Q*lc=base->rev()->next;
            if(valid(lc))while(circle(base->to(),base->from,lc->to(),lc->next->to())){Q*t=lc->next;erase(lc);lc=t;}
            Q*rc=base->oprev();
            if(valid(rc))while(circle(base->to(),base->from,rc->to(),rc->oprev()->to())){Q*t=rc->oprev();erase(rc);rc=t;}
            if(!valid(lc)&&!valid(rc))break;
            if(!valid(lc)||(valid(rc)&&circle(lc->to(),lc->from,rc->from,rc->to())))base=connect(rc,base->rev());
            else base=connect(base->rev(),lc->rev());
        }return {ldo,rdo};
    }
    vector<vector<int>> run(){
        order.resize(p.size());iota(order.begin(),order.end(),0);sort(order.begin(),order.end(),[&](int a,int b){return tie(p[a].x,p[a].y)<tie(p[b].x,p[b].y);});
        build(0,(int)p.size()-1);vector<vector<int>>adj(p.size());
        for(const auto&z:storage)if(z->active){
            int a=z->e[0].from,b=z->e[1].from;adj[a].push_back(b);adj[b].push_back(a);
#ifdef GEOMETRY_AUDIT
            for(int k=0;k<2;k++){
                Q*e=&z->e[k];Q*f=e->lnext();Q*g=f->lnext();
                if(g->lnext()==e&&e->from<f->from&&e->from<g->from&&orient(p[e->from],p[f->from],p[g->from])>0)
                    for(P v:p)assert(incircle(p[e->from],p[f->from],p[g->from],v)<=0);
            }
#endif
        }
        return adj;
    }
};
struct Line{ll a,b,c;}; // a*x+b*y <= c
I det(Line a,Line b){return (I)a.a*b.b-(I)a.b*b.a;}
bool outside(Line a,Line b,Line c){
    I d=det(a,b);assert(d!=0);
    I x=(I)a.c*b.b-(I)a.b*b.c,y=(I)a.a*b.c-(I)a.c*b.a;
    I v=(I)c.a*x+(I)c.b*y-(I)c.c*d;return d>0?v>0:v<0;
}
vector<pair<ld,ld>> intersect(vector<Line> a){
    auto half=[](Line x){return x.b>0||(x.b==0&&x.a>0);};
    sort(a.begin(),a.end(),[&](Line x,Line y){if(half(x)!=half(y))return half(x)>half(y);return det(x,y)>0;});
    vector<Line>uniq;
    for(auto l:a){
        if(!uniq.empty()&&half(uniq.back())==half(l)&&det(uniq.back(),l)==0){
            auto z=uniq.back();ll u=abs(l.a?l.a:l.b),v=abs(z.a?z.a:z.b);
            if((I)l.c*v<(I)z.c*u)uniq.back()=l;
        }else uniq.push_back(l);
    }
    deque<Line>q;
    for(auto l:uniq){
        while(q.size()>1&&outside(q[q.size()-2],q.back(),l))q.pop_back();
        while(q.size()>1&&outside(q[0],q[1],l))q.pop_front();
        if(!q.empty()&&det(q.back(),l)==0)return {};
        q.push_back(l);
    }
    while(q.size()>2&&outside(q[q.size()-2],q.back(),q.front()))q.pop_back();
    while(q.size()>2&&outside(q[0],q[1],q.back()))q.pop_front();
    if(q.size()<3||det(q.back(),q.front())==0)return {};
    vector<pair<ld,ld>>p;
    for(int i=0;i<(int)q.size();i++){
        Line u=q[i],v=q[(i+1)%q.size()];ld d=(ld)det(u,v);
        p.push_back({((ld)u.c*v.b-(ld)u.b*v.c)/d,((ld)u.a*v.c-(ld)u.c*v.a)/d});
    }return p;
}
ld integral(const vector<pair<ld,ld>>&p,P s){
    ld a2=0,x6=0,y6=0;
    for(int i=0;i<(int)p.size();i++){
        auto [x,y]=p[i];auto [u,v]=p[(i+1)%p.size()];ld c=x*v-y*u;
        a2+=c;x6+=(x+u)*c;y6+=(y+v)*c;
    }
    return ((ld)s.x*s.x+(ld)s.y*s.y)*a2/2-s.x*x6/3-s.y*y6/3;
}
vector<int> convex_hull(const vector<P>&p){
    vector<int>ord(p.size());iota(ord.begin(),ord.end(),0);
    sort(ord.begin(),ord.end(),[&](int a,int b){return tie(p[a].x,p[a].y)<tie(p[b].x,p[b].y);});
    vector<int>h;
    for(int i:ord){while(h.size()>1&&orient(p[h[h.size()-2]],p[h.back()],p[i])<=0)h.pop_back();h.push_back(i);}
    int low=h.size();
    for(int k=(int)ord.size()-2;k>=0;k--){int i=ord[k];while((int)h.size()>low&&orient(p[h[h.size()-2]],p[h.back()],p[i])<=0)h.pop_back();h.push_back(i);}
    h.pop_back();return h;
}
// Exact incircle predicate; infinitesimal z_i += eps^(i+1) resolves cocircular ties.
bool inside_circle(const vector<P>&p,int a,int b,int c,int d){
    ll ax=p[a].x-p[d].x,ay=p[a].y-p[d].y,bx=p[b].x-p[d].x,by=p[b].y-p[d].y,cx=p[c].x-p[d].x,cy=p[c].y-p[d].y;
    I ab=(I)ax*by-(I)ay*bx,bc=(I)bx*cy-(I)by*cx,ca=(I)cx*ay-(I)cy*ax;
    I z=((I)ax*ax+(I)ay*ay)*bc+((I)bx*bx+(I)by*by)*ca+((I)cx*cx+(I)cy*cy)*ab;
    if(z)return z>0;
    array<pair<int,I>,4>coef={{{a,bc},{b,ca},{c,ab},{d,-(I)orient(p[a],p[b],p[c])}}};
    sort(coef.begin(),coef.end(),[](auto u,auto v){return u.first<v.first;});
    for(auto [id,v]:coef)if(v)return v>0;
    assert(false);return false;
}
vector<vector<int>> farthest_neighbors(const vector<P>&p,mt19937_64&rng){
    int n=p.size();vector<vector<int>>adj(n);auto h=convex_hull(p);int m=h.size();
    auto edge=[&](int a,int b){adj[a].push_back(b);adj[b].push_back(a);};
    if(m==2){edge(h[0],h[1]);return adj;}
    vector<int>ord(m),left(m),right(m),L(m),R(m);iota(ord.begin(),ord.end(),0);shuffle(ord.begin(),ord.end(),rng);
    for(int i=0;i<m;i++){left[i]=(i+m-1)%m;right[i]=(i+1)%m;}
    for(int i=m-1;i>=3;i--){int v=ord[i];L[v]=left[v];R[v]=right[v];right[left[v]]=right[v];left[right[v]]=left[v];}
    // For oriented triangle edge a->b, store its third vertex on the left.
    map<pair<int,int>,int>tri;
    auto add=[&](int a,int b,int c){assert(orient(p[a],p[b],p[c])>0);tri[{a,b}]=c;tri[{b,c}]=a;tri[{c,a}]=b;};
    auto remove=[&](int a,int b,int c){tri.erase({a,b});tri.erase({b,c});tri.erase({c,a});};
    vector<int>start={ord[0],ord[1],ord[2]};sort(start.begin(),start.end());add(h[start[0]],h[start[1]],h[start[2]]);
    for(int k=3;k<m;k++){
        int index=ord[k],v=h[index];vector<pair<int,int>>stack={{h[L[index]],h[R[index]]}};
        while(!stack.empty()){
            auto [a,b]=stack.back();stack.pop_back();auto it=tri.find({a,b});
            if(it!=tri.end()&&!inside_circle(p,a,b,it->second,v)){
                int c=it->second;remove(a,b,c);stack.push_back({a,c});stack.push_back({c,b});
            }else add(a,v,b);
        }
    }
    for(auto [ab,c]:tri){
#ifdef GEOMETRY_AUDIT
        if(ab.first<ab.second&&ab.first<c)for(int v:h)
            if(v!=ab.first&&v!=ab.second&&v!=c)assert(inside_circle(p,ab.first,ab.second,c,v));
#endif
        edge(ab.first,ab.second);
    }
    for(auto&v:adj){sort(v.begin(),v.end());v.erase(unique(v.begin(),v.end()),v.end());}
    return adj;
}
int main(){
    ios::sync_with_stdio(false);cin.tie(nullptr);ll xl,yl,xr,yr;if(!(cin>>xl>>yl>>xr>>yr))return 0;
    // Exact integer translation reduces cancellation in thin, distant rectangles.
    ll ox=(xl+xr)/2,oy=(yl+yr)/2;xl-=ox;xr-=ox;yl-=oy;yr-=oy;
    int n;cin>>n;vector<P>p(n);for(auto&v:p){cin>>v.x>>v.y;v.x-=ox;v.y-=oy;}
    auto near=NearestDelaunay(p).run();
#ifdef FIXED_SEED
    mt19937_64 rng(FIXED_SEED);
#else
    mt19937_64 rng(chrono::steady_clock::now().time_since_epoch().count());
#endif
    auto far=farthest_neighbors(p,rng);ld answer=0;
    for(int sign:{1,-1}){
        const auto&adj=sign==1?near:far;
        for(int i=0;i<n;i++){
            if(sign==-1&&adj[i].empty())continue;
            vector<Line>lines={{-1,0,-xl},{1,0,xr},{0,-1,-yl},{0,1,yr}};
            for(int j:adj[i])lines.push_back({2*(p[j].x-p[i].x)*sign,2*(p[j].y-p[i].y)*sign,(p[j].x*p[j].x+p[j].y*p[j].y-p[i].x*p[i].x-p[i].y*p[i].y)*sign});
            answer-=sign*integral(intersect(move(lines)),p[i]);
        }
    }
    cout<<fixed<<setprecision(15)<<answer*acosl(-1.L)/((ld)(xr-xl)*(yr-yl))<<'\n';
}
