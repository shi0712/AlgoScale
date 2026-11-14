#include <bits/stdc++.h>
using namespace std;

struct Frame { int parent, next; };

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    int R, C;
    if (!(cin >> R >> C)) return 0;
    vector<string> a(R);
    for (auto &s : a) if (!(cin >> s)) return 0;
    const int dx[4] = {-1, 0, 1, 0};
    const int dy[4] = {0, 1, 0, -1};
    vector<vector<int>> id(R, vector<int>(C, -1));
    vector<pair<int,int>> cell;
    for (int x=0; x<R; ++x) for (int y=0; y<C; ++y)
        if (a[x][y]=='.') { id[x][y]=cell.size(); cell.emplace_back(x,y); }
    int n=cell.size(), ns=4*n;
    vector<array<int,4>> adj(n), dist(n);
    for (int i=0; i<n; ++i) {
        auto [x,y]=cell[i];
        for (int d=0; d<4; ++d) {
            int xx=x+dx[d], yy=y+dy[d];
            adj[i][d]=(xx>=0 && xx<R && yy>=0 && yy<C)?id[xx][yy]:-1;
        }
    }
    for (int x=0; x<R; ++x) for (int y=0; y<C; ++y) if (id[x][y]>=0) {
        int i=id[x][y];
        dist[i][0]=adj[i][0]<0?0:dist[adj[i][0]][0]+1;
        dist[i][3]=adj[i][3]<0?0:dist[adj[i][3]][3]+1;
    }
    for (int x=R-1; x>=0; --x) for (int y=C-1; y>=0; --y) if (id[x][y]>=0) {
        int i=id[x][y];
        dist[i][2]=adj[i][2]<0?0:dist[adj[i][2]][2]+1;
        dist[i][1]=adj[i][1]<0?0:dist[adj[i][1]][1]+1;
    }

    // The normalized component, with the facing direction made north,
    // together with the normalized current cell, identifies an equivalence class.
    map<vector<int>, int> shapes;
    unordered_map<unsigned long long,int> classIndex;
    classIndex.reserve(ns*2);
    vector<int> cls(ns), component;
    vector<long long> sumX, sumY;
    vector<int> classSize;
    vector<char> multipleComponents;
    vector<char> seen(n);
    vector<int> cellComponent(n,-1);
    int componentNumber=0;
    for (int start=0; start<n; ++start) if (!seen[start]) {
        vector<int> comp{start}; seen[start]=1;
        for (int k=0; k<(int)comp.size(); ++k)
            for (int j: adj[comp[k]]) if (j>=0 && !seen[j]) {
                seen[j]=1; comp.push_back(j);
            }
        for (int i:comp) cellComponent[i]=componentNumber;
        for (int d=0; d<4; ++d) {
            vector<pair<int,int>> rotated;
            int minX=100000, minY=100000;
            for (int i:comp) {
                auto [x,y]=cell[i];
                for (int t=0; t<d; ++t) { int z=x; x=-y; y=z; }
                rotated.emplace_back(x,y);
                minX=min(minX,x); minY=min(minY,y);
            }
            vector<int> shape;
            for (auto [x,y]:rotated) shape.push_back((x-minX)*128+y-minY);
            sort(shape.begin(),shape.end());
            auto it=shapes.emplace(shape,shapes.size()).first;
            int shapeId=it->second;
            for (int k=0; k<(int)comp.size(); ++k) {
                int i=comp[k], state=4*i+d;
                auto [x,y]=rotated[k];
                unsigned long long key=(static_cast<unsigned long long>(shapeId)<<14)
                    | ((x-minX)*128+y-minY);
                auto [jt,fresh]=classIndex.emplace(key,classSize.size());
                int z=jt->second;
                if (fresh) {
                    component.push_back(componentNumber);
                    sumX.push_back(0); sumY.push_back(0);
                    classSize.push_back(0);
                    multipleComponents.push_back(0);
                }
                cls[state]=z;
                sumX[z]+=cell[i].first; sumY[z]+=cell[i].second;
                ++classSize[z];
                if (component[z]!=componentNumber) multipleComponents[z]=1;
            }
        }
        ++componentNumber;
    }
    int nc=classSize.size();
    // Different components can never meet. Within one component, the mean
    // of a rotational orbit is its fixed point (or the cell itself if unique).
    vector<int> target(nc,-1);
    for (int z=0; z<nc; ++z) {
        if (multipleComponents[z]) continue;
        if (sumX[z]%classSize[z] || sumY[z]%classSize[z]) continue;
        int x=sumX[z]/classSize[z], y=sumY[z]/classSize[z];
        if (id[x][y]>=0 && cellComponent[id[x][y]]==component[z]) target[z]=id[x][y];
    }
    // A bit in belief[d] denotes a possible square while facing d.
    // Whole equivalence classes always survive or disappear together.
    using Word=unsigned long long;
    int words=(R*C+63)/64;
    array<vector<Word>,4> belief;
    array<array<vector<Word>,100>,4> observationMask;
    vector<Word> localizable(words);
    for (int d=0; d<4; ++d) {
        belief[d].assign(words,0);
        for (auto &mask:observationMask[d]) mask.assign(words,0);
    }
    for (int i=0; i<n; ++i) {
        auto [x,y]=cell[i];
        int p=x*C+y;
        Word bit=1ULL<<(p%64);
        if (target[cls[4*i]]>=0) localizable[p/64]|=bit;
        for (int d=0; d<4; ++d) {
            belief[d][p/64]|=bit;
            observationMask[d][dist[i][d]][p/64]|=bit;
        }
    }
    int stateCount=0, firstState=-1, agreedPosition=-1;
    bool canLocate=false, positionAmbiguous=false;

    // Coordinates here are relative to the initial position and orientation.
    constexpr int L=203, O=101;
    unsigned char known[L][L]{}; // 0 unknown, 1 open, 2 wall
    bool visited[L][L]{};
    int x=O,y=O,heading=0;
    known[x][y]=1; visited[x][y]=true;
    vector<Frame> stack{{3,0}};
    int observation;

    auto readObservation = [&]() -> bool {
        if (!(cin >> observation)) return false;
        stateCount=0; firstState=-1; agreedPosition=-1;
        canLocate=false; positionAmbiguous=false;
        for (int w=0; w<words; ++w) {
            Word positions=0;
            for (int d=0; d<4; ++d) {
                Word bits=(belief[d][w]&=observationMask[d][observation][w]);
                positions|=bits;
                stateCount+=__builtin_popcountll(bits);
                if (firstState<0 && bits) {
                    int p=64*w+__builtin_ctzll(bits);
                    firstState=4*id[p/C][p%C]+d;
                }
            }
            if (positions) {
                canLocate |= (positions&localizable[w])!=0;
                if (agreedPosition>=0 || (positions&(positions-1))) positionAmbiguous=true;
                if (agreedPosition<0) agreedPosition=64*w+__builtin_ctzll(positions);
            }
        }
        for (int t=1; t<=observation; ++t)
            known[x+dx[heading]*t][y+dy[heading]*t]=1;
        known[x+dx[heading]*(observation+1)][y+dy[heading]*(observation+1)]=2;
        return true;
    };
    if (!readObservation()) return 0;

    auto perform = [&](int action) -> bool {
        // 0 = left, 1 = right, 2 = step.
        if (action==2) {
            x+=dx[heading]; y+=dy[heading];
            const int delta[4]={-C,1,C,-1};
            for (int d=0; d<4; ++d) {
                auto &bits=belief[d];
                int shift=abs(delta[d]), whole=shift/64, part=shift%64;
                if (delta[d]>0) {
                    for (int w=words-1; w>=0; --w) {
                        Word v=w>=whole?bits[w-whole]<<part:0;
                        if (part && w>whole) v|=bits[w-whole-1]>>(64-part);
                        bits[w]=v;
                    }
                } else {
                    for (int w=0; w<words; ++w) {
                        Word v=w+whole<words?bits[w+whole]>>part:0;
                        if (part && w+whole+1<words) v|=bits[w+whole+1]<<(64-part);
                        bits[w]=v;
                    }
                }
            }
        } else {
            int turn=action==0?3:1;
            heading=(heading+turn)%4;
            if (action==0) rotate(belief.begin(),belief.begin()+1,belief.end());
            else rotate(belief.begin(),belief.end()-1,belief.end());
        }
        cout << (action==0?"left":action==1?"right":"step") << endl;
        return readObservation();
    };
    auto turnTo = [&](int d) -> int {
        return ((d-heading+4)%4==3)?0:1;
    };

    vector<int> route;
    int routeAt=0;
    bool navigating=false;
    while (true) {
        if (!positionAmbiguous && agreedPosition>=0) {
            cout << "yes " << agreedPosition/C+1 << ' ' << agreedPosition%C+1 << endl;
            return 0;
        }
        if (!canLocate) { cout << "no" << endl; return 0; }

        if (stateCount==classSize[cls[firstState]] && !navigating) {
            int source=firstState, goal=target[cls[source]];
            vector<int> parent(ns,-1), action(ns,-1), q{source};
            parent[source]=source;
            int finish=-1;
            // Minimize commands, including turns, on the known component.
            for (int k=0; k<(int)q.size(); ++k) {
                int u=q[k], i=u/4, d=u%4;
                if (i==goal) { finish=u; break; }
                int next[3]={4*i+(d+3)%4,4*i+(d+1)%4,
                    adj[i][d]<0?-1:4*adj[i][d]+d};
                for (int t=0; t<3; ++t) {
                    int v=next[t];
                    if (v>=0 && parent[v]<0) {
                        parent[v]=u; action[v]=t; q.push_back(v);
                    }
                }
            }
            for (int v=finish; v!=source; v=parent[v]) route.push_back(action[v]);
            reverse(route.begin(),route.end());
            navigating=true;
        }
        if (navigating) {
            if (routeAt==(int)route.size()) return 0; // The agreement check above must have succeeded.
            if (!perform(route[routeAt++])) return 0;
            continue;
        }

        Frame &f=stack.back();
        int count=stack.size()==1?4:3;
        while (f.next<count) {
            int d=(f.parent+1+f.next)%4;
            int xx=x+dx[d], yy=y+dy[d];
            if (known[xx][yy]==2 || visited[xx][yy]) ++f.next;
            else break;
        }
        if (f.next<count) {
            int d=(f.parent+1+f.next)%4;
            if (heading!=d) { if (!perform(turnTo(d))) return 0; }
            else {
                // The observation in this direction has made the neighbor known.
                if (observation==0) { ++f.next; continue; }
                ++f.next;
                if (!perform(2)) return 0;
                visited[x][y]=true;
                stack.push_back({(d+2)%4,0});
            }
        } else {
            if (stack.size()==1) return 0; // A full exploration leaves one equivalence class.
            int d=f.parent;
            if (heading!=d) { if (!perform(turnTo(d))) return 0; }
            else {
                stack.pop_back();
                if (!perform(2)) return 0;
            }
        }
    }
}
