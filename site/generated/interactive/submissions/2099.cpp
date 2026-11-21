#include <bits/stdc++.h>
using namespace std;

struct Rect {
    int lx, rx, ly, ry; // inclusive
    int w() const { return rx-lx+1; }
    int h() const { return ry-ly+1; }
};

class Explorer {
    static constexpr int unknown=-2, emptyCell=-1, unclear=2;
    static constexpr int dx[4]={0,1,0,-1};
    static constexpr int dy[4]={-1,0,1,0};
    int n, side, stride, start, pos, moves=0;
    int delta[4];
    vector<int8_t> board;
    vector<int> marked;
    Rect seen;

    int id(int x,int y) const { return (y+1)*stride+x+1; }
    int x(int p) const { return p%stride-1; }
    int y(int p) const { return p/stride-1; }
    int dist(int a,int b) const { return abs(x(a)-x(b))+abs(y(a)-y(b)); }
    bool inside(int p,const Rect& r) const {
        return x(p)>=r.lx && x(p)<=r.rx && y(p)>=r.ly && y(p)<=r.ry;
    }
    unordered_map<int,int> jump;
    int blanksSeen=0;
    bool homing=false;

    int tip(int p) {
        int q=p;
        while (board[q]>=0) q=jump.at(q);
        while (p!=q) {
            int next=jump.at(p);
            jump[p]=q;
            p=next;
        }
        return q;
    }
    bool hasIncoming(int p) const {
        for (int d=0;d<4;++d)
            if (board[p+delta[d]]==(d^2)) return true;
        return false;
    }
    bool canContinue(int p) {
        for (int d=0;d<4;++d) {
            int q=p+delta[d];
            if (q!=start && (board[q]==unknown || board[q]>=0) &&
                !hasIncoming(q) && tip(q)!=p) return true;
        }
        return false;
    }

    // Propagate facts that follow from simplicity of the single path.
    // In particular, adding the last edge of a directed cycle is impossible.
    void learn(int cell,int value) {
        if (board[cell]!=unknown) return;
        vector<int> work;
        int goal=-1;
        auto record=[&](int p,int v) {
            board[p]=static_cast<int8_t>(v);
            if (v>=0) {
                jump[p]=p+delta[v];
                marked.push_back(p);
                seen.lx=min(seen.lx,x(p)); seen.rx=max(seen.rx,x(p));
                seen.ly=min(seen.ly,y(p)); seen.ry=max(seen.ry,y(p));
            }
            work.push_back(p);
            for (int d=0;d<4;++d)
                if (board[p+delta[d]]!=-3) work.push_back(p+delta[d]);
        };
        record(cell,value);
        while (!work.empty()) {
            int p=work.back(); work.pop_back();
            if (p==start || board[p]==emptyCell) continue;
            if (hasIncoming(p)) {
                if (board[p]==unknown && !canContinue(p)) goal=p;
                continue;
            }
            int end=board[p]>=0?tip(p):p;
            int count=0,k=-1;
            for (int d=0;d<4;++d) {
                int q=p+delta[d];
                if (board[q]==unknown && q!=end && d!=board[p]) { ++count; k=d; }
            }
            if (count==1 && board[p]>=0) record(p+delta[k],k^2);
            if (count==0 && board[p]==unknown) record(p,emptyCell);
        }
        if (goal>=0 && !homing) {
            homing=true;
            go(goal);
        }
    }

    void step(int d) {
        static constexpr char cmd[]="^>v<";
        int q=pos+delta[d];
        if (board[q]==-3) exit(0);
        cout << cmd[d] << '\n' << flush;
        char c;
        if (!(cin>>c) || c=='G') exit(0);
        int v=emptyCell;
        if (c!='.') {
            const char* t=strchr(cmd,c);
            if (!t) exit(0);
            v=int(t-cmd);
        }
        pos=q;
        ++moves;
        if (v==emptyCell && board[q]==unknown) ++blanksSeen;
        learn(q,v);
    }

    void go(int q,bool stopOnDiscovery=false) {
        while (pos!=q) {
            int a=x(q)-x(pos), b=y(q)-y(pos);
            int h=a>0?1:3, v=b>0?2:0;
            int d;
            if (a==0) d=v;
            else if (b==0) d=h;
            else {
                auto score=[&](int dir) {
                    return 4*(board[pos]==dir)+2*(board[pos+delta[dir]]==unknown);
                };
                int sh=score(h), sv=score(v);
                d=sh==sv ? (abs(a)>=abs(b)?h:v) : (sh>sv?h:v);
            }
            bool discover=stopOnDiscovery && board[pos+delta[d]]==unknown;
            step(d);
            if (discover) return;
        }
    }

    // Probe across a hairpin. Parallel passes of a snake, or successive
    // windings of an outward spiral, are often adjacent along this ray.
    bool probe(int d,int gap,int limit) {
        int base=pos, best=pos, blanks=0;
        for (int i=0;i<limit && blanks<gap;++i) {
            int q=pos+delta[d];
            if (board[q]!=unknown) break;
            step(d);
            if (board[pos]==emptyCell) ++blanks;
            else {
                if (board[pos]==(d^2)) break;
                best=pos;
                blanks=0;
            }
        }
        go(best);
        return best!=base;
    }

    void trace() {
        vector<pair<int,int>> runs;
        int lastJump=-100000, inwardAt=-1;
        int inwardDir=0, inwardLength=0;
        int laneDirection=-1, laneNormal=0, laneGap=0, nextSample=0;
        while (board[pos]>=0) {
            int d=board[pos];
            if (d==laneDirection && moves>=nextSample) {
                int base=pos;
                for (int k=0;k<laneGap && board[pos+delta[laneNormal]]==unknown;++k) {
                    step(laneNormal);
                    if (board[pos]>=0) break;
                }
                if (pos!=base && board[pos]==(d^2)) {
                    runs.clear();
                    laneDirection=-1;
                    continue;
                }
                go(base);
                nextSample=moves+max(5,int(sqrt(2.*laneGap*max(seen.w(),seen.h()))));
            }
            if (d!=laneDirection) laneDirection=-1;
            bool turn=!runs.empty() && d!=runs.back().first;
            if (turn) inwardAt=-1;
            // The west (or reflected east) side is the last side of each
            // complete square in a unit spiral. Cross all these sides at once.
            if (turn && runs.size()==4 && runs[0]==make_pair(0,1) &&
                runs[1].second==1 && runs[2]==make_pair(2,2) &&
                runs[3]==make_pair(runs[1].first^2,2) && d==0 &&
                x(pos)==n+dx[runs[3].first] && y(pos)==n+1) {
                if (probe(runs[3].first,1,side)) {
                    lastJump=moves;
                    runs.clear();
                    continue;
                }
            }
            if (turn && runs.size()>=2 && runs[runs.size()-2].first==(d^2)) {
                auto [across,len]=runs.back();
                int prevLen=runs[runs.size()-2].second;
                if ((moves>=7 || (len==1 && prevLen>=3)) && board[pos+delta[across]]==unknown) {
                    int gap=1;
                    if (len<=5 && prevLen>=4*len) gap=len;
                    if (runs.size()>=4) {
                        int oldLen=runs[runs.size()-3].second;
                        if (runs[runs.size()-3].first==(across^2) && len>oldLen)
                            gap=max(gap,min(6,len-oldLen));
                        if (runs[runs.size()-3].first==across && len==oldLen)
                            gap=max(gap,min(10,len));
                    }
                    int before=moves;
                    if (probe(across,gap,side)) {
                        if (moves-before>=6) {
                            lastJump=moves;
                            if ((board[pos]&1)!=(across&1)) {
                                laneDirection=board[pos];
                                laneNormal=across;
                                laneGap=gap;
                                nextSample=moves+max(5,int(sqrt(2.*gap*max(seen.w(),seen.h()))));
                            }
                        }
                        runs.clear();
                        inwardAt=-1;
                        continue;
                    }
                }
                // Consecutive contracting turns indicate an inward spiral.
                // Cross its nested sides from the middle of the next side.
                int k=int(runs.size());
                if (k>=4 && runs[k-3].first==(across^2) &&
                    runs[k-4].first==d && len<=runs[k-3].second &&
                    prevLen<=runs[k-4].second &&
                    (len<runs[k-3].second || prevLen<runs[k-4].second) &&
                    min(prevLen,len)>=6) {
                    inwardAt=max(1,(prevLen-1)/2);
                    inwardDir=across^2;
                    inwardLength=max(1,(len-1)/2);
                } else inwardAt=-1;
            }
            int span=seen.w()+seen.h()-2;
            if (moves>max(100,5*span)) break;
            bool sparse=blanksSeen>=4 && blanksSeen*12>int(marked.size());
            int traceBudget=sparse?max(145,3*span):max(64,2*span+span/2);
            if (moves>traceBudget && moves-lastJump>span &&
                (turn || moves>4*span+64) && inwardAt<0) break;
            int q=pos+delta[d];
            if (board[q]>=0) {
                // A probe can rejoin an already observed piece of the path.
                // Walk directly to its first unseen successor.
                q=tip(q);
                go(q);
                runs.clear();
                inwardAt=-1;
                continue;
            }
            if (runs.empty() || turn) runs.push_back({d,0});
            ++runs.back().second;
            step(d);
            if (inwardAt>=0 && board[pos]==d && runs.back().second==inwardAt) {
                inwardAt=-1;
                if (probe(inwardDir,2,inwardLength)) {
                    lastJump=moves;
                    runs.clear();
                }
            }
        }
    }

    // Signed flow a -> b across an edge, or 'unclear'. Both endpoints
    // need not be visited: a footprint has exactly one predecessor,
    // except at the start, and its successor cannot be its predecessor.
    int flow(int a,int d) const {
        int b=a+delta[d], va=board[a], vb=board[b];
        if (va==d) return 1;
        if (vb==(d^2)) return -1;
        if (va<=-1 && va!=unknown) return 0;
        if (vb<=-1 && vb!=unknown) return 0;
        if (va!=unknown && vb!=unknown) return 0;
        if (va==unknown && vb==unknown) return unclear;
        int p=va!=unknown?a:b, across=va!=unknown?d:(d^2);
        if (p==start) return 0;
        bool possible=false;
        for (int k=0;k<4;++k) if (k!=across && k!=board[p]) {
            int v=board[p+delta[k]];
            if (v==(k^2)) return 0;
            if (v==unknown) possible=true;
        }
        return possible?unclear:(p==a?-1:1);
    }
    int balance(const Rect& r) const {
        int sum=inside(start,r);
        auto add=[&](int p,int d) {
            int f=flow(p,d);
            if (f!=unclear) sum-=f;
        };
        for (int i=r.lx;i<=r.rx;++i) {
            add(id(i,r.ly),0);
            add(id(i,r.ry),2);
        }
        for (int i=r.ly;i<=r.ry;++i) {
            add(id(r.lx,i),3);
            add(id(r.rx,i),1);
        }
        return sum;
    }

    struct Cut {
        int dir, split, from, to;
        double cost;
    };
    int cutCell(int d,int s,int t) const { return d==1?id(s,t):id(t,s); }
    Cut chooseCut(const Rect& r,bool midpointOnly) const {
        Cut best{0,0,0,0,1e100};
        for (int d : {1,2}) {
            int l=d==1?r.lx:r.ly, h=d==1?r.rx:r.ry;
            int lo=d==1?r.ly:r.lx, hi=d==1?r.ry:r.rx;
            int here=d==1?x(pos):y(pos);
            int size=h-l+1;
            if (size<2) continue;
            vector<int> options;
            if (midpointOnly) {
                if (size<max(r.w(),r.h())) continue;
                options.push_back(l+(size-1)/2);
            } else if (size<=16) {
                for (int i=l;i<h;++i) options.push_back(i);
            } else {
                for (double f : {0.22,0.30,0.38,0.45,0.5,0.55,0.62,0.70,0.78})
                    options.push_back(l+int(size*f)-1);
                for (int i : {here-1,here,n-1,n,(d==1?seen.lx:seen.ly)-1,
                              (d==1?seen.rx:seen.ry)}) {
                    if (i>=l && i<h) options.push_back(i);
                }
            }
            sort(options.begin(),options.end());
            options.erase(unique(options.begin(),options.end()),options.end());
            for (int s : options) {
                int first=hi+1,last=lo-1, unresolved=0;
                for (int t=lo;t<=hi;++t) if (flow(cutCell(d,s,t),d)==unclear) {
                    first=min(first,t);
                    last=t;
                    ++unresolved;
                }
                int from=first,to=last;
                double work=0;
                if (unresolved) {
                    auto reach=[&](int t) {
                        int p=cutCell(d,s,t);
                        return min(dist(pos,p)+(board[p]!=unknown),
                                   dist(pos,p+delta[d])+(board[p+delta[d]]!=unknown));
                    };
                    int da=reach(first),db=reach(last);
                    if (db<da) swap(from,to);
                    work=min(da,db)+last-first+0.15*unresolved;
                }
                int a=s-l+1,b=h-s;
                // Reward eliminating a substantial fraction on either outcome.
                double gain=(2.0*a*b)/size;
                double cost=(0.3+work)/gain;
                if (cost<best.cost) best={d,s,from,to,cost};
            }
        }
        return best;
    }
    void inspect(const Cut& c) {
        int d=c.dir, inc=c.from<=c.to?1:-1;
        for (int t=c.from;;t+=inc) {
            int a=cutCell(d,c.split,t), b=a+delta[d];
            if (flow(a,d)==unclear) {
                int p=board[a]!=unknown?b:board[b]!=unknown?a:
                      dist(pos,a)<=dist(pos,b)?a:b;
                go(p);
                if (flow(a,d)==unclear) {
                    int advance=d==1?(inc==1?2:0):(inc==1?1:3);
                    int across=p==a?d:(d^2);
                    bool nextResolves=t!=c.to && board[p+delta[advance]]==unknown &&
                                      advance!=board[p];
                    for (int k=0;k<4;++k)
                        if (k!=advance && k!=across && k!=board[p] &&
                            board[p+delta[k]]==unknown) nextResolves=false;
                    if (nextResolves) step(advance);
                    else step(across);
                }
            }
            if (t==c.to) break;
        }
    }

    // Try nearby open ends of observed path segments before paying for a cut.
    void chase(const Rect& r,int budget) {
        int until=moves+budget;
        while (moves<until) {
            int best=-1, len=INT_MAX;
            if (board[pos]>=0) {
                int q=pos+delta[board[pos]];
                if (inside(q,r) && board[q]==unknown) { step(board[pos]); continue; }
            }
            for (auto it=marked.rbegin();it!=marked.rend();++it) {
                int q=*it+delta[board[*it]];
                if (board[q]!=unknown || !inside(q,r)) continue;
                int v=dist(q,pos);
                if (v<len) { best=q; len=v; }
                if (len==1) break;
            }
            if (len>min(3,until-moves)) break;
            go(best);
        }
    }
    void small(const Rect& r) {
        for (;;) {
            int best=-1,score=INT_MAX;
            for (int j=r.ly;j<=r.ry;++j) for (int i=r.lx;i<=r.rx;++i) {
                int p=id(i,j);
                if (board[p]!=unknown) continue;
                bool incoming=false,possible=false;
                for (int d=0;d<4;++d) {
                    int v=board[p+delta[d]];
                    incoming |= v==(d^2);
                    possible |= v==unknown;
                }
                if (!incoming && !possible) continue;
                int s=4*dist(p,pos)-3*incoming-(board[pos]>=0 && pos+delta[board[pos]]==p);
                if (s<score) { best=p; score=s; }
            }
            if (best<0) return;
            go(best);
        }
    }
    Rect guide(const Rect& r,int pad) const {
        auto low=[&](int v,int l) { return v-pad<l+pad/2?l:v-pad; };
        auto high=[&](int v,int h) { return v+pad>h-pad/2?h:v+pad; };
        return {low(seen.lx,r.lx), high(seen.rx,r.rx),
                low(seen.ly,r.ly), high(seen.ry,r.ry)};
    }

    // At a provisional end of a separator, keep scanning outwards while
    // there is evidence that the footprint extends beyond the estimate.
    void extend(const Cut& c,int t,int sign,int limit) {
        int chunk=8;
        while (t!=limit) {
            bool active=false;
            for (int k=0;k<4;++k) {
                int u=t-sign*k;
                if (u<0 || u>=side) break;
                int a=cutCell(c.dir,c.split,u);
                active|=board[a]>=0 || board[a+delta[c.dir]]>=0;
            }
            if (!active) break;
            int nxt=t+sign*min(chunk,abs(t-limit));
            inspect({c.dir,c.split,t+sign,nxt,0});
            t=nxt;
            chunk=min(24,chunk+4);
        }
    }

    // On a 5 by 5 field there are only 29,290 legal paths, including
    // all possible stopping points. Condition this set on the replies.
    struct TinyPath {
        array<int8_t,25> cells;
        int goal, length;
        double weight;
    };
    void tiny() {
        step(0);
        int m=side*side;
        vector<TinyPath> paths;
        int lengths[25]{};
        array<int8_t,25> cells;
        cells.fill(emptyCell);
        cells[m/2]=0;
        auto enumerate=[&](auto&& self,int p,int len,double mass)->void {
            int next[4],degree=0;
            for (int d=0;d<4;++d) {
                int xx=p%side+dx[d],yy=p/side+dy[d];
                if (xx>=0 && xx<side && yy>=0 && yy<side && cells[yy*side+xx]==emptyCell)
                    next[degree++]=d;
            }
            cells[p]=4;
            double stop=degree?0.18:1.;
            paths.push_back({cells,p,len,mass*stop});
            ++lengths[len];
            for (int k=0;k<degree;++k) {
                int d=next[k];
                cells[p]=static_cast<int8_t>(d);
                self(self,p+dy[d]*side+dx[d],len+1,mass*(1-stop)/degree);
            }
            cells[p]=emptyCell;
        };
        enumerate(enumerate,m/2-side,1,1.);
        for (auto& h:paths) {
            double prior=0.35*h.weight+0.45/((m-1)*lengths[h.length])
                         +0.20/static_cast<double>(paths.size());
            h.weight=n==2?prior/sqrt(1.0+0.12*h.length):prior;
        }

        int cell[25],distance[25][25];
        for (int i=0;i<m;++i) cell[i]=id(i%side,i/side);
        for (int i=0;i<m;++i) for (int j=0;j<m;++j)
            distance[i][j]=dist(cell[i],cell[j]);
        using Worlds=vector<const TinyPath*>;
        Worlds pool;
        for (const auto& h:paths) pool.push_back(&h);
        auto plan=[&](auto&& self,const Worlds& worlds,int here,int depth,int& choice)->double {
            double total=0;
            bool sameGoal=true;
            for (const auto* h:worlds) {
                total+=h->weight;
                sameGoal&=h->goal==worlds.front()->goal;
            }
            if (sameGoal) {
                choice=worlds.front()->goal;
                return total*distance[here][choice];
            }
            double best=1e100;
            for (int i=0;i<m;++i) if (board[cell[i]]==unknown) {
                double cost=total*distance[here][i];
                if (cost>=best) continue;
                double mass[6][25]{};
                unsigned mask=0;
                for (const auto* h:worlds) {
                    int answer=h->cells[i]+1;
                    mass[answer][h->goal]+=h->weight;
                    mask|=1u<<answer;
                }
                if (popcount(mask)<2) continue;
                if (depth>1) {
                    Worlds group[5];
                    for (const auto* h:worlds) if (h->goal!=i)
                        group[h->cells[i]+1].push_back(h);
                    int unused;
                    for (const auto& g:group) if (!g.empty()) {
                        cost+=self(self,g,i,depth-1,unused);
                        if (cost>=best) break;
                    }
                } else {
                    for (int k=0;k<5;++k) if (mask&(1u<<k)) {
                        auto& goals=mass[k];
                        for (int j=0;j<m;++j) cost+=goals[j]*distance[i][j];
                        // Estimate later probes from the posterior rank of G.
                        sort(goals,goals+m,greater<double>());
                        for (int j=1;j<m;++j) cost+=goals[j]*j;
                    }
                }
                if (cost<best) { choice=i; best=cost; }
            }
            return best;
        };
        for (;;) {
            pool.erase(remove_if(pool.begin(),pool.end(),[&](const TinyPath* h) {
                for (int i=0;i<m;++i)
                    if (board[cell[i]]!=unknown && board[cell[i]]!=h->cells[i]) return true;
                return false;
            }),pool.end());
            int choice=-1;
            plan(plan,pool,y(pos)*side+x(pos),n==1?7:2,choice);
            if (choice<0) return;
            go(cell[choice],true);
        }
    }

    // With full-length separators every boundary flow is determined, so
    // balance is exactly the indicator that the goal lies in the rectangle.
    void locate(int pad,int moveLimit) {
        Rect r{0,side-1,0,side-1};
        while (moves<moveLimit) {
            Rect g=guide(r,pad);
            if (g.lx>g.rx || g.ly>g.ry) return;
            if (g.w()*g.h()<=30) { small(g); return; }
            chase(g,min(10,2+min(g.w(),g.h())/14));
            Cut c=chooseCut(g,pad==side && moves>12*side);
            int lo=c.dir==1?g.ly:g.lx, hi=c.dir==1?g.ry:g.rx;
            int rl=c.dir==1?r.ly:r.lx, rh=c.dir==1?r.ry:r.rx;
            bool forward=c.from<=c.to;
            if (c.from>=lo && c.from<=hi) {
                inspect({c.dir,c.split,c.from,c.from,0});
                if (forward) extend(c,lo,-1,rl);
                else extend(c,hi,1,rh);
                inspect(c);
            }
            if (forward) extend(c,hi,1,rh);
            else extend(c,lo,-1,rl);
            Rect a=r,b=r;
            if (c.dir==1) a.rx=c.split,b.lx=c.split+1;
            else a.ry=c.split,b.ly=c.split+1;
            int fa=balance(a), fb=balance(b);
            if (max(fa,fb)<=0) return;
            r=fa==fb?(inside(pos,a)?a:b):(fa>fb?a:b);
        }
    }
public:
    explicit Explorer(int radius): n(radius),side(2*n+1),stride(side+2),
        start(id(n,n)),pos(start),board(stride*stride,unknown),seen{n,n,n,n} {
        delta[0]=-stride; delta[1]=1; delta[2]=stride; delta[3]=-1;
        for (int k=0;k<stride;++k) {
            board[k]=board[(stride-1)*stride+k]=-3;
            board[k*stride]=board[k*stride+stride-1]=-3;
        }
        learn(start,0);
    }
    void run() {
        if (n<=2) tiny();
        trace();
        int pad=max(6,max(seen.w(),seen.h())/3);
        int speculationLimit=moves+6*side;
        for (;;) {
            locate(pad,pad==side?INT_MAX:speculationLimit);
            if (pad==side) return;
            pad=moves>=speculationLimit?side:min(side,max(pad*3,max(seen.w(),seen.h())/2));
        }
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    int n;
    if (cin>>n && n>=1 && n<=2000) Explorer(n).run();
}
