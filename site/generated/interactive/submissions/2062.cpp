#include <bits/stdc++.h>
using namespace std;

// All 400 initial positions are defendable inside [1,31]^2 using its
// northern and eastern boundary at coordinate 32. The fixed-point game
// below tests this same condition for each smaller candidate enclosure.
// Certified moves keep every surviving reply in a defendable position.
// A short forced capture can take over directly. All moves paint a fresh
// cell in [1,32]^2, so even an adversarial play is finite.
constexpr int N = 32;
constexpr int STR = N + 2;
using Mask = unsigned __int128;
using Line = uint64_t;
int cell(int x, int y) { return x * STR + y; }
int xpos(int p) { return p / STR; }
int ypos(int p) { return p % STR; }
Mask bit(int v) { return Mask(1) << v; }
Line interval(int a, int b) { return ((Line(2) << b) - 1) ^ ((Line(1) << a) - 1); }

// A variable is true when its boundary cell is still white.
struct Node {
    int v, lo, hi;
    bool operator==(const Node&) const = default;
};
struct NodeHash {
    size_t operator()(Node n) const {
        return (uint64_t(n.v) * 314159265 + n.lo) * 998244353 + n.hi;
    }
};
struct Games {
    vector<Node> nodes{{100,0,0},{100,1,1}};
    unordered_map<Node,int,NodeHash> unique;
    unordered_map<uint64_t,int> cache[2];
    unordered_map<int,int> deletionCache;
    int lose[STR][STR]{}, threat[STR][STR]{};

    int node(int v, int lo, int hi) {
        if (lo == hi) return lo;
        auto [it, fresh] = unique.emplace(Node{v,lo,hi}, int(nodes.size()));
        if (fresh) nodes.push_back({v,lo,hi});
        return it->second;
    }
    int combine(int a, int b, bool both) {
        if (a == b) return a;
        if (a > b) swap(a,b);
        if (a == 0) return both ? 0 : b;
        if (a == 1) return both ? b : 1;
        uint64_t key = (uint64_t(a) << 32) | b;
        auto it = cache[both].find(key);
        if (it != cache[both].end()) return it->second;
        Node s = nodes[a], t = nodes[b];
        int v = min(s.v,t.v);
        int lo = combine(s.v == v ? s.lo : a, t.v == v ? t.lo : b, both);
        int hi = combine(s.v == v ? s.hi : a, t.v == v ? t.hi : b, both);
        return cache[both][key] = node(v,lo,hi);
    }
    // The robot wins despite the painter being able to erase any one variable.
    int afterAnyDeletion(int a) {
        if (a <= 1) return a;
        auto it = deletionCache.find(a);
        if (it != deletionCache.end()) return it->second;
        auto [v,lo,hi] = nodes[a];
        int l = afterAnyDeletion(lo);
        int h = combine(lo,afterAnyDeletion(hi),true);
        return deletionCache[a] = node(v,l,h);
    }
    // At a boundary cell the escape predicate is its white bit. For an
    // interior cell, OR over the eight replies gives the escape predicate
    // after painting. Universal quantification over one deletion then gives
    // the predicate before painting. Iterating from false computes the
    // finite escape game's least fixed point, simultaneously for all masks.
    void build() {
        for (int a = 0; a <= N; ++a) lose[a][0] = node(N-a,0,1);
        for (int b = 1; b <= N; ++b) lose[0][b] = node(N+b,0,1);
        for (;;) {
            int next[STR][STR]{};
            bool changed = false;
            for (int a=1; a<=N; ++a) for (int b=1; b<=N; ++b) {
                int g=0;
                for (int i=-1;i<=1;++i) for (int j=-1;j<=1;++j)
                    if (i || j) g=combine(g,lose[a+i][b+j],false);
                threat[a][b]=g;
                next[a][b]=afterAnyDeletion(g);
                changed |= next[a][b]!=lose[a][b];
            }
            if (!changed) break;
            for (int a=1;a<=N;++a) copy_n(next[a]+1,N,lose[a]+1);
        }
        unique.clear(); cache[0].clear(); cache[1].clear(); deletionCache.clear();
    }
    bool eval(int a, Mask m) const {
        while (a>1) {
            const Node& n=nodes[a];
            a=(m & bit(n.v)) ? n.hi : n.lo;
        }
        return a==1;
    }
} game;

constexpr int HORIZON=3, SIDE=2*HORIZON+1;
struct LocalGame {
    Games bdd;
    int form[HORIZON+1][HORIZON+2][HORIZON+2][HORIZON+2][HORIZON+2]{};
    unordered_map<uint64_t,int> shifted;
    int shift(int f,int d) {
        if(f<2 || d==0)return f;
        uint64_t k=(uint64_t(f)<<8)|(d+64);
        auto it=shifted.find(k);
        if(it!=shifted.end())return it->second;
        auto [v,lo,hi]=bdd.nodes[f];
        return shifted[k]=bdd.node(v+d,shift(lo,d),shift(hi,d));
    }
    void build(){
        for(int h=1;h<=HORIZON;h++)for(int l=1;l<=h+1;l++)for(int r=1;r<=h+1;r++)
          for(int b=1;b<=h+1;b++)for(int t=1;t<=h+1;t++){
            int g=0;
            for(int i=-1;i<=1;i++)for(int j=-1;j<=1;j++)if(i||j){
                int v=(i+HORIZON)*SIDE+(j+HORIZON);
                int z=bdd.node(v,0,1);
                if(l+i && r-i && b+j && t-j){
                    int f=form[h-1][min(h,l+i)][min(h,r-i)][min(h,b+j)][min(h,t-j)];
                    z=bdd.combine(z,shift(f,i*SIDE+j),true);
                }
                g=bdd.combine(g,z,false);
            }
            form[h][l][r][b][t]=bdd.afterAnyDeletion(g);
          }
        bdd.unique.clear(); bdd.cache[0].clear();bdd.cache[1].clear();bdd.deletionCache.clear();shifted.clear();
    }
} local;

struct Box {
    int l,r,b,t; // boundary coordinates, interior is strictly between them
    bool contains(int x,int y) const {return l<x && x<r && b<y && y<t;}
    bool operator==(const Box&) const = default;
};
struct Cage {
    Box box;
    int cx,cy,sx,sy;
    Mask white;
    int variable(int x,int y) const {
        if (y==cy) {
            int d=(x-cx)*sx;
            if (d>=0 && d<=N) return N-d;
        }
        if (x==cx) {
            int d=(y-cy)*sy;
            if (d>=0 && d<=N) return N+d;
        }
        return -1;
    }
    int formula(int x,int y, bool before) const {
        int a=(x-cx)*sx,b=(y-cy)*sy;
        return before ? game.lose[a][b] : game.threat[a][b];
    }
};

Line rows[STR]{}, cols[STR]{}; // set bits are white cells
int x,y;
struct Board {
    Line row[STR], col[STR];
    int prefix[STR][STR]{};
    Board(Box area, bool assumeClosed) {
        copy_n(rows,STR,row); copy_n(cols,STR,col);
        if (assumeClosed) {
            row[area.b]=row[area.t]=col[area.l]=col[area.r]=0;
            for (int i=area.l+1;i<area.r;i++) col[i]&=~((Line(1)<<area.b)|(Line(1)<<area.t));
            for (int j=area.b+1;j<area.t;j++) row[j]&=~((Line(1)<<area.l)|(Line(1)<<area.r));
        }
        for(int i=1;i<STR;i++)for(int j=1;j<STR;j++)
            prefix[i][j]=prefix[i-1][j]+prefix[i][j-1]-prefix[i-1][j-1]+int((row[j]>>i)&1);
    }
    int interior(Box z) const {
        return prefix[z.r-1][z.t-1]-prefix[z.l][z.t-1]-prefix[z.r-1][z.b]+prefix[z.l][z.b];
    }
};
Mask wallMask(Line horiz,Line vert,int cx,int cy,int sx,int sy) {
    Mask m=0;
    while(horiz) {
        int i=countr_zero(horiz); horiz &= horiz-1;
        m |= bit(N-(i-cx)*sx);
    }
    while(vert) {
        int j=countr_zero(vert); vert &= vert-1;
        m |= bit(N+(j-cy)*sy);
    }
    return m;
}

// Two adjacent sides can have holes. Both backing sides (including their
// shared corner) are solid. The free ends belong to the defended sides.
Cage bestCage(Box area, bool assumeClosed, int buffer=3) {
    Board board(area,assumeClosed);
    Cage best{};
    long long bestCost=LLONG_MAX;
    for(int l=area.l;l<x;l++)for(int r=x+1;r<=area.r;r++) {
        Line fullX=interval(l,r), midX=interval(l+1,r-1);
        for(int b=area.b;b<y;b++)for(int t=y+1;t<=area.t;t++) {
            Box z{l,r,b,t};
            Line midY=interval(b+1,t-1);
            bool left=!(board.col[l]&midY),right=!(board.col[r]&midY);
            bool down=!(board.row[b]&midX),up=!(board.row[t]&midX);
            if (!(left||right) || !(down||up)) continue;
            int w=r-l-1,h=t-b-1;
            Line fullY=interval(b,t);
            int holes=popcount(board.row[b]&fullX)+popcount(board.row[t]&fullX)
                     +popcount(board.col[l]&midY)+popcount(board.col[r]&midY);
            int count=board.interior(z);
            long long cost=24LL*count + 50*(w+h) + 38*holes + 40*min(w,h);
            if(cost>=bestCost) continue;
            for(int sx : {-1,1}) {
                if(!(sx==-1?left:right))continue;
                int cx=sx==-1?r:l,backX=sx==-1?l:r;
                for(int sy : {-1,1}) {
                    if(!(sy==-1?down:up))continue;
                    int cy=sy==-1?t:b,backY=sy==-1?b:t;
                    if((board.row[backY]>>backX)&1)continue;
                    Mask m=wallMask(board.row[cy]&fullX, board.col[cx]&fullY,cx,cy,sx,sy);
                    Cage c{z,cx,cy,sx,sy,m};
                    if(!game.eval(c.formula(x,y,true),m)){
                        long long value=cost;
                        if(!assumeClosed){
                            if(game.eval(c.formula(x,y,false),m))value+=buffer==0?0:9500;
                            else{
                                int danger=0;
                                for(int i=x-buffer;i<=x+buffer;i++)for(int j=y-buffer;j<=y+buffer;j++)
                                    if((i!=x||j!=y)&&z.contains(i,j)&&((rows[j]>>i)&1))
                                        if(game.eval(c.formula(i,j,false),m)) danger=max(danger,buffer+1-max(abs(i-x),abs(j-y)));
                                value+=danger*2200;
                            }
                        }
                        if(value<bestCost){bestCost=value;best=c;}
                    }
                }
            }
        }
    }
    return best;
}

bool white(int i,int j){return (rows[j]>>i)&1;}
struct Move {int x,y; double value;};

struct PatchGame {
    // Four interior cells in a 4 by 4 patch. Reaching the fringe ends
    // this local attempt. Values are discounted capture probabilities.
    using Table=vector<array<float,4>>;
    array<Table,11> values;
    void build(){
        int adj[16]{},near[4],index[16];
        fill_n(index,16,-1);
        for(int i=0;i<4;i++)index[(1+i/2)*4+1+i%2]=i;
        for(int p=0;p<16;p++)for(int q=0;q<16;q++)
            if(p!=q && max(abs(p/4-q/4),abs(p%4-q%4))<=1)adj[p]|=1<<q;
        for(int i=0;i<4;i++)near[i]=adj[(1+i/2)*4+1+i%2];
        Table post(65536);
        for(int k=0;k<11;k++){
            auto& v=values[k];v.resize(65536);
            double weights[9];
            for(int j=0;j<9;j++)weights[j]=exp((k==9?1.7:-1.7)*j);
            double dir[4][16]{};
            for(int i=0;i<4;i++)for(int q=0;q<16;q++){
                int u=q/4-1-i/2,w=q%4-1-i%2;
                int d=k-1+(k>=5);
                dir[i][q]=k==0?1.:exp(2.8*(u*(d/3-1)+w*(d%3-1)));
            }
            for(unsigned mask=0;mask<65536;mask++){
                for(int i=0;i<4;i++){
                    float best=0;
                    for(unsigned m=mask;m;m&=m-1)best=max(best,post[mask^(m&-m)][i]);
                    v[mask][i]=mask?best:1;
                }
                for(int i=0;i<4;i++){
                    double sum=0,value=0;
                    for(unsigned m=mask&near[i];m;m&=m-1){
                        int q=countr_zero(m);
                        double w=k<9?dir[i][q]:weights[8-popcount(adj[q]&~mask)];
                        sum+=w;
                        if(index[q]>=0)value+=w*v[mask][index[q]];
                    }
                    post[mask][i]=sum?0.94*value/sum:1.;
                }
            }
        }
    }
} patches;
struct Footprint {
    int centerX,centerY;
    array<int,4> masks{};
    void init(int a,int b){
        centerX=a;centerY=b;
        for(int i=0;i<4;i++){
            int x0=a-1-i/2,y0=b-1-i%2;
            for(int j=0;j<16;j++){
                int u=x0+j/4,v=y0+j%4;
                if(u>0&&v>0&&u<=N&&v<=N&&white(u,v))masks[i]|=1<<j;
            }
        }
    }
    float utility(const int* paints,int n,int model)const {
        float best=0;
        for(int i=0;i<4;i++){
            int mask=masks[i],x0=centerX-1-i/2,y0=centerY-1-i%2;
            for(int j=0;j<n;j++){
                int a=xpos(paints[j])-x0,b=ypos(paints[j])-y0;
                if(unsigned(a)<4 && unsigned(b)<4)mask&=~(1<<(a*4+b));
            }
            best=max(best,patches.values[model][mask][i]);
        }
        return best;
    }
};

struct Access {
    uint64_t moves=0;
    bool any=false;
};
Access accessFor(int f,uint64_t free) {
    const auto &nodes=local.bdd.nodes;
    if(!local.bdd.eval(f,free)) return {0,true};
    uint64_t pivots=0;
    while(f>1){
        auto n=nodes[f];
        uint64_t bit=1ull<<n.v;
        if(free&bit) {
            if(!local.bdd.eval(n.lo,free))pivots|=bit;
            f=n.hi;
        } else f=n.lo;
    }
    return {pivots,false};
}
constexpr int AREA=STR*STR;
struct LocalValues {
    array<float,AREA> cost;
    int q;
    int mobility;
    Footprint patch;
};

LocalValues forecast(int qx,int qy,const Board& board) {
    LocalValues out;
    out.cost.fill(1e5);
    out.patch.init(qx,qy);
    out.q=cell(qx,qy);out.mobility=0;
    uint64_t free=0;
    for(int i=-HORIZON;i<=HORIZON;i++)for(int j=-HORIZON;j<=HORIZON;j++){
        int a=qx+i,b=qy+j;
        if(a>0&&a<=N&&b>0&&b<=N&&white(a,b)){
            free|=1ull<<((i+HORIZON)*SIDE+j+HORIZON);
            if((i||j)&&abs(i)<=1&&abs(j)<=1)out.mobility++;
        }
    }
    Access valid[5][5][5][5];
    for(int a=1;a<=4;a++)for(int b=1;b<=4;b++)for(int c=1;c<=4;c++)for(int d=1;d<=4;d++){
        valid[a][b][c][d]=accessFor(local.form[3][a][b][c][d],free);
    }
    float base=1e5;
    constexpr int reach=5;
    for(int l=max(0,qx-reach);l<qx;l++)for(int r=qx+1;r<=min(N,qx+reach);r++){
        Line fullX=interval(l,r);
        for(int b=max(0,qy-reach);b<qy;b++)for(int t=qy+1;t<=min(N,qy+reach);t++){
            auto ok=valid[min(qx-l,4)][min(r-qx,4)][min(qy-b,4)][min(t-qy,4)];
            if(!ok.any&&!ok.moves)continue;
            Line hb=rows[b]&fullX,ht=rows[t]&fullX;
            Line midY=interval(b+1,t-1),vl=cols[l]&midY,vr=cols[r]&midY;
            int holes=popcount(hb)+popcount(ht)+popcount(vl)+popcount(vr);
            int count=board.interior({l,r,b,t});
            int w=r-l-1,h=t-b-1;
            float cost=holes+0.42f*count+0.65f*min(w,h);
            if(cost-1.0f>=base)continue;
            if(ok.any){
                base=min(base,cost);
                auto rowUpdate=[&](Line line,int j){
                    while(line){int a=countr_zero(line);line&=line-1;
                        auto &s=out.cost[cell(a,j)];s=min(s,cost-1);}
                };
                auto colUpdate=[&](Line line,int i){
                    while(line){int a=countr_zero(line);line&=line-1;
                        auto &s=out.cost[cell(i,a)];s=min(s,cost-1);}
                };
                rowUpdate(hb,b);rowUpdate(ht,t);colUpdate(vl,l);colUpdate(vr,r);
                if(cost-0.42f<base){
                    for(int i=l+1;i<r;i++)for(int j=b+1;j<t;j++){
                        auto &s=out.cost[cell(i,j)];s=min(s,cost-0.42f);
                    }
                }
            }else{
                for(uint64_t mask=ok.moves;mask;mask&=mask-1){
                    int v=countr_zero(mask),i=qx+v/SIDE-HORIZON,j=qy+v%SIDE-HORIZON;
                    if(i<l||i>r||j<b||j>t||i<1||j<1||i>N||j>N)continue;
                    float delta=(i==l||i==r||j==b||j==t)?1.0f:0.42f;
                    auto &s=out.cost[cell(i,j)];s=min(s,cost-delta);
                }
            }
        }
    }
    for(auto &s:out.cost)s=min(s,base);
    return out;
}

int firstBit(Mask m) {
    uint64_t low=uint64_t(m);
    return low?countr_zero(low):64+countr_zero(uint64_t(m>>64));
}
int lastBit(Mask m) {
    uint64_t high=uint64_t(m>>64);
    return high?127-countl_zero(high):63-countl_zero(uint64_t(m));
}

// A survival proof uses just a subset of the white cells. Any first mark
// outside this subset leaves the proof intact, so it need not be searched.
struct Tactics {
    static constexpr int R=5,S=R*2+1,center=R*S+R;
    Mask adj[S*S]{}, whiteMask=0, boundary=0;
    int mark=-1;
    Tactics(Box area) {
        for(int i=0;i<S;i++)for(int j=0;j<S;j++){
            int a=x+i-R,b=y+j-R,p=i*S+j;
            if(a<area.l||a>area.r||b<area.b||b>area.t||!white(a,b))continue;
            whiteMask|=bit(p);
            if(!area.contains(a,b))boundary|=bit(p);
        }
        for(int i=0;i<S;i++)for(int j=0;j<S;j++)
            for(int a=max(0,i-1);a<=min(S-1,i+1);a++)for(int b=max(0,j-1);b<=min(S-1,j+1);b++)
                if(i!=a || j!=b)adj[i*S+j]|=bit(a*S+b);
    }
    // Nonzero result proves that the robot survives h more marks. For a
    // losing state mark receives a winning deletion (in local coordinates).
    Mask survive(int p,Mask free,int h) {
        Mask moves=adj[p]&free;
        if(!moves){mark=-1;return 0;}
        Mask exits=moves&boundary;
        Mask first=moves&-moves;
        if(moves==first){mark=firstBit(moves);return 0;}
        if(h==1)return first|bit(lastBit(moves));
        if(exits && (exits&(exits-1)))return (exits&-exits)|bit(lastBit(exits));
        Mask candidates=free, proof=0, cut=0;
        for(;;){
            Mask available=moves&~cut, witness=available&exits;
            if(witness)witness &= -witness;
            else{
                // Try opposite directions first when forming independent proofs.
                bool high=proof!=0;
                while(available){
                    int i=high?lastBit(available):firstBit(available);
                    Mask child=survive(i,free^cut,h-1);
                    if(child){witness=child|bit(i);break;}
                    available^=bit(i);
                }
            }
            if(!witness){mark=firstBit(cut?cut:first);return 0;}
            proof|=witness;
            candidates&=witness;
            if(!candidates)return proof;
            cut=candidates&-candidates;
        }
    }
    int global(int i)const {return cell(x+i/S-R,y+i%S-R);}
    int localIndex(int a,int b)const {
        if(abs(a-x)>R||abs(b-y)>R)return -1;
        return (a-x+R)*S+b-y+R;
    }
};

struct Predictor {
    static constexpr int K=22;
    int turns=0,oldX=0,oldY=0,dx=0,dy=0;
    array<double,K> logWeight{},weight{};
    double previous[K][9]{},base[9][3]{};
    int degree[9]{},visits[STR][STR]{};
    vector<int> paint, path;
    static double influence(int d,int e) {
        static const auto table=[]{
            array<array<double,3>,STR+1> t{};
            for(int i=0;i<=STR;i++){
                t[i][0]=1.0/(i+0.1);t[i][1]=t[i][0]*t[i][0];t[i][2]=t[i][1]*t[i][0];
            }
            return t;
        }();
        return table[d][e];
    }
    void begin() {
        if(turns){
            dx=x-oldX;dy=y-oldY;
            int k=(dx+1)*3+dy+1;
            for(int i=0;i<K;i++)
                logWeight[i]=0.90*logWeight[i]+log(max(0.0001,previous[i][k]));
        }
        double hi=*max_element(logWeight.begin(),logWeight.end()),sum=0;
        for(int i=0;i<K;i++){weight[i]=exp(logWeight[i]-hi);sum+=weight[i];}
        for(double &w:weight)w/=sum;
        visits[x][y]++;path.push_back(cell(x,y));
        for(int k=0;k<9;k++){
            int a=x+k/3-1,b=y+k%3-1;
            if((a==x&&b==y)||a<=0||b<=0||!white(a,b))continue;
            degree[k]=0;
            for(int i=max(1,a-1);i<=min(N,a+1);i++)for(int j=max(1,b-1);j<=min(N,b+1);j++)
                if(i!=a||j!=b)degree[k]+=white(i,j);
            fill_n(base[k],3,0.);
            for(int p:paint){
                int d=max(abs(a-xpos(p)),abs(b-ypos(p)));
                for(int e=0;e<3;e++)base[k][e]+=influence(d,e);
            }
        }
    }
    array<double,9> predict(int mx,int my,bool save=false) {
        array<double,9> prob{};
        double raw[K][9];
        unsigned mask=0;
        for(int k=0;k<9;k++){
            int u=k/3-1,v=k%3-1,a=x+u,b=y+v;
            if((!u&&!v)||a<=0||b<=0||!white(a,b)||(a==mx&&b==my))continue;
            mask|=1u<<k;
            int d=max(abs(a-mx),abs(b-my));
            raw[0][k]=0;
            for(int j=0;j<9;j++)if(j!=4)
                raw[1+j-(j>4)][k]=3*(u*(j/3-1)+v*(j%3-1));
            raw[9][k]=2.4*(u*dx+v*dy);
            raw[10][k]=2.6*(degree[k]-(d==1));
            raw[11][k]=-raw[10][k];
            for(int e=0;e<3;e++){
                double rep=base[k][e]+influence(d,e);
                double dir=0.5/a+0.5/b;
                raw[12+e][k]=-5*rep-3*dir;
                raw[15+e][k]=-65*rep-45*dir;
            }
            raw[18][k]=min(visits[a][b],4)*2.0;
            raw[19][k]=-min(visits[a][b],4)*2.0;
            raw[20][k]=-abs(a-b)*2.0+u+v;
            raw[21][k]=2.0*(u==-dx && v==-dy);
        }
        for(int i=0;i<K;i++){
            if(!save && weight[i]<0.001)continue;
            double high=-1e99,sum=0;
            for(unsigned m=mask;m;m&=m-1) high=max(high,raw[i][countr_zero(m)]);
            for(unsigned m=mask;m;m&=m-1) {
                int k=countr_zero(m);
                raw[i][k]=exp(raw[i][k]-high);sum+=raw[i][k];
            }
            for(unsigned m=mask;m;m&=m-1){
                int k=countr_zero(m);
                double z=raw[i][k]/sum;
                if(save)previous[i][k]=z;
                prob[k]+=weight[i]*z;
            }
        }
        if(save){paint.push_back(cell(mx,my));turns++;oldX=x;oldY=y;}
        return prob;
    }
} predictor;

struct Future {
    struct Reply {
        int p,x,y,df;
        double prior;
    };
    vector<Reply> replies;
    vector<int> possible;
    int qx,qy;
    static int distance(int p,int q){return max(abs(xpos(p)-xpos(q)),abs(ypos(p)-ypos(q)));}
    static int model() {
        int i=max_element(predictor.weight.begin(),predictor.weight.end())-predictor.weight.begin();
        return predictor.weight[i]>0.15?i:0;
    }
    static int patchModel(){
        int k=model();
        if(k<=8)return k;
        if(k==9 && (predictor.dx||predictor.dy)){
            int d=(predictor.dx+1)*3+predictor.dy+1;
            return d+1-(d>=4);
        }
        if(k==11||k==18||k==21)return 10;
        if(k>=10&&k<=17)return 9;
        return 0;
    }
    Future()=default;
    explicit Future(int q,const array<int,AREA>& ids,const vector<LocalValues>& f) {
        qx=xpos(q);qy=ypos(q);
        int modelId=model();
        double high=-1e99;
        for(int z=max(1,qx-1);z<=min(N,qx+1);z++)for(int w=max(1,qy-1);w<=min(N,qy+1);w++){
            int u=z-qx,v=w-qy,p=cell(z,w);
            if((!u&&!v)||!white(z,w))continue;
            double a=0;int id=ids[p];
            if(modelId>=1 && modelId<=8){
                int k=modelId-1+(modelId>=5);
                a=3*(u*(k/3-1)+v*(k%3-1));
            }else if(modelId==9)a=2.4*(u*(qx-x)+v*(qy-y));
            else if(modelId==10||modelId==11)a=(modelId==10?2.6:-2.6)*f[id].mobility;
            else if(modelId>=12&&modelId<=17){
                int e=(modelId-12)%3;
                for(int b:predictor.paint)a-=Predictor::influence(distance(b,p),e);
                a=(modelId<15?5:65)*a-(modelId<15?3:45)*(0.5/z+0.5/w);
            }else if(modelId==18||modelId==19)a=(modelId==18?2:-2)*min(4,predictor.visits[z][w]);
            else if(modelId==20)a=-2*abs(z-w)+u+v;
            else if(modelId==21)a=2*(z==x&&w==y);
            high=max(high,a);
            replies.push_back({p,z,w,id,a});
        }
        for(auto& t:replies)t.prior=exp(max(-400.,t.prior-high));
        for(int i=max(1,qx-2);i<=min(N,qx+2);i++)for(int j=max(1,qy-2);j<=min(N,qy+2);j++)
            if(white(i,j))possible.push_back(cell(i,j));
    }
    double value(int p,int earlier,const vector<LocalValues>& forecasts,const Cage& root,
                 const vector<Future>* deeper=nullptr) const {
        int modelId=model(),patchId=patchModel();
        auto maskAfter=[&](Mask m,int pos){
            if(pos<0)return m;
            int v=root.variable(xpos(pos),ypos(pos));
            return v<0?m:m&~bit(v);
        };
        Mask m=maskAfter(maskAfter(root.white,p),earlier);
        if(!root.box.contains(qx,qy))return 100;
        int g=root.formula(qx,qy,false);
        bool forced=game.eval(g,m);
        static const auto factors=[]{
            array<array<double,STR>,22> table{};
            for(int id=0;id<22;id++)for(int d=0;d<STR;d++){
                double a=0;
                if((id==10||id==11)&&d==1)a=(id==10?-2.6:2.6);
                if(id>=12&&id<=17)a=-(id<15?5:65)*Predictor::influence(d,(id-12)%3);
                table[id][d]=exp(a);
            }
            return table;
        }();
        double best=1e99;
        int bestS=-1;
        auto evaluate=[&](int s,bool recurse){
            if(s==p||s==earlier)return;
            if(forced){
                int v=root.variable(xpos(s),ypos(s));
                if(v<0||game.eval(g,m&~bit(v)))return;
            }
            double worst=0,mean=0,weight=0,weighted=0;
            int num=0;
            for(const auto& t:replies)if(t.p!=p&&t.p!=s&&t.p!=earlier){
                int d1=distance(t.p,p),d2=distance(t.p,s);
                const LocalValues& f=forecasts[t.df];
                int mob=f.mobility-(d1==1)-(d2==1);
                double room=f.cost[p]+f.cost[s]-f.cost[0];
                double pr=t.prior*factors[modelId][d1]*factors[modelId][d2];
                if(earlier>=0){
                    int d3=distance(t.p,earlier);
                    mob-=d3==1;
                    pr*=factors[modelId][d3];
                    room+=f.cost[earlier]-f.cost[0];
                }
                int moves[]={p,s,earlier};
                double z=mob<=1?2.0:0.64*room+2.1*mob-16*f.patch.utility(moves,earlier<0?2:3,patchId);
                if(recurse && mob>1)
                    z=0.18*z+0.82*((*deeper)[t.df].value(s,p,forecasts,root)+1.5);
                worst=max(worst,z);mean+=z;weighted+=z*pr;weight+=pr;num++;
            }
            double cost=num?0.19*worst+0.20*mean/num+0.61*weighted/max(1e-300,weight):0;
            if(cost<best){best=cost;bestS=s;}
        };
        for(int s:possible)evaluate(s,false);
        if(forced && bestS<0){
            for(int i=root.box.l;i<=root.box.r;i++)for(int j=root.box.b;j<=root.box.t;j++)
                if(i&&j&&white(i,j)&&(i==root.cx||j==root.cy))evaluate(cell(i,j),false);
        }
        if(deeper && bestS>=0){
            int s=bestS;best=1e99;evaluate(s,true);
        }
        return best==1e99?90:best;
    }
};

// When the search budget runs low, finish the certified walls and bisect
// the resulting room. Each step still paints a previously white cell.
int finishCage(Cage& c,Box& room) {
    c.white=wallMask(rows[c.cy]&interval(room.l,room.r),
                     cols[c.cx]&interval(room.b,room.t),c.cx,c.cy,c.sx,c.sy);
    if(c.white){
        int g=c.formula(x,y,false),best=-1,dist=INT_MAX;
        for(int i=room.l;i<=room.r;i++)for(int j=room.b;j<=room.t;j++){
            int v=c.variable(i,j);
            if(v<0 || !(c.white&bit(v)) || game.eval(g,c.white&~bit(v)))continue;
            int d=max(abs(i-x),abs(j-y));
            if(d<dist){dist=d;best=cell(i,j);}
        }
        return best;
    }
    for(;;){
        bool vertical=room.r-room.l>=room.t-room.b;
        int cut=vertical?(room.l+room.r)/2:(room.b+room.t)/2;
        int lo=vertical?room.b:room.l,hi=vertical?room.t:room.r;
        int best=-1,dist=INT_MAX;
        for(int pos=lo+1;pos<hi;pos++){
            int i=vertical?cut:pos,j=vertical?pos:cut;
            int d=max(abs(i-x),abs(j-y));
            if(white(i,j)&&d<dist){dist=d;best=cell(i,j);}
        }
        if(best>=0)return best;
        if(vertical){if(x<cut)room.r=cut;else room.l=cut;}
        else{if(y<cut)room.t=cut;else room.b=cut;}
        c={room,room.r,room.t,-1,-1,0};
    }
}

int choose(Box &area) {
    static clock_t start;
    static Cage certificate;
    if(predictor.turns==0){
        start=clock();
        certificate={area,area.r,area.t,-1,-1,0};
    }
    predictor.begin();
    Tactics tactics(area);
    for(int h=1;h<=5;h++)if(!tactics.survive(Tactics::center,tactics.whiteMask,h)){
        int which=tactics.mark;
        if(which<0)which=firstBit(tactics.whiteMask);
        int p=tactics.global(which);
        predictor.predict(xpos(p),ypos(p),true);
        return p;
    }
    if(clock()-start>5*CLOCKS_PER_SEC){
        int p=finishCage(certificate,area);
        predictor.predict(xpos(p),ypos(p),true);
        return p;
    }
    int direction=Future::model();
    int cycle=0,here=cell(x,y),other=here;
    const auto& path=predictor.path;
    if(path.size()>=2){
        other=path[path.size()-2];
        for(int i=int(path.size())-1;i>=2 && path[i]==path[i-2];i--)cycle++;
    }
    bool movingOut=direction==8 || direction==20 || (direction==9 && predictor.dx>0 && predictor.dy>0);
    int buffer=movingOut?0:3;
    if(cycle>=2 && predictor.turns)
        area={max(0,area.l-2),min(N,area.r+2),max(0,area.b-2),min(N,area.t+2)};
    Cage root=bestCage(area,false,buffer);
    area=root.box;certificate=root;
    vector<Cage> cages{root};
    for(;;){
        Cage c=bestCage(cages.back().box,true);
        if(c.box==cages.back().box)break;
        cages.push_back(c);
    }

    vector<Move> choices;
    for(int i=max(1,area.l);i<=area.r;i++)for(int j=max(1,area.b);j<=area.t;j++)
        if(white(i,j))choices.push_back({i,j,0});

    // Real containment has highest priority. Nested cuts can be attempted
    // during turns on which their enclosing walls need no incompatible mark.
    for(const Cage& c:cages) {
        int g=c.formula(x,y,false);
        if(!game.eval(g,c.white))continue;
        auto defends=[&](const Move& m) {
            int v=c.variable(m.x,m.y);
            return v>=0 && (c.white&bit(v)) && !game.eval(g,c.white^bit(v));
        };
        if(&c==&cages.front())erase_if(choices,[&](const Move& m){return !defends(m);});
        else for(auto &m:choices)if(defends(m))m.value+=1.1;
    }

    vector<LocalValues> forecasts;
    Board plain(area,false);
    for(int a=x-1;a<=x+1;a++)for(int b=y-1;b<=y+1;b++)
        if((a!=x||b!=y)&&white(a,b))forecasts.push_back(forecast(a,b,plain));
    int victim=here;
    if(cycle>=2){
        auto degree=[](int p){
            int n=0;
            for(int i=xpos(p)-1;i<=xpos(p)+1;i++)for(int j=ypos(p)-1;j<=ypos(p)+1;j++)
                if(i!=xpos(p)||j!=ypos(p))n+=white(i,j);
            return n;
        };
        int a=degree(here),b=degree(other);
        if(b<a || (b==a && a%2==0))victim=other;
    }
    auto cycleValue=[&](int p){
        if(cycle<2)return 0.;
        bool useful=p!=here && p!=other && Future::distance(p,victim)==1;
        return useful?min(10.,1.8*cycle):0.;
    };
    double best=-1e50;
    int answer=-1;
    for(auto &m:choices){
        int p=cell(m.x,m.y);
        int dist=max(abs(m.x-x),abs(m.y-y));
        double score=0;
        for(size_t k=0;k<cages.size();k++){
            const Cage& c=cages[k];
            int v=c.variable(m.x,m.y);
            if(v<0 || !(c.white&bit(v)))continue;
            int da=abs(x-c.cx),db=abs(y-c.cy);
            int dn=m.y==c.cy?db:da;
            int along=m.y==c.cy?abs(x-m.x):abs(y-m.y);
            double proximity=1.0/(1+0.23*dist+0.17*along);
            score+= (k+1==cages.size()?12.0:8.5)*proximity;
            score+=max(0,5-dn)*0.40/(1+along);
            int neighbors=0;
            for(auto [u,w]:{pair{m.x-1,m.y},pair{m.x+1,m.y},pair{m.x,m.y-1},pair{m.x,m.y+1}})
                if(u>=0&&u<STR&&w>=0&&w<STR&&!white(u,w))neighbors++;
            score+=0.40*neighbors;
        }
        score=0.095*score+m.value;
        int mi=tactics.localIndex(m.x,m.y);
        Mask next=tactics.whiteMask;
        if(mi>=0)next&=~bit(mi);
        auto prob=predictor.predict(m.x,m.y);
        double worst=0,mean=0,weighted=0; int n=0;
        for(const auto& f:forecasts)if(f.q!=p){
            int d=max(abs(m.x-xpos(f.q)),abs(m.y-ypos(f.q)));
            double c=0.64*f.cost[p]+2.1*(f.mobility-(d==1))-16*f.patch.utility(&p,1,Future::patchModel());
            if(f.mobility-(d==1)<=3){
                int qi=tactics.localIndex(xpos(f.q),ypos(f.q));
                if(!(tactics.boundary&bit(qi))){
                    for(int h=1;h<=3;h++)if(!tactics.survive(qi,next,h)){
                        c=2.2*h;break;
                    }
                }
            }
            worst=max(worst,c);mean+=c;n++;
            weighted+=c*prob[(xpos(f.q)-x+1)*3+ypos(f.q)-y+1];
        }
        score -= 0.18*worst+0.15*mean/max(1,n)+0.67*weighted;
        score+=0.50/(1+dist)+cycleValue(p);
        m.value=score;
        if(score>best){best=score;answer=p;}
    }
    if(choices.size()>1){
        int keep=min(24,int(choices.size()));
        partial_sort(choices.begin(),choices.begin()+keep,choices.end(),[](auto a,auto b){return a.value>b.value;});
        array<int,AREA> ids;ids.fill(-1);
        int count=forecasts.size();
        for(int i=0;i<count;i++)ids[forecasts[i].q]=i;
        bool deep=predictor.weight[direction]>0.23 && direction>8;
        int reach=deep?3:2;
        for(int i=max(1,x-reach);i<=min(N,x+reach);i++)for(int j=max(1,y-reach);j<=min(N,y+reach);j++){
            int p=cell(i,j);
            if(white(i,j)&&ids[p]<0){ids[p]=forecasts.size();forecasts.push_back(forecast(i,j,plain));}
        }
        vector<Future> follow;
        for(auto& f:forecasts){
            if(!deep && int(follow.size())==count)break;
            if(Future::distance(f.q,cell(x,y))<=2)follow.emplace_back(f.q,ids,forecasts);
            else follow.emplace_back();
        }
        best=-1e50;
        for(int i=0;i<keep;i++){
            auto m=choices[i];
            int p=cell(m.x,m.y);
            auto prob=predictor.predict(m.x,m.y);
            double worst=0,mean=0,weighted=0;int n=0;
            for(int j=0;j<count;j++)if(forecasts[j].q!=p){
                const auto& a=follow[j];
                double z=a.value(p,-1,forecasts,root,deep?&follow:nullptr);
                worst=max(worst,z);mean+=z;n++;
                weighted+=prob[(a.qx-x+1)*3+a.qy-y+1]*z;
            }
            double s=0.36*m.value+cycleValue(p)-(0.18*worst+0.15*mean/max(1,n)+0.67*weighted);
            if(s>best){best=s;answer=p;}
        }
    }
    predictor.predict(xpos(answer),ypos(answer),true);
    return answer;
}

int main(){
    ios::sync_with_stdio(false);cin.tie(nullptr);
    if(!(cin>>x>>y)||x<=0||y<=0)return 0;
    game.build(); local.build();patches.build();
    for(int i=1;i<=N;i++)rows[i]=cols[i]=interval(1,N);
    Box area{0,N,0,N};
    while(true){
        int p=choose(area);
        int i=xpos(p),j=ypos(p);
        cout<<i<<' '<<j<<endl;
        rows[j]&=~(Line(1)<<i);cols[i]&=~(Line(1)<<j);
        if(!(cin>>x>>y)||x<=0||y<=0)return 0;
    }
}
