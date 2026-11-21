#include <bits/stdc++.h>
using namespace std;

int R,C,N,ds[4];
vector<string> gridmap;
vector<int> obs,kind,weight,goalDist;
vector<array<int,4>> rotpos;
vector<double> wlog;
const int INF=100000000;
const char *cmd[]={"step","left","right"};

int readobs(){
    int d;
    if(!(cin>>d) || d<0 || d>99) exit(0);
    return d;
}
int go(int s,int a){
    if(!a) return s+4*ds[s%4];
    return (s&~3)+((s+(a==1?3:1))&3);
}
pair<int,int> rot(int x,int y,int k){
    while(k--){int t=x;x=y;y=-t;}
    return {x,y};
}
// -2: no, -1: unresolved, otherwise the known square
int verdict(const vector<int>& b){
    bool live=false,agree=true;
    int p=b.front()/4;
    for(int s:b){
        int q=s/4;
        live|=kind[q]!=-2;
        agree &= q==p && kind[q]!=-2 && (kind[q]<0 || kind[q]==q);
    }
    return !live?-2:(agree?p:-1);
}

// Exact minimum weighted sum of remaining actions under a short depth bound.
double shortplan(const vector<int>& b,int depth,int &choice){
    if(verdict(b)!=-1) return 0;
    int W=0;
    for(int s:b){
        if(goalDist[s]>depth) return 1e50;
        W+=weight[s/4];
    }
    if(!depth) return 1e50;
    if(b.size()==1){
        int s=b[0];
        for(int a=0;a<3;a++) if((a||obs[s]) && goalDist[go(s,a)]<goalDist[s]){
            choice=a;break;
        }
        return double(W)*goalDist[s];
    }
    double best=1e50;
    for(int a=0;a<3;a++){
        if(!a && !obs[b[0]]) continue;
        int idx[100];
        fill(idx,idx+100,-1);
        vector<vector<int>> parts;
        for(int s:b){
            int t=go(s,a),&j=idx[obs[t]];
            if(j<0){j=parts.size();parts.emplace_back();}
            parts[j].push_back(t);
        }
        double val=W;
        for(auto &v:parts){
            int dummy=-1;
            val+=shortplan(v,depth-1,dummy);
            if(val>=best) break;
        }
        if(val<best){best=val;choice=a;}
        if(best==W) break;
    }
    return best;
}

struct Bucket{
    int w=0,dead=0,n=0,place=-1;
    double wl=0;
    void add(int s){
        int p=s/4,m=weight[p];
        w+=m;
        if(kind[p]==-2) dead+=m;
        else wl+=wlog[m];
        if(!n) place=p;
        if(place!=p || kind[p]==-2 || (kind[p]>=0 && kind[p]!=p)) place=-2;
        ++n;
    }
    double potential() const{
        if(dead==w || place>=0 || n==1) return 0;
        return w+wlog[w]-wl-wlog[dead];
    }
};

vector<int> plan(const vector<int>& b){
    int root=b.front();
    if(b.size()==1){
        for(int a=0;a<3;a++) if((a||obs[root]) && goalDist[go(root,a)]<goalDist[root])
            return {a};
    }
    if(b.size()<=48){
        int act=-1;
        if(shortplan(b,b.size()<=12?5:4,act)<1e49) return {act};
    }
    // Until the first different prediction, every pose is a fixed rigid
    // transform of the representative pose. Thus BFS needs only 4*N nodes.
    struct Rel{int k,off;};
    vector<Rel> rel;
    Bucket start;
    double startdist=0;
    for(int s:b){
        int k=(s-root+4*N)%4;
        rel.push_back({k,s/4-rotpos[root/4][k]});
        start.add(s);
        startdist+=double(weight[s/4])*goalDist[s];
    }
    double pot=start.potential(),best=-1e100;
    vector<int> par(4*N,-1),via(4*N),depth(4*N);
    vector<int> q={root};
    par[root]=root;
    int target=-1,first=INF;
    long long work=0;
    for(size_t i=0;i<q.size();i++){
        int s=q[i],L=depth[s];
        if(L>first+5 || (L>first && work>300000)) break;
        auto pose=[&](Rel t){
            return 4*(rotpos[s/4][t.k]+t.off)+((s+t.k)&3);
        };
        bool split=false,agree=false;
        if(L){
            for(auto t:rel) if(obs[pose(t)]!=obs[s]){split=true;break;}
            if(!split && b.size()<=4){
                agree=true;
                for(auto t:rel){
                    int p=pose(t)/4;
                    if(p!=s/4 || kind[p]==-2 || (kind[p]>=0 && kind[p]!=p)){agree=false;break;}
                }
            }
        }
        if(split || agree){
            if(first==INF) first=L;
            work+=b.size();
            double left=0,dist=0;
            if(split){
                Bucket bins[100];
                int used[100],nu=0;
                for(auto t:rel){
                    int u=pose(t),d=obs[u];
                    if(!bins[d].n) used[nu++]=d;
                    bins[d].add(u);
                    dist+=double(weight[u/4])*goalDist[u];
                }
                for(int j=0;j<nu;j++) left+=bins[used[j]].potential();
            }
            double cost=L+0.9*(dist-startdist)/start.w;
            double score=(pot-left)/max(0.1*L,cost);
            if(score>best+1e-10){best=score;target=s;}
            continue;
        }
        if(first!=INF) work+=b.size();
        for(int a=0;a<3;a++) if(a||obs[s]){
            int u=go(s,a);
            if(par[u]<0){
                par[u]=s;via[u]=a;depth[u]=L+1;q.push_back(u);
            }
        }
    }
    vector<int> path;
    for(int u=target;u!=root;u=par[u]) path.push_back(via[u]);
    reverse(path.begin(),path.end());
    return path;
}

int main(){
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    if(!(cin>>R>>C)) return 0;
    N=R*C;
    gridmap.resize(R);
    for(auto &row:gridmap) if(!(cin>>row)) return 0;
    int initial=readobs();
    ds[0]=-C;ds[1]=1;ds[2]=C;ds[3]=-1;
    int dx[]={-1,0,1,0},dy[]={0,1,0,-1};
    auto open=[&](int x,int y){
        return x>=0 && x<R && y>=0 && y<C && gridmap[x][y]=='.';
    };
    obs.resize(4*N);
    rotpos.resize(N);
    for(int p=0;p<N;p++) for(int h=0;h<4;h++){
        auto [u,v]=rot(p/C,p%C,h);
        rotpos[p][h]=u*C+v;
        if(!open(p/C,p%C)) continue;
        int x=p/C+dx[h],y=p%C+dy[h];
        while(open(x,y)){
            obs[4*p+h]++;
            x+=dx[h];y+=dy[h];
        }
    }

    // Normalize the whole connected component in the robot's frame.
    // Equal normalized shapes and equal normalized robot coordinates are
    // exactly the permanently observation-equivalent states.
    vector<int> comp(N,-1);
    vector<vector<int>> cells;
    map<vector<int>,int> shapes;
    vector<int> embeddings,owner;
    vector<array<int,4>> shape(N),coord(N);
    for(int p=0;p<N;p++) if(open(p/C,p%C) && comp[p]<0){
        int id=cells.size();
        cells.push_back({p});comp[p]=id;
        auto &v=cells.back();
        for(size_t i=0;i<v.size();i++){
            int u=v[i];
            for(int h=0;h<4;h++) if(obs[4*u+h]){
                int z=u+ds[h];
                if(comp[z]<0){comp[z]=id;v.push_back(z);}
            }
        }
        for(int h=0;h<4;h++){
            vector<pair<int,int>> pts;
            int mx=INF,my=INF;
            for(int u:v){
                auto xy=rot(u/C,u%C,(4-h)%4);
                pts.push_back(xy);
                mx=min(mx,xy.first);my=min(my,xy.second);
            }
            vector<int> key;
            for(auto [x,y]:pts) key.push_back((x-mx)*101+y-my);
            for(size_t j=0;j<v.size();j++) coord[v[j]][h]=key[j];
            sort(key.begin(),key.end());
            auto [it,fresh]=shapes.emplace(move(key),int(shapes.size()));
            int k=it->second;
            if(fresh){embeddings.push_back(0);owner.push_back(id);}
            if(owner[k]!=id) owner[k]=-1;
            embeddings[k]++;
            for(int u:v) shape[u][h]=k;
        }
    }
    kind.resize(N,-2);weight.resize(N);
    goalDist.resize(4*N);
    for(auto &v:cells){
        int id=comp[v[0]],k=shape[v[0]][0],g=-2;
        if(owner[k]==id){
            if(embeddings[k]==1) g=-1;
            else{
                // Any agreement of distinct rotations must be at their
                // common center, the centroid of the component.
                int x=0,y=0,n=v.size();
                for(int u:v){x+=u/C;y+=u%C;}
                if(x%n==0 && y%n==0 && comp[(x/n)*C+y/n]==id)
                    g=(x/n)*C+y/n;
            }
        }
        for(int u:v){
            kind[u]=g;weight[u]=embeddings[k];
            if(g>=0) for(int h=0;h<4;h++) goalDist[4*u+h]=INF;
        }
        if(g>=0){
            queue<int> qu;
            for(int h=0;h<4;h++){goalDist[4*g+h]=0;qu.push(4*g+h);}
            while(!qu.empty()){
                int s=qu.front();qu.pop();
                auto visit=[&](int u){
                    if(goalDist[u]==INF){goalDist[u]=1+goalDist[s];qu.push(u);}
                };
                visit(go(s,1));visit(go(s,2));
                if(obs[(s&~3)+((s+2)&3)]) visit(s-4*ds[s%4]);
            }
        }
    }
    wlog.resize(4*N+1);
    for(int i=1;i<=4*N;i++) wlog[i]=i*log2(double(i));

    set<pair<int,int>> seen;
    vector<int> b;
    for(auto &v:cells) for(int u:v) for(int h=0;h<4;h++)
        if(obs[4*u+h]==initial && seen.emplace(shape[u][h],coord[u][h]).second)
            b.push_back(4*u+h);

    for(;;){
        if(b.empty()) return 0;
        int z=verdict(b);
        if(z!=-1){
            if(z==-2) cout<<"no\n";
            else cout<<"yes "<<z/C+1<<' '<<z%C+1<<'\n';
            cout.flush();return 0;
        }
        auto path=plan(b);
        for(int a:path){
            cout<<cmd[a]<<'\n'<<flush;
            int d=readobs();
            size_t n=0;
            for(int s:b){
                int t=go(s,a);
                if(obs[t]==d) b[n++]=t;
            }
            b.resize(n);
            if(!n) return 0;
        }
    }
}
