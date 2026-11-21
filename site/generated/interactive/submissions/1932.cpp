#include <bits/stdc++.h>
using namespace std;

static bool blackCell[1001][1001];
static vector<signed char> memo[10];
int D, px, py;

bool good(int h, int m);

bool ready(int h, int m) {
    if (h == 1) return m == 7;
    int s = (1 << (2*h-1))-1;
    for (int j : {1,0,2})
        if (!good(h-1, (m >> j) & s)) return false;
    return true;
}

bool good(int h, int m) {
    signed char &a = memo[h][m];
    if (a != -1) return a;
    if (h == 1) return a = (__builtin_popcount((unsigned)m) >= 2);
    if (ready(h,m)) return a = 1;
    for (int k=0;k<=h;k++) {
        for (int i : {h+k,h-k}) {
            if (!(m>>i&1) && ready(h,m|(1<<i))) return a=1;
        }
    }
    return a=0;
}

using P=pair<int,int>;
int distanceTo(P p) {
    return max(abs(px-p.first),abs(py-p.second));
}
void play(P p) {
    blackCell[p.first][p.second]=true;
    cout << p.first << ' ' << p.second << endl;
    if (!(cin>>px) || px<=0 || !(cin>>py) || py<=0) exit(0);
}
P immediate() {
    P p={0,0};
    for(int i=max(1,px-1);i<=px+1;i++)
        for(int j=max(1,py-1);j<=py+1;j++)
            if ((i!=px || j!=py) && !blackCell[i][j]) {
                if(p.first) return {0,0};
                p={i,j};
            }
    return p;
}

int xl,xr,yl,yr;

struct Side {
    bool vert;
    int fixed,lo,hi;
    P point(int t) const {return vert?P{fixed,t}:P{t,fixed};}
    int center() const {return vert?py:px;}
    int mask(int h) const {
        int m=0;
        for(int i=0;i<=2*h;i++) {
            int t=center()-h+i;
            if(t<lo || t>hi) m|=1<<i;
            else {
                auto [x,y]=point(t);
                if(blackCell[x][y]) m|=1<<i;
            }
        }
        return m;
    }
};

vector<Side> sides() {
    vector<Side> s;
    if(xl) s.push_back({true,xl,max(1,yl),yr});
    s.push_back({true,xr,max(1,yl),yr});
    if(yl) s.push_back({false,yl,max(1,xl),xr});
    s.push_back({false,yr,max(1,xl),xr});
    return s;
}

// Prepare two outer walls. Paint a cap on one so the remaining
// live portions have disjoint D-neighborhoods.
void hardBox() {
    int best=INT_MAX, bx=0,by=0;
    vector<P> plan;
    for(int x=px+1;x<=px+3*D+2;x++)
        for(int y=py+1;y<=py+3*D+2;y++)
            for(int v=0;v<2;v++) {
                vector<P> cap;
                int near=10000;
                auto check=[&](P p,bool inCap) {
                    if(blackCell[p.first][p.second]) return;
                    if(inCap) cap.push_back(p);
                    else near=min(near,distanceTo(p));
                };
                for(int i=1;i<=x;i++)
                    check({i,y},!v && i>=x-2*D);
                for(int j=1;j<=y;j++) {
                    if(j==y && !v) continue;
                    check({x,j},v && j>=y-2*D);
                }
                // In the vertical-cap case the corner was also visited
                // as a non-cap cell: this merely makes the test stricter.
                int n=cap.size();
                if(near<n+D) continue;
                sort(cap.begin(),cap.end(),[](P a,P b){
                    return distanceTo(a)<distanceTo(b);
                });
                bool ok=true;
                for(int k=0;k<n;k++)
                    if(distanceTo(cap[k])<k+1) ok=false;
                int score=3*n+x+y+max(x,y);
                if(ok && score<best) {
                    best=score; bx=x; by=y; plan=std::move(cap);
                }
            }
    xl=yl=0; xr=bx; yr=by;
    for(P p:plan) {
        P win=immediate();
        if(win.first) play(win);
        play(p);
    }
}

struct Threat {
    Side s;
    int h, m;
    double weight;
};

P chooseMove(bool hard) {
    // Find the robot's 8-connected component inside the working box.
    int W=xr-xl-1,H=yr-yl-1;
    vector<char> seen(W*H);
    vector<P> cells{{px,py}};
    auto id=[&](int x,int y){return (x-xl-1)*H+y-yl-1;};
    seen[id(px,py)]=1;
    vector<int> cx(xr+1),cy(yr+1);
    for(size_t i=0;i<cells.size();i++){
        auto [x,y]=cells[i];
        cx[x]++; cy[y]++;
        for(int a=max(xl+1,x-1);a<=min(xr-1,x+1);a++)
            for(int b=max(yl+1,y-1);b<=min(yr-1,y+1);b++)
                if(!blackCell[a][b] && !seen[id(a,b)]) {
                    seen[id(a,b)]=1; cells.emplace_back(a,b);
                }
    }
    map<P,double> val;
    // A completed cut eliminates the cut itself and at least the
    // smaller of the two portions. Prefer high gain per mark.
    vector<double> vx(xr+1),vy(yr+1);
    auto cuts=[&](const vector<int>& cnt,vector<double>& out,int l,int r){
        int left=0;
        for(int i=l+1;i<r;i++) if(cnt[i]){
            int right=(int)cells.size()-left-cnt[i];
            out[i]=1.+(double)min(left,right)/cnt[i];
            left+=cnt[i];
        }
    };
    cuts(cx,vx,xl,xr); cuts(cy,vy,yl,yr);
    for(P p:cells) {
        auto [x,y]=p;
        val[p]=max(vx[x],vy[y]) + 0.35/(1+distanceTo(p))
              + (distanceTo(p)==1 ? 0.35 : 0.);
    }
    vector<Threat> threats;
    for(Side s:sides()) {
        int dist=10000;
        for(int t=s.lo;t<=s.hi;t++){
            auto p=s.point(t);
            if(!blackCell[p.first][p.second]) dist=min(dist,distanceTo(p));
        }
        if(dist>D) continue;
        int m=s.mask(dist);
        bool needed=!ready(dist,m);
        if(needed) threats.push_back({s,dist,m,1.0+0.04/(double)dist});
        for(int k=-dist;k<=dist;k++) {
            int t=s.center()+k;
            if(t<s.lo || t>s.hi) continue;
            P p=s.point(t);
            if(!blackCell[p.first][p.second])
                val[p]+=0.18/(1+abs(k))+(distanceTo(p)==1?0.25:0.);
        }
    }
    double best=-1e100;
    P result={0,0};
    for(auto [p,base]:val) {
        double score=base;
        for(auto &q:threats) {
            auto &s=q.s;
            int n=q.m;
            int off=(s.vert?p.second:p.first)-s.center()+q.h;
            if((s.vert?p.first:p.second)==s.fixed && off>=0 && off<=2*q.h)
                n|=1<<off;
            if(ready(q.h,n)) score+=100000*q.weight;
            else if(n!=q.m) {
                if(q.h==1) score+=1000;
                else {
                    int bits=(1<<(2*q.h-1))-1;
                    for(int j=0;j<3;j++)
                        if(good(q.h-1,(n>>j)&bits)) score+=15;
                }
            }
        }
        if(score>best) best=score,result=p;
    }
    (void)hard;
    return result;
}

int main(){
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    if(!(cin>>px) || px<=0 || !(cin>>py) || py<=0) return 0;
    for(D=1;D<=9;D++) {
        memo[D].assign(1<<(2*D+1),-1);
        if(good(D,0)) break;
    }
    // First try a tight working box. Its sides can be pushed back
    // on conflicts, but only in a bounded neighborhood of the start.
    int sx=px,sy=py;
    auto low=[](int t){return t<=7?0:t-4;};
    xl=low(px); yl=low(py);
    xr=px+4; yr=py+4;
    bool hard=false;
    while(true) {
        P win=immediate();
        if(win.first) {play(win);continue;}
        if(!hard && (px>sx+12 || py>sy+12)) {
            hardBox(); hard=true;
            continue;
        }
        if(!hard) {
            if(px<=xl) xl=low(px);
            if(py<=yl) yl=low(py);
            if(px>=xr) xr=px+4;
            if(py>=yr) yr=py+4;
        }
        play(chooseMove(hard));
    }
}
