#include <bits/stdc++.h>
using namespace std;
struct Box {int x1,x2,y1,y2;};
struct Explorer {
    int n,x,y,side,steps=0;
    vector<string> field;
    function<char(int,int,char)> read;
    Explorer(int n,function<char(int,int,char)> read):n(n),x(n),y(n),side(2*n+1),
        field(side,string(side,'?')),read(read) {field[y][x]='^';}
    void go(int tx,int ty) {
        while (x!=tx || y!=ty) {
            char key;
            if (x<tx) {++x;key='>';} else if (x>tx) {--x;key='<';}
            else if (y<ty) {++y;key='v';} else {--y;key='^';}
            ++steps;char seen=read(x,y,key);
            if (seen=='G') throw 0;
            field[y][x]=seen;
        }
    }
    bool contains_goal(Box b) {
        int f=(b.x1<=n && n<=b.x2 && b.y1<=n && n<=b.y2);
        for (int j=b.y1;j<=b.y2;++j) {
            if (b.x1) f+=(field[j][b.x1-1]=='>')-(field[j][b.x1]=='<');
            if (b.x2+1<side) f+=(field[j][b.x2+1]=='<')-(field[j][b.x2]=='>');
        }
        for (int i=b.x1;i<=b.x2;++i) {
            if (b.y1) f+=(field[b.y1-1][i]=='v')-(field[b.y1][i]=='^');
            if (b.y2+1<side) f+=(field[b.y2+1][i]=='^')-(field[b.y2][i]=='v');
        }
        return f==1;
    }
    void run() {
        bool vertical=true;Box b={0,side-1,0,side-1};
        while (b.x1<b.x2 && b.y1<b.y2) {
            int mid=vertical?(b.x1+b.x2)/2:(b.y1+b.y2)/2;
            int lo=vertical?b.y1:b.x1,hi=vertical?b.y2:b.x2;
            int a=vertical?y:x,target=(lo+hi)/2;
            int lane=(vertical?x:y)-mid;
            auto visit=[&](int i,int j){if(vertical)go(mid+j,i);else go(i,mid+j);};
            // Orient logical coordinates so that the desired final index follows a.
            if(a>target) {swap(lo,hi);}
            int d=lo<=hi?1:-1;
            if (a==target) {
                visit(lo,lane);visit(lo,lane^1);visit(hi,lane^1);
                visit(hi,lane);visit(a,lane);
            } else {
                visit(lo,lane);lane^=1;visit(lo,lane);visit(a,lane);
                for (int i=a+d;i!=target;i+=d) {
                    visit(i,lane);lane^=1;visit(i,lane);
                }
                visit(target,lane);visit(hi,lane);lane^=1;visit(hi,lane);visit(target,lane);
            }
            Box first=b,second=b;
            if(vertical){first.x2=mid;second.x1=mid+1;}
            else {first.y2=mid;second.y1=mid+1;}
            b=contains_goal(first)?first:second;
            go(clamp(x,b.x1,b.x2),clamp(y,b.y1,b.y2));
            vertical=!vertical;
        }
        int dl=x-b.x1+y-b.y1,dr=b.x2-x+b.y2-y;
        if(dl<dr){go(b.x1,b.y1);go(b.x2,b.y2);}
        else {go(b.x2,b.y2);go(b.x1,b.y1);}
        throw runtime_error("Goal absent");
    }
};
#ifdef VERIFY
int main(){
    mt19937 rng(101947);int total=0;
    for(int n:{1,2,3,4,7,15,16,30,40,99,200,500,2000})
        for(int t=0;t<(n<=500?32:4);++t){
            int side=2*n+1,x=n,y=n-1;
            vector<string> map(side,string(side,'.'));map[n][n]='^';map[y][x]='G';
            string moves="<>^v";
            for(int k=0;k<(t%2?side:side*side);++k) {
                shuffle(moves.begin(),moves.end(),rng);bool ok=false;
                for(char c:moves){int xx=x+(c=='>')-(c=='<'),yy=y+(c=='v')-(c=='^');
                    if(xx>=0 && yy>=0 && xx<side && yy<side && map[yy][xx]=='.'){
                        map[y][x]=c;x=xx;y=yy;map[y][x]='G';ok=true;break;
                    }
                }if(!ok)break;
            }
            Explorer explorer(n,[&](int a,int b,char){return map[b][a];});
            try{explorer.run();}catch(int){}
            if(explorer.x!=x || explorer.y!=y || explorer.steps>24046)return 1;
            ++total;
        }
    cout<<total<<" paths verified\n";
}
#else
int main(){int n;if(!(cin>>n)) return 0;
    Explorer explorer(n,[](int,int,char key){cout<<key<<endl;char c;if(!(cin>>c))exit(0);return c;});
    try{explorer.run();}catch(int){}
}
#endif
