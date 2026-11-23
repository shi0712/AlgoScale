#include <bits/stdc++.h>
using namespace std;

int n, N;
vector<string> arr;
int cx, cy;

void mv(char d){
    if(d=='^') cy--; else if(d=='v') cy++; else if(d=='<') cx--; else cx++;
    cout << d << '\n' << flush;
    string s;
    if(!(cin >> s)) exit(0);
    if(s=="G") exit(0);
    if(s=="-1") exit(0);
    arr[cy][cx] = s[0];
}

void goTo(int tx,int ty){
    while(cx<tx) mv('>');
    while(cx>tx) mv('<');
    while(cy<ty) mv('v');
    while(cy>ty) mv('^');
}

bool pointsTo(int ux,int uy,int vx,int vy){
    char c = arr[uy][ux];
    if(c=='^') return vx==ux && vy==uy-1;
    if(c=='v') return vx==ux && vy==uy+1;
    if(c=='<') return vx==ux-1 && vy==uy;
    if(c=='>') return vx==ux+1 && vy==uy;
    return false;
}
bool cross(int ux,int uy,int vx,int vy){
    return pointsTo(ux,uy,vx,vy) || pointsTo(vx,vy,ux,uy);
}

// parity of crossings of region [a1,a2]x[b1,b2]
int parityRegion(int a1,int a2,int b1,int b2){
    int p=0;
    for(int y=b1;y<=b2;y++){
        if(a1>0 && cross(a1,y,a1-1,y)) p^=1;
        if(a2<N-1 && cross(a2,y,a2+1,y)) p^=1;
    }
    for(int x=a1;x<=a2;x++){
        if(b1>0 && cross(x,b1,x,b1-1)) p^=1;
        if(b2<N-1 && cross(x,b2,x,b2+1)) p^=1;
    }
    return p;
}

int main(){
    if(!(cin>>n)) return 0;
    N=2*n+1;
    arr.assign(N,string(N,'\0'));
    cx=cy=n;
    arr[n][n]='^';
    int x1=0,x2=N-1,y1=0,y2=N-1;
    while(true){
        int w=x2-x1+1,h=y2-y1+1;
        if((long long)w*h<=2){
            // visit cells nearest first
            vector<pair<int,int>> cells;
            for(int x=x1;x<=x2;x++)for(int y=y1;y<=y2;y++)cells.push_back({x,y});
            sort(cells.begin(),cells.end(),[&](auto&a,auto&b){
                return abs(a.first-cx)+abs(a.second-cy) < abs(b.first-cx)+abs(b.second-cy);
            });
            for(auto&c:cells) goTo(c.first,c.second);
            return 0;
        }
        if(w>=h){
            int m=x1+w/2-1;
            int sr = (abs(cy-y1)<=abs(cy-y2))? y1:y2;
            int er = (sr==y1)? y2:y1;
            int dy = (er>=sr)?1:-1;
            int sc = (abs(cx-m)<=abs(cx-(m+1)))? m : m+1;
            goTo(sc,sr);
            int r=sr;
            while(true){
                if(cx==m) mv('>'); else mv('<');
                if(r==er) break;
                r+=dy;
                mv(dy>0?'v':'^');
            }
            int p=parityRegion(x1,m,y1,y2);
            bool cin_=(n>=x1&&n<=m&&n>=y1&&n<=y2);
            bool inR = p ^ cin_;
            if(inR) x2=m; else x1=m+1;
        } else {
            int m=y1+h/2-1;
            int sc = (abs(cx-x1)<=abs(cx-x2))? x1:x2;
            int ec = (sc==x1)? x2:x1;
            int dx = (ec>=sc)?1:-1;
            int sr = (abs(cy-m)<=abs(cy-(m+1)))? m : m+1;
            goTo(sc,sr);
            int c=sc;
            while(true){
                if(cy==m) mv('v'); else mv('^');
                if(c==ec) break;
                c+=dx;
                mv(dx>0?'>':'<');
            }
            int p=parityRegion(x1,x2,y1,m);
            bool cin_=(n>=x1&&n<=x2&&n>=y1&&n<=m);
            bool inR = p ^ cin_;
            if(inR) y2=m; else y1=m+1;
        }
    }
}
