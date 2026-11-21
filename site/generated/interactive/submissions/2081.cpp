#include <bits/stdc++.h>
using namespace std;

pair<int,int> ask(int d) {
    cout << "? " << d << endl;
    int v, w;
    if (!(cin >> v) || v<0) exit(0);
    if (!(cin >> w) || w<0) exit(0);
    return {v, w};
}

void answer(const vector<int>& path) {
    cout << '!';
    for (int v : path) cout << ' ' << v;
    cout << endl;
}

// Solve small cases by maintaining the exact set of consistent complements.
// A path is certified when it avoids the union of their missing edges.
class SmallSolver {
    struct Plan { int cost, depth, query; };
    static constexpr int inf = 100000000;
    int n, full, edge[8][8]{};
    vector<int> label;
    vector<pair<int,int>> prefix;
    array<int,10> fixedAdj{};
    vector<vector<int>> paths;
    map<pair<int,vector<int>>,Plan> memo;
    unordered_map<int,int> certificates;
    unordered_set<int> stars;
    int starWeight=0;

    int certificate(int missing) {
        auto it=certificates.find(missing);
        if (it!=certificates.end()) return it->second;
        int result=-1;
        // The prefix vertices can separate at most prefix.size()+1 runs
        // consisting entirely of the other vertices.
        if (n*(n-1)/2-popcount((unsigned)missing)>=n-1-(int)prefix.size()) {
            auto adj=fixedAdj;
            for (int i=0;i<n;++i) for (int j=0;j<i;++j) {
                if (!(missing & edge[i][j])) {
                    adj[label[i]] |= 1<<label[j];
                    adj[label[j]] |= 1<<label[i];
                }
            }
            int start=(1<<full)-1, leaves=0;
            for (int i=0;i<full;++i) {
                if (!adj[i]) { start=0; break; }
                if (has_single_bit((unsigned)adj[i])) { ++leaves; start=1<<i; }
            }
            if (start && leaves<=2) {
                array<unsigned short,1024> dead{};
                vector<int> path;
                auto search=[&](auto&& self,int v,int left)->bool {
                    if (dead[left] & (1<<v)) return false;
                    path.push_back(v+1);
                    if (!left) return true;
                    for (int s=left&adj[v];s;s&=s-1) {
                        int u=countr_zero((unsigned)s);
                        if (self(self,u,left^(1<<u))) return true;
                    }
                    path.pop_back();
                    dead[left] |= 1<<v;
                    return false;
                };
                for (int s=start;s;s&=s-1) {
                    int v=countr_zero((unsigned)s);
                    if (search(search,v,((1<<full)-1)^(1<<v))) {
                        result=static_cast<int>(paths.size());
                        paths.push_back(move(path));
                        break;
                    }
                }
            }
        }
        certificates.emplace(missing,result);
        return result;
    }

    int response(int graph, int live, int d) const {
        int best=n, result=0;
        for (int i=0;i<n;++i) if (live>>i & 1) {
            int degree=-1, w=0;
            for (int j=0;j<n;++j) if (live>>j & 1) {
                if (!(graph & edge[i][j])) ++degree;
                else if (!w) w=j+1;
            }
            if (degree>=d && degree<best) {
                best=degree;
                result=(i+1)*16+w;
            }
        }
        return result;
    }

    Plan plan(int live, const vector<int>& graphs) {
        int all=0, weight=(int)graphs.size();
        for (int g : graphs) {
            all|=g;
            if (stars.count(g)) weight+=starWeight;
        }
        int p=certificate(all);
        if (p>=0) return {0,0,-1-p};
        auto key=make_pair(live,graphs);
        auto found=memo.find(key);
        if (found!=memo.end()) return found->second;
        Plan best{inf,inf,0};
        for (int d=max(0,popcount((unsigned)live)-1);d>=0;--d) {
            map<int,vector<int>> groups;
            for (int g : graphs) groups[response(g,live,d)].push_back(g);
            if (groups.size()==1 && groups.begin()->first==0) continue;
            int cost=weight, depth=1;
            for (auto& [r,group] : groups) {
                int next=live;
                if (r) next^=1<<(r/16-1);
                auto child=plan(next,group);
                if (child.cost==inf) { cost=inf; break; }
                cost+=child.cost;
                depth=max(depth,1+child.depth);
                if (cost>best.cost) break;
            }
            if (cost<inf && tie(cost,depth)<tie(best.cost,best.depth))
                best={cost,depth,d};
        }
        memo.emplace(move(key),best);
        return best;
    }

    bool matchesPrefix(int graph) const {
        if (prefix.empty()) return true;
        array<int,10> deg;
        deg.fill(-1);
        for (int i : label) deg[i]=0;
        for (int i=0;i<n;++i) for (int j=0;j<i;++j) {
            if (graph & edge[i][j]) { ++deg[label[i]]; ++deg[label[j]]; }
        }
        for (auto it=prefix.rbegin();it!=prefix.rend();++it) {
            auto [v,w]=*it;
            deg[v]=(w>=0);
            if (w>=0) ++deg[w];
            for (int i=0;i<full;++i) {
                if (i<v && deg[i]==deg[v]) return false;
                if (w<0 && deg[i]==1) return false;
            }
        }
        return true;
    }

public:
    void run(int size) {
        full=size;
        label.resize(full);
        iota(label.begin(),label.end(),0);
        int count=full-2;
        while (label.size()>8) {
            auto [v,w]=ask((int)label.size()-2);
            --v; --w;
            auto it=find(label.begin(),label.end(),v);
            if (it==label.end()) return;
            label.erase(it);
            if (w>=0 && !binary_search(label.begin(),label.end(),w)) return;
            prefix.emplace_back(v,w);
            for (int x : label) if (x!=w) {
                fixedAdj[v]|=1<<x;
                fixedAdj[x]|=1<<v;
            }
            count-=(w>=0);
        }
        n=static_cast<int>(label.size());
        int e=0;
        for (int i=0;i<n;++i) for (int j=0;j<i;++j)
            edge[i][j]=edge[j][i]=1<<e++;
        // Include the concentrated missing-edge case in the query cost.
        // Its center can be impossible to attach if removed too early.
        long long total=1;
        for (int k=1;k<=full-2;++k)
            total=total*(full*(full-1)/2-k+1)/k;
        starWeight=(int)(total/(12*full*(full-1)));
        for (int h=0;h<full;++h) for (int u=0;u<full;++u) if (h!=u) {
            auto missing=[&](int x,int y) { return (x==h || y==h) && x!=u && y!=u; };
            bool ok=true;
            for (auto [x,w] : prefix) for (int y=0;y<full;++y) if (x!=y) {
                if (missing(x,y)==bool(fixedAdj[x]&(1<<y))) ok=false;
            }
            if (!ok) continue;
            int g=0;
            for (int i=0;i<n;++i) for (int j=0;j<i;++j)
                if (missing(label[i],label[j])) g|=edge[i][j];
            if (matchesPrefix(g)) stars.insert(g);
        }
        vector<int> graphs;
        int all=0;
        for (unsigned g=(1u<<count)-1;g<(1u<<e);) {
            if (matchesPrefix(g)) { graphs.push_back(g); all|=g; }
            if (!g) break;
            unsigned bit=g&-g, next=g+bit;
            g=next|(((next^g)/bit)>>2);
        }
        int live=(1<<n)-1;
        bool first=true;
        while (!graphs.empty()) {
            int p=certificate(all);
            if (p>=0) { answer(paths[p]); return; }
            int d;
            if (first && n==8)
                d=!prefix.empty() && !stars.empty() ? n-1 : n-2;
            else d=plan(live,graphs).query;
            first=false;
            auto [v,w]=ask(d);
            auto code=[&](int u) {
                if (!u) return 0;
                auto it=lower_bound(label.begin(),label.end(),u-1);
                if (it==label.end() || *it!=u-1) exit(0);
                return 1+(int)(it-label.begin());
            };
            v=code(v); w=code(w);
            all=0;
            erase_if(graphs,[&](int g) {
                if (response(g,live,d)!=16*v+w) return true;
                all|=g;
                return false;
            });
            if (v) live^=1<<(v-1);
        }
    }
};

class Solver {
    int n, budget;
    set<int> live;
    vector<int> when, forbidden, successor, roots, history;
    vector<char> good;
    // A root is a vertex with at most one missing edge when it was removed.
    // Unused roots are grouped by their one exception (zero means none).
    set<pair<int,int>> guards;
    // Other chain ends can attach to any still live label below their cutoff.
    set<pair<int,int>> tails;
    vector<int> frequency, secondMissing, intervalHits, intervalTag, certifiedBy;
    int coreVertex=0;
    bool boundedCore=false;
    int leafCount=0;
    long long collisions=0;
    bool concentrated=false;

    bool adjacent(int a, int b) const {
        if (when[a] > when[b]) swap(a,b);
        if (when[a] == when[b]) return false;
        if (certifiedBy[b]==a) return true;
        return good[a] ? b!=forbidden[a] && b!=secondMissing[a] : b<forbidden[a];
    }

    bool protectedQuery() const {
        int cutoff = tails.empty() ? 0 : tails.rbegin()->first;
        if (cutoff > *live.rbegin()) return true;
        if (guards.empty()) return false;
        int w = guards.begin()->first;
        return !w || guards.rbegin()->first != w || !live.count(w) || cutoff > w;
    }

    void addRoot(int v,int w) {
        good[v] = true;
        guards.emplace(w,v);
        roots.push_back(v);
    }

    void attach(int v,int w) {
        auto it = tails.upper_bound({v,INT_MAX});
        int prev;
        if (it != tails.end()) {
            prev = it->second;
            tails.erase(it);
        } else {
            // Prefer spending an imperfect guard and retaining a universal one.
            auto g = guards.lower_bound({1,0});
            if (g != guards.end() && g->first == v)
                g = guards.upper_bound({v,INT_MAX});
            if (g == guards.end()) g = guards.begin();
            if (g == guards.end() || g->first == v) exit(0);
            prev = g->second;
            guards.erase(g);
        }
        successor[prev] = v;
        tails.emplace(w,v);
    }

    void finishClique() const {
        deque<int> p(live.begin(),live.end());
        for (auto it=roots.rbegin();it!=roots.rend();++it) {
            int g = *it;
            bool front = p.front()!=forbidden[g];
            for (int v=g;v;v=successor[v]) {
                if (front) p.push_front(v);
                else p.push_back(v);
            }
        }
        answer(vector<int>(p.begin(),p.end()));
    }

    // Starting with the unknown vertices as singleton paths, insert queried
    // vertices in reverse. Each insertion can join two distinct components.
    int tryPath() const {
        vector<int> opp(n+1);
        vector<array<int,2>> edges(n+1);
        vector<int> deg(n+1);
        set<int> ends;
        for (int x : live) { opp[x]=x; ends.insert(ends.end(),x); }
        int components = static_cast<int>(live.size());
        for (auto it=history.rbegin();it!=history.rend();++it) {
            int v=*it, w=forbidden[v], a=0,b=0;
            auto pos = good[v] ? ends.end() : ends.lower_bound(w);
            while (pos != ends.begin()) {
                int x=*--pos;
                if (x==w || x==secondMissing[v]) continue;
                if (!a) a=x;
                else if (x!=opp[a]) { b=x; break; }
            }
            auto join=[&](int x) {
                edges[x][deg[x]++]=v;
                edges[v][deg[v]++]=x;
                if (opp[x]!=x) ends.erase(x);
            };
            if (a) join(a);
            if (b) join(b);
            else ends.insert(v);
            int left=a?opp[a]:v, right=b?opp[b]:v;
            opp[left]=right;
            opp[right]=left;
            components += 1-(a!=0)-(b!=0);
        }
        if (components > 6) return components;
        vector<deque<int>> paths;
        for (int x : ends) if (x<=opp[x]) {
            deque<int> p;
            int last=0, cur=x;
            while (cur) {
                p.push_back(cur);
                int next=0;
                for (int k=0;k<deg[cur];++k)
                    if (edges[cur][k]!=last) next=edges[cur][k];
                last=cur; cur=next;
            }
            paths.push_back(move(p));
        }
        // Splice an entire path into an edge, or concatenate at an endpoint.
        while (paths.size()>1) {
            bool merged=false;
            for (size_t i=0;i<paths.size() && !merged;++i)
                for (size_t j=0;j<paths.size() && !merged;++j) if (i!=j) {
                    auto &p=paths[i], &q=paths[j];
                    for (int rev=0;rev<2 && !merged;++rev) {
                        int a=rev?q.back():q.front(), b=rev?q.front():q.back();
                        for (size_t k=0;k<=p.size();++k) {
                            if (k && !adjacent(p[k-1],a)) continue;
                            if (k<p.size() && !adjacent(b,p[k])) continue;
                            if (rev) reverse(q.begin(),q.end());
                            p.insert(p.begin()+k,q.begin(),q.end());
                            paths.erase(paths.begin()+j);
                            merged=true;
                            break;
                        }
                    }
                }
            if (!merged) return static_cast<int>(paths.size());
        }
        answer(vector<int>(paths[0].begin(),paths[0].end()));
        return 1;
    }

public:
    explicit Solver(int size): n(size),budget(n-2),when(n+1,n),forbidden(n+1),
        successor(n+1),good(n+1),frequency(n+1),secondMissing(n+1),intervalHits(n+1),
        intervalTag(n+1),certifiedBy(n+1) {
        for (int i=1;i<=n;++i) live.insert(live.end(),i);
    }

    void run() {
        int nextTry=n/2, backoff=1, lastComponents=n, attempts=0;
        bool preferMax=false, triedUniversal=false, mustMax=false;
        bool probeTwo=false, noLeaves=false;
        bool sawLeaf=false;
        int minPositive=1;
        while (true) {
            int m=static_cast<int>(live.size()), used=n-m;
            if (budget==0) { finishClique(); return; }
            if (used>=nextTry) {
                int c=tryPath();
                if (c==1) return;
                backoff = c>=lastComponents ? min(max(1,n/96),backoff*2) : 1;
                lastComponents=c;
                ++attempts;
                int stride=attempts<=12 ? 1 : max(1,n/80);
                nextTry=used+max({backoff,(c-1)/2,stride});
            }
            bool hub = concentrated;
            bool canMax=protectedQuery();
            bool takeMax=!probeTwo && (mustMax || budget<=1 || (canMax && (preferMax || hub)));
            bool possibleStar=used==1 && forbidden[history[0]] &&
                              history[0]-1-(forbidden[history[0]]<history[0])<=1;
            bool universal=!probeTwo && !takeMax && !triedUniversal &&
                           (possibleStar || (hub && !canMax));
            int d=probeTwo ? m-3 : takeMax ? 0 : universal ? m-1 : m-2;
            auto [v,w]=ask(d);
            if (!v) {
                if (!universal || w) return;
                triedUniversal=true;
                continue;
            }
            if (!live.count(v) || (w && (!live.count(w) || v==w))) return;
            live.erase(v);
            when[v]=used;
            history.push_back(v);
            forbidden[v]=w;
            mustMax=false;
            if (probeTwo && w) {
                // The preceding query excluded complement degree one.
                // This vertex has exactly two missing neighbors.
                attach(v,w);
                budget-=2;
                coreVertex=v;
                boundedCore=true;
                preferMax=false;
                minPositive=1;
                noLeaves=false;
                probeTwo=false;
                sawLeaf=false;
            } else if (takeMax) {
                coreVertex=noLeaves ? v : 0;
                boundedCore=false;
                bool known = budget<=1 || w==0 || live.upper_bound(w)==live.end();
                if (known) addRoot(v,w);
                else attach(v,w);
                // E - ceil(2E/m) is nondecreasing in E.
                budget = w ? budget-max(minPositive,(2*budget+m-1)/m) : 0;
                preferMax = hub;
                if (frequency[v]>=2 || (used==2 && forbidden[history[0]]==v)) preferMax=true;
                triedUniversal=false;
                minPositive=1;
                noLeaves=false;
                sawLeaf=false;
            } else {
                addRoot(v,w);
                if (coreVertex) {
                    // Restore v's degree in the complement just before the
                    // core vertex was removed, except for their unknown edge.
                    int hits=(intervalTag[v]==coreVertex ? intervalHits[v] : 0)+(w!=0);
                    if (!hits) certifiedBy[v]=coreVertex;
                    if (hits==1 && boundedCore && forbidden[coreVertex]!=v) {
                        secondMissing[coreVertex]=v;
                        good[coreVertex]=true;
                    }
                    if (w) {
                        if (intervalTag[w]!=coreVertex) {
                            intervalTag[w]=coreVertex;
                            intervalHits[w]=0;
                        }
                        ++intervalHits[w];
                    }
                }
                if (w) {
                    sawLeaf=true;
                    --budget;
                    ++leafCount;
                    collisions+=frequency[w]++;
                    if (collisions>=2 && collisions*n>6LL*leafCount*leafCount)
                        concentrated=true;
                } else {
                    noLeaves=!universal;
                    minPositive=universal ? 1 : probeTwo ? 3 : 2;
                    if (budget < minPositive*(minPositive+1)/2) budget=0;
                    probeTwo=!probeTwo && !universal && sawLeaf && m>=5 && !hub;
                    preferMax=true;
                    mustMax=true;
                }
            }
        }
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    int n;
    if (!(cin>>n)) return 0;
    if (n<=10) SmallSolver().run(n);
    else Solver(n).run();
}
