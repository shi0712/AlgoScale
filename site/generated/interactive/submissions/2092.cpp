#include <algorithm>
#include <array>
#include <cstdint>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <numeric>
#include <unordered_map>
#include <vector>
using namespace std;

constexpr int N = 500, K = 2 * N, W = (N + 63) / 64;
using Mask = array<uint64_t, W>;
int n, times, words, m;
int grid[N][N], rowOf[N*N+1], colOf[N*N+1], extraMax[N];
struct Diagonal {
    vector<int> values, rows;
    vector<Mask> prefix;
    Mask all{};
    int cachedLimit = -1, cachedRank = 0;
    const Mask& permitted(int limit) {
        if (limit != cachedLimit) {
            cachedLimit = limit;
            cachedRank = upper_bound(values.begin(), values.end(), limit) - values.begin();
        }
        return prefix[cachedRank];
    }
};
Diagonal diag[K];
int baseLower[N+1][K];

int ask(int length, int timeIndex) {
    cout << "? " << length << " " << timeIndex + 1 << endl;
    int value;
    if (!(cin >> value) || value < 1 || value > n*n) exit(0);
    return value;
}

void prepareGrid() {
    for (int d=0; d<times; ++d) {
        auto& a = diag[d];
        for (int r=max(d ? 1 : 0, d-n+1); r<=min(n-1,d); ++r)
            a.rows.push_back(r);
        sort(a.rows.begin(), a.rows.end(), [d](int r, int s) {
            return grid[r][d-r] < grid[s][d-s];
        });
        a.prefix.push_back({});
        for (int r : a.rows) {
            a.values.push_back(grid[r][d-r]);
            a.all[r/64] |= 1ULL << (r%64);
            a.prefix.push_back(a.all);
        }
    }
    for (int j=1; j<n; ++j) extraMax[j] = max(extraMax[j-1], grid[0][j]);
}

// Activate cells in increasing order. The longest active run on a diagonal
// gives exactly the snake lengths that could have a maximum this small.
void prepareLower() {
    static int run[N][N], longest[K], queueCells[N*N*2];
    int prefix = 0;
    for (int v=1; v<=n*n; ++v) {
        int r=rowOf[v], c=colOf[v];
        if (r==0) {
            if (c!=prefix) continue;
            while (prefix<n && grid[0][prefix]<=v) ++prefix;
            c=0;
        }
        int begin=0, end=0;
        queueCells[end++]=r*n+c;
        while (begin<end) {
            int cell=queueCells[begin++], x=cell/n, y=cell%n;
            int len=prefix;
            if (x) {
                len=min(n,1+max(run[x-1][y], y ? run[x][y-1] : 0));
            }
            if (len<=run[x][y]) continue;
            run[x][y]=len;
            int d=x+y;
            while (longest[d]<len) baseLower[++longest[d]][d]=v;
            if (x+1<n && grid[x+1][y]<=v) queueCells[end++]=cell+n;
            if (x && y+1<n && grid[x][y+1]<=v) queueCells[end++]=cell+1;
        }
    }
}

bool hasRow(const Mask& a, int r) {
    return (a[r/64] >> (r%64)) & 1;
}
int votes[K][N], mostVotes[K], popular[K], samples[K];
bool reliable(int t) {
    return mostVotes[t]>=2 && mostVotes[t]*3>=samples[t]*2;
}
int direction=0;

struct Snake {
    int length=0;
    vector<Mask> domain;
    vector<int> minRank,maxRank,headLo,headHi,lower,upper;
    void init(int l) {
        length=l;
        domain.resize(times);
        minRank.resize(times); maxRank.resize(times);
        headLo.resize(times); headHi.resize(times);
        lower.assign(baseLower[l],baseLower[l]+times);
        upper.assign(times,n*n);
        for (int t=0;t<times;++t) {
            const auto& a=diag[t];
            domain[t]=a.all;
            maxRank[t]=(int)a.rows.size()-1;
            headLo[t]=a.values.front();
            headHi[t]=a.values.back();
        }
    }
    void refresh(int t) {
        const auto& a=diag[t];
        int &lo=minRank[t], &hi=maxRank[t];
        while (lo<=hi && !hasRow(domain[t],a.rows[lo])) ++lo;
        if (lo>hi) exit(0);
        while (!hasRow(domain[t],a.rows[hi])) --hi;
        if (lo==hi && headLo[t]!=headHi[t]) {
            int r=a.rows[lo];
            ++samples[t];
            if (++votes[t][r]>mostVotes[t]) {
                mostVotes[t]=votes[t][r];
                popular[t]=r;
            }
        }
        headLo[t]=a.values[lo];
        headHi[t]=a.values[hi];
    }
    template<bool forward>
    bool propagate(int t) {
        const auto& from=domain[t+(forward ? -1 : 1)];
        auto& to=domain[t];
        uint64_t diff=0;
        for (int w=0;w<words;++w) {
            uint64_t shifted;
            if constexpr (forward)
                shifted=(from[w]<<1) | (w ? from[w-1]>>63 : 0);
            else
                shifted=(from[w]>>1) | (w+1<words ? from[w+1]<<63 : 0);
            uint64_t next=to[w] & (from[w] | shifted);
            diff |= next ^ to[w];
            to[w]=next;
        }
        if (diff) refresh(t);
        return diff!=0;
    }
    void observe(int t,int v) {
        // The value identifies the peak cell. Negative positions refer to
        // the fixed initial body to the right of the starting head.
        int r=rowOf[v], c=colOf[v], p=r ? r+c : -c;
        int first=times,last=-1;
        for (int d=max(0,t-length+1); d<=t; ++d) {
            if (d==p && headLo[d]!=headHi[d]) {
                domain[d]={};
                domain[d][r/64]=1ULL << (r%64);
            } else if (headHi[d]>v) {
                const auto& mask=diag[d].permitted(v);
                for (int w=0;w<words;++w) domain[d][w] &= mask[w];
            } else continue;
            refresh(d);
            first=min(first,d);
            last=d;
        }
        for (int d=first+1;d<times;++d)
            if (!propagate<true>(d) && d>last) break;
        for (int d=last-1;d>=0;--d)
            if (!propagate<false>(d) && d<first) break;
    }
    // Every head cell in the current domain extends to a complete trajectory.
    // Intersect a short segment with the proposed threshold to test whether
    // the whole segment could be traversed below that threshold.
    int firstFailure(int start,int end,int limit,bool backward) {
        Mask reach{};
        for (int d=start;;d+=backward ? -1 : 1) {
            const auto& mask=diag[d].permitted(limit);
            uint64_t any=0,carry=0;
            for (int i=0;i<words;++i) {
                int w=backward ? words-1-i : i;
                uint64_t old=reach[w];
                uint64_t step=backward ? (old>>1) : (old<<1);
                uint64_t cells=domain[d][w] & mask[w];
                if (d!=start) cells &= old | step | carry;
                carry=backward ? (old<<63) : (old>>63);
                any |= reach[w]=cells;
            }
            if (!any) return d;
            if (d==end) return -1;
        }
    }
    bool feasible(int t,int limit) {
        int start=max(0,t-length+1);
        int d=firstFailure(start,t,limit,false);
        if (d<0) return true;
        int a=firstFailure(d,start,limit,true);
        for (int i=d;i<times && i<a+length;++i)
            lower[i]=max(lower[i],limit+1);
        return false;
    }
};

vector<Snake> snakes;
vector<int> upperCount;
int ceilingValue, countThroughCeiling;
void recordUpper(int old,int value) {
    if (old==value) return;
    --upperCount[old]; ++upperCount[value];
    if (old>ceilingValue && value<=ceilingValue) ++countThroughCeiling;
}
void tightenCeiling() {
    while (countThroughCeiling-upperCount[ceilingValue]>=m) {
        countThroughCeiling-=upperCount[ceilingValue--];
    }
}

int loQueue[K],hiQueue[K];
void updateBounds(Snake& s) {
    if (direction) {
        for (int t=0;t<times;++t) {
            int p=direction>0 ? t : max(0,t-s.length+1);
            int initial=extraMax[max(0,s.length-t-1)];
            s.lower[t]=max(initial,s.headLo[p]);
            int value=max(initial,s.headHi[p]);
            recordUpper(s.upper[t],value);
            s.upper[t]=value;
        }
        tightenCeiling();
        return;
    }
    int a=0,b=0,c=0,e=0,l=s.length;
    for (int t=0;t<times;++t) {
        if (a<b && loQueue[a]<=t-l) ++a;
        if (c<e && hiQueue[c]<=t-l) ++c;
        while (a<b && s.headLo[loQueue[b-1]]<=s.headLo[t]) --b;
        while (c<e && s.headHi[hiQueue[e-1]]<=s.headHi[t]) --e;
        loQueue[b++]=hiQueue[e++]=t;
        int initial=t<l ? extraMax[l-t-1] : 0;
        s.lower[t]=max({s.lower[t],initial,s.headLo[loQueue[a]]});
        int value=max(initial,s.headHi[hiQueue[c]]);
        recordUpper(s.upper[t],value);
        s.upper[t]=value;
    }
    tightenCeiling();
}

// A sampled feasible path predicts which location a query might reveal.
// It is used only to choose probes. All stopping tests use proved bounds.
uint32_t randomState=17263541;
uint32_t randomWord() {
    randomState ^= randomState << 13;
    randomState ^= randomState >> 17;
    randomState ^= randomState << 5;
    return randomState;
}
void model(const Snake& s,int path[]) {
    int r=0, next=0, goal=0;
    for (int t=0;t<times;++t) {
        if (t>next) {
            next=t;
            while (s.headLo[next]!=s.headHi[next]) ++next;
            goal=rowOf[s.headLo[next]];
        }
        if (t) {
            bool right=hasRow(s.domain[t],r);
            bool down=r<goal && hasRow(s.domain[t],r+1);
            bool takeDown=false;
            if (down) {
                if (reliable(t)) takeDown=r<popular[t];
                else takeDown=(randomWord()*(uint64_t)(next-t+1)>>32)<(unsigned)(goal-r);
            }
            if (!right || takeDown) ++r;
        }
        path[t]=grid[r][t-r];
    }
}
// On an increasing path the head is the only moving candidate for the
// maximum. On a decreasing path the oldest path cell is the candidate.
int chooseMonotone(const Snake& s,int limit) {
    static int relevant[K],prefix[K+1];
    int shift=direction>0 ? 0 : s.length-1, end=times-shift;
    for (int p=0;p<times;++p) {
        int t=p+shift;
        relevant[p]=p<end && s.lower[t]<=limit && s.lower[t]<s.upper[t];
        prefix[p+1]=prefix[p]+relevant[p];
    }
    if (!prefix[times]) return -1;
    int left=0,right=0,best=-1;
    long long bestScore=-1;
    for (int p=1;p<end;++p) {
        if (p>=right) {
            left=right;
            right=p;
            while (s.headLo[right]!=s.headHi[right]) ++right;
        }
        if (p==right) continue;
        int t=p+shift;
        if (s.lower[t]==s.upper[t]) continue;
        int before=prefix[p]-prefix[left];
        int after=prefix[right]-prefix[p+1];
        int middle=min(before,after);
        int straight=0;
        if (reliable(p) && hasRow(s.domain[p],popular[p])) {
            int r=popular[p], rl=rowOf[s.headLo[left]], rr=rowOf[s.headLo[right]];
            int initial=extraMax[max(0,s.length-t-1)];
            if (grid[r][p-r]>initial) {
                if (r==rl || r-rl==p-left) straight+=before;
                if (r==rr || rr-r==right-p) straight+=after;
            }
        }
        if (!relevant[p] && straight<3) continue;
        long long score=(relevant[p] ? 1024LL*(middle+1) : 0)
                      + (relevant[p] ? 8192LL : 4096LL)*straight
                      + min(before+after,1023);
        if (score>bestScore) bestScore=score,best=t;
    }
    return best;
}

// For a one-cell snake each probe is a waypoint. Along a proposed path,
// the times whose waypoints exclude a particular cell form one interval.
// Intersect those intervals for all low cells on an unresolved diagonal.
int chooseSparse(const Snake& s,int limit) {
    int uncertain=0;
    for(int t=0;t<times;++t) uncertain+=s.lower[t]<=limit && s.lower[t]!=s.upper[t];
    if(!uncertain) return -1;
    int gains[K+1]{}, path[K], firstR[N],lastR[N],firstC[N],lastC[N];
    for(int sample=0;sample<4;++sample) {
        model(s,path);
        for(int t=0;t<times;++t) {
            int r=rowOf[path[t]],c=t-r;
            lastR[r]=lastC[c]=t;
        }
        for(int t=times-1;t>=0;--t) {
            int r=rowOf[path[t]],c=t-r;
            firstR[r]=firstC[c]=t;
        }
        for(int t=0;t<times;++t) {
            if(s.lower[t]>limit || s.lower[t]==s.upper[t]) continue;
            int a=t,b=t;
            if(path[t]>limit) {
                const auto& low=diag[t].permitted(limit);
                a=0; b=times-1;
                for(int w=0;w<words;++w) {
                    uint64_t cells=low[w]&s.domain[t][w];
                    while(cells) {
                        int r=w*64+__builtin_ctzll(cells),c=t-r;
                        a=max(a,1+min(lastR[r],lastC[c]));
                        b=min(b,max(firstR[r],firstC[c])-1);
                        cells&=cells-1;
                    }
                }
            }
            ++gains[a]; --gains[b+1];
        }
    }
    int best=-1,total=0,scoreBest=-1;
    for(int t=0;t<times;++t) {
        total+=gains[t];
        if(s.lower[t]==s.upper[t]) continue;
        bool active=s.lower[t]<=limit;
        if (!active && total<8) continue;
        int score=total*16+(active ? 16 : 0);
        if(score>scoreBest) best=t,scoreBest=score;
    }
    return best;
}

int choose(const Snake& s,int limit) {
    static int active[K], leftGain[K], pref[K+1], winner[K], value[K], path[K];
    static int used[K], count[K+1], balance[K], straight[K], q[K];
    int l=s.length;
    for (int t=0;t<times;++t) {
        active[t]=s.lower[t]<=limit && s.lower[t]<s.upper[t];
        pref[t+1]=pref[t]+active[t];
    }
    if (!pref[times]) return -1;
    model(s,path);
    fill(used,used+times,0);
    int a=0,b=0;
    for (int t=0;t<times;++t) {
        if (a<b && q[a]<=t-l) ++a;
        while (a<b && path[q[b-1]]<=path[t]) --b;
        q[b++]=t;
        int p=q[a],v=path[p],tail=extraMax[max(0,l-t-1)];
        if (tail>v) v=tail,p=-colOf[v];
        winner[t]=p;
        value[t]=v;
        if (p>=0 && active[t]) used[p]=1;
    }
    int total=0,anchor=0;
    for (int t=0;t<times;++t) {
        if (s.headLo[t]==s.headHi[t]) total=0,anchor=t;
        balance[t]=total;
        int delta=rowOf[path[t]]-rowOf[path[anchor]];
        straight[t]=(delta==0 || delta==t-anchor) ? total : 0;
        total+=used[t];
    }
    total=0;
    anchor=times-1;
    for (int t=times-1;t>=0;--t) {
        if (s.headLo[t]==s.headHi[t]) total=0,anchor=t;
        balance[t]=min(balance[t],total);
        int delta=rowOf[path[anchor]]-rowOf[path[t]];
        if (delta==0 || delta==anchor-t) straight[t]+=total;
        total+=used[t];
    }
    for (int t=0;t<times;++t) {
        leftGain[t]=active[t];
        if (t && winner[t-1]==winner[t]) {
            int outside=t-l<0 ? grid[0][l-t] : s.headHi[t-l];
            if (outside<=value[t]) leftGain[t]+=leftGain[t-1];
        }
        count[t+1]=count[t]+used[t];
    }
    int best=-1,rightGain=0;
    long long bestScore=-1;
    for (int t=times-1;t>=0;--t) {
        if (t+1==times || winner[t]!=winner[t+1] || s.headHi[t+1]>value[t]) rightGain=0;
        rightGain+=active[t];
        int p=winner[t];
        bool unknown=p>=0 && s.headLo[p]!=s.headHi[p];
        if (!active[t] && !(unknown && straight[p]>=4 && reliable(p)
                           && popular[p]==rowOf[path[p]] && s.lower[t]<s.upper[t])) continue;
        int cover=pref[min(times,p+l)]-pref[max(0,p)];
        int direct=leftGain[t]+rightGain-active[t];
        if (value[t]>limit) direct=cover;
        int geometry=unknown ? balance[p]+6*straight[p] : 0;
        // Balance informative positions between consecutive known waypoints.
        // Among windows covering the same peak prefer an informative cap.
        int nearby=count[t+1]-count[max(0,t-l+1)];
        long long score=(12LL*direct+cover+3LL*geometry+nearby)*4096
                       +min(pref[t],pref[times]-pref[t+1]);
        if (!active[t]) score=score*2/3;
        if (score>bestScore) bestScore=score,best=t;
    }
    return best;
}

void solveAt(Snake& s,int target) {
    while (true) {
        int limit=min(target,ceilingValue-1);
        int t;
        if(direction) t=chooseMonotone(s,limit);
        else if(s.length==1 && m<=128) t=chooseSparse(s,limit);
        else t=choose(s,limit);
        if (t<0) return;
        if (!direction && s.lower[t]<=limit && s.upper[t]>limit && !s.feasible(t,limit)) continue;
        s.observe(t,ask(s.length,t));
        updateBounds(s);
    }
}

int orderStatistic(const vector<int>& counts) {
    int k=m;
    for (int v=1;v<=n*n;++v) if ((k-=counts[v])<=0) return v;
    return n*n;
}
int estimatedCutoff() {
    vector<int> cnt(n*n+1);
    int path[K], q[K];
    for (int l=1;l<=n;++l) {
        auto& s=snakes[l];
        model(s,path);
        int a=0,b=0;
        for (int t=0;t<times;++t) {
            if (a<b && q[a]<=t-l) ++a;
            while (a<b && path[q[b-1]]<=path[t]) --b;
            q[b++]=t;
            int v=max(extraMax[max(0,l-1-t)],path[q[a]]);
            ++cnt[v];
        }
    }
    return orderStatistic(cnt);
}
// For small boards all paths fit in memory. Work with sorted per-path
// answers, so paths that contribute the same multiset need not be separated.
namespace small {
constexpr int M=15; // 2*8-1
using Line=array<int,M>;
struct Path { Line value{}, sorted{}; };
struct Group {
    vector<Path> paths;
    Line lo{},hi{};
    void bounds() {
        lo.fill(n*n); hi.fill(0);
        for (const auto& p:paths) for (int i=0;i<times;++i) {
            lo[i]=min(lo[i],p.sorted[i]);
            hi[i]=max(hi[i],p.sorted[i]);
        }
    }
};
// With at most 64 candidate worlds the full decision tree is small enough
// to optimize the expected number of further queries exactly.
struct Endgame {
    using Bits=uint64_t;
    vector<vector<Bits>> partitions;
    vector<pair<int,int>> queries;
    vector<Bits> equivalent;
    unordered_map<Bits,int> memo;
    Bits all;
    pair<int,int> best;
    int cost(Bits set) {
        if ((set & ~equivalent[__builtin_ctzll(set)])==0) return 0;
        if (auto it=memo.find(set);it!=memo.end()) return it->second;
        int optimal=1000000;
        for (int j=0;j<(int)queries.size();++j) {
            int sum=__builtin_popcountll(set);
            for (Bits mask:partitions[j]) {
                Bits child=mask&set;
                if (child==set) {sum=optimal;break;}
                if (child) sum+=cost(child);
                if (sum>=optimal) break;
            }
            if (sum<optimal) {
                optimal=sum;
                if(set==all) best=queries[j];
            }
        }
        return memo[set]=optimal;
    }
    void add(int k,int t,const array<Bits,65>& split) {
        vector<Bits> masks;
        for(Bits mask:split) if(mask) masks.push_back(mask);
        if(masks.size()<2) return;
        sort(masks.begin(),masks.end());
        if(find(partitions.begin(),partitions.end(),masks)!=partitions.end()) return;
        queries.emplace_back(k,t);
        partitions.push_back(move(masks));
    }
    template<class Signature>
    pair<int,int> solve(const vector<Signature>& outputs) {
        int worlds=outputs.size();
        all=worlds==64 ? ~Bits(0) : (Bits(1)<<worlds)-1;
        equivalent.resize(worlds);
        for(int i=0;i<worlds;++i) for(int j=0;j<worlds;++j)
            if(outputs[i]==outputs[j]) equivalent[i] |= Bits(1)<<j;
        best={-1,-1};
        cost(all);
        return best;
    }
};
pair<int,int> globalPlan(const vector<Group>& groups,int worlds) {
    Endgame game;
    vector<vector<int>> outputs(worlds);
    int stride=1;
    for(int k=0;k<n;++k) {
        const auto& paths=groups[k].paths;
        for (int t=0;t<times;++t) {
            array<uint64_t,65> split{};
            for(int id=0;id<worlds;++id) {
                int v=paths[id/stride%paths.size()].value[t];
                outputs[id].push_back(v);
                split[v] |= uint64_t(1)<<id;
            }
            game.add(k,t,split);
        }
        stride*=paths.size();
    }
    for(auto& out:outputs) { sort(out.begin(),out.end()); out.resize(m); }
    return game.solve(outputs);
}
pair<int,int> selectQuery(const vector<Group>& groups,const vector<int>& low,const vector<int>& high) {
    array<double,65> expected{};
    for(const auto& g:groups) {
        double weight=1.0/g.paths.size();
        for(const auto& p:g.paths) for(int i=0;i<min(m,times);++i)
            expected[p.sorted[i]]+=weight;
    }
    double need=m;
    int target=1;
    while(target<n*n && need>expected[target]+1e-9) need-=expected[target++];
    target=min(target,high[m-1]-1);
    int length=0;
    double priority=-1;
    // Count how often a snake could change an early order statistic.
    for(int k=0;k<n;++k) {
        const auto& g=groups[k];
        double importance=0;
        bool undecided=false;
        for(int i=0;i<min(m,times);++i) {
            if(g.lo[i]==g.hi[i] || g.lo[i]>=high[m-1]) continue;
            undecided=true;
            if(g.lo[i]>target) continue;
            int pos=lower_bound(low.begin(),low.begin()+m,g.lo[i])-low.begin();
            importance+=1.0/(2+pos);
        }
        if(undecided && importance>priority) priority=importance,length=k;
    }
    const auto& g=groups[length];
    vector<int> background;
    for(int k=0;k<n;++k) if(k!=length)
        background.insert(background.end(),groups[k].hi.begin(),groups[k].hi.begin()+times);
    sort(background.begin(),background.end());
    Line cutoff{};
    for(int i=0;i<min(m,times);++i) {
        int j=m-1-i;
        cutoff[i]=j<(int)background.size() ? background[j] : n*n+1;
    }
    for(int& v:cutoff) v=min(v,target+1);
    vector<Line> signatures;
    for(const auto& p:g.paths) {
        Line key{};
        for(int i=0;i<min(m,times);++i) key[i]=min(cutoff[i],p.sorted[i]);
        signatures.push_back(key);
    }
    vector<int> order(signatures.size()),labels(order.size());
    iota(order.begin(),order.end(),0);
    sort(order.begin(),order.end(),[&](int a,int b){return signatures[a]<signatures[b];});
    int label=0;
    for(int i=0;i<(int)order.size();++i) {
        if(i && signatures[order[i]]!=signatures[order[i-1]]) ++label;
        labels[order[i]]=label;
    }
    int query=-1;
    if(g.paths.size()<=64) {
        Endgame game;
        for(int t=0;t<times;++t) {
            array<uint64_t,65> split{};
            for(int i=0;i<(int)g.paths.size();++i)
                split[g.paths[i].value[t]] |= uint64_t(1)<<i;
            game.add(length,t,split);
        }
        query=game.solve(labels).second;
    } else {
        // Maximize mutual information between the response and the
        // contribution to the answer, assuming equiprobable paths.
        vector<double> xlog(order.size()+1);
        for(int i=1;i<(int)xlog.size();++i) xlog[i]=i*log(double(i));
        double best=-1e100;
        for(int t=0;t<times;++t) {
            array<int,65> counts{},joint{};
            for(int id:order) ++counts[g.paths[id].value[t]];
            double score=0;
            for(int v:counts) score-=xlog[v];
            int current=-1;
            joint.fill(0);
            for(int id:order) {
                if(labels[id]!=current) joint.fill(0),current=labels[id];
                int v=g.paths[id].value[t];
                score+=xlog[joint[v]+1]-xlog[joint[v]];
                ++joint[v];
            }
            if(*max_element(counts.begin(),counts.end())<(int)order.size() && score>best)
                query=t,best=score;
        }
    }
    if(query<0) {
        // A rank can be immaterial at the pessimistic background when
        // all values below it are tied. Split any remaining ambiguity.
        for(int t=0;t<times && query<0;++t)
            for(const auto& p:g.paths) if(p.value[t]!=g.paths[0].value[t]) { query=t; break; }
    }
    return {length,query};
}
void solve() {
    vector<Group> groups(n);
    vector<int> moves(2*n-3,0);
    fill(moves.end()-(n-2),moves.end(),1);
    do {
        int r=1,c=0;
        Line heads{};
        heads[0]=grid[0][0]; heads[1]=grid[1][0];
        for(int t=2;t<times;++t) {
            r+=moves[t-2]; c+=1-moves[t-2];
            heads[t]=grid[r][c];
        }
        for(int l=1;l<=n;++l) {
            Path path;
            for(int t=0;t<times;++t)
                path.value[t]=max(extraMax[max(0,l-1-t)],
                                  *max_element(heads.begin()+max(0,t-l+1),heads.begin()+t+1));
            path.sorted=path.value;
            fill(path.sorted.begin()+times,path.sorted.end(),n*n+1);
            sort(path.sorted.begin(),path.sorted.end());
            groups[l-1].paths.push_back(path);
        }
    } while(next_permutation(moves.begin(),moves.end()));
    for(auto& g:groups) g.bounds();
    for(;;) {
        vector<int> low,high;
        for(auto& g:groups) {
            low.insert(low.end(),g.lo.begin(),g.lo.begin()+times);
            high.insert(high.end(),g.hi.begin(),g.hi.begin()+times);
        }
        sort(low.begin(),low.end()); sort(high.begin(),high.end());
        if(equal(low.begin(),low.begin()+m,high.begin())) {
            cout<<"!";
            for(int i=0;i<m;++i) cout<<" "<<low[i];
            cout<<endl;
            return;
        }
        int worlds=1;
        for (const auto& g:groups) {
            worlds*=g.paths.size();
            if(worlds>64) break;
        }
        auto [length,query]=worlds<=64 ? globalPlan(groups,worlds)
                                      : selectQuery(groups,low,high);
        auto& g=groups[length];
        int v=ask(length+1,query);
        erase_if(g.paths,[&](const Path& p){return p.value[query]!=v;});
        if(g.paths.empty()) exit(0);
        g.bounds();
    }
}
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    if (!(cin>>n>>m)) return 0;
    times=2*n-1; words=(n+63)/64;
    for(int r=0;r<n;++r) for(int c=0;c<n;++c) {
        int v; if (!(cin>>v)) return 0;
        grid[r][c]=v; rowOf[v]=r; colOf[v]=c;
    }
    bool inc=true,dec=true;
    for (int r=0;r<n;++r) for (int c=0;c<n;++c) {
        if (!r && c) continue;
        auto edge=[&](int v) {
            inc &= grid[r][c]<v;
            dec &= grid[r][c]>v;
        };
        if (r+1<n) edge(grid[r+1][c]);
        if (r && c+1<n) edge(grid[r][c+1]);
    }
    direction=inc ? 1 : (dec ? -1 : 0);
    prepareGrid();
    if (n<=8) { small::solve(); return 0; }
    if (!direction) prepareLower();
    upperCount.resize(n*n+1);
    ceilingValue=n*n; countThroughCeiling=upperCount[n*n]=n*times;
    snakes.resize(n+1);
    for (int l=1;l<=n;++l) {
        snakes[l].init(l);
        updateBounds(snakes[l]);
    }
    randomState+=n;
    int previous=0;
    for (int pass=0;;++pass) {
        vector<int> cnt(n*n+1);
        bool done=true;
        for(int l=1;l<=n;++l) {
            const auto& s=snakes[l];
            for(int t=0;t<times;++t) {
                ++cnt[s.lower[t]];
                if(s.lower[t]<ceilingValue && s.lower[t]!=s.upper[t]) done=false;
            }
        }
        // At least m values are at most ceilingValue. Every value strictly
        // below it is now known exactly, including its multiplicity.
        if (done) {
            cout<<"!";
            int k=m;
            for(int v=1;k;++v)
                for(int i=0;i<cnt[v] && k;++i,--k) cout<<" "<<v;
            cout<<endl;
            return 0;
        }
        int low=orderStatistic(cnt), target=ceilingValue-1;
        if (low<target && pass<8) {
            if (pass==0 || (m<=8 && pass<4)) target=low;
            else {
                int estimate=estimatedCutoff();
                int step=max(0,estimate-low);
                if (pass==1 || (m<=8 && pass==4)) step/=2;
                target=min(target,low+step);
                if(pass>=6) target=max(target,previous+(ceilingValue-1-previous)/4);
            }
        }
        for(int l=1;l<=n;++l) solveAt(snakes[l],target);
        previous=target;
    }
}
