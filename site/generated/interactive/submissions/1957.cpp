#include <bits/stdc++.h>
using namespace std;

static const int MAXC = 1005;

static bool black_[MAXC][MAXC];
static int Rx, Ty;
static const int MARGIN = 14;
static const int BUFFER = 5;
static const int HALFW = 1;
static const int CORNER_EXTRA = 2;
static const int RETRY_K = 20;
static const int CUT_OFF = 3;
static const int CUT_BUF = 4;

static int Lx, Hx, Ly, Hy;
static bool avoidTrack = false;
static bool outerPhase = true;

struct Cut { bool active=false; bool track=false; char typ=0; int line=0; int intended=0; };
static Cut cut;

static vector<pair<int,int>> hist;
static vector<pair<int,int>> cornerCells;
static vector<pair<int,int>> outerList;

static void rebuildOuterList(){
    outerList.clear();
    for(int yy=1; yy<=Ty; ++yy) outerList.push_back({Rx,yy});
    for(int xx=1; xx<=Rx; ++xx) if(xx!=Rx) outerList.push_back({xx,Ty});
}
static void rebuildCornerCells(){
    cornerCells.clear();
    int C = BUFFER + CORNER_EXTRA;
    for(int d=0; d<=C; ++d){ int yy=Ty-d; if(yy>=1) cornerCells.push_back({Rx,yy}); }
    for(int d=1; d<=C; ++d){ int xx=Rx-d; if(xx>=1) cornerCells.push_back({xx,Ty}); }
    sort(cornerCells.begin(),cornerCells.end(),[&](const pair<int,int>&a,const pair<int,int>&b){
        return abs(a.first-Rx)+abs(a.second-Ty) < abs(b.first-Rx)+abs(b.second-Ty);
    });
}
static void setupOuter(int newRx,int newTy){
    Rx=newRx; Ty=newTy;
    rebuildOuterList(); rebuildCornerCells();
    Lx=1; Hx=Rx-1; Ly=1; Hy=Ty-1;
    cut.active=false; outerPhase=true; avoidTrack=false;
}

static void missingCol(int y,int w,int vel,vector<pair<int,int>>& out){
    out.clear();
    for(int d=-w; d<=w; ++d){ int yy=y+d; if(yy<1||yy>Ty) continue; if(!black_[Rx][yy]) out.push_back({Rx,yy}); }
    if(vel>0) sort(out.begin(),out.end(),[](const pair<int,int>&a,const pair<int,int>&b){return a.second>b.second;});
    else if(vel<0) sort(out.begin(),out.end(),[](const pair<int,int>&a,const pair<int,int>&b){return a.second<b.second;});
    else sort(out.begin(),out.end(),[&](const pair<int,int>&a,const pair<int,int>&b){return abs(a.second-y)<abs(b.second-y);});
}
static void missingRow(int x,int w,int vel,vector<pair<int,int>>& out){
    out.clear();
    for(int d=-w; d<=w; ++d){ int xx=x+d; if(xx<1||xx>Rx) continue; if(!black_[xx][Ty]) out.push_back({xx,Ty}); }
    if(vel>0) sort(out.begin(),out.end(),[](const pair<int,int>&a,const pair<int,int>&b){return a.first>b.first;});
    else if(vel<0) sort(out.begin(),out.end(),[](const pair<int,int>&a,const pair<int,int>&b){return a.first<b.first;});
    else sort(out.begin(),out.end(),[&](const pair<int,int>&a,const pair<int,int>&b){return abs(a.first-x)<abs(b.first-x);});
}
static bool outerMove(int x,int y,pair<int,int>& res){
    int vx=0,vy=0;
    if(hist.size()>=2){
        vx=hist.back().first-hist[hist.size()-2].first;
        vy=hist.back().second-hist[hist.size()-2].second;
        vx=max(-1,min(1,vx)); vy=max(-1,min(1,vy));
    }
    int dX=Rx-x,dY=Ty-y;
    vector<pair<int,int>> mx,my;
    if(dX<=BUFFER+HALFW) missingCol(y,HALFW,vy,mx);
    if(dY<=BUFFER+HALFW) missingRow(x,HALFW,vx,my);
    if(!mx.empty()||!my.empty()){
        if(!mx.empty()&&!my.empty()){
            if(mx.size()>my.size()) res=mx[0];
            else if(my.size()>mx.size()) res=my[0];
            else res=(dX<=dY)?mx[0]:my[0];
        } else if(!mx.empty()) res=mx[0];
        else res=my[0];
        return true;
    }
    for(auto&c:cornerCells) if(!black_[c.first][c.second]){ res=c; return true; }
    int best=INT_MAX; bool found=false;
    for(auto&c:outerList) if(!black_[c.first][c.second]){
        int d=abs(c.first-x)+abs(c.second-y);
        if(d<best){best=d;res=c;found=true;}
    }
    return found;
}

// ---- cut phase ----
static void chooseCut(int x,int y){
    int W=Hx-Lx+1,H=Hy-Ly+1;
    cut.active=false; cut.track=false;
    if(W>=H && W>=2){
        int off=CUT_OFF;
        int wL = x-Lx+off;
        int wR = Hx-x+off;
        int c_mid=(Lx+Hx)/2;
        int mid_width=max(c_mid-Lx, Hx-c_mid);
        if(c_mid==x){
            int c2 = (c_mid+1<=Hx)? c_mid+1 : c_mid-1;
            mid_width=min(mid_width, max(c2-Lx, Hx-c2));
        }
        if(!avoidTrack && min(wL,wR) < mid_width-1){
            int c;
            if(wL<=wR){ c=min(Hx, x+off); if(c<=x) c=min(Hx,x+1); cut.intended=-1; }
            else { c=max(Lx, x-off); if(c>=x) c=max(Lx,x-1); cut.intended=+1; }
            if(c==x) c=min(Hx,x+1);
            cut.active=true; cut.track=true; cut.typ='v'; cut.line=c;
        } else {
            if(c_mid==x) c_mid = (c_mid+1<=Hx)? c_mid+1 : c_mid-1;
            cut.active=true; cut.track=false; cut.typ='v'; cut.line=c_mid;
        }
    } else if(H>=2){
        int off=CUT_OFF;
        int wD = y-Ly+off;
        int wU = Hy-y+off;
        int r_mid=(Ly+Hy)/2;
        int mid_width=max(r_mid-Ly, Hy-r_mid);
        if(r_mid==y){
            int r2 = (r_mid+1<=Hy)? r_mid+1 : r_mid-1;
            mid_width=min(mid_width, max(r2-Ly, Hy-r2));
        }
        if(!avoidTrack && min(wD,wU) < mid_width-1){
            int r;
            if(wD<=wU){ r=min(Hy, y+off); if(r<=y) r=min(Hy,y+1); cut.intended=-1; }
            else { r=max(Ly, y-off); if(r>=y) r=max(Ly,y-1); cut.intended=+1; }
            if(r==y) r=min(Hy,y+1);
            cut.active=true; cut.track=true; cut.typ='h'; cut.line=r;
        } else {
            if(r_mid==y) r_mid = (r_mid+1<=Hy)? r_mid+1 : r_mid-1;
            cut.active=true; cut.track=false; cut.typ='h'; cut.line=r_mid;
        }
    }
}
static bool cutMove(int x,int y,pair<int,int>& res){
    if(!cut.active){
        chooseCut(x,y);
        if(!cut.active) return false;
    }
    int vx=0,vy=0;
    if(hist.size()>=2){
        vx=hist.back().first-hist[hist.size()-2].first;
        vy=hist.back().second-hist[hist.size()-2].second;
        vx=max(-1,min(1,vx)); vy=max(-1,min(1,vy));
    }
    if(cut.typ=='v'){
        int c=cut.line;
        if(cut.track && abs(x-c)<=CUT_BUF){
            vector<pair<int,int>> cand;
            for(int d=-HALFW; d<=HALFW; ++d){
                int yy=y+d; if(yy<Ly||yy>Hy) continue;
                if(!black_[c][yy]) cand.push_back({c,yy});
            }
            if(!cand.empty()){
                if(vy>0) sort(cand.begin(),cand.end(),[](const pair<int,int>&a,const pair<int,int>&b){return a.second>b.second;});
                else if(vy<0) sort(cand.begin(),cand.end(),[](const pair<int,int>&a,const pair<int,int>&b){return a.second<b.second;});
                else sort(cand.begin(),cand.end(),[&](const pair<int,int>&a,const pair<int,int>&b){return abs(a.second-y)<abs(b.second-y);});
                res=cand[0]; return true;
            }
        }
        int bestd=INT_MAX; bool found=false;
        for(int yy=Ly; yy<=Hy; ++yy) if(!black_[c][yy]){
            int d=abs(c-x)+abs(yy-y);
            if(d<bestd){bestd=d;res={c,yy};found=true;}
        }
        if(found) return true;
        if(cut.track){
            int act = (x<c)? -1 : +1;
            if(act != cut.intended) avoidTrack=true;
        }
        if(x<c) Hx=c-1; else Lx=c+1;
        cut.active=false;
        return cutMove(x,y,res);
    } else {
        int r=cut.line;
        if(cut.track && abs(y-r)<=CUT_BUF){
            vector<pair<int,int>> cand;
            for(int d=-HALFW; d<=HALFW; ++d){
                int xx=x+d; if(xx<Lx||xx>Hx) continue;
                if(!black_[xx][r]) cand.push_back({xx,r});
            }
            if(!cand.empty()){
                if(vx>0) sort(cand.begin(),cand.end(),[](const pair<int,int>&a,const pair<int,int>&b){return a.first>b.first;});
                else if(vx<0) sort(cand.begin(),cand.end(),[](const pair<int,int>&a,const pair<int,int>&b){return a.first<b.first;});
                else sort(cand.begin(),cand.end(),[&](const pair<int,int>&a,const pair<int,int>&b){return abs(a.first-x)<abs(b.first-x);});
                res=cand[0]; return true;
            }
        }
        int bestd=INT_MAX; bool found=false;
        for(int xx=Lx; xx<=Hx; ++xx) if(!black_[xx][r]){
            int d=abs(xx-x)+abs(r-y);
            if(d<bestd){bestd=d;res={xx,r};found=true;}
        }
        if(found) return true;
        if(cut.track){
            int act = (y<r)? -1 : +1;
            if(act != cut.intended) avoidTrack=true;
        }
        if(y<r) Hy=r-1; else Ly=r+1;
        cut.active=false;
        return cutMove(x,y,res);
    }
}
static bool killMove(int x,int y,pair<int,int>& res){
    if(!black_[x][y]){res={x,y};return true;}
    int best=INT_MAX; bool found=false;
    for(int dx=-3;dx<=3;++dx) for(int dy=-3;dy<=3;++dy){
        int xx=x+dx,yy=y+dy;
        if(xx<Lx||xx>Hx||yy<Ly||yy>Hy) continue;
        if(black_[xx][yy]) continue;
        int d=abs(dx)+abs(dy);
        if(d<best){best=d;res={xx,yy};found=true;}
    }
    if(found) return true;
    for(int xx=Lx;xx<=Hx;++xx) for(int yy=Ly;yy<=Hy;++yy) if(!black_[xx][yy]){res={xx,yy};return true;}
    return false;
}
static pair<int,int> decide(int x,int y){
    if(outerPhase){
        pair<int,int> res;
        if(outerMove(x,y,res)) return res;
        outerPhase=false;
    }
    while(true){
        int W=Hx-Lx+1,H=Hy-Ly+1;
        if(W*H<=9){
            pair<int,int> res;
            if(killMove(x,y,res)) return res;
            return {1,1};
        }
        pair<int,int> res;
        if(cutMove(x,y,res)) return res;
        // cut finished inside cutMove; loop to choose next / kill
        if(!cut.active){
            // avoid infinite loop: if no cut and box not small, force kill
            if(killMove(x,y,res)) return res;
            return {1,1};
        }
    }
}

int main(){
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    int sx,sy;
    if(!(cin>>sx>>sy)) return 0;
    setupOuter(min(1000,sx+MARGIN),min(1000,sy+MARGIN));
    hist.clear(); hist.push_back({sx,sy});
    int curx=sx,cury=sy;
    while(true){
        pair<int,int> m=decide(curx,cury);
        black_[m.first][m.second]=true;
        cout<<m.first<<' '<<m.second<<'\n'<<flush;
        int nx,ny;
        if(!(cin>>nx>>ny)) return 0;
        if(nx==0&&ny==0) return 0;
        hist.push_back({nx,ny});
        curx=nx;cury=ny;
        if(curx>=Rx||cury>=Ty){
            int nRx=min(1000,max(Rx,curx+RETRY_K));
            int nTy=min(1000,max(Ty,cury+RETRY_K));
            if(nRx<=curx) nRx=min(1000,curx+1);
            if(nTy<=cury) nTy=min(1000,cury+1);
            setupOuter(nRx,nTy);
        }
    }
    return 0;
}
