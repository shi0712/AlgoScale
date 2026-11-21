#include <bits/stdc++.h>
using namespace std;

namespace {
constexpr int W = 24;
constexpr int R = 13;
using Triple = array<int, 3>;
int areaCode[4][4][4];
map<int, int> codes;
int n;
unordered_map<long long, int> cache;

long long key(Triple t) {
    sort(t.begin(), t.end());
    return (1LL * t[0] * n + t[1]) * n + t[2];
}

int ask(Triple t) {
    sort(t.begin(), t.end());
    auto k = key(t);
    auto it = cache.find(k);
    if (it != cache.end()) return it->second;
    cout << "? " << t[0]+1 << ' ' << t[1]+1 << ' ' << t[2]+1 << endl;
    int x;
    if (!(cin >> x) || !codes.count(x)) exit(0);
    return cache[k] = codes.at(x);
}

void answer(const vector<int>& values) {
    cout << '!';
    for (int v : values) cout << ' ' << v+1;
    cout << endl;
    exit(0);
}

void ambiguous() {
    cout << "! -1" << endl;
    exit(0);
}

// For n <= 5, optimize the entire decision tree, merging all assignments
// that require the same answer -1.
struct SmallSolver {
    vector<Triple> qs;
    vector<vector<unsigned char>> sig;
    vector<vector<int>> vals;
    vector<unsigned char> bad;
    vector<double> weight, weightLog;
    map<vector<int>, pair<double,int>> memo;

    double mass(const vector<int>& s) const {
        double sum=0;
        for(int i:s) sum+=weight[i];
        return sum;
    }

    clock_t start=clock();
    unsigned visits=0;
    struct TimeLimit {};
    bool limitReached() const {
        return n>5 && clock()-start>2*CLOCKS_PER_SEC;
    }

    pair<double,int> plan(const vector<int>& s) {
        if (s.size()==1 || all_of(s.begin(),s.end(),[&](int i){return bad[i];}))
            return {0,-1};
        if((++visits&511)==0 && limitReached()) throw TimeLimit{};
        auto it = memo.find(s);
        if (it != memo.end()) return it->second;
        double best=1e100, baseCost=mass(s);
        int choice=-1;
        vector<pair<double,int>> candidates;
        if(s.size()<=20 && n>5) {
            set<__uint128_t> seen;
            for(int q=0;q<(int)qs.size();q++) {
                array<int,R> label{}, size{}, good{};
                array<double,R> w{};
                int next=0;
                __uint128_t partition=0;
                for(int i:s) {
                    int x=sig[i][q];
                    if(size[x]++==0) label[x]=next++;
                    partition=(partition<<4)|label[x];
                    good[x]+=!bad[i];
                    w[x]+=weight[i];
                }
                if(next==1 || !seen.insert(partition).second) continue;
                double lowerBound=0;
                for(int x=0;x<R;x++)
                    if(good[x] && size[x]>1) lowerBound+=w[x];
                if(lowerBound==0) return memo[s]={baseCost,q};
                candidates.emplace_back(lowerBound,q);
            }
            sort(candidates.begin(),candidates.end());
        } else {
            for(int q=0;q<(int)qs.size();q++) candidates.emplace_back(0.,q);
        }
        for (auto [lowerBound,q]:candidates) {
            if(baseCost+lowerBound>=best) break;
            array<vector<int>,R> buckets;
            for (int i:s) buckets[sig[i][q]].push_back(i);
            double cost=baseCost;
            for (auto& b:buckets) {
                if (b.size()==s.size()) {cost=1e100; break;}
                if (b.size()>1) cost+=plan(b).first;
                if (cost>=best) break;
            }
            if (cost<best) best=cost, choice=q;
            if(best<=baseCost) break;
        }
        return memo[s]={best,choice};
    }

    bool terminal(const vector<int>& s) const {
        return s.size()<=1 || all_of(s.begin(),s.end(),[&](int i){return bad[i];});
    }

    // Estimate the remaining path length after one query. Terminal buckets
    // contribute nothing; indistinguishable arrays share an output label.
    vector<pair<double,int>> rankQueries(const vector<int>& s) {
        vector<pair<double,int>> order;
        double baseCost=mass(s);
        for(int q=0;q<(int)qs.size();q++) {
            array<int,R> sizes{}, good{};
            array<double,R> w{}, amb{}, sumLogs{};
            for(int i:s) {
                int v=sig[i][q];
                sizes[v]++;
                w[v]+=weight[i];
                if(bad[i]) amb[v]+=weight[i];
                else {
                    good[v]++;
                    sumLogs[v]+=weightLog[i];
                }
            }
            if(*max_element(sizes.begin(),sizes.end())==(int)s.size()) continue;
            double cost=baseCost;
            for(int v=0;v<R;v++) if(sizes[v]>1 && good[v]) {
                double info=w[v]*log2(w[v])-sumLogs[v];
                if(amb[v]>0) info-=amb[v]*log2(amb[v]);
                cost+=max(w[v],.60*w[v]+info/2.35);
            }
            order.emplace_back(cost,q);
        }
        sort(order.begin(),order.end());
        return order;
    }

    int lookahead(const vector<int>& s) {
        if(n<=5 || s.size()<=20) return plan(s).second;
        if(terminal(s)) return -1;
        auto order=rankQueries(s);
        int choice=order.front().second;
        double best=1e100;
        int tried=0;
        double previous=-1;
        for(auto [estimate,q]:order) {
            // Symmetric triples often have identical partitions up to an
            // index permutation. Spend the lookahead on different scores.
            if(abs(estimate-previous)<1e-8) continue;
            if(tried++>=5) break;
            previous=estimate;
            array<vector<int>,R> buckets;
            for(int i:s) buckets[sig[i][q]].push_back(i);
            double cost=mass(s);
            for(auto& b:buckets) {
                if(terminal(b)) continue;
                if(b.size()<=20) cost+=plan(b).first;
                else cost+=rankQueries(b).front().first;
                if(cost>=best) break;
            }
            if(cost<best) best=cost, choice=q;
        }
        return choice;
    }

    int choose(const vector<int>& s) {
        if(terminal(s)) return -1;
        if(!limitReached()) {
            try {return lookahead(s);}
            catch(const TimeLimit&) {}
        }
        return rankQueries(s).front().second;
    }

    int identify(bool firstTriple) {
        weightLog.resize(weight.size());
        for(int i=0;i<(int)weight.size();i++)
            weightLog[i]=weight[i]*log2(weight[i]);
        vector<int> s(sig.size());
        iota(s.begin(),s.end(),0);
        int q=firstTriple ? 0 : choose(s);
        while(q>=0) {
            int r=ask(qs[q]);
            erase_if(s,[&](int i){return sig[i][q]!=r;});
            q=choose(s);
        }
        if(bad[s[0]]) ambiguous();
        return s[0];
    }

    void run() {
        for(int i=0;i<n;i++) for(int j=i+1;j<n;j++) for(int k=j+1;k<n;k++)
            qs.push_back({i,j,k});
        map<vector<unsigned char>,int> freq;
        for(int mask=0;mask<(1<<(2*n));mask++) {
            vector<int> v(n);
            vector<unsigned char> f;
            f.reserve(qs.size());
            for(int i=0;i<n;i++) v[i]=(mask>>(2*i))&3;
            for(auto [a,b,c]:qs) f.push_back(areaCode[v[a]][v[b]][v[c]]);
            if(n<=5) freq[f]++;
            sig.push_back(f);
            vals.push_back(v);
        }
        for(int i=0;i<(int)sig.size();i++) {
            if(n<=5) bad.push_back(freq[sig[i]]>1);
            else {
                array<int,4> f{};
                for(int v:vals[i]) f[v]++;
                bad.push_back(f[0]<n && max({f[1],f[2],f[3]})<=1);
            }
        }
        weight.assign(sig.size(),1.);
        answer(vals[identify(true)]);
    }
};

struct Row {
    array<unsigned char,W> v{};
    double weight=1;
};

// Track both the estimated probability and the fraction of surviving
// assignments. Rare alternatives must also be eliminated to certify a value.
struct Mass {
    double probability=0, fraction=0;
    void operator+=(Mass b) {
        probability+=b.probability;
        fraction+=b.fraction;
    }
    Mass operator*(Mass b) const {
        return {probability*b.probability,fraction*b.fraction};
    }
};

constexpr double blend=.70;
struct Group {
    vector<int> ids;
    vector<Row> rows;
    array<Mass,4> one{};
    array<Mass,16> two{};
    int kind=-1;
};

// The candidate space is the Cartesian product of work, the queued groups,
// and untouched positions. Only a judge reply removes rows. A fixed column
// becomes a known index, so at most two representatives per value suffice
// for all future queries that involve at least one unknown.
struct BlockSolver {
    vector<int> known;
    array<vector<int>,4> reps;
    array<int,4> counts{};
    array<vector<array<int,3>>,R> triples;
    vector<Group> queue;
    Group work;
    int generated=0;
    clock_t start=clock();

    BlockSolver():known(n,-1) {
        for(int a=0;a<4;a++) for(int b=0;b<4;b++) for(int c=0;c<4;c++)
            triples[areaCode[a][b][c]].push_back({a,b,c});
        work.rows.emplace_back();
    }

    void fix(int i,int v) {
        known[i]=v;
        counts[v]++;
        if(reps[v].size()<2) reps[v].push_back(i);
    }

    void compact() {
        int m=work.ids.size();
        if(!m) return;
        vector<int> keep;
        for(int i=0;i<m;i++) {
            int v=work.rows[0].v[i];
            bool fixed=all_of(work.rows.begin(),work.rows.end(),
                              [&](const Row& r){return r.v[i]==v;});
            if(fixed) fix(work.ids[i],v);
            else keep.push_back(i);
        }
        if((int)keep.size()==m) return;
        for(int i=0;i<(int)keep.size();i++) work.ids[i]=work.ids[keep[i]];
        work.ids.resize(keep.size());
        for(auto& r:work.rows)
            for(int i=0;i<(int)keep.size();i++) r.v[i]=r.v[keep[i]];
    }

    // A repeated pattern only affects which fresh indices are grouped.
    // Every resulting assignment is still certified by actual replies.
    vector<int> period() {
        if(generated && find(known.begin(),known.begin()+generated,-1)==known.begin()+generated) {
            vector<int> prefix(generated);
            for(int i=1;i<generated;i++) {
                int j=prefix[i-1];
                while(j && known[i]!=known[j]) j=prefix[j-1];
                prefix[i]=j+(known[i]==known[j]);
            }
            int length=generated-prefix.back();
            if(length*2<=generated) return {known.begin(),known.begin()+length};
        }
        int begin=max(0,generated-192);
        for(int p=1;p<=min(48,(generated-begin)/2);p++) {
            vector<int> pat(p,-1), samples(p);
            bool ok=true;
            for(int i=begin;i<generated;i++) if(known[i]>=0) {
                int& v=pat[i%p];
                if(v>=0 && v!=known[i]) {ok=false; break;}
                v=known[i];
                ++samples[i%p];
            }
            if(ok && *min_element(samples.begin(),samples.end())>=2) return pat;
        }
        // Check approximate periods against held-out, already decoded values.
        // A phase with no clear majority gets its own unsorted bucket.
        int middle=(begin+generated)/2;
        vector<int> bestPattern;
        double bestGain=0;
        for(int p=2;p<=min(48,(middle-begin)/3);p++) {
            vector<array<int,4>> train(p);
            array<int,4> overall{};
            for(int i=begin;i<middle;i++) if(known[i]>=0) {
                train[i%p][known[i]]++;
                overall[known[i]]++;
            }
            vector<int> pat(p,4);
            for(int i=0;i<p;i++) {
                auto it=max_element(train[i].begin(),train[i].end());
                int count=accumulate(train[i].begin(),train[i].end(),0);
                if(*it>=2 && 3*(*it)>=2*count) pat[i]=it-train[i].begin();
            }
            int base=max_element(overall.begin(),overall.end())-overall.begin();
            int gain=0, tried=0;
            for(int i=middle;i<generated;i++) if(known[i]>=0) {
                tried++;
                if(pat[i%p]<4)
                    gain+=(pat[i%p]==known[i])-(base==known[i]);
            }
            double score=gain-.20*p;
            if(score>max({bestGain,2.,.12*tried})) {
                bestGain=score;
                bestPattern=move(pat);
            }
        }
        return bestPattern;
    }

    bool scatter=false;
    mt19937 random{0x81a4037};
    void generate() {
        auto pat=period();
        int chunk = generated==0 ? 12 : (generated<36 ? 24 : 144);
        if(!pat.empty()) {
            int unit=3*pat.size();
            chunk=(chunk+unit-1)/unit*unit;
        }
        int end=min(n,generated+chunk);
        array<vector<int>,5> byValue;
        for(int i=generated;i<end;i++)
            byValue[pat.empty()?0:pat[i%pat.size()]].push_back(i);
        vector<int> order, tail;
        for(auto& b:byValue) {
            int cut=b.size()/3*3;
            order.insert(order.end(),b.begin(),b.begin()+cut);
            tail.insert(tail.end(),b.begin()+cut,b.end());
        }
        order.insert(order.end(),tail.begin(),tail.end());
        int sample=order.size();
        if(scatter && pat.empty() && sample>18) {
            sample=12;
            shuffle(order.begin()+sample,order.end(),random);
        }
        int zeros=0, observed=0;
        vector<pair<vector<int>,int>> pending;
        for(int i=0;i<(int)order.size();i+=3) {
            vector<int> ids(order.begin()+i,order.begin()+min(i+3,(int)order.size()));
            int code=-1;
            if(ids.size()==3) {
                code=ask({ids[0],ids[1],ids[2]});
                if(i+3<=sample && (pat.empty() || pat.size()==1)) {
                    observed++;
                    zeros+=(code==areaCode[0][0][1]);
                }
                if(triples[code].size()==1) {
                    for(int j=0;j<3;j++) fix(ids[j],triples[code][0][j]);
                    continue;
                }
            }
            pending.push_back({ids,code});
        }
        if(observed>=3) scatter=4*zeros>=3*observed;
        generated=end;

        // Estimate value frequencies from known entries and the multisets
        // compatible with the new replies. These are query-selection priors.
        array<double,4> p{.25,.25,.25,.25};
        for(int iter=0;iter<10;iter++) {
            array<double,4> f;
            for(int v=0;v<4;v++) f[v]=counts[v]+2;
            for(auto& [ids,code]:pending) if(code>=0) {
                double sum=0;
                array<double,4> add{};
                for(auto t:triples[code]) {
                    double w=p[t[0]]*p[t[1]]*p[t[2]];
                    sum+=w;
                    for(int v:t) add[v]+=w;
                }
                for(int v=0;v<4;v++) f[v]+=add[v]/sum;
            }
            double total=accumulate(f.begin(),f.end(),0.);
            for(int v=0;v<4;v++) p[v]=max(.008,f[v]/total);
        }
        for(auto& [ids,code]:pending) {
            Group g;
            g.ids=ids;
            if(code>=0) {
                for(auto t:triples[code]) {
                    Row r;
                    copy(t.begin(),t.end(),r.v.begin());
                    r.weight=p[t[0]]*p[t[1]]*p[t[2]];
                    g.rows.push_back(r);
                }
            } else {
                for(int s=0;s<(1<<(2*ids.size()));s++) {
                    Row r;
                    for(int j=0;j<(int)ids.size();j++) {
                        r.v[j]=(s>>(2*j))&3;
                        r.weight*=p[r.v[j]];
                    }
                    g.rows.push_back(r);
                }
            }
            g.kind=code<0 ? R+ids.size() : code;
            double total=0;
            for(auto& r:g.rows) total+=r.weight;
            for(auto& r:g.rows) {
                r.weight/=total;
                Mass w{r.weight,1./g.rows.size()};
                g.one[r.v[0]]+=w;
                if(ids.size()>1) g.two[4*r.v[0]+r.v[1]]+=w;
            }
            queue.push_back(move(g));
        }
    }

    void append(int index) {
        Group& g=queue[index];
        int m=work.ids.size();
        vector<Row> rows;
        rows.reserve(work.rows.size()*g.rows.size());
        for(auto& a:work.rows) for(auto& b:g.rows) {
            Row r=a;
            copy_n(b.v.begin(),g.ids.size(),r.v.begin()+m);
            r.weight*=b.weight;
            rows.push_back(r);
        }
        work.rows.swap(rows);
        work.ids.insert(work.ids.end(),g.ids.begin(),g.ids.end());
        queue.erase(queue.begin()+index);
    }

    // With at least five indices, a repeated side >1 certifies uniqueness.
    // On five indices this follows by enumerating the 4^5 signatures. On a
    // larger set, include the repeated pair in a five-set with each index.
    // Otherwise the non-ones are all distinct: two of them can be swapped,
    // or a lone non-one can change value, without changing any reply.
    bool bad(const Row& r) const {
        auto f=counts;
        for(int i=0;i<(int)work.ids.size();i++) f[r.v[i]]++;
        return f[0]<n && max({f[1],f[2],f[3]})<=1;
    }

    using Single = array<Mass,4>;
    using Pair = array<Mass,16>;
    using Histogram = array<Mass,R>;

    static double entropy(const Histogram& h) {
        double score=0;
        int outcomes=0;
        for(Mass w:h) if(w.fraction>0) {
            score-=w.probability*log(w.fraction*blend+w.probability*(1-blend));
            outcomes++;
        }
        if(outcomes<=1) return 0;
        return score;
    }

    static double pairScore(const Pair& xy, const Single& z) {
        Histogram h{};
        for(int x=0;x<4;x++) for(int y=0;y<4;y++) {
            Mass w=xy[4*x+y];
            if(w.fraction>0) for(int t=0;t<4;t++)
                h[areaCode[x][y][t]]+=w*z[t];
        }
        return entropy(h);
    }

    struct Query {
        Triple local;
        Triple ids;
    };

    bool refine() {
        int m=work.ids.size();
        vector<int> ids=work.ids, type(m,-1);
        for(int v=0;v<4;v++) for(int id:reps[v]) {
            for(auto& r:work.rows) r.v[ids.size()]=v;
            ids.push_back(id);
            type.push_back(v);
        }
        bool finalGroup=generated==n && queue.empty();
        vector<char> isBad(work.rows.size());
        bool mixed=false;
        if(finalGroup && max({counts[1],counts[2],counts[3]})<=1) {
            for(int i=0;i<(int)isBad.size();i++) isBad[i]=bad(work.rows[i]);
            int b=count(isBad.begin(),isBad.end(),1);
            if(b==(int)isBad.size()) ambiguous();
            mixed=b>0;
        }

        vector<Query> qs;
        for(int a=0;a<m;a++) for(int b=a+1;b<(int)ids.size();b++) {
            if(b>m && type[b]==type[b-1]) continue;
            for(int c=b+1;c<(int)ids.size();c++) {
                if(c>m && type[c]==type[c-1] && c!=b+1) continue;
                qs.push_back({{a,b,c},{ids[a],ids[b],ids[c]}});
            }
        }
        double total=0;
        for(auto& r:work.rows) total+=r.weight;
        for(auto& r:work.rows) r.weight=max(1e-60,r.weight/total);
        if(finalGroup && (work.rows.size()<=20 || (n<=36 && work.rows.size()<=1800))) {
            SmallSolver solver;
            solver.bad.assign(isBad.begin(),isBad.end());
            for(const auto& q:qs) solver.qs.push_back(q.ids);
            for(const auto& r:work.rows) {
                vector<unsigned char> sign;
                sign.reserve(qs.size());
                for(auto& q:qs) {
                    auto [a,b,c]=q.local;
                    sign.push_back(areaCode[r.v[a]][r.v[b]][r.v[c]]);
                }
                solver.sig.push_back(move(sign));
                solver.weight.push_back(r.weight*work.rows.size());
            }
            int which=solver.identify(false);
            for(int i=0;i<m;i++) known[ids[i]]=work.rows[which].v[i];
            answer(known);
        }
        double best=-1e100;
        int choice=-1;
        for(int q=0;q<(int)qs.size();q++) {
            auto [a,b,c]=qs[q].local;
            Histogram hist{};
            array<double,R> ambHist{};
            int first=-1;
            bool split=false;
            for(int i=0;i<(int)work.rows.size();i++) {
                auto& r=work.rows[i];
                int x=areaCode[r.v[a]][r.v[b]][r.v[c]];
                if(first<0) first=x;
                else if(first!=x) split=true;
                hist[x]+={r.weight,1./work.rows.size()};
                if(mixed && isBad[i]) ambHist[x]+=r.weight;
            }
            if(!split) continue;
            if(cache.count(key(qs[q].ids))) {choice=q; break;}
            double h=entropy(hist);
            if(mixed) for(double v:ambHist) if(v>0) h+=v*log(v);
            if(h>best+1e-12) {best=h; choice=q;}
        }
        Triple selected{};
        if(choice>=0) selected=qs[choice].ids;
        vector<int> extra;

        if(!queue.empty() && clock()-start<3*CLOCKS_PER_SEC) {
            // Compare queries across independent groups by their one- and
            // two-position marginals, without expanding their Cartesian product.
            vector<Single> one(m);
            vector<Pair> two(m*m);
            for(const auto& r:work.rows) for(int i=0;i<m;i++) {
                Mass w{r.weight,1./work.rows.size()};
                one[i][r.v[i]]+=w;
                for(int j=i+1;j<m;j++)
                    two[i*m+j][4*r.v[i]+r.v[j]]+=w;
            }
            vector<int> fresh;
            array<int,R+3> used{};
            for(int j=0;j<(int)queue.size();j++) {
                auto& g=queue[j];
                if(work.rows.size()*g.rows.size()>6000 || m+g.ids.size()>15) continue;
                if(used[g.kind]++<2) fresh.push_back(j);
            }
            auto consider=[&](double score,Triple q,int j,int k=-1) {
                if(score<=max(1e-14,best+1e-11)) return;
                best=score;
                selected=q;
                extra={j};
                if(k>=0) extra.push_back(k);
            };
            array<bool,R+3> considered{};
            for(int j:fresh) {
                const auto& g=queue[j];
                if(considered[g.kind]) continue;
                considered[g.kind]=true;
                for(int i=0;i<m;i++) {
                    for(int t=i+1;t<m;t++)
                        consider(pairScore(two[i*m+t],g.one),
                                 {ids[i],ids[t],g.ids[0]},j);
                    if(g.ids.size()>=2)
                        consider(pairScore(g.two,one[i]),{ids[i],g.ids[0],g.ids[1]},j);

                    Pair combined{};
                    for(int x=0;x<4;x++) for(int y=0;y<4;y++)
                        combined[4*x+y]=one[i][x]*g.one[y];
                    for(int v=0;v<4;v++) if(!reps[v].empty()) {
                        Histogram h{};
                        for(int x=0;x<4;x++) for(int y=0;y<4;y++)
                            h[areaCode[x][y][v]]+=combined[4*x+y];
                        consider(entropy(h),{ids[i],g.ids[0],reps[v][0]},j);
                    }
                    array<bool,R+3> paired{};
                    for(int k:fresh) {
                        const auto& other=queue[k];
                        if(k<=j || paired[other.kind] ||
                           work.rows.size()*g.rows.size()*other.rows.size()>6000 ||
                           m+g.ids.size()+other.ids.size()>15) continue;
                        paired[other.kind]=true;
                        consider(pairScore(combined,other.one),
                                 {ids[i],g.ids[0],other.ids[0]},j,k);
                    }
                }
            }
        }
        if(choice<0 && extra.empty()) return false;
        int result=ask(selected);
        sort(extra.rbegin(),extra.rend());
        for(int j:extra) append(j);
        Triple at;
        for(int i=0;i<3;i++) {
            if(known[selected[i]]>=0) {
                at[i]=work.ids.size()+i;
                for(auto& r:work.rows) r.v[at[i]]=known[selected[i]];
            } else {
                at[i]=find(work.ids.begin(),work.ids.end(),selected[i])-work.ids.begin();
            }
        }
        erase_if(work.rows,[&](const Row& r){
            return areaCode[r.v[at[0]]][r.v[at[1]]][r.v[at[2]]]!=result;
        });
        compact();
        return true;
    }

    void run() {
        for (;;) {
            // An injective pair decodes every remaining position directly.
            // Reserve enough CPU time for completion on costly search paths.
            if(clock()-start>=3*CLOCKS_PER_SEC) {
                for(int a=0;a<4;a++) for(int b=a;b<4;b++) {
                    if(reps[a].empty() || reps[b].size()<=size_t(a==b)) continue;
                    array<int,R> inverse;
                    inverse.fill(-1);
                    for(int v=0;v<4;v++) inverse[areaCode[a][b][v]]=v;
                    if(count(inverse.begin(),inverse.end(),-1)!=R-4) continue;
                    int i=reps[a][0], j=reps[b][a==b];
                    for(int k=0;k<n;k++) if(known[k]<0)
                        known[k]=inverse[ask({i,j,k})];
                    answer(known);
                }
            }
            if(work.ids.empty()) {
                if(queue.empty()) {
                    if(generated==n) answer(known);
                    generate();
                    if(queue.empty()) continue;
                }
                append(0);
            }
            // When only a few exceptional triples interrupt a run of ones,
            // finish probing fresh triples before resolving their orientations.
            // Several different orientations can all call for the answer -1.
            if(generated<n && counts[0]>=9 && counts[0]+9>=generated &&
               counts[1]+counts[2]+counts[3]==0) {
                generate();
                continue;
            }
            if(generated==n && !queue.empty()) {
                size_t combinations=work.rows.size(), width=work.ids.size();
                size_t limit=n<=36 ? 1800 : 128;
                for(const auto& g:queue) {
                    combinations*=g.rows.size();
                    width+=g.ids.size();
                    if(combinations>limit || width>15) break;
                }
                if(combinations<=limit && width<=15) {
                    while(!queue.empty()) append(queue.size()-1);
                }
            }
            if(refine()) continue;
            if(!queue.empty()) append(0);
            else if(generated<n) generate();
            else ambiguous();
        }
    }
};
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    if(!(cin>>n)) return 0;
    for(int a=1;a<=4;a++) for(int b=1;b<=4;b++) for(int c=1;c<=4;c++) {
        int x=max(0,(a+b+c)*(a+b-c)*(a+c-b)*(b+c-a));
        if(!codes.count(x)) {
            int id=codes.size();
            codes[x]=id;
        }
        areaCode[a-1][b-1][c-1]=codes[x];
    }
    if(n<=8) SmallSolver().run();
    else BlockSolver().run();
}
