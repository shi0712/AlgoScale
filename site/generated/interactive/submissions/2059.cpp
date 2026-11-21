#include <bits/stdc++.h>
using namespace std;

struct Pool {
    vector<int> pos;
    int ones;
};
int n, total;
string fixedSheet, orientation;

string reorient(string q,const string& pivot) {
    for(int i=0;i<n;i++)if(pivot[i]=='T')q[i]=(q[i]=='F'?'T':'F');
    return q;
}
struct SparsePivot { string sheet; int distance; };
vector<Pool> pools;
mt19937 rng(76438913);

int submit(const string &q) {
    string physical=orientation.empty()?q:reorient(q,orientation);
    cout << physical << endl;
    int r;
    if (!(cin >> r) || r < 0 || r >= n) exit(0);
    if (r == 0) {
        string other=physical;
        for (char &c:other) c=(c=='F'?'T':'F');
        cout << other << endl;
        cin >> r;
        exit(0);
    }
    return r;
}

struct Pattern {string sheet; bool alive=true;};
vector<Pattern> patterns;
map<string,int> history;
int speculative=0, simpleCount=0;

void makePatterns() {
    if (n<25 || min(total,n-total)<2) return;
    unordered_set<string> seen;
    auto add=[&](string q) {
        if (count(q.begin(),q.end(),'T')==total && seen.insert(q).second)
            patterns.push_back({move(q),true});
    };
    string alternating(n,'F');
    for(int i=1;i<n;i+=2)alternating[i]='T';
    add(alternating);
    for(char &c:alternating)c=c=='F'?'T':'F';
    add(alternating);
    add(string(total,'T')+string(n-total,'F'));
    add(string(n-total,'F')+string(total,'T'));
    simpleCount=patterns.size();
    for (int p=1;p<=10 && 2*p<=n;p++)
        for (int bits=0;bits<(1<<p);bits++) {
            string q(n,'F');
            for(int i=0;i<n;i++) if(bits>>(i%p)&1) q[i]='T';
            add(move(q));
        }
    for(int p=11;p<=min(64,n/3);p++) {
        int whole=n/p,tail=n%p;
        for(int k=max(1,(total-tail+whole-1)/whole);k<=min(p-1,total/whole);k++)
            for(int shift=0;shift<p;shift++) {
                string q(n,'F');
                for(int i=0;i<n;i++)if((i+shift)%p<k)q[i]='T';
                add(move(q));
            }
    }
    for(int start=0;start<n;start++) {
        string q(n,'F');
        for(int j=0;j<total;j++) q[(j+start)%n]='T';
        add(move(q));
    }
}

void remember(const string &q,int r) {
    history.emplace(q,r);
    string inv=q;
    for(char &c:inv)c=c=='F'?'T':'F';
    history.emplace(inv,n-r);
    int distance=min(r,n-r);
    if(orientation.empty() && n>=40 && history.size()<=24 &&
       distance<=min(18,n/15) && 4*distance<min(total,n-total))
        throw SparsePivot{r<n-r?inv:q,distance};
    for(auto &p:patterns) if(p.alive) {
        int d=inner_product(q.begin(),q.end(),p.sheet.begin(),0,plus<>(),not_equal_to<>());
        if(d!=n-r)p.alive=false;
    }
}

int query(const string &q) {
    auto it=history.find(q);
    if(it!=history.end()) return it->second;
    int r=submit(q);
    remember(q,r);
    if(speculative<2 && history.size()>=6) {
        int survivors=0,first=-1;
        for(int i=0;i<(int)patterns.size();i++) if(patterns[i].alive) {
            if(first<0)first=i;
            if(++survivors>1)break;
        }
        if(survivors==1 || (first>=0 && first<simpleCount && speculative==0)) {
            ++speculative;
            const string &guess=patterns[first].sheet;
            int a=submit(guess);
            remember(guess,a);
        }
    }
    return r;
}

int dotQuery(const string &q) {
    int r=query(q);
    return (r-n+total+count(q.begin(),q.end(),'T'))/2;
}
void keep(vector<int> pos,int c) {
    if(c==0 || c==(int)pos.size()) {
        for(int i:pos)fixedSheet[i]=c?'T':'F';
    } else pools.push_back({move(pos),c});
}

int rankOf(int s) {return __builtin_popcount((unsigned)s);}
int capacity(int m) {
    int cap=1;
    for(int i=1;i<m;i++) cap+=rankOf(i);
    return cap;
}

// For every nonempty S, the alternating sum on the subcube of S
// annihilates a column depending on T unless T contains S. Subtracting
// already decoded columns in decreasing label order isolates the S block.
struct Batch {
    int m;
    vector<string> sheets;
    vector<int> residual;
    explicit Batch(int rows):m(rows),sheets(rows,fixedSheet),residual(rows){}

    vector<unsigned char> column(int s,int coeff,const vector<int>& positions) {
        vector<unsigned char> table(s+1);
        if(s==0)table[0]=coeff;
        else {
            int need=abs(coeff);
            for(int t=0;t<=s && need;t++) {
                if((t&s)==t && (rankOf(t)%2)==(coeff>0)) {
                    table[t]=1;--need;
                }
            }
        }
        for(int r=0;r<m;r++)for(int j:positions)sheets[r][j]=table[r&s]?'T':'F';
        return table;
    }
    void read() {
        int fixedT=count(fixedSheet.begin(),fixedSheet.end(),'T');
        for(int r=0;r<m;r++)residual[r]=dotQuery(sheets[r])-fixedT;
    }
    int value(int s) const {
        int result=0;
        for(int t=s;;t=(t-1)&s) {
            result+=(!s || rankOf(t)%2)?residual[t]:-residual[t];
            if(!t)return result;
        }
    }
    void subtract(int s,const vector<unsigned char>& table,int ones) {
        for(int r=0;r<m;r++)residual[r]-=table[r&s]*ones;
    }
};
struct Part {
    vector<int> pos;
    vector<unsigned char> table;
};

// Each nonempty label carries |S|-1 low binary digits and a pool count.
// The initial all-F score supplies the count of the omitted pool.
void poolStage(vector<int> pos,int ones,int m) {
    int len=pos.size(),freeCount=len-(capacity(m)-m),at=0;
    Batch batch(m);
    vector<vector<Part>> parts(m);
    for(int s=1;s<m;s++) {
        int d=rankOf(s);
        for(int j=0;j<d;j++) {
            int k=j+1==d?freeCount/m+(s<freeCount%m):1;
            if(!k)continue;
            vector<int> indices(pos.begin()+at,pos.begin()+at+k);at+=k;
            auto table=batch.column(s,1<<j,indices);
            parts[s].push_back({move(indices),move(table)});
        }
    }
    batch.read();
    for(int s=m-1;s>0;s--) {
        int v=batch.value(s);
        for(int j=0;j<(int)parts[s].size();j++) {
            auto &part=parts[s][j];
            int x=j+1==(int)parts[s].size()?v:v%2;
            v/=2;
            batch.subtract(s,part.table,x);
            ones-=x;
            keep(move(part.pos),x);
        }
    }
    keep(vector<int>(pos.begin()+at,pos.end()),ones);
}
long double logChoose(int g,int c) {
    c=min(c,g-c);
    long double a=0;
    for(int i=1;i<=c;i++)a+=log2l((g-i+1.0L)/i);
    return a;
}
int coarseWays(int g,int h,int b,int limit) {
    vector<int> dp(h+1);dp[0]=1;
    for(int i=0;i<b;i++) {
        int size=g/b+(i<g%b);
        vector<int> next(h+1);
        for(int j=0;j<=h;j++)for(int x=0;x<=min(j,size);x++)
            next[j]=min(limit+1,next[j]+dp[j-x]);
        dp=move(next);
    }
    return dp[h];
}

void oneError() {
    auto &p=pools[0];
    int g=p.pos.size(),minor=p.ones==1?1:0;
    string base=fixedSheet;
    for(int i:p.pos)base[i]=minor?'F':'T';
    vector<int> possible=p.pos;
    for(const auto &[q,r]:history) {
        int score=inner_product(q.begin(),q.end(),base.begin(),0,plus<>(),equal_to<>());
        const bool mustMatch=r<score;
        possible.erase(remove_if(possible.begin(),possible.end(),
                       [&](int i){return (base[i]==q[i])!=mustMatch;}),possible.end());
    }
    if(possible.size()!=p.pos.size()) {
        for(int i:p.pos)fixedSheet[i]=base[i];
        for(int i:possible)fixedSheet[i]='F';
        pools.clear();
        int c=minor?1:(int)possible.size()-1;
        keep(move(possible),c);
        return;
    }
    static vector<int> cost(1001),depth(1001),split(1001);
    if(!cost[1]) {
        cost[1]=depth[1]=1;
        for(int i=2;i<=1000;i++) {
            cost[i]=i+cost[i-1];depth[i]=1+depth[i-1];split[i]=1;
            for(int j=2;j<=i/2;j++) {
                int a=i+cost[j]+cost[i-j],b=1+max(depth[j],depth[i-j]);
                if(pair{a,b}<pair{cost[i],depth[i]})cost[i]=a,depth[i]=b,split[i]=j;
            }
        }
    }
    string q=fixedSheet;
    for(int i:p.pos)q[i]=minor?'F':'T';
    int k=split[g];
    for(int i=0;i<k;i++)q[p.pos[i]]=minor?'T':'F';
    int score=query(q);
    bool hit=score==n-1-k+2;
    vector<int> left(p.pos.begin(),p.pos.begin()+k),right(p.pos.begin()+k,p.pos.end());
    pools.clear();
    keep(move(left),minor?hit:k-hit);
    keep(move(right),minor?!hit:g-k-!hit);
}

using Mask=uint64_t;
struct Candidate {
    array<Mask,10> bits{};
    double mass=1;
};
struct Cell {vector<int> pos;bool inverted;};

// For small posterior sets, minimize the total remaining path length and then
// the longest path. A correct guess has no child interaction.
Mask smallOptimal(const vector<Candidate>& keys,int k,int errors) {
    struct Split { Mask query; vector<Mask> children; };
    vector<Split> splits;
    Mask all=(Mask(1)<<keys.size())-1;
    for(Mask q=0;q<(Mask(1)<<k);q++) {
        vector<Mask> parts(errors+1);
        for(unsigned j=0;j<keys.size();j++) {
            Mask x=keys[j].bits[0];
            if(x!=q)parts[__builtin_popcountll(x&q)]|=Mask(1)<<j;
        }
        parts.erase(remove(parts.begin(),parts.end(),0),parts.end());
        if(parts.size()==1 && parts[0]==all)continue;
        splits.push_back({q,move(parts)});
    }
    unordered_map<Mask,pair<int,int>> memo;
    Mask result=0;
    auto solve=[&](auto&& self,Mask state)->pair<int,int> {
        int size=__builtin_popcountll(state);
        if(size==1)return {1,1};
        if(auto it=memo.find(state);it!=memo.end())return it->second;
        pair<int,int> best={INT_MAX,INT_MAX};
        for(const auto &p:splits) {
            int cost=size,depth=1;
            for(Mask child:p.children)if((child&=state)) {
                if(child==state){cost=INT_MAX;break;}
                auto [c,h]=self(self,child);
                cost+=c;depth=max(depth,1+h);
                if(cost>best.first)break;
            }
            if(cost!=INT_MAX && pair{cost,depth}<best) {
                best={cost,depth};if(state==all)result=p.query;
            }
        }
        return memo[state]=best;
    };
    solve(solve,all);
    return result;
}

void coarseRound() {
    int ng=pools.size(),limit=6200;
    vector<int> b(ng,1),h(ng),g(ng),ways(ng,1);
    long double entropy=0;
    for(int i=0;i<ng;i++) {
        g[i]=pools[i].pos.size();
        h[i]=min(pools[i].ones,g[i]-pools[i].ones);
        entropy+=logChoose(g[i],h[i]);
    }
    if(entropy<=15)limit=33000;
    int waysAll=1,buckets=ng;
    while(buckets<62) {
        int best=-1,newWays=0;
        double merit=-1;
        for(int i=0;i<ng;i++)if(b[i]<g[i]) {
            int cap=limit/(waysAll/ways[i]);
            int w=coarseWays(g[i],h[i],b[i]+1,cap);
            if(w>cap)continue;
            double a=(1.0+min(h[i],g[i]/2))*log2(1.0+(double)g[i]/b[i])/b[i];
            if(a>merit){merit=a;best=i;newWays=w;}
        }
        if(best<0)break;
        waysAll=waysAll/ways[best]*newWays;
        ways[best]=newWays;
        ++b[best];++buckets;
    }
    vector<Cell> cells;
    
    vector<Candidate> cand(1);
    int maxCount=0,errors=accumulate(h.begin(),h.end(),0);
    for(int i=0;i<ng;i++) {
        int start=cells.size(),at=0;
        for(int j=0;j<b[i];j++) {
            int size=g[i]/b[i]+(j<g[i]%b[i]);
            cells.push_back({vector<int>(pools[i].pos.begin()+at,pools[i].pos.begin()+at+size),
                             pools[i].ones>g[i]/2});
            at+=size;
            maxCount=max(maxCount,min(h[i],size));
        }
        vector<Candidate> local;
        auto gen=[&](auto&& self,int j,int rem,int room,Candidate current)->void {
            if(j==b[i]) {local.push_back(current);return;}
            int ix=start+j,sz=cells[ix].pos.size();
            double comb=1;
            for(int x=0;x<=min(rem,sz);x++) {
                if(rem-x<=room-sz) {
                    Candidate v=current;
                    v.mass*=comb;
                    for(int t=x,k=0;t;t>>=1,k++)if(t&1)v.bits[k]|=Mask(1)<<ix;
                    self(self,j+1,rem-x,room-sz,v);
                }
                comb*=double(sz-x)/(x+1);
            }
        };
        gen(gen,0,h[i],g[i],Candidate());
        vector<Candidate> next;
        next.reserve(cand.size()*local.size());
        for(const auto &x:cand)for(const auto &y:local) {
            Candidate v=x;
            v.mass*=y.mass;
            for(int k=0;(1<<k)<=maxCount;k++)v.bits[k]|=y.bits[k];
            next.push_back(v);
        }
        cand=move(next);
    }
    int K=cells.size(),planes=0;
    while((1<<planes)<=maxCount)++planes;
    Mask all=(Mask(1)<<K)-1;
    auto dot=[planes](const Candidate &v,Mask q) {
        int a=__builtin_popcountll(v.bits[0]&q);
        for(int k=1;k<planes;k++)a+=__builtin_popcountll(v.bits[k]&q)<<k;
        return a;
    };
    auto restrictTo=[&](Mask mask,int target) {
        cand.erase(remove_if(cand.begin(),cand.end(),[&](const Candidate &v){
            return dot(v,mask)!=target;
        }),cand.end());
    };
    // Reuse every past score that depends only on the current cell counts.
    for(auto &[q,score]:history) {
        Mask mask=0;
        int base=n-errors;
        bool usable=true;
        for(int j=0;j<K && usable;j++) {
            const auto &c=cells[j];
            char ch=q[c.pos[0]];
            for(int pos:c.pos)if(q[pos]!=ch){usable=false;break;}
            if((ch=='T')!=c.inverted){mask|=Mask(1)<<j;base-=c.pos.size();}
        }
        if(!usable)continue;
        // The cells cover exactly the unresolved positions.
        int wrong=0;
        for(int j=0;j<n;j++)wrong+=q[j]!=fixedSheet[j];
        for(auto &c:cells)if(q[c.pos[0]]=='T')wrong-=c.pos.size();
        restrictTo(mask,(score-base+wrong)/2);
    }
    Mask single=0;
    for(int j=0;j<K;j++)if(cells[j].pos.size()==1)single|=Mask(1)<<j;
    while(cand.size()>1) {
        Mask bestQ=0;
        if(single==all && (K<=7 || (K<=10 && cand.size()<=12))) {
            bestQ=smallOptimal(cand,K,errors);
        } else {
            vector<Mask> tests;
            if(K<=10) {
                for(Mask q=1;q<all;q++)tests.push_back(q);
            } else {
                for(int j=0;j<K;j++)tests.push_back(Mask(1)<<j);
                for(int i=0;i<170;i++)tests.push_back(((Mask(rng())<<32)|rng())&all);
            }
            for(int i=0;i<min(200,(int)cand.size());i++)
                tests.push_back(cand[size_t(i)*cand.size()/min(200,(int)cand.size())].bits[0]);
            vector<double> hist(errors+1);
            double sum=0;
            for(auto &v:cand)sum+=v.mass;
            double best=-1;
            for(Mask q:tests) {
                fill(hist.begin(),hist.end(),0.0);
                double direct=0;
                for(auto &v:cand) {
                    int d=dot(v,q);hist[d]+=v.mass;
                    if(!(q&~single) && d==errors && __builtin_popcountll(q)==errors)
                        direct+=v.mass;
                }
                double value=direct/sum*max(1.0,log2(1.0+sqrt(errors)));
                for(double f:hist)if(f>0){double p=f/sum;value-=p*log2(p);}
                if(value>best)best=value,bestQ=q;
            }
        }
        string q=fixedSheet;
        int base=n-errors;
        for(int j=0;j<K;j++) {
            bool selected=bestQ>>j&1;
            if(selected)base-=cells[j].pos.size();
            for(int pos:cells[j].pos)q[pos]=(selected!=cells[j].inverted)?'T':'F';
        }
        int score=query(q);
        restrictTo(bestQ,(score-base)/2);
    }
    pools.clear();
    for(int j=0;j<K;j++) {
        int x=dot(cand[0],Mask(1)<<j);
        int actual=cells[j].inverted?(int)cells[j].pos.size()-x:x;
        keep(move(cells[j].pos),actual);
    }
}

// Large constant-weight pools are split together before enumeration. A known
// weight lets us center the column coefficients and gain one extra low bit.
void preparePools() {
    vector<Pool> ready;
    while(!pools.empty()) {
        vector<Pool> big;
        for(auto &p:pools) {
            if(p.pos.size()<=63 && logChoose(p.pos.size(),p.ones)<=18.2L)
                ready.push_back(move(p));
            else big.push_back(move(p));
        }
        pools.clear();
        if(big.empty())break;
        int m=big.size();
        Batch batch(m);
        vector<vector<Part>> parts(m);
        for(int s=0;s<m;s++) {
            auto &p=big[s];
            int d=rankOf(s),M=s?(1<<(d-1)):0;
            int g=p.pos.size(),lo=min(g,d),half=(g-lo)/2;
            for(int i=0;i<lo+2;i++) {
                int begin=i<lo?i:(i==lo?lo:lo+half);
                int end=i<lo?i+1:(i==lo?lo+half:g);
                int w=i<lo?(1<<i):(i==lo?0:1<<d);
                vector<int> a(p.pos.begin()+begin,p.pos.begin()+end);
                auto table=batch.column(s,w-M,a);
                parts[s].push_back({move(a),move(table)});
            }
        }
        batch.read();
        for(int s=m-1;s>=0;s--) {
            int d=rankOf(s),M=s?(1<<(d-1)):0,rem=big[s].ones;
            int v=batch.value(s)+M*rem;
            int lo=parts[s].size()-2;
            for(int i=0;i<lo+2;i++) {
                auto &part=parts[s][i];
                int x=i<lo?(v>>i)&1:(i==lo?rem-(v>>d):rem);
                rem-=x;
                batch.subtract(s,part.table,x);
                keep(move(part.pos),x);
            }
        }
    }
    pools=move(ready);
}

struct Block {
    vector<int> pos;
    vector<Mask> keys;
    int ones;
};

void clean(Block &b) {
    Mask sameOne=~Mask(0),anyOne=0;
    for(Mask x:b.keys){sameOne&=x;anyOne|=x;}
    Mask varying=anyOne^sameOne;
    if(varying==((Mask(1)<<b.pos.size())-1))return;
    vector<int> pos;
    vector<pair<Mask,int>> runs;
    int skipped=0;
    for(int i=0;i<(int)b.pos.size();i++) {
        if(varying>>i&1) {
            pos.push_back(b.pos[i]);
            if(runs.empty() || runs.back().second!=skipped)runs.push_back({0,skipped});
            runs.back().first|=Mask(1)<<i;
        } else {
            ++skipped;
            if(sameOne>>i&1)fixedSheet[b.pos[i]]='T',--b.ones;
        }
    }
    for(Mask &x:b.keys) {
        Mask y=0;for(auto [m,d]:runs)y|=(x&m)>>d;x=y;
    }
    b.pos=move(pos);
}

Block mergeBlocks(Block &a,Block &b) {
    Block v;
    v.ones=a.ones+b.ones;
    v.pos=a.pos;v.pos.insert(v.pos.end(),b.pos.begin(),b.pos.end());
    v.keys.reserve(a.keys.size()*b.keys.size());
    for(Mask x:b.keys) {
        Mask high=x<<a.pos.size();
        for(Mask y:a.keys)v.keys.push_back(high|y);
    }
    return v;
}

struct Probe {
    vector<int> w;
    vector<int> bits;
    Mask high=0;
    bool arbitrary=false;
    vector<Mask> planes;
};

int eval(const Probe& p,Mask key) {
    if(p.arbitrary) {
        int a=0;
        for(int j=0;j<(int)p.planes.size();j++)a+=__builtin_popcountll(key&p.planes[j])<<j;
        return a;
    }
    int a=__builtin_popcountll(p.high&key);
    for(auto i=p.bits.rbegin();i!=p.bits.rend();++i)a=a*2+((key>>*i)&1);
    return a;
}

Probe chooseProbe(const Block &b,int s,bool last) {
    int d=rankOf(s),L=1<<d,g=b.pos.size();
    Mask full=(Mask(1)<<g)-1;
    const int sample=min(350,(int)b.keys.size());
    vector<Mask> keys;
    if(sample==(int)b.keys.size())keys=b.keys;
    else for(int i=0;i<sample;i++) {
        size_t left=i*b.keys.size()/sample,right=(i+1)*b.keys.size()/sample;
        keys.push_back(b.keys[left+rng()%(right-left)]);
    }
    vector<int> order(g);iota(order.begin(),order.end(),0);
    int maxV=L*b.ones;
    vector<int> hist(maxV+1);
    vector<double> nlog(sample+1);
    for(int j=1;j<=sample;j++)nlog[j]=j*log2(j);
    auto score=[&](const Probe &p) {
        fill(hist.begin(),hist.end(),0);
        double v=0;
        for(Mask x:keys) {
            int z=eval(p,x);
            ++hist[z];
            if(last && x==p.high)v+=2.1;
        }
        for(int c:hist)v-=nlog[c];
        return v;
    };
    double best=-1e100;
    Probe ans;
    int iters=90+min(90,g*2);
    for(int it=0;it<iters;it++) {
        Probe p;
        shuffle(order.begin(),order.end(),rng);
        p.high=((Mask(rng())<<32)|rng())&full;
        if(it%4==0) p.high=b.keys[rng()%b.keys.size()];
        if(it<g)p.high=Mask(1)<<it;
        int cnt=min(d,g);
        for(int j=0;j<cnt;j++){p.bits.push_back(order[j]);p.high&=~(Mask(1)<<order[j]);}
        // If fewer than d bits remain, all of them can be read directly.
        double val=score(p);
        if(val>best){best=val;ans=move(p);}
    }
    for(int round=0;round<2;round++) {
        shuffle(order.begin(),order.end(),rng);
        for(int j:order) {
            Probe p=ans;
            if(find(p.bits.begin(),p.bits.end(),j)!=p.bits.end())continue;
            p.high^=Mask(1)<<j;
            double a=score(p);
            if(a>best){best=a;ans=move(p);}
        }
    }
    ans.w.resize(g);
    for(int j=0;j<g;j++)ans.w[j]=(ans.high>>j&1)?L:0;
    for(int j=0;j<(int)ans.bits.size();j++)ans.w[ans.bits[j]]=1<<j;
    if(!s)return ans;
    auto setPlanes=[&](Probe &p) {
        p.arbitrary=true;
        p.planes.assign(d+1,0);
        for(int j=0;j<g;j++)for(int k=0;k<=d;k++)
            if(p.w[j]>>k&1)p.planes[k]|=Mask(1)<<j;
    };
    int minority=min(b.ones,g-b.ones);
    // Encode counts in several buckets as digits in base minority+1.
    // Complementing every bit gives the same separation for a T-majority.
    if(minority<=5) {
        vector<int> alphabet{0};
        for(int w=1;w<=L;w*=minority+1)alphabet.push_back(w);
        if(minority==1){alphabet.resize(L+1);iota(alphabet.begin(),alphabet.end(),0);}
        for(int it=0;it<50;it++) {
            shuffle(order.begin(),order.end(),rng);
            Probe p;p.w.resize(g);
            for(int j=0;j<g;j++)p.w[order[j]]=alphabet[j%alphabet.size()];
            setPlanes(p);
            double a=score(p);
            if(a>best){best=a;ans=move(p);}
        }
    }
    if(b.keys.size()<=230 && best<0) {
        vector<int> marks(maxV+1,-1);
        vector<Mask> rep(maxV+1);
        for(int it=0;it<200;it++) {
            Probe p;p.w.resize(g);
            for(int &w:p.w)w=rng()%(L+1);
            setPlanes(p);
            vector<Mask> classes(L+1);
            for(int j=0;j<g;j++)classes[p.w[j]]|=Mask(1)<<j;
            fill(hist.begin(),hist.end(),0);
            bool good=true;
            for(Mask x:b.keys) {
                int z=eval(p,x);
                hist[z]++;
                if(marks[z]==it) {
                    for(Mask cls:classes)if(__builtin_popcountll(rep[z]&cls)!=__builtin_popcountll(x&cls)) {
                        good=false;break;
                    }
                    if(!good)break;
                } else {marks[z]=it;rep[z]=x;}
            }
            if(good) {
                double a=0;for(int v:hist)a-=nlog[v];
                if(a>best){ans=move(p);best=a;if(a==0)break;}
            }
        }
    }
    return ans;
}

vector<Mask> masksWithCount(int g,int c) {
    vector<Mask> keys;
    Mask limit=Mask(1)<<g;
    if(!c)return {0};
    for(Mask x=(Mask(1)<<c)-1;x<limit;) {
        keys.push_back(x);
        Mask bit=x&-x,up=x+bit;
        x=up|((x^up)/bit>>2);
    }
    return keys;
}

// For a joint search, equal class IDs denote compatible score histories
// on opposite halves. Each query refines these classes by its exact dot product.
struct Entry {unsigned cls,id;};
struct Side {
    vector<int> groups;
    vector<Entry> live;
    int size=1;
};
struct Match {int l,nl,r,nr;long long end;};

bool jointSolve(vector<Block>& blocks,bool applyTotal=false) {
    Side sides[2];
    int ng=blocks.size();
    vector<int> ord(ng);iota(ord.begin(),ord.end(),0);
    sort(ord.begin(),ord.end(),[&](int a,int b){return blocks[a].keys.size()>blocks[b].keys.size();});
    for(int i:ord) {
        auto &side=sides[sides[0].size>sides[1].size];
        if((long long)side.size*blocks[i].keys.size()>(1<<18))return false;
        side.groups.push_back(i);side.size*=blocks[i].keys.size();
    }
    for(auto &side:sides) {
        side.live.resize(side.size);
        for(int j=0;j<side.size;j++)side.live[j]={0,(unsigned)j};
    }
    vector<Match> matches{{0,sides[0].size,0,sides[1].size,(long long)sides[0].size*sides[1].size}};
    auto refine=[&](const vector<Mask> &q,int target) {
        for(int h=0;h<2;h++) {
            auto &side=sides[h];
            vector<short> dots(1);
            for(int i:side.groups) {
                int old=dots.size(),count=blocks[i].keys.size();
                dots.resize(old*count);
                for(int j=count-1;j>=0;j--) {
                    int add=__builtin_popcountll(q[i]&blocks[i].keys[j]);
                    for(int k=0;k<old;k++)dots[j*old+k]=dots[k]+add;
                }
            }
            for(auto &e:side.live) {
                int d=dots[e.id];
                e.cls=(e.cls<<11)+n+(h?target-d:d);
            }
            sort(side.live.begin(),side.live.end(),[](Entry a,Entry b){return a.cls<b.cls;});
        }
        matches.clear();long long count=0;
        auto &a=sides[0].live;auto &b=sides[1].live;
        int i=0,j=0,na=0,nb=0;
        while(i<(int)a.size() && j<(int)b.size()) {
            if(a[i].cls<b[j].cls){++i;continue;}
            if(b[j].cls<a[i].cls){++j;continue;}
            unsigned cls=a[i].cls,id=matches.size();
            int left=na,right=nb;
            do {a[na++]={id,a[i++].id};}while(i<(int)a.size() && a[i].cls==cls);
            do {b[nb++]={id,b[j++].id};}while(j<(int)b.size() && b[j].cls==cls);
            count+=(long long)(na-left)*(nb-right);
            matches.push_back({left,na-left,right,nb-right,count});
        }
        a.resize(na);b.resize(nb);
    };
    auto get=[&](long long k) {
        auto it=upper_bound(matches.begin(),matches.end(),k,[](long long v,const Match &m){return v<m.end;});
        long long offset=k-(it->end-(long long)it->nl*it->nr);
        unsigned ids[2]={sides[0].live[it->l+offset/it->nr].id,sides[1].live[it->r+offset%it->nr].id};
        vector<Mask> x(ng);
        for(int h=0;h<2;h++)for(int i:sides[h].groups) {
            unsigned len=blocks[i].keys.size();
            x[i]=blocks[i].keys[ids[h]%len];ids[h]/=len;
        }
        return x;
    };
    vector<Mask> full(ng);
    vector<pair<int,int>> coordinates;
    for(int j=0;j<ng;j++) {
        full[j]=(Mask(1)<<blocks[j].pos.size())-1;
        for(int k=0;k<(int)blocks[j].pos.size();k++)coordinates.push_back({j,k});
    }
    if(applyTotal)refine(full,total);
    int fixedT=count(fixedSheet.begin(),fixedSheet.end(),'T');
    while(matches.back().end>1) {
        long long number=matches.back().end;
        int sample=min(480LL,number);
        vector<vector<Mask>> xs;
        for(int j=0;j<sample;j++) {
            // Stratification includes disjoint portions of the candidate space.
            long long left=j*number/sample,right=(j+1)*number/sample;
            xs.push_back(get(left+((Mask(rng())<<32)|rng())%(right-left)));
        }
        vector<int> hist(n+1);
        vector<double> nl(sample+1);
        for(int j=1;j<=sample;j++)nl[j]=j*log2(j);
        vector<Mask> bestQ;
        double best=-1e100;
        auto test=[&](const vector<Mask>& q) {
            fill(hist.begin(),hist.end(),0);
            double val=0;
            for(int j=0;j<sample;j++) {
                int d=0;for(int i=0;i<ng;i++)d+=__builtin_popcountll(q[i]&xs[j][i]);
                ++hist[d];
                if(d==total-fixedT && xs[j]==q)val+=2.5;
            }
            for(int c:hist)val-=nl[c];
            if(val>best){best=val;bestQ=q;}
        };
        vector<Mask> q(ng);
        for(int it=0;it<135;it++) {
            if(it%3==0)q=xs[rng()%sample];
            else for(int i=0;i<ng;i++)q[i]=((Mask(rng())<<32)|rng())&full[i];
            test(q);
        }
        shuffle(coordinates.begin(),coordinates.end(),rng);
        for(auto [i,k]:coordinates) {
            q=bestQ;q[i]^=Mask(1)<<k;test(q);
        }
        string sheet=fixedSheet;
        for(int i=0;i<ng;i++)for(int j=0;j<(int)blocks[i].pos.size();j++)
            sheet[blocks[i].pos[j]]=(bestQ[i]>>j&1)?'T':'F';
        refine(bestQ,dotQuery(sheet)-fixedT);
    }
    auto x=get(0);
    for(int i=0;i<ng;i++)for(int j=0;j<(int)blocks[i].pos.size();j++)
        fixedSheet[blocks[i].pos[j]]=(x[i]>>j&1)?'T':'F';
    return true;
}

void blockSolve() {
    vector<Block> blocks;
    for(auto &p:pools)
        blocks.push_back({move(p.pos),{},p.ones});
    pools.clear();
    for(auto &b:blocks)b.keys=masksWithCount(b.pos.size(),b.ones);
    while(!blocks.empty()) {
        if(jointSolve(blocks))return;
        sort(blocks.begin(),blocks.end(),[](auto &a,auto &b){return a.keys.size()<b.keys.size();});
        // Merge small lists so that a score can distinguish joint assignments
        // across several of the original groups.
        for(int i=0;i+1<(int)blocks.size();) {
            int other=-1;
            for(int j=i+1;j<(int)blocks.size();j++) {
                if(blocks[i].keys.size()*blocks[j].keys.size()>80000)break;
                if(blocks[i].pos.size()+blocks[j].pos.size()<=63){other=j;break;}
            }
            if(other<0){++i;continue;}
            Block v=mergeBlocks(blocks[i],blocks[other]);
            blocks.erase(blocks.begin()+other);
            blocks.erase(blocks.begin()+i);
            auto at=lower_bound(blocks.begin()+i,blocks.end(),v.keys.size(),
                                [](const Block& a,size_t k){return a.keys.size()<k;});
            blocks.insert(at,move(v));
        }
        int m=blocks.size();
        vector<int> labels(m);iota(labels.begin(),labels.end(),0);
        stable_sort(labels.begin(),labels.end(),[](int a,int b){return rankOf(a)<rankOf(b);});
        vector<Probe> probes(m);
        vector<vector<vector<unsigned char>>> columns(m);
        Batch batch(m);
        for(int i=0;i<m;i++) {
            int s=labels[i],M=s?(1<<(rankOf(s)-1)):0;
            auto &b=blocks[i];auto &p=probes[s];p=chooseProbe(b,s,m==1);
            for(int j=0;j<(int)b.pos.size();j++) {
                columns[s].push_back(batch.column(s,p.w[j]-M,{b.pos[j]}));
            }
        }
        vector<int> which(m);
        for(int i=0;i<m;i++)which[labels[i]]=i;
        batch.read();
        for(int s=m-1;s>=0;s--) {
            int value=batch.value(s);
            auto &b=blocks[which[s]];
            const auto &p=probes[s];
            if(s)value+=(1<<(rankOf(s)-1))*b.ones;
            b.keys.erase(remove_if(b.keys.begin(),b.keys.end(),
                        [&](Mask x){return eval(p,x)!=value;}),b.keys.end());
            Mask x=b.keys.front();
            // Every probe preserves the count at each distinct weight among
            // its survivors, so any representative gives the same correction.
            while(x) {
                int j=__builtin_ctzll(x);x&=x-1;
                batch.subtract(s,columns[s][j],1);
            }
            clean(b);
        }
        blocks.erase(remove_if(blocks.begin(),blocks.end(),
                               [](const Block& b){return b.pos.empty();}),blocks.end());
    }
}

int main() {
    ios::sync_with_stdio(false);cin.tie(nullptr);
    if(!(cin>>n) || n<=0)return 0;
    fixedSheet.assign(n,'F');
    int first=submit(fixedSheet);
    total=n-first;
    remember(fixedSheet,first);
    makePatterns();
    vector<int> pos(n);iota(pos.begin(),pos.end(),0);
    keep(move(pos),total);
    if(n>=17 && n<=36 && min(total,n-total)>3) {
        vector<Block> blocks;
        for(int i=0;i<2;i++) {
            int l=i*n/2,r=(i+1)*n/2;
            Block b;b.pos.resize(r-l);iota(b.pos.begin(),b.pos.end(),l);
            b.keys.resize(1<<(r-l));iota(b.keys.begin(),b.keys.end(),0);
            blocks.push_back(move(b));
        }
        if(jointSolve(blocks,true)) {submit(fixedSheet);return 0;}
    }
    try {
        while(!pools.empty()) {
            int errors=0;
            long double entropy=0;
            for(const auto &p:pools) {
                int g=p.pos.size();
                errors+=min(p.ones,g-p.ones);
                entropy+=logChoose(g,p.ones);
            }
            if(errors==1)oneError();
            else if(errors<=min(26,max(10,n/18)) || entropy<=16 || n<=29)
                coarseRound();
            else {
                vector<int> positions=move(pools[0].pos);
                pools.clear();
                int m=1;
                int size=min(120,max(32,int(3.2*n/min(total,n-total))));
                while(capacity(m)+(size-1)*m<n)++m;
                if(m>2) {
                    int pw=1;while(2*pw<=m)pw*=2;
                    m=5*m<7*pw?pw:2*pw;
                }
                poolStage(move(positions),total,m);
                preparePools();
                blockSolve();
                break;
            }
        }
    } catch(const SparsePivot &pivot) {
        orientation=pivot.sheet;
        map<string,int> shifted;
        for(const auto &[q,r]:history)shifted.emplace(reorient(q,orientation),r);
        history.swap(shifted);
        patterns.clear();simpleCount=0;
        total=pivot.distance;
        fixedSheet.assign(n,'F');
        pools.clear();
        vector<int> all(n);iota(all.begin(),all.end(),0);
        keep(move(all),total);
        while(!pools.empty()) {
            int errors=0;
            for(const auto &p:pools)errors+=min(p.ones,(int)p.pos.size()-p.ones);
            if(errors==1)oneError();else coarseRound();
        }
    }
    submit(fixedSheet);
}
