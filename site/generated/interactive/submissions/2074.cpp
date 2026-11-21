#include <bits/stdc++.h>
using namespace std;

// Saturating leaves in label order partitions the tree into downward paths.
// Path i starts at the first new vertex and ends at leaf i. Only its attachment
// to a vertex of an earlier path is unknown. parents[i] retains those possible
// attachment vertices, numbered by path and then by offset from its start.
//
// A structural collection visits side paths before continuing the current
// path. Capacity probes supply independent constraints without raising B.
// Once the internal paths are placed, a collection can read a color histogram
// for many sets of sibling leaves at once.
constexpr int MMAX = 200, NMAX = 1000;
using Leaves = bitset<MMAX>;
using Nodes = bitset<NMAX>;
int m, n, maxValue;
vector<int> len, startAt, owner, offsetAt, physicalLen, tailExtra;
vector<Nodes> parents;
mt19937 rng(81379623);

int answerInt() {
    int x;
    if (!(cin >> x) || x < 0) exit(0);
    return x;
}
bool put(int u, int x) {
    cout << "? 1 " << u << ' ' << x << endl;
    maxValue = max(maxValue, x);
    return answerInt() == 1;
}
vector<int> collect() {
    cout << "? 2" << endl;
    vector<int> a(answerInt());
    for (int &x : a) x = answerInt();
    return a;
}
int fillLeaf(int i, int limit) {
    int k = 0;
    int extra=tailExtra.empty()?0:tailExtra[i];
    while (k < limit+extra && put(i, 0)) ++k;
    return k-extra;
}

struct Position {
    int parent, off, type = -1, orbit = -1;
    vector<int> code;
    Leaves domain;
};
using Observation = vector<Position>;
vector<Observation> observations;
map<vector<pair<int,int>>, int> types;
map<tuple<int,int,int>, int> orbits;
vector<Leaves> allowedOrbit;
vector<vector<int>> masks;
vector<int> internalFront;
vector<int> degreeLo, degreeHi;

void classify(Observation &obs) {
    vector<vector<pair<int,int>>> children(m);
    for (int j = m-1; j >= 0; --j) {
        auto &p = obs[j];
        auto &sig = children[j];
        sort(sig.begin(),sig.end());
        sig.emplace_back(-1,(int)p.code.size());
        p.type = types.emplace(sig, (int)types.size()).first->second;
        int q = p.parent;
        if (q >= 0) children[q].emplace_back(p.off,p.type);
    }
    for (auto &p : obs) {
        int up = p.parent < 0 ? -1 : obs[p.parent].orbit;
        p.orbit = orbits.emplace(make_tuple(up,p.off,p.type), (int)orbits.size()).first->second;
    }
}

// Remove assignments which occur in no perfect matching of slots to labels.
bool matchingFilter(Observation &obs) {
    vector<int> mate(m,-1), order(m);
    iota(order.begin(),order.end(),0);
    sort(order.begin(),order.end(),[&](int a,int b) {
        return obs[a].domain.count() < obs[b].domain.count();
    });
    Leaves seen;
    auto augment = [&](auto &&self, int j) -> bool {
        Leaves d = obs[j].domain & ~seen;
        for (int i = d._Find_first(); i < m; i=d._Find_next(i)) {
            if (mate[i]<0) { mate[i]=j; return true; }
        }
        for (int i=d._Find_first(); i<m; i=d._Find_next(i)) {
            if (seen[i]) continue;
            seen.set(i);
            if (self(self,mate[i])) { mate[i]=j; return true; }
        }
        return false;
    };
    for (int j : order) {
        seen.reset();
        if (!augment(augment,j)) exit(0);
    }
    vector<int> index(m,-1), low(m), component(m,-1), st;
    int time=0, groups=0;
    auto dfs = [&](auto &&self,int j)->void {
        index[j]=low[j]=time++;
        st.push_back(j);
        auto &d=obs[j].domain;
        for (int i=d._Find_first();i<m;i=d._Find_next(i)) {
            int v=mate[i];
            if (index[v]<0) { self(self,v); low[j]=min(low[j],low[v]); }
            else if (component[v]<0) low[j]=min(low[j],index[v]);
        }
        if (index[j]==low[j]) {
            while (true) {
                int v=st.back(); st.pop_back(); component[v]=groups;
                if(v==j) break;
            }
            ++groups;
        }
    };
    for(int j=0;j<m;++j) if(index[j]<0) dfs(dfs,j);
    bool changed=false;
    for(int j=0;j<m;++j) {
        auto &d=obs[j].domain;
        for(int i=d._Find_first();i<m;i=d._Find_next(i))
            if(component[j]!=component[mate[i]]) {d.reset(i);changed=true;}
    }
    return changed;
}

bool filterObservation(Observation &obs) {
    bool changed=false;
    for(auto &p:obs) {
        for(int i=p.domain._Find_first(); i<m; i=p.domain._Find_next(i))
            if (!allowedOrbit[i][p.orbit]) {p.domain.reset(i); changed=true;}
    }
    if(!internalFront.empty()) {
        vector<int> front(m);
        for(int j=1;j<m;++j) if(obs[j].code.size()>1)
            front[obs[j].parent]=max(front[obs[j].parent],obs[j].off+1);
        for(int j=0;j<m;++j) {
            auto &d=obs[j].domain;
            for(int i=d._Find_first();i<m;i=d._Find_next(i))
                if(internalFront[i]!=front[j]) {d.reset(i);changed=true;}
        }
    }
    for(const auto &h:masks) {
        vector<int> mandatory(m);
        vector<Nodes> possible(m);
        for(auto &b:possible) b.set(0);
        for(int j=m-1;j>=0;--j) {
            auto &p=obs[j]; bool hit=false,miss=false;
            for(int i=p.domain._Find_first();i<m;i=p.domain._Find_next(i)) {
                if(h[i]<len[i] && (h[i]<mandatory[j] || !possible[j][h[i]])) {
                    p.domain.reset(i);changed=true;continue;
                }
                if(h[i]) hit=true; else miss=true;
            }
            if(j && hit) {
                possible[p.parent].set(p.off+1);
                if(!miss) mandatory[p.parent]=max(mandatory[p.parent],p.off+1);
            }
        }
    }
    vector<Nodes> next(m);
    for(int j=m-1;j>0;--j) {
        auto &p=obs[j];
        auto &up=obs[p.parent].domain;
        Nodes loc, possible;
        for(int a=up._Find_first();a<m;a=up._Find_next(a)) loc.set(startAt[a]+p.off);
        for(int i=p.domain._Find_first();i<m;i=p.domain._Find_next(i)) {
            Nodes hits=parents[i]&loc;
            if(hits.none()) {p.domain.reset(i); changed=true;}
            else {next[i]|=hits; possible|=hits;}
        }
        for(int a=up._Find_first();a<m;a=up._Find_next(a))
            if(!possible[startAt[a]+p.off]) {up.reset(a);changed=true;}
    }
    changed |= matchingFilter(obs);
    vector<Leaves> os(m);
    for(auto &p:obs)
        for(int i=p.domain._Find_first();i<m;i=p.domain._Find_next(i)) os[i].set(p.orbit);
    for(int i=0;i<m;++i) {
        if(os[i]!=allowedOrbit[i]) {allowedOrbit[i]=os[i];changed=true;}
        if(i && next[i]!=parents[i]) {parents[i]=next[i];changed=true;}
    }
    return changed;
}

vector<int> reverseLen,depthLo,depthHi;
vector<pair<vector<int>,vector<int>>> evidence;

void reverseQuery() {
    reverseLen.resize(m);
    int occupied=0;
    for(int i=m-1;i>=0;--i) {
        reverseLen[i]=fillLeaf(i,n-occupied-i);
        occupied+=reverseLen[i];
        depthLo[i]=max(depthLo[i],reverseLen[i]);
    }
    vector<int> order(m);
    iota(order.rbegin(),order.rend(),0);
    evidence.emplace_back(order,vector<int>(reverseLen.rbegin(),reverseLen.rend()));
    depthHi.back()=reverseLen.back();
    collect();
}

// The forward-owned and reverse-owned paths for leaf i are both suffixes
// ending at i. Their intersection consists precisely of its private tail.
void compressTails() {
    tailExtra.resize(m);
    n=0;owner.clear();offsetAt.clear();
    Nodes old;
    for(int i=0;i<m;++i) {
        tailExtra[i]=min(len[i],reverseLen[i])-1;
        len[i]-=tailExtra[i];reverseLen[i]-=tailExtra[i];
        depthLo[i]=max(len[i],reverseLen[i]);
        parents[i]=old;startAt[i]=n;
        for(int k=0;k<len[i];++k,++n) {
            owner.push_back(i);offsetAt.push_back(k);
            if(k+1<len[i]) old.set(n);
        }
    }
    depthHi.assign(m,n);
    depthHi[0]=len[0];depthHi.back()=reverseLen.back();
    evidence.back().second.assign(reverseLen.rbegin(),reverseLen.rend());
}

bool filterDepth() {
    bool changed=false;
    for(int i=1;i<m;++i) {
        int lower=n+1,upper=0, common=-1, upLo=n+1,upHi=0;
        auto &opts=parents[i];
        for(int v=opts._Find_first();v<n;v=opts._Find_next(v)) {
            int a=owner[v],delta=len[i]-len[a]+offsetAt[v]+1;
            if(depthLo[a]+delta>depthHi[i] || depthHi[a]+delta<depthLo[i]) {
                opts.reset(v);changed=true;continue;
            }
            lower=min(lower,depthLo[a]+delta); upper=max(upper,depthHi[a]+delta);
            if(common==-1) common=a;
            else if(common!=a) common=-2;
            upLo=min(upLo,depthLo[i]-delta);upHi=max(upHi,depthHi[i]-delta);
        }
        if(lower>depthLo[i]) {depthLo[i]=lower;changed=true;}
        if(upper<depthHi[i]) {depthHi[i]=upper;changed=true;}
        if(common>=0) {
            if(upLo>depthLo[common]) {depthLo[common]=upLo;changed=true;}
            if(upHi<depthHi[common]) {depthHi[common]=upHi;changed=true;}
        }
    }
    return changed;
}

bool filterReverse() {
    if(reverseLen.empty()) return false;
    // A vertex's owner in reverse order is its largest descendant leaf.
    // The two ownership counts give the exact number of vertices shared by
    // the two sides of every cut in leaf-label order.
    vector<int> latest(n), front(m);
    for(int i=0;i<m;++i) {
        front[i]=max(0,len[i]-reverseLen[i]);
        for(int d=0;d<len[i];++d) latest[startAt[i]+d]=i+(d<front[i]);
    }
    latest[0]=m-1;
    for(int i=m-1;i>0;--i) {
        int first=parents[i]._Find_first();
        int a=owner[first];
        int last=parents[i]._Find_next(startAt[a]+len[a]-1);
        if(last<n) continue;
        for(int d=0;d<=offsetAt[first];++d)
            latest[startAt[a]+d]=max(latest[startAt[a]+d],latest[startAt[i]]);
    }
    vector<int> diff(m+1);
    for(int v=0;v<n;++v) {++diff[owner[v]];--diff[latest[v]];}
    bool changed=false;
    int crossing=0, known=0;
    for(int i=1;i<m;++i) {
        crossing+=len[i-1]-reverseLen[i-1];
        known+=diff[i-1];
        int slack=crossing-known, last=-1, k=0;
        auto &d=parents[i];
        for(int v=d._Find_first();v<n;v=d._Find_next(v)) {
            int a=owner[v],off=offsetAt[v];
            if(a!=last) {
                k=0; last=a;
                while(k<front[a] && latest[startAt[a]+k]>=i) ++k;
            }
            if(off>=front[a] || off+1>k+slack ||
               depthLo[a]-len[a]+off+1>crossing ||
               (reverseLen[i]>len[i] && latest[v]>i)) {
                d.reset(v);changed=true;
            }
        }
    }
    return changed;
}

void propagate() {
    bool changed=true;
    while(changed) {
        changed=filterDepth();
        changed|=filterReverse();
        for(auto it=observations.rbegin();it!=observations.rend();++it)
            changed |= filterObservation(*it);
        if(!observations.empty()) {
            const auto &obs=observations.front();
            vector<vector<int>> cnt(m);
            for(int j=0;j<m;++j) cnt[j].resize(obs[j].code.size());
            for(int j=1;j<m;++j) ++cnt[obs[j].parent][obs[j].off];
            degreeLo.assign(n,m+1); degreeHi.assign(n,0);
            for(int j=0;j<m;++j)
                for(int a=obs[j].domain._Find_first();a<m;a=obs[j].domain._Find_next(a))
                    for(int d=0;d+1<len[a];++d) {
                        int v=startAt[a]+d;
                        degreeLo[v]=min(degreeLo[v],cnt[j][d]);
                        degreeHi[v]=max(degreeHi[v],cnt[j][d]);
                    }
            vector<int> possible(n),fixed(n);
            for(int i=1;i<m;++i) {
                const auto &d=parents[i];
                for(int v=d._Find_first();v<n;v=d._Find_next(v)) ++possible[v];
                if(d.count()==1) ++fixed[d._Find_first()];
            }
            Nodes full,must;
            for(int v=0;v<n;++v) {
                if(fixed[v]==degreeHi[v]) full.set(v);
                if(possible[v] && possible[v]==degreeLo[v]) must.set(v);
            }
            for(int i=1;i<m;++i) if(parents[i].count()>1) {
                auto d=parents[i]&~full;
                if((d&must).any()) d&=must;
                if(d!=parents[i]) {parents[i]=d;changed=true;}
            }
        }
        for(const auto &h:masks) {
            Nodes covered, ends;
            for(int i=0;i<m;++i) {
                for(int d=0;d<h[i];++d) covered.set(startAt[i]+d);
                if(h[i] && h[i]<len[i]) ends.set(startAt[i]+h[i]-1);
            }
            Nodes onlyHit;
            if(!degreeHi.empty()) {
                for(int v=ends._Find_first();v<n;v=ends._Find_next(v))
                    if(degreeHi[v]==1) onlyHit.set(v);
                for(int i=1;i<m;++i) if(!h[i] && (parents[i]&onlyHit).any()) {
                    parents[i]&=~onlyHit;changed=true;
                }
            }
            Nodes once,twice;
            for(int i=1;i<m;++i) if(h[i]) {
                Nodes d=parents[i]&covered;
                if(d!=parents[i]) {parents[i]=d;changed=true;}
                twice|=once&d; once|=d;
            }
            Nodes forced=ends&~twice;
            for(int i=1;i<m;++i) if(h[i] && parents[i].count()>1) {
                Nodes hits=parents[i]&forced;
                if(hits.any() && hits!=parents[i]) {parents[i]=hits;changed=true;}
            }
        }
    }
}

// Removing singleton paths leaves the first-pass capacities of all other
// paths unchanged. Their reverse capacities locate their deepest mutual forks.
void internalQuery() {
    vector<int> order,counts;
    internalFront.assign(m,0);
    for(int i=m-1;i>=0;--i) if(len[i]>1) {
        int k=fillLeaf(i,depthHi[i]);
        if(order.empty()) depthHi[i]=k;
        depthLo[i]=max(depthLo[i],k);
        internalFront[i]=max(0,len[i]-k);
        order.push_back(i);counts.push_back(k);
    }
    collect();
    evidence.emplace_back(order,counts);
    for(int i=1;i<m;++i) if(len[i]>1) {
        auto &d=parents[i];
        for(int v=d._Find_first();v<n;v=d._Find_next(v))
            if(offsetAt[v]>=internalFront[owner[v]]) d.reset(v);
    }
}

void subsetQuery(const Leaves &s) {
    vector<int> h(len);
    int used=0;
    vector<int> order,counts;
    for(int i=0;i<m;++i) if(s[i]) {
        int gained=fillLeaf(i,min(depthHi[i],n-used));
        order.push_back(i);counts.push_back(gained);
        if(!used) depthHi[i]=gained;
        depthLo[i]=max(depthLo[i],gained);
        used+=gained;
    }
    int last=m-1;
    while(!s[last]) --last;
    for(int i=0;i<m;++i) if(!s[i]) {
        if(i>last) { h[i]=0; continue; }
        int gained=len[i]==1 ? 1 : fillLeaf(i,min(len[i],n-used));
        order.push_back(i);counts.push_back(gained);
        h[i]-=gained;
        used+=gained;
    }
    collect();
    evidence.emplace_back(order,counts);
    if(s.count()==1) {
        int last=-1;
        for(int i=0;i<m;++i) if(h[i]) {
            if(last>=0) {parents[i].reset();parents[i].set(startAt[last]+h[last]-1);}
            last=i;
        }
    }
    masks.push_back(h);
}

// Heuristic graph coloring. Labels that might occupy the same positions get
// different digits when possible. Fixed labels can share a background digit.
vector<int> digits(vector<int> ids, int alphabet, bool sparse) {
    vector<int> result(m);
    if(alphabet<=1 || ids.empty()) return result;
    shuffle(ids.begin(),ids.end(),rng);
    if(sparse) {
        stable_sort(ids.begin(),ids.end(),[&](int a,int b){
            return (parents[a].count()>1) > (parents[b].count()>1);
        });
        vector<Nodes> used(alphabet);
        vector<int> size(alphabet);
        for(int i:ids) {
            if(parents[i].count()<=1) continue;
            int best=-1;
            for(int c=1;c<alphabet;++c)
                if((used[c]&parents[i]).none() && (best<0 || size[c]<size[best])) best=c;
            if(best>=0) {result[i]=best;used[best]|=parents[i];++size[best];}
        }
        return result;
    }
    vector<vector<int>> groups(alphabet);
    for(int i:ids) {
        int best=0, bestCost=INT_MAX;
        for(int c=0;c<alphabet;++c) {
            int cost=groups[c].size();
            for(int j:groups[c]) {
                if(len[i]!=len[j]) continue;
                if((allowedOrbit[i]&allowedOrbit[j]).any()) cost+=6;
                if((parents[i]&parents[j]).any()) cost+=3;
            }
            if(cost<bestCost) best=c,bestCost=cost;
        }
        groups[best].push_back(i);result[i]=best;
    }
    return result;
}

void shapeQuery(int satColors, int headColors, int tailColors, bool sparse, bool local=false) {
    vector<int> singles,internals;
    for(int i=0;i<m;++i) (len[i]==1 ? singles:internals).push_back(i);
    if(singles.empty()) satColors=0;
    vector<int> sd=digits(singles,satColors,sparse);
    // Code pairs are balanced separately for every path length.
    vector<vector<int>> code(m);
    map<int,vector<int>> byLen;
    for(int i:internals) byLen[len[i]].push_back(i);
    vector<int> headLength(headColors,-1);
    for(auto &[l,ids]:byLen) {
        auto id=digits(ids,headColors*tailColors,false);
        if(local) {
            vector<int> next(n);
            for(int i:ids) if(i) id[i]=next[parents[i]._Find_first()]++;
        }
        for(int i:ids) {
            int a=id[i]%headColors, b=id[i]/headColors;
            code[i].resize(l);
            code[i][0]=satColors+a;
            code[i].back()=satColors+headColors+b;
            if(headLength[a]!=l) headLength[a]=headLength[a]==-1?l:0;
        }
    }
    for(int i:singles) code[i]={sd[i]};
    const int high=satColors+headColors;
    for(int i:internals) {
        int l=len[i], a=code[i][0]-satColors;
        for(int k=1;k+1<l;++k) {
            // If the head already fixes the length, use interior digits as well.
            if(headLength[a]) code[i][k]=high+rng()%tailColors;
            else code[i][k]=high+tailColors;
        }
    }
    for(int i=0;i<m;++i) for(int x:code[i]) if(!put(i,x)) exit(0);
    auto data=collect();
    Observation obs;
    vector<int> stack;
    for(int x:data) {
        if(x<high) {
            int p=stack.empty()?-1:stack.back();
            int d=p<0?0:(int)obs[p].code.size()-1;
            Position slot;
            slot.parent=p; slot.off=d; slot.code.push_back(x);
            obs.push_back(move(slot));
            if(x>=satColors) stack.push_back(obs.size()-1);
        } else {
            if(stack.empty()) exit(0);
            auto &seq=obs[stack.back()].code;
            seq.push_back(x);
            int l=headLength[seq[0]-satColors];
            if(l ? (int)seq.size()==l : x<high+tailColors) stack.pop_back();
        }
    }
    if(!stack.empty() || (int)obs.size()!=m) exit(0);
    for(int j=0;j<m;++j) for(int i=0;i<m;++i)
        if((i==0)==(j==0) && code[i]==obs[j].code) obs[j].domain.set(i);
    classify(obs);
    observations.push_back(move(obs));
}

struct Histogram {
    vector<int> tag;
    vector<vector<int>> count;
};
using U64 = uint64_t;

void narrowHistograms(const vector<int> &labels,const vector<int> &total,
                      const vector<Histogram> &hist,const vector<U64> &hash,
                      const vector<U64> &target) {
    bool changed=true;
    while(changed) {
        changed=false;
        for(int v=0;v<n;++v) if(total[v]) {
            vector<int> free, fixed;
            U64 remaining=target[v];
            for(int i:labels) if(parents[i][v]) {
                if(parents[i].count()>1) free.push_back(i);
                else {fixed.push_back(i);remaining-=hash[i];}
            }
            if(free.empty()) continue;
            Leaves possible,required;
            for(int i:free) possible.set(i);
            int best=-1;
            double work=20001;
            vector<vector<int>> groups;
            vector<int> quotas;
            for(int r=0;r<(int)hist.size();++r) {
                const auto &h=hist[r];
                vector<int> quota=h.count[v];
                vector<vector<int>> byColor(quota.size());
                for(int i:fixed) --quota[h.tag[i]];
                for(int i:free) byColor[h.tag[i]].push_back(i);
                double combinations=1;
                for(int c=0;c<(int)quota.size();++c) {
                    int k=quota[c], size=byColor[c].size();
                    if(!k) for(int i:byColor[c]) possible.reset(i);
                    if(k==size) for(int i:byColor[c]) required.set(i);
                    for(int j=1;j<=min(k,size-k) && combinations<work;++j)
                        combinations*=double(size+1-j)/j;
                }
                if(combinations<work) {
                    best=r;work=combinations;
                    groups=move(byColor);quotas=move(quota);
                }
            }
            if(best>=0) {
                Leaves any,all; all.set();
                Leaves chosen;
                auto search=[&](auto &&self,int c,int at,int need,U64 sum)->void {
                    if(!need) {
                        do { ++c; } while(c<(int)groups.size() && !quotas[c]);
                        if(c==(int)groups.size()) {
                            if(sum==remaining) {any|=chosen;all&=chosen;}
                            return;
                        }
                        at=0;need=quotas[c];
                    }
                    for(int j=at;j+need<=(int)groups[c].size();++j) {
                        int i=groups[c][j];
                        chosen.set(i);self(self,c,j+1,need-1,sum+hash[i]);chosen.reset(i);
                    }
                };
                search(search,-1,0,0,0);
                possible&=any;required|=all;
            }
            for(int i:free) {
                if(!possible[i]) {parents[i].reset(v);changed=true;}
                else if(required[i]) {parents[i].reset();parents[i].set(v);changed=true;}
            }
        }
    }
}

struct Bucket {
    vector<int> labels;
    struct Site {
        int v;
        vector<unsigned> masks;
    };
    vector<Site> sites;
    unsigned test = 0;
};

// Once the internal skeleton is identified, it can serve as a fixed traversal
// for reading simultaneous counts of leaf colors at its vertices.
bool solveSatellites(const vector<int> &unknown, bool denseOnly=false) {
    if (any_of(unknown.begin(),unknown.end(),[](int i){return len[i]>1;})) return false;
    int u=unknown.size();
    if(u<2) return false;
    Nodes potential,needed;
    for(int i:unknown) potential|=parents[i];
    needed=potential;
    for(int v=n-1;v>0;--v) if(needed[v])
        needed.set(offsetAt[v]?v-1:(int)parents[owner[v]]._Find_first());
    vector<vector<int>> kids(n);
    vector<int> rank(n), total(n,-1);
    for(int i=0;i<m;++i) for(int d=0;d+1<len[i];++d) {
        int v=startAt[i]+d;
        if(v==0 || !needed[v]) continue;
        int p=d?v-1:(int)parents[i]._Find_first();
        rank[v]=kids[p].size();
        kids[p].push_back(v);
    }
    int degree=0,gap=0;
    for(int v=0;v<n;++v) {
        degree=max(degree,(int)kids[v].size());
        if(potential[v]) gap=max(gap,(int)kids[v].size());
    }
    vector<map<int,vector<int>>> branches(n);
    int siblings=1;
    for(int i=1;i<m;++i) if(len[i]>1 && needed[startAt[i]]) {
        auto &group=branches[parents[i]._Find_first()][len[i]];
        group.push_back(i);
        siblings=max(siblings,(int)group.size());
    }
    bool packets=degree-1>max({maxValue,5,(int)sqrt(u)+2});
    if(packets && !degreeHi.empty()) {
        bool narrow=true;
        for(int v=potential._Find_first();v<n;v=potential._Find_next(v))
            if(degreeHi[v]>1) narrow=false;
        if(narrow && masks.size()<12) return false;
    }
    // Recover known site capacities from the uncolored shape, if available.
    if(!observations.empty()) {
        const auto &obs=observations.front();
        vector<vector<int>> cnt(m);
        for(int j=0;j<m;++j) cnt[j].resize(obs[j].code.size());
        for(int j=1;j<m;++j) if(obs[j].code.size()==1) ++cnt[obs[j].parent][obs[j].off];
        for(int j=0;j<m;++j) if(obs[j].code.size()>1) {
            for(int a=obs[j].domain._Find_first();a<m;a=obs[j].domain._Find_next(a))
                for(int d=0;d+1<len[a];++d) {
                    int &v=total[startAt[a]+d], k=cnt[j][d];
                    if(v==-1) v=k;
                    else if(v!=k) v=-2;
                }
        }
    }
    for(int i=0;i<m;++i) if(len[i]==1 && parents[i].count()==1) {
        int v=parents[i]._Find_first();
        if(total[v]>=0) --total[v];
    }
    bool calibration=false;
    for(int v=0;v<n;++v) {
        if(!potential[v]) total[v]=0;
        if(total[v]<0) calibration=true;
    }

    vector<int> packetSite(m,-1);
    bool simplePackets=packets;
    for(int v=potential._Find_first();v<n;v=potential._Find_next(v)) {
        int i=owner[v];
        if(packetSite[i]!=-1) simplePackets=false;
        packetSite[i]=v;
    }
    for(int v=0;v<n;++v)
        if(!branches[v].empty() && packetSite[owner[v]]>=0 && v>=packetSite[owner[v]])
            simplePackets=false;
    if(packets && calibration && !simplePackets) {
        int heads=max(1,(int)sqrt(siblings));
        shapeQuery(1,heads,(siblings+heads-1)/heads,false,true);
        propagate();
        vector<int> rest;
        for(int i:unknown) if(parents[i].count()>1) rest.push_back(i);
        if(rest.empty()) return true;
        if(rest.size()==1) {
            Leaves select;select.set(rest[0]);subsetQuery(select);return true;
        }
        return solveSatellites(rest,denseOnly);
    }
    int bins=min(u,max({2,(u+16)/17,maxValue-gap,(int)round(sqrt(0.65*u))}));
    // Preserve the measured palette when only a small extra batch is needed.
    if(bins+gap>maxValue && (int)observations.size()>=2 && u<16)
        bins=max(2,min(bins,maxValue-gap));
    bins=max(bins,(u+16)/17);
    vector<Bucket> buckets(bins);
    vector<int> order=unknown;
    shuffle(order.begin(),order.end(),rng);
    for(int j=0;j<u;++j) buckets[j%bins].labels.push_back(order[j]);

    vector<int> walk;
    auto traverse=[&](auto &&self,int v)->void {
        walk.push_back(v);
        for(int w:kids[v]) self(self,w);
    };
    traverse(traverse,0);

    vector<int> tag(m,-1);
    // With calibration, leaf colors precede the old children. Afterwards the
    // known capacities allow all children to precede the colored leaves.
    auto query=[&](bool calibrate,int colors) {
        if(packets) {
            // Each backbone is one consecutive packet: the continuation
            // digits are smaller than every side branch. Packet length and
            // digits identify a backbone among its siblings.
            int low=1,high=1,best=INT_MAX;
            for(int d=1;d<=siblings;++d) {
                int q=colors;
                for(const auto &lists:branches) for(const auto &[l,group]:lists) {
                    int needed=group.size();
                    for(int j=1;j<l && needed>1;++j) needed=(needed+d-1)/d;
                    q=max(q,needed);
                }
                if(q+d<best) {best=q+d;low=d;high=q;}
                if(d>=best) break;
            }
            vector<int> indices(m);
            for(const auto &lists:branches) for(const auto &[l,group]:lists)
                for(int j=0;j<(int)group.size();++j) indices[group[j]]=j;
            for(int i=0;i<m;++i) {
                if(len[i]>1 && needed[startAt[i]]) {
                    int k=indices[i];
                    if(!put(i,low+k%high)) exit(0);
                    k/=high;
                    for(int j=1;j<len[i];++j) {
                        if(!put(i,k%low)) exit(0);
                        k/=low;
                    }
                } else if(tag[i]>=0 && !put(i,low+tag[i])) exit(0);
            }
            auto seq=collect();
            vector<vector<int>> count(n,vector<int>(colors));
            int at=0;
            auto read=[&](auto &&self,int parent)->void {
                int begin=at++;
                while(at<(int)seq.size() && seq[at]<low) ++at;
                int l=at-begin;
                if(l==1) {++count[parent][seq[begin]-low];return;}
                int k=0;
                for(int j=at-1;j>begin;--j) k=k*low+seq[j];
                k=k*high+seq[begin]-low;
                int i=parent<0 ? 0 : branches[parent].at(l)[k];
                if(simplePackets && packetSite[i]>=0) {
                    int v=packetSite[i];
                    total[v]=0;
                    while(at<(int)seq.size() && (at+1==(int)seq.size() || seq[at+1]>=low)) {
                        ++count[v][seq[at++]-low];++total[v];
                    }
                }
                for(int j=l-2;j>=0;--j) {
                    int v=startAt[i]+j;
                    if(!simplePackets) for(int a=0;a<total[v];++a) self(self,v);
                    for(const auto &[length,group]:branches[v])
                        for(size_t a=0;a<group.size();++a) self(self,v);
                }
            };
            read(read,-1);
            if(at!=(int)seq.size()) exit(0);
            return count;
        }
        for(int i=0;i<m;++i) {
            for(int d=0;d+1<len[i] && needed[startAt[i]+d];++d) {
                int x=rank[startAt[i]+d]+(calibrate?colors:0);
                if(!put(i,x)) exit(0);
            }
            if(tag[i]>=0 && !put(i,tag[i]+(calibrate?0:gap))) exit(0);
        }
        vector<int> data=collect();
        vector<vector<int>> counts(n,vector<int>(colors));
        int at=0;
        if(calibrate) {
            for(int v:walk) {
                ++at;
                total[v]=0;
                while(at<(int)data.size() && data[at]<colors) {
                    ++counts[v][data[at++]];
                    ++total[v];
                }
            }
        } else {
            auto read=[&](auto &&self,int v)->void {
                ++at;
                for(int w:kids[v]) self(self,w);
                for(int j=0;j<total[v];++j) ++counts[v][data[at++]-gap];
            };
            read(read,0);
        }
        if(at!=(int)data.size()) exit(0);
        return counts;
    };
    if(!denseOnly && u <= 8*(int)potential.count()) {
        int palette=min(u,max({3,min(6,(int)round(pow(u,0.32))),maxValue-(packets?4:gap)+1}));
        if(packets) {
            int freeColors=1,best=INT_MAX;
            for(int d=1;d<=siblings;++d) {
                int q=palette;
                for(const auto &lists:branches) for(const auto &[l,group]:lists) {
                    int k=group.size();
                    for(int j=1;j<l && k>1;++j) k=(k+d-1)/d;
                    q=max(q,k);
                }
                if(q+d<best) {best=q+d;freeColors=q;}
            }
            palette=max(palette,freeColors);
        }
        vector<Histogram> history;
        vector<U64> hash(m), target(n);
        for(int round=0;round<7;++round) {
            int q=palette;
            if(calibration && !packets) q=max(1,min(q,max({maxValue,gap+q-1,degree})-degree+1));
            vector<int> order=unknown;
            shuffle(order.begin(),order.end(),rng);
            vector<vector<int>> used(n,vector<int>(q));
            for(int i:order) {
                if(parents[i].count()==1) {tag[i]=0;continue;}
                vector<int> load(q);
                for(int v=parents[i]._Find_first();v<n;v=parents[i]._Find_next(v))
                    for(int c=0;c<q;++c) load[c]+=used[v][c];
                int shift=rng()%q, best=shift;
                for(int d=1;d<q;++d) if(load[(shift+d)%q]<load[best]) best=(shift+d)%q;
                tag[i]=best;
                for(int v=parents[i]._Find_first();v<n;v=parents[i]._Find_next(v)) ++used[v][best];
            }
            auto count=query(calibration,q);
            calibration=false;
            vector<U64> salt(q);
            for(auto &x:salt) x=(U64(rng())<<32)+rng();
            for(int i:unknown) hash[i]+=salt[tag[i]];
            for(int v=0;v<n;++v) for(int c=0;c<q;++c) target[v]+=count[v][c]*salt[c];
            history.push_back({tag,move(count)});
            narrowHistograms(unknown,total,history,hash,target);
            vector<int> rest;
            for(int i:unknown) if(parents[i].count()>1) rest.push_back(i);
            if(rest.empty()) return true;
            if(round==6) {
                if(rest.size()==1) {
                    Leaves select;select.set(rest[0]);subsetQuery(select);return true;
                }
                return solveSatellites(rest,true);
            }
        }
    }
    if(calibration && !packets && degree+bins-1>max(maxValue,gap+bins)) {
        for(int i:unknown) tag[i]=0;
        query(true,1);
        calibration=false;
    }
    for(int g=0;g<bins;++g) for(int i:buckets[g].labels) tag[i]=g;
    auto counts=query(calibration,bins);
    for(int g=0;g<bins;++g) {
        auto &bucket=buckets[g];
        int size=bucket.labels.size();
        for(int v=0;v<n;++v) if(counts[v][g]) {
            unsigned opts=0;
            for(int j=0;j<size;++j) if(parents[bucket.labels[j]][v]) opts|=1U<<j;
            Bucket::Site site{v,{}};
            for(unsigned sub=opts;sub;sub=(sub-1)&opts)
                if(__builtin_popcount(sub)==counts[v][g]) site.masks.push_back(sub);
            bucket.sites.push_back(move(site));
        }
    }
    while(true) {
        bool pending=false;
        for(auto &bucket:buckets) {
            int size=bucket.labels.size();
            unsigned full=(1U<<size)-1;
            bool changed=true;
            while(changed) {
                changed=false;
                unsigned covered=0, overlap=0;
                for(auto &s:bucket.sites) {
                    unsigned any=0,all=full;
                    for(unsigned x:s.masks) {any|=x;all&=x;}
                    overlap|=covered&any;covered|=any;
                    for(int j=0;j<size;++j) {
                        int i=bucket.labels[j];
                        if(!(any>>j&1U)) parents[i].reset(s.v);
                        if(all>>j&1U) {parents[i].reset();parents[i].set(s.v);}
                    }
                }
                for(auto &s:bucket.sites) {
                    unsigned forbidden=0,mandatory=0;
                    for(int j=0;j<size;++j) {
                        auto &d=parents[bucket.labels[j]];
                        if(!d[s.v]) forbidden|=1U<<j;
                        else if(d.count()==1 || !(overlap>>j&1U)) mandatory|=1U<<j;
                    }
                    size_t old=s.masks.size();
                    erase_if(s.masks,[&](unsigned x){return (x&forbidden) || (mandatory&~x);});
                    changed|=old!=s.masks.size();
                }
            }
            for(int j=0;j<size;++j) {
                int i=bucket.labels[j];
                Nodes d;
                for(auto &s:bucket.sites)
                    if(any_of(s.masks.begin(),s.masks.end(),[&](unsigned x){return x>>j&1U;}))
                        d.set(s.v);
                parents[i]&=d;
            }
            double best=0;
            unsigned choice=0;
            for(int t=0;t<56+size;++t) {
                unsigned test=t<size ? 1U<<t : rng()&full;
                double score=0;
                for(auto &s:bucket.sites) if(s.masks.size()>1) {
                    int hist[18]{};
                    int step=max(1,(int)s.masks.size()/80), sample=0;
                    for(int a=0;a<(int)s.masks.size();a+=step) {
                        ++hist[__builtin_popcount(s.masks[a]&test)];++sample;
                    }
                    double pairs=0;
                    for(int x:hist) pairs+=x*x;
                    score+=1-pairs/(sample*sample);
                }
                if(score>best) best=score,choice=test;
            }
            bucket.test=choice;
            pending|=choice!=0;
        }
        if(!pending) return true;
        for(int g=0;g<bins;++g)
            for(int j=0;j<(int)buckets[g].labels.size();++j)
                tag[buckets[g].labels[j]]=(buckets[g].test>>j&1U)?g+1:0;
        counts=query(false,bins+1);
        for(int g=0;g<bins;++g) {
            auto &b=buckets[g];
            for(auto &s:b.sites) {
                int k=counts[s.v][g+1];
                erase_if(s.masks,[&](unsigned x){return __builtin_popcount(x&b.test)!=k;});
            }
        }
    }
}

uint64_t mix64(uint64_t x) {
    x=(x^(x>>30))*0xbf58476d1ce4e5b9ULL;
    x=(x^(x>>27))*0x94d049bb133111ebULL;
    return x^(x>>31);
}

vector<int> capacities(const vector<int> &p,const vector<int> &order) {
    vector<int> used(m),counts;
    for(int i:order) {
        int a=i,k=len[i],added=0;
        while(k>used[a]) {
            added+=k-used[a];
            int before=exchange(used[a],k);
            if(before || !a) break;
            int v=p[a];
            a=owner[v];k=offsetAt[v]+1;
        }
        counts.push_back(added);
    }
    return counts;
}

// Small remaining hypothesis sets often admit a single distinguishing
// permutation. Simulate the exact insertion counts before spending a query.
bool exactStep(const vector<int> &unknown) {
    int limit=min(16000,240000/m);
    int product=1;
    for(int i:unknown) {
        product*=parents[i].count();
        if(product>limit) return false;
    }
    vector<int> p(m);
    for(int i=1;i<m;++i) p[i]=parents[i]._Find_first();
    vector<vector<int>> candidates;
    vector<vector<uint64_t>> seeds;
    vector<uint64_t> targets;
    for(const auto &obs:observations) {
        vector<uint64_t> seed(m),hash(m);
        for(int j=0;j<m;++j) {
            uint64_t x=27;
            for(int z:obs[j].code) x=x*1009+z+1;
            hash[j]=mix64(x);
            for(int i=obs[j].domain._Find_first();i<m;i=obs[j].domain._Find_next(i)) seed[i]=hash[j];
        }
        for(int j=m-1;j>0;--j)
            hash[obs[j].parent]+=mix64(hash[j]+(obs[j].off+1)*0xd1342543de82ef95ULL);
        targets.push_back(hash[0]);seeds.push_back(move(seed));
    }
    auto enumerate=[&](auto &&self,int j)->void {
        if(j<(int)unknown.size()) {
            int i=unknown[j];
            for(int v=parents[i]._Find_first();v<n;v=parents[i]._Find_next(v)) {
                p[i]=v;self(self,j+1);
            }
            return;
        }
        for(const auto &[order,counts]:evidence) if(capacities(p,order)!=counts) return;
        for(int t=0;t<(int)seeds.size();++t) {
            auto h=seeds[t];
            for(int i=m-1;i>0;--i)
                h[owner[p[i]]]+=mix64(h[i]+(offsetAt[p[i]]+1)*0xd1342543de82ef95ULL);
            if(h[0]!=targets[t]) return;
        }
        candidates.push_back(p);
    };
    enumerate(enumerate,0);
    if(candidates.empty()) exit(0);
    for(int i:unknown) {
        parents[i].reset();
        for(auto &a:candidates) parents[i].set(a[i]);
    }
    if(candidates.size()==1) return true;
    vector<int> order(m), best;
    iota(order.begin(),order.end(),0);
    long long bestScore=LLONG_MAX;
    int samples=min(100, max(6,1200000/(m*(int)candidates.size())));
    for(int t=0;t<samples || best.empty();++t) {
        if(t==0) reverse(order.begin(),order.end());
        else if(t<samples) shuffle(order.begin(),order.end(),rng);
        else {
            if(t-samples>= (int)unknown.size()) return false;
            iota(order.begin(),order.end(),0);
            rotate(order.begin(),order.begin()+unknown[t-samples],order.begin()+unknown[t-samples]+1);
        }
        unordered_map<uint64_t,int> hist;
        long long collision=0;
        for(auto &a:candidates) {
            uint64_t h=0;
            for(int k:capacities(a,order)) h=h*1009+k;
            collision+=hist[h]++;
            if(collision>=bestScore) break;
        }
        if(collision<bestScore && collision<1LL*(int)candidates.size()*((int)candidates.size()-1)/2) {
            bestScore=collision;best=order;
            if(!collision) break;
        }
    }
    vector<int> counts;
    vector<int> alive(candidates.size());
    iota(alive.begin(),alive.end(),0);
    vector<vector<int>> predictions;
    for(auto &a:candidates) predictions.push_back(capacities(a,best));
    for(int j=0;j<m && alive.size()>1;++j) {
        int upper=0;
        for(int t:alive) upper=max(upper,predictions[t][j]);
        int k=fillLeaf(best[j],upper);
        counts.push_back(k);
        erase_if(alive,[&](int t){return predictions[t][j]!=k;});
    }
    collect();
    best.resize(counts.size());
    evidence.emplace_back(best,counts);
    for(int i:unknown) {
        parents[i].reset();
        for(int t:alive) parents[i].set(candidates[t][i]);
    }
    return true;
}

Leaves maskChoice(vector<int> pool, int divisor) {
    // Spread the sides of a split evenly over each still possible site.
    shuffle(pool.begin(),pool.end(),rng);
    vector<int> balance(n);
    Leaves s;
    for(int i:pool) {
        long long cost=0;
        for(int v=parents[i]._Find_first();v<n;v=parents[i]._Find_next(v))
            cost+=2*balance[v]+divisor-2;
        bool take=cost<0 || (cost==0 && (rng()&1));
        if(take) s.set(i);
        for(int v=parents[i]._Find_first();v<n;v=parents[i]._Find_next(v))
            balance[v]+=take?divisor-1:-1;
    }
    if(s.none()) s.set(pool.front());
    return s;
}
int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    m=answerInt();
    len.resize(m); startAt.resize(m);
    for(int i=0;i<m;++i) {
        startAt[i]=n;
        len[i]=fillLeaf(i,NMAX-(m-i-1)-n);
        for(int k=0;k<len[i];++k) owner.push_back(i),offsetAt.push_back(k);
        n+=len[i];
    }
    collect();
    physicalLen=len;
    parents.resize(m);
    Nodes old;
    for(int i=0;i<m;++i) {
        parents[i]=old;
        for(int k=0;k+1<len[i];++k) old.set(startAt[i]+k);
    }
    depthLo=len; depthHi.assign(m,n); depthHi[0]=len[0];
    allowedOrbit.resize(m);
    for(auto &b:allowedOrbit) b.set();
    int shapeCount=0, maskCount=0, previous=INT_MAX, slow=0;
    int singles=count(len.begin(),len.end(),1);
    while(true) {
        vector<int> unknown;
        int mass=0;
        for(int i=1;i<m;++i) if(parents[i].count()>1) {
            unknown.push_back(i);
            mass+=parents[i].count();
        }
        if(unknown.empty()) break;
        if(exactStep(unknown)) {propagate();continue;}
        if(previous!=INT_MAX && mass>0.78*previous) ++slow;
        else slow=0;
        previous=mass;
        if(reverseLen.empty() && observations.empty() && masks.empty() && unknown.size()>1 &&
           (m<=12 || singles<m/3)) {
            reverseQuery();compressTails();
            singles=count(len.begin(),len.end(),1);
            propagate();continue;
        }
        vector<int> unknownInternal;
        for(int i:unknown) if(len[i]>1) unknownInternal.push_back(i);
        if(shapeCount && singles && !unknownInternal.empty() && internalFront.empty() &&
           (slow || shapeCount>=2 || len[0]<=4)) {
            internalQuery();propagate();continue;
        }
        if(shapeCount && !unknownInternal.empty() && unknownInternal.size()<=2 && unknown.size()>4) {
            Leaves select;select.set(unknownInternal.back());
            subsetQuery(select);propagate();continue;
        }
        bool direct=(int)unknown.size()<= (shapeCount ? 1 : (m<=10?m:4));
        if(shapeCount>24 && slow>=2) direct=true;
        if (!direct && solveSatellites(unknown)) break;
        Nodes possible;
        bool satellitesOnly=true;
        for(int i:unknown) {possible|=parents[i];satellitesOnly&=len[i]==1;}
        bool smallSites=satellitesOnly && !degreeHi.empty();
        for(int v=possible._Find_first();smallSites && v<n;v=possible._Find_next(v))
            if(degreeHi[v]>1) smallSites=false;
        if(direct) {
            Leaves s; s.set(unknown.back());
            subsetQuery(s);
        } else if((smallSites && slow<2 && maskCount<16) || (shapeCount && singles>=12 && maskCount<7 &&
                  (slow>=1 || ((int)observations.size()==1 && singles>=m/3))
                  && (maskCount<=1 || shapeCount>=maskCount))) {
            Leaves s;
            vector<int> pool;
            for(int i:unknown) if(len[i]==1) pool.push_back(i);
            if(pool.size()<4) pool=unknown;
            s=maskChoice(pool,smallSites?2:(slow>=2?4:2));
            subsetQuery(s);
            ++maskCount;
        } else {
            int satColors=singles?min(singles, m<25?2:3):0;
            int headColors=(m-singles>=8?2:1);
            int tailColors=(m-singles>=4?2:1);
            if(shapeCount>=2 && slow && singles>20)
                satColors=min(singles,max(satColors,5));
            bool sparse=shapeCount>=2 && (slow || shapeCount%3==0);
            if(!shapeCount) {
                int changes=0;
                for(int i=1;i<m;++i) changes+=(len[i]==1)!=(len[i-1]==1);
                if(len[0]<=4 || changes*10>=9*(m-1)) {
                    satColors=min(satColors,1);headColors=1;tailColors=1;
                } else {
                    satColors=min(satColors,2);
                }
            }
            if(shapeCount>=8 && !unknownInternal.empty()) {
                // A distinct (head, tail) pair per path of a given length
                // completely identifies the remaining internal skeleton.
                vector<int> frequency(n+1);
                for(int l:len) if(l>1) ++frequency[l];
                int most=*max_element(frequency.begin(),frequency.end());
                headColors=max(1,(int)sqrt(most));
                tailColors=(most+headColors-1)/headColors;
                satColors=singles?1:0;
            }
            shapeQuery(satColors,headColors,tailColors,sparse);
            ++shapeCount;
        }
        propagate();
    }
    int physicalN=accumulate(physicalLen.begin(),physicalLen.end(),0);
    vector<int> pos(m),id(physicalN),par(physicalN-1);
    int next=m, at=0;
    for(int i=0;i<m;++i) {
        pos[i]=at;
        for(int d=0;d<physicalLen[i];++d,++at)
            id[at]=(d+1==physicalLen[i]?i:(at==0?physicalN-1:next++));
    }
    for(int i=0;i<m;++i) for(int d=0;d<physicalLen[i];++d) {
        int v=pos[i]+d;
        if(!v) continue;
        int up=v-1;
        if(!d) {
            int a=parents[i]._Find_first();
            up=pos[owner[a]]+offsetAt[a];
        }
        par[id[v]]=id[up];
    }
    cout << "! " << physicalN;
    for(int v:par) cout << ' ' << v;
    cout << endl;
}
