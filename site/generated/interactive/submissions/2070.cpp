#include <bits/stdc++.h>
using namespace std;

using Int = long long;
using Mask = uint64_t;

struct Line {
    Int a, b;
    Int operator()(Int s) const { return a * s + b; }
    auto operator<=>(const Line&) const = default;
};

struct Range {
    Line lo, hi;
    auto operator<=>(const Range&) const = default;
};

// For each integer s in [l,r], the two center coordinates independently
// range between affine bounds. The side of one square is 2*s+1.
struct Box {
    Int l, r;
    Range x, y;
};

struct Board {
    Int s, x, y;
    Int edge(bool vertical, int i) const {
        return (vertical ? y : x) - 5*s - 2 + i*(2*s+1);
    }
    auto operator<=>(const Board&) const = default;
};

Int n, startX, startY;
mt19937_64 rng(19260817);

Int randomInt(Int l, Int r) {
    return uniform_int_distribution<Int>(l,r)(rng);
}

// Restrict [l,r] to the integers for which f <= g.
bool restrictLE(Int& l, Int& r, Line f, Line g) {
    bool left = f(l) <= g(l), right = f(r) <= g(r);
    if (left == right) return left;
    Int a = f.a - g.a, b = g.b - f.b;
    if (left) r = b/a;
    else l = ( -b + (-a) - 1 )/(-a);
    return true;
}

struct Strip {
    Int l,r;
    Range v;
    int parity;
};

// Intersect a center-coordinate interval with each of the seven bands
// made by the six tile edges at the probe. The two outer bands have parity 2.
vector<Strip> bands(const Box& box, Range v, Int probe) {
    vector<Strip> out;
    for (int i=-1; i<=5; ++i) {
        bool lower = i >= 0, upper = i < 5;
        Line low{2*i-5,probe+i-2}, high{2*i-3,probe+i-2};
        Int l=box.l, r=box.r;
        if (lower && !restrictLE(l,r,low,v.hi)) continue;
        if (upper && !restrictLE(l,r,v.lo,high)) continue;

        auto addUpper = [&](Int p, Int q, Line lo) {
            int parity = lower && upper ? i%2 : 2;
            if (!upper || (v.hi(p)<=high(p) && v.hi(q)<=high(q))) {
                out.push_back({p,q,{lo,v.hi},parity});
            } else if (high(p)<=v.hi(p) && high(q)<=v.hi(q)) {
                out.push_back({p,q,{lo,high},parity});
            } else {
                Int a=p,b=q;
                restrictLE(a,b,v.hi,high);
                out.push_back({a,b,{lo,v.hi},parity});
                if (a>p) out.push_back({p,a-1,{lo,high},parity});
                else out.push_back({b+1,q,{lo,high},parity});
            }
        };
        if (!lower || (low(l)<=v.lo(l) && low(r)<=v.lo(r))) {
            addUpper(l,r,v.lo);
        } else if (v.lo(l)<=low(l) && v.lo(r)<=low(r)) {
            addUpper(l,r,low);
        } else {
            Int a=l,b=r;
            restrictLE(a,b,low,v.lo);
            addUpper(a,b,v.lo);
            if (a>l) addUpper(l,a-1,low);
            else addUpper(b+1,r,low);
        }
    }
    return out;
}

void mergeBoxes(vector<Box>& boxes) {
    if (boxes.size()<2) return;
    size_t old;
    do {
        old=boxes.size();
        for (bool swapAxes : {false,true}) {
            if (swapAxes) for (auto &b:boxes) swap(b.x,b.y);
            sort(boxes.begin(),boxes.end(),[](const Box& a,const Box& b) {
                return tie(a.l,a.r,a.y,a.x.lo) < tie(b.l,b.r,b.y,b.x.lo);
            });
            size_t k=0;
            for (Box b:boxes) {
                if (k) {
                    Box &a=boxes[k-1];
                    if (a.l==b.l && a.r==b.r && a.y==b.y &&
                        a.x.hi(a.l)+1==b.x.lo(a.l) && a.x.hi(a.r)+1==b.x.lo(a.r)) {
                        a.x.hi=b.x.hi;
                        continue;
                    }
                }
                boxes[k++]=b;
            }
            boxes.resize(k);
            if (swapAxes) for (auto &b:boxes) swap(b.x,b.y);
        }
        sort(boxes.begin(),boxes.end(),[](const Box& a,const Box& b){
            return tie(a.x,a.y,a.l) < tie(b.x,b.y,b.l);
        });
        size_t k=0;
        for (Box b:boxes) {
            if (k && boxes[k-1].x==b.x && boxes[k-1].y==b.y && boxes[k-1].r+1==b.l)
                boxes[k-1].r=b.r;
            else boxes[k++]=b;
        }
        boxes.resize(k);
    } while (boxes.size()<old);
}

vector<Box> observe(const vector<Box>& boxes,Int x,Int y,bool value) {
    vector<Box> next;
    for (const Box& b:boxes) {
        auto xs=bands(b,b.x,x), ys=bands(b,b.y,y);
        size_t begin=next.size();
        bool removed=false;
        for (const Strip &u:xs) {
            if (u.parity==2) {
                if (!value) next.push_back({u.l,u.r,u.v,b.y});
                else removed=true;
                continue;
            }
            for (const Strip &v:ys) {
                Int l=max(u.l,v.l), r=min(u.r,v.r);
                if (l>r) continue;
                if ((u.parity==v.parity)==value) next.push_back({l,r,u.v,v.v});
                else removed=true;
            }
        }
        if (!removed) {
            next.resize(begin);
            next.push_back(b);
        }
    }
    mergeBoxes(next);
    return next;
}

constexpr int sampleSize=384;
bool enumerateSmall(const vector<Box>& boxes,vector<Board>& boards) {
    int total=0;
    for (const Box& b:boxes) {
        if (b.r-b.l+1>sampleSize-total) return false;
        for (Int s=b.l;s<=b.r;++s) {
            Int count=(b.x.hi(s)-b.x.lo(s)+1)*(b.y.hi(s)-b.y.lo(s)+1);
            if (count>sampleSize-total) return false;
            total+=int(count);
        }
    }
    for (const Box& b:boxes)
        for (Int s=b.l;s<=b.r;++s)
            for (Int x=b.x.lo(s);x<=b.x.hi(s);++x)
                for (Int y=b.y.lo(s);y<=b.y.hi(s);++y)
                    boards.push_back({s,x,y});
    return true;
}

struct Component {
    Range v;
    Int l,r;
    double weight;
    bool fixed;
};

vector<Component> components(const Box& b, bool vertical) {
    Range axis=vertical ? b.y : b.x;
    Int start=vertical ? startY : startX;
    vector<Component> result{{axis,b.l,b.r,1,false}};
    auto add=[&](Line line,double weight) {
        Int l=b.l,r=b.r;
        if (restrictLE(l,r,axis.lo,line) && restrictLE(l,r,line,axis.hi))
            result.push_back({{line,line},l,r,weight,true});
    };
    // Give additional weight to starts on a tile edge or in its middle,
    // and to placements touching the meadow boundary.
    for (int tile=-2;tile<=2;++tile)
        for (int offset=-1;offset<=1;++offset)
            add({2*tile+offset,start+tile},0.08);
    add({5,3},0.09);
    add({-5,n-2},0.09);
    add({0,(n+1)/2},0.06);
    if (n%2==0) add({0,n/2+1},0.03);
    return result;
}

double coordinateSpan(bool vertical,double m,double slack) {
    Int start=vertical ? startY : startX;
    return min({m,slack,double(start),double(n-start+1)});
}

// Spread query-selection weight across scales of both tile size and free
// meadow space. These weights never participate in eliminating a board.
double sizeWeight(double m,double slack,bool endpoint) {
    double weight=1/m+5/slack+(endpoint ? 2 : 0);
    double factor=n/max(m,slack);
    return weight*factor*factor;
}

double boardWeight(Board board) {
    double m=2*board.s+1,slack=n-5*m+1;
    double weight=sizeWeight(m,slack,board.s==1 || board.s==(n-5)/10);
    Box point{board.s,board.s,{{0,board.x},{0,board.x}},{{0,board.y},{0,board.y}}};
    for (bool vertical:{false,true}) {
        double axis=0;
        for (auto c:components(point,vertical))
            axis+=c.fixed ? c.weight : 1/coordinateSpan(vertical,m,slack);
        weight*=axis;
    }
    return weight;
}

struct Bucket {
    Box box;
    double cumulative;
};

vector<Board> drawSample(const vector<Box>& boxes, bool structured=true) {
    vector<Bucket> buckets;
    double total=0;
    for (const Box& b:boxes) {
        auto xs=structured ? components(b,false) : vector<Component>{{b.x,b.l,b.r,1,false}};
        auto ys=structured ? components(b,true) : vector<Component>{{b.y,b.l,b.r,1,false}};
        for (auto& x:xs) for (auto& y:ys) {
            Int end=min(x.r,y.r);
            for (Int l=max(x.l,y.l);l<=end;) {
                Int r=min({end,l+l/4,l+(n-10*l-4)/50});
                // Sum the quadratic number of placements over integer s.
                double wx=x.v.hi(l)-x.v.lo(l)+1, wy=y.v.hi(l)-y.v.lo(l)+1;
                double dx=x.v.hi.a-x.v.lo.a, dy=y.v.hi.a-y.v.lo.a;
                double d=r-l;
                double area=wx*wy + d*0.5*(wx*dy+wy*dx) + dx*dy*d*(2*d+1)/6;
                double m=l+r+1, slack=n-5*m+1;
                double spanX=x.fixed ? 1 : coordinateSpan(false,m,slack);
                double spanY=y.fixed ? 1 : coordinateSpan(true,m,slack);
                bool endpoint=l==r && (l==1 || l==(n-5)/10);
                total+=(d+1)*area*sizeWeight(m,slack,endpoint)*x.weight*y.weight/(spanX*spanY);
                if (structured && buckets.size()==120000) return drawSample(boxes,false);
                buckets.push_back({{l,r,x.v,y.v},total});
                l=r+1;
            }
        }
    }
    vector<Board> result;
    size_t idx=0;
    for (int i=0;i<sampleSize;++i) {
        double target=total*(i+generate_canonical<double,48>(rng))/sampleSize;
        while (idx+1<buckets.size() && buckets[idx].cumulative<target) ++idx;
        const Box& b=buckets[idx].box;
        Int l=b.l,r=b.r;
        auto width=[](Range v,Int s){return v.hi(s)-v.lo(s)+1;};
        Int cap=max(width(b.x,l),width(b.x,r))*max(width(b.y,l),width(b.y,r));
        Int s;
        do {
            s=randomInt(l,r);
        } while (randomInt(1,cap)>width(b.x,s)*width(b.y,s));
        result.push_back({s,randomInt(b.x.lo(s),b.x.hi(s)),randomInt(b.y.lo(s),b.y.hi(s))});
    }
    return result;
}

vector<Int> positions(const vector<Board>& boards,bool vertical,bool exact) {
    Int initial=vertical?startY:startX;
    vector<Int> pos{initial,1,n};
    for (int edge=0;edge<6;++edge) {
        vector<Int> v;
        for (auto b:boards) v.push_back(b.edge(vertical,edge));
        sort(v.begin(),v.end());
        int k=exact && boards.size()<=40 ? v.size() : min<size_t>(35,v.size());
        for (int j=0;j<k;++j) {
            Int p=v[k==1?0:size_t(j)*(v.size()-1)/(k-1)];
            if (1<=p && p<=n) pos.push_back(p);
            if (1<=p-1 && p-1<=n) pos.push_back(p-1);
        }
    }
    sort(pos.begin(),pos.end());
    pos.erase(unique(pos.begin(),pos.end()),pos.end());
    // Prefer the known row or column on otherwise identical partitions.
    iter_swap(pos.begin(),lower_bound(pos.begin(),pos.end(),initial));
    return pos;
}

constexpr int words=(sampleSize+63)/64;
struct Slice {
    Int pos;
    array<Mask,words> even{},odd{};
};

vector<Slice> slices(const vector<Board>& boards,bool vertical,bool exact) {
    auto pos=positions(boards,vertical,exact);
    vector<Slice> slices;
    set<pair<array<Mask,words>,array<Mask,words>>> used;
    for (Int p:pos) {
        Slice line{p};
        for (size_t i=0;i<boards.size();++i) {
            const auto& b=boards[i];
            if (p<b.edge(vertical,0) || p>=b.edge(vertical,5)) continue;
            bool odd=p<b.edge(vertical,2)?p>=b.edge(vertical,1):
                     p>=b.edge(vertical,3) && p<b.edge(vertical,4);
            (odd?line.odd:line.even)[i/64] |= Mask(1)<<(i%64);
        }
        if (used.emplace(line.even,line.odd).second) slices.push_back(line);
    }
    return slices;
}

struct Choice {
    int mask;
    Int x,y;
};

// Exact decision tree on a small endgame. Its leaves are distinct centers.
pair<Int,Int> endgame(const vector<Board>& boards,const vector<Choice>& choices,const vector<double>& weights) {
    int full=(1<<boards.size())-1;
    vector<int> same(boards.size());
    for (size_t i=0;i<boards.size();++i) for (size_t j=0;j<boards.size();++j)
        if (boards[i].x==boards[j].x && boards[i].y==boards[j].y) same[i]|=1<<j;
    vector<double> cost(full+1,-1),mass(full+1);
    vector<int> height(full+1),which(full+1);
    for (int state=1;state<=full;++state)
        mass[state]=mass[state&(state-1)]+weights[__builtin_ctz((unsigned)state)];
    auto solve=[&](auto&& self,int state)->double {
        double& best=cost[state];
        if (best>=0) return best;
        if (!(state & ~same[__builtin_ctz((unsigned)state)])) return best=0;
        best=numeric_limits<double>::infinity();
        for (size_t k=0;k<choices.size();++k) {
            int a=state & choices[k].mask, b=state^a;
            if (!a || !b) continue;
            double ca=self(self,a);
            if (mass[state]+ca>best) continue;
            double current=mass[state]+ca+self(self,b);
            int h=1+max(height[a],height[b]);
            if (current<best || (current==best && h<height[state])) {
                best=current; height[state]=h; which[state]=k;
            }
        }
        return best;
    };
    solve(solve,full);
    const auto& c=choices[which[full]];
    return {c.x,c.y};
}

pair<Int,Int> choose(const vector<Board>& boards,bool exact) {
    // Mask intersections score both coordinates of a probe together. On a
    // small endgame every distinct response pattern occurs at tile edges.
    auto xs=slices(boards,false,exact),ys=slices(boards,true,exact);
    int count=boards.size(), blocks=(count+63)/64;
    bool small=exact && count<=12;
    vector<bool> used(small?(1<<count):0);
    vector<Choice> choices;
    vector<double> weights;
    vector<array<double,256>> sums;
    double total=count;
    if (exact) {
        for (Board b:boards) weights.push_back(boardWeight(b));
        double sum=accumulate(weights.begin(),weights.end(),0.0);
        for (double &w:weights) w=0.92*w/sum+0.08/count;
        total=1;
        sums.resize((count+7)/8);
        for (int j=0;j<int(sums.size());++j) {
            sums[j][0]=0;
            for (int mask=1;mask<256;++mask) {
                int i=j*8+__builtin_ctz((unsigned)mask);
                sums[j][mask]=sums[j][mask&(mask-1)]+(i<count ? weights[i] : 0);
            }
        }
    }
    double best=0;
    pair<Int,Int> query{0,0};
    for (const Slice& x:xs) for (const Slice& y:ys) {
        double yes=0;
        for (int j=0;j<blocks;++j) {
            Mask m=(x.even[j]&y.even[j])|(x.odd[j]&y.odd[j]);
            if (!exact) yes+=__builtin_popcountll(m);
            else if (!small) {
                Mask remaining=m;
                for (int k=8*j;remaining;++k,remaining>>=8)
                    yes+=sums[k][remaining&255];
            }
            if (small) {
                int key=min<int>(m,((1<<count)-1)^int(m));
                if (key && !used[key]) {
                    choices.push_back({key,x.pos,y.pos});
                    used[key]=true;
                }
            }
        }
        double score=min(yes,total-yes);
        if (score>best && score>1e-12) {
            best=score;
            query={x.pos,y.pos};
            if (!small && best>=total*0.49999) return query;
        }
    }
    if (small && !choices.empty()) return endgame(boards,choices,weights);
    return query;
}

// Two opposite red corners prove that the actual tiles are at least this
// large. A red run starting at the first corner and ending within m cells
// proves the opposite inequality. Together they also pin down the placement.
pair<Int,Int> certify(const Board& b,const vector<Box>& boxes) {
    Int x=b.edge(false,0), y=b.edge(true,0);
    vector<tuple<Int,Int,bool>> checks{
        {x,y,true}, {b.edge(false,5)-1,b.edge(true,5)-1,true},
        {x-1,y,false}, {b.edge(false,1),y,false}
    };
    for (auto [u,v,value]:checks)
        if (u>=1 && !observe(boxes,u,v,!value).empty())
            return {u,v};
    return {0,0};
}

map<pair<Int,Int>,bool> history;

bool probe(Int x,Int y) {
    if (x<1 || x>n || y<1 || y>n) return false;
    auto it=history.find({x,y});
    if (it!=history.end()) return it->second;
    cout<<"? "<<x<<' '<<y<<endl;
    string reply;
    if (!(cin>>reply) || (reply!="true" && reply!="false")) exit(0);
    return history[{x,y}]=reply=="true";
}

// A bounded geometric search also handles exceptionally fragmented sets.
pair<Int,Int> locateByRuns() {
    auto horizontalRun=[](int direction) {
        Int lo=0,hi=1;
        while (probe(startX+direction*hi,startY)) {
            lo=hi;
            hi*=2;
        }
        // Since lo is within the starting tile, lo < m. The jump from lo
        // to 2*lo cannot skip the entire m-cell gap to the next red tile.
        while (lo+1<hi) {
            Int mid=(lo+hi)/2;
            if (probe(startX+direction*mid,startY)) lo=mid;
            else hi=mid;
        }
        return lo;
    };
    Int left=horizontalRun(-1),right=horizontalRun(1),m=left+right+1;
    Int lo=0,hi=m;
    while (lo+1<hi) {
        Int mid=(lo+hi)/2;
        if (probe(startX,startY+mid)) lo=mid;
        else hi=mid;
    }
    Int tileX=startX-left+m/2,tileY=startY+lo-m/2;
    auto countTiles=[&](int dx,int dy) {
        if (!probe(tileX+2*m*dx,tileY+2*m*dy)) return 0;
        return 1+int(probe(tileX+4*m*dx,tileY+4*m*dy));
    };
    int before=countTiles(-1,0),after=countTiles(1,0);
    int parity=2-before-after;
    int below=countTiles(0,-1);
    return {tileX+(2-2*before-parity)*m,tileY+(2-2*below-parity)*m};
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    if (!(cin>>n>>startX>>startY)) return 0;
    history[{startX,startY}]=true;
    Range meadow{{5,3},{-5,n-2}};
    vector<Box> boxes=observe({{1,(n-5)/10,meadow,meadow}},startX,startY,true);
    while (!boxes.empty()) {
        Board a{boxes[0].l,boxes[0].x.lo(boxes[0].l),boxes[0].y.lo(boxes[0].l)};
        Board other=a;
        for (const Box& b:boxes) {
            for (Int s:{b.l,b.r}) {
                Board lo{s,b.x.lo(s),b.y.lo(s)}, hi{s,b.x.hi(s),b.y.hi(s)};
                if (lo.x!=a.x || lo.y!=a.y) other=lo;
                if (hi.x!=a.x || hi.y!=a.y) other=hi;
            }
            if (other!=a) break;
        }
        if (a==other) {
            cout<<"! "<<a.x<<' '<<a.y<<endl;
            return 0;
        }
        if (boxes.size()>1000 || history.size()>115 || clock()>3*CLOCKS_PER_SEC) {
            auto [x,y]=locateByRuns();
            cout<<"! "<<x<<' '<<y<<endl;
            return 0;
        }
        vector<Board> boards;
        bool exact=enumerateSmall(boxes,boards);
        if (!exact) boards=drawSample(boxes);
        Int x=0,y=0;
        if (!exact) {
            // A concentrated sample can miss very unlikely alternatives.
            // Resolve them with a short geometric certificate.
            Board mode=boards.front();
            int balance=0;
            for (Board b:boards) {
                if (!balance) mode=b;
                balance+=b==mode ? 1 : -1;
            }
            if (count(boards.begin(),boards.end(),mode)>int(boards.size())*0.6)
                tie(x,y)=certify(mode,boxes);
        }
        if (!x) tie(x,y)=choose(boards,exact);
        if (!x) tie(x,y)=choose({a,other},true);
        boxes=observe(boxes,x,y,probe(x,y));
    }
    return 0;
}
