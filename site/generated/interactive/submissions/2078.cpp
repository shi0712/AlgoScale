#include <bits/stdc++.h>
using namespace std;

int ask(int a, int b) {
    cout << "? " << a + 1 << ' ' << b + 1 << endl;
    int g;
    if (!(cin >> g) || g < 1) exit(0);
    return g;
}

[[noreturn]] void answer(int a, int b) {
    cout << "! " << a + 1 << ' ' << b + 1 << endl;
    int verdict;
    cin >> verdict;
    exit(0);
}

// Randomize the positions so that fresh disjoint pairs cover the permutation
// quickly. Inspect an already touched position when its replies favor zero.
// Only arithmetic deductions can remove a position or trigger the answer.
struct Search {
    int n, limit, live;
    vector<int> order, multiple, largest, observed;
    vector<double> logFactors, weight;
    vector<bool> possible;
    vector<vector<pair<int,int>>> edges;
    vector<vector<int>> divisors;
    vector<int> divisible, phiSum;
    unordered_map<long long,int> known;
    unordered_set<long long> incident;
    set<pair<double,int>, greater<>> priority;
    set<pair<int,int>, greater<>> witnesses;
    int next = 0;

    Search(int size, vector<int> permutation)
        : n(size), limit(n-1), live(n), order(move(permutation)), multiple(n,1),
          largest(n), observed(n), logFactors(n), weight(n,1), possible(n,true),
          edges(n), divisors(n), divisible(n), phiSum(n) {
        vector<int> phi(n);
        iota(phi.begin(), phi.end(), 0);
        for (int d=1; d<n; ++d) {
            if (d>=2 && phi[d]==d)
                for (int j=d; j<n; j+=d) phi[j]-=phi[j]/d;
            for (int j=d; j<n; j+=d) divisors[j].push_back(d);
            phiSum[d]=phiSum[d-1]+phi[d];
        }
        known.reserve(2*n);
        incident.reserve(4*n);
    }

    long long key(int i, int j) const {
        return (long long) min(i,j)*n+max(i,j);
    }

    void remove(int i) {
        if (!possible[i]) return;
        possible[i]=false;
        --live;
        priority.erase({weight[i],i});
        if (live<=2) {
            int a=-1, b=-1;
            for (int k=0;k<n;++k) if (possible[k]) {
                if (a<0) a=k;
                b=k;
            }
            answer(a,b);
        }
    }

    // If i is zero, its neighbors' replies are their distinct exact values.
    // If it is positive, its value must be a multiple of every reply at i.
    void update(int i, int j, int g) {
        int old=multiple[i];
        int now=lcm(old,g);
        if (now>limit) answer(i,i);
        if (!incident.insert((long long)i*n+g).second) remove(i);
        if (now!=old) {
            witnesses.erase({old,i});
            witnesses.emplace(now,i);
            multiple[i]=now;
            for (auto [k,h]:edges[i])
                if (h!=now) remove(k);
            // At most floor(limit/d) positive values are divisible by d.
            for (int d:divisors[now]) if (old%d) {
                if (++divisible[d] > limit/d)
                    for (int k=0;k<n;++k) if (multiple[k]%d) remove(k);
            }
        }
        if (now!=g) remove(j);
        edges[i].emplace_back(j,g);
        if (possible[i]) priority.erase({weight[i],i});
        ++observed[i];
        largest[i]=max(largest[i],g);
        logFactors[i]+=log(double(g)/limit);
        const int m=limit/largest[i];
        // Bayes factor from one edge relative to the initial odds of zero.
        weight[i]=double(limit)/(2.0*phiSum[m]-1);
        if (observed[i]>1) {
            // Approximate the star likelihood for choosing a query only.
            weight[i]*=double(now)/largest[i]*exp(logFactors[i]
                        -log(double(largest[i])/limit)+(observed[i]-1)*0.42);
        }
        if (possible[i]) priority.emplace(weight[i],i);
    }

    void learn(int a, int b, int g) {
        if (2*g>limit) answer(a,b);
        known[key(a,b)]=g;
        update(a,b,g);
        update(b,a,g);
    }

    void query(int a, int b) {
        if (!known.count(key(a,b))) learn(a,b,ask(a,b));
    }

    // A witness with a new divisor either rules out a as zero or
    // increases the known multiple at a. When their lcm exceeds the
    // limit, one comparison settles the hypothesis a=0.
    int witness(int a, bool decisive) const {
        int best=-1, checked=0;
        double bestScore=-1;
        for (auto [d,b]:witnesses) {
            if (++checked>48) break;
            if (multiple[a]%d==0 || known.count(key(a,b))) continue;
            int combined=lcm(multiple[a],d);
            if (decisive && combined<=limit) continue;
            double score=(combined>limit ? 2.0 : 0.0)
                        + 1.0-double(gcd(multiple[a],d))/d
                        + (possible[b] ? 0.3*min(1.0,weight[b]/max(weight[a],1.0)) : 0.0);
            if (score>bestScore) best=b,bestScore=score;
        }
        return best;
    }

    void run() {
        const int fresh=order.size();
        while (next<fresh) {
            if (!possible[order[next]]) { ++next; continue; }
            int a=priority.empty() ? -1 : priority.begin()->second;
            if (a!=-1 && (weight[a]>0.85 || next+1==fresh)) {
                int b=-1, count=0;
                for (auto [w,i]:priority) {
                    if (w<=0.85 || ++count>12) break;
                    if (i!=a && !known.count(key(a,i))) {
                        b=i;
                        break;
                    }
                }
                if (b<0 && weight[a]>0.18*limit) b=witness(a,true);
                if (b<0) b=order[next++];
                query(a,b);
            } else {
                int u=order[next++];
                if (next<fresh) query(u,order[next++]);
                else if (!priority.empty()) query(u,priority.begin()->second);
            }
        }
        while (true) {
            // Compare plausible endpoints of different pairs when possible:
            // a single reply can rule out a candidate from each pair.
            auto it=priority.begin();
            int a=(it++)->second;
            int b=-1, scanned=0;
            double bestScore=-1;
            for (auto other=it;other!=priority.end() && scanned<24;++other,++scanned) {
                auto [w,j]=*other;
                if (w<0.10*weight[a] || known.count(key(a,j))) continue;
                int both=lcm(multiple[a],multiple[j]);
                double score=w/max(weight[a],1e-100)
                    + 0.65*(both>limit)
                    + 0.15*(multiple[a]==multiple[j])
                    + 0.12*(both!=multiple[a] && both!=multiple[j]);
                if (score>bestScore) bestScore=score,b=j;
            }
            if (b<0) b=witness(a,false);
            if (b!=-1) { query(a,b); continue; }
            // For three candidates, two comparisons at a common index
            // always eliminate one, including when the replies are equal.
            b=(it++)->second;
            int c=it->second;
            query(a,b);
            if (possible[a] && possible[b] && possible[c]) query(a,c);
        }
    }
};

// For the smallest cases, minimize the sum of future query counts over all
// permutations still consistent with the transcript. A nonterminal state S
// costs |S| for its next query, plus the optimal costs of the reply classes.
class ExactTree {
    struct Row {
        array<unsigned char,28> reply;
        int zero;
    };
    struct Entry { int lower, upper, choice; };
    struct Option { int query, lower; double priority; };

    int n;
    vector<pair<int,int>> pairs;
    vector<Row> rows;
    unordered_map<uint64_t,Entry> memo;

    int support(const vector<int> &ids) const {
        int mask=0;
        for (int id:ids) mask|=1<<rows[id].zero;
        return mask;
    }

    int optimize(const vector<int> &ids, int cap, int touched, uint64_t history) {
        int size=ids.size();
        auto &entry=memo.try_emplace(history,Entry{size,INT_MAX,-1}).first->second;
        if (entry.lower>=cap || entry.lower==entry.upper) return entry.lower;
        int bound=min(cap,entry.upper);
        int unused=((1<<n)-1)^touched;
        int options=touched;
        for (int k=0;k<2 && unused;++k) {
            options|=unused&-unused;
            unused&=unused-1;
        }

        vector<Option> choices;
        for (int q=0;q<(int)pairs.size();++q) {
            auto [a,b]=pairs[q];
            if (history>>(2*q)&3 || ((1<<a | 1<<b)&~options)) continue;
            int counts[8]{}, masks[8]{};
            for (int id:ids) {
                int g=rows[id].reply[q];
                ++counts[g];
                masks[g]|=1<<rows[id].zero;
            }
            int lower=size, outcomes=0;
            double rank=0;
            for (int g=1;g<n;++g) if (counts[g]) {
                ++outcomes;
                int positions=popcount(unsigned(masks[g]));
                if (positions>2) {
                    lower+=counts[g];
                    rank+=counts[g]*(1+0.25*positions);
                }
            }
            if (outcomes>1 && lower<bound) choices.push_back({q,lower,rank});
        }
        sort(choices.begin(),choices.end(),[](auto &a,auto &b){return a.priority<b.priority;});
        for (auto [q,lower,rank]:choices) {
            if (lower>=bound) continue;
            array<vector<int>,8> groups;
            int masks[8]{};
            for (int id:ids) {
                int g=rows[id].reply[q];
                groups[g].push_back(id);
                masks[g]|=1<<rows[id].zero;
            }
            int cost=lower;
            auto [a,b]=pairs[q];
            for (int g=1;g<n && cost<bound;++g) if (popcount(unsigned(masks[g]))>2) {
                cost-=groups[g].size();
                // A nonterminal reply is at most three for n <= 8.
                cost+=optimize(groups[g],bound-cost,touched|1<<a|1<<b,
                               history|(uint64_t(g)<<(2*q)));
            }
            if (cost<bound) {
                bound=entry.upper=cost;
                entry.choice=q;
            }
        }
        entry.lower=max(entry.lower,bound);
        return entry.lower;
    }

public:
    explicit ExactTree(int size):n(size) {
        for (int a=0;a<n;++a) for (int b=a+1;b<n;++b) pairs.emplace_back(a,b);
    }

    void run(const vector<int> &names) {
        array<int,8> p{};
        iota(p.begin(),p.begin()+n,0);
        do {
            Row row{};
            row.zero=find(p.begin(),p.begin()+n,0)-p.begin();
            for (int q=0;q<(int)pairs.size();++q) {
                auto [a,b]=pairs[q];
                row.reply[q]=gcd(p[a],p[b]);
            }
            rows.push_back(row);
        } while (next_permutation(p.begin(),p.begin()+n));
        vector<int> ids(rows.size());
        iota(ids.begin(),ids.end(),0);
        uint64_t history=0;
        int touched=0,q=0;
        while (true) {
            auto [a,b]=pairs[q];
            int g=ask(names[a],names[b]);
            if (2*g>=n) answer(names[a],names[b]);
            erase_if(ids,[&](int id){return rows[id].reply[q]!=g;});
            unsigned positions=support(ids);
            if (popcount(positions)<=2)
                answer(names[countr_zero(positions)],names[bit_width(positions)-1]);
            touched|=1<<a|1<<b;
            history|=uint64_t(g)<<(2*q);
            optimize(ids,INT_MAX,touched,history);
            q=memo.at(history).choice;
        }
    }
};

template<int K>
struct Sample {
    array<unsigned char,K> p{};
    int zero=0;
    double mass=1;
};

template<int K>
class Planner {
    using Row=Sample<K>;
    using Mask=uint64_t;
    int n;
    mt19937 &rng;
    int table[K][K]{}, history[K][K]{}, degree[K]{};
    Mask compatible[K][K]{};
    vector<pair<int,int>> neighbors[K];
    vector<int> names;
    array<double,K+1> factorial{};

    void record(int a,int b,int g) {
        history[a][b]=history[b][a]=g;
        ++degree[a]; ++degree[b];
        neighbors[a].emplace_back(b,g);
        neighbors[b].emplace_back(a,g);
    }

    double estimate(array<double,K> &p, double total) const {
        sort(p.begin(),p.begin()+n,greater<>());
        int count=0;
        while (count<n && p[count]>0) ++count;
        if (count<=2) return 0;
        double cost=0;
        int last=(count-1)/2;
        for (int j=0;j<count;++j)
            cost+=p[j]*(min(1+j/2,last)+0.12*log2(j+1.0));
        return cost+0.012*total*count;
    }

    struct Choice { double cost; int a,b; };

    Choice choose(const vector<Row> &rows, int depth) {
        array<bool,K> present{};
        double mass=0;
        for (auto &s:rows) present[s.zero]=true,mass+=s.mass;
        if (count(present.begin(),present.end(),true)<=2)
            return {0,-1,-1};
        vector<int> options;
        int fresh=0;
        for (int i=0;i<n;++i) if (degree[i] || ++fresh<=2) options.push_back(i);
        vector<Choice> choices;
        for (int a:options) for (int b:options) if (b>a && !history[a][b]) {
            array<array<double,K>,K> bucket{};
            array<double,K> totals{};
            for (auto &s:rows) {
                int g=table[s.p[a]][s.p[b]];
                bucket[g][s.zero]+=s.mass;
                totals[g]+=s.mass;
            }
            double cost=0;
            int outcomes=0;
            for (int g=1;g<n;++g) if (totals[g]>0) {
                ++outcomes;
                if constexpr (K>10) {
                    // Positions still untouched after this comparison
                    // have exactly the same posterior probability.
                    int unseen=0;
                    double sum=0;
                    for (int i=0;i<n;++i) if (!degree[i] && i!=a && i!=b)
                        sum+=bucket[g][i], ++unseen;
                    if (unseen)
                        for (int i=0;i<n;++i) if (!degree[i] && i!=a && i!=b)
                            bucket[g][i]=sum/unseen;
                }
                cost+=estimate(bucket[g],totals[g]);
            }
            choices.push_back({1+cost/mass+(outcomes<=1 ? 100.0 : 0.0),a,b});
        }
        sort(choices.begin(),choices.end(),[](auto &x,auto &y){return x.cost<y.cost;});
        if (choices.empty()) return {0,-1,-1};
        if (!depth || choices.front().cost<=1.000000001) return choices.front();
        Choice best={1e100,0,1};
        for (int k=0;k<min(4,(int)choices.size());++k) {
            auto [score,a,b]=choices[k];
            if (score>=100) break;
            array<vector<Row>,K> branches;
            for (auto &s:rows) branches[table[s.p[a]][s.p[b]]].push_back(s);
            history[a][b]=history[b][a]=1;
            ++degree[a]; ++degree[b];
            double cost=1;
            for (auto &child:branches) if (!child.empty()) {
                auto nextChoice=choose(child,depth-1);
                cost+=nextChoice.cost*child.size()/rows.size();
                if (cost>=best.cost) break;
            }
            --degree[a]; --degree[b];
            history[a][b]=history[b][a]=0;
            if (cost<best.cost) best={cost,a,b};
        }
        return best.cost==1e100 ? choices.front() : best;
    }

    using Domains=array<Mask,K>;
    static Mask bit(int val) { return Mask(1)<<val; }

    // Arc consistency and the uniqueness of fixed permutation values.
    bool initial(int z, Domains &dom) const {
        Mask all=bit(n)-1;
        dom.fill(all-1);
        dom[z]=1;
        bool changed=true;
        while (changed) {
            changed=false;
            Mask fixed=0;
            for (int i=0;i<n;++i) {
                if (!dom[i]) return false;
                if (has_single_bit(dom[i])) {
                    if (fixed&dom[i]) return false;
                    fixed|=dom[i];
                }
            }
            for (int i=0;i<n;++i) {
                Mask d=dom[i];
                if (!has_single_bit(d)) d&=~fixed;
                for (auto [j,g]:neighbors[i]) {
                    Mask candidates=d;
                    while (candidates) {
                        int v=countr_zero(candidates);
                        candidates&=candidates-1;
                        if (!(compatible[v][g]&dom[j])) d^=bit(v);
                    }
                }
                if (d!=dom[i]) dom[i]=d,changed=true;
            }
        }
        return true;
    }

    bool draw(Domains dom, Mask remaining, Row &s, bool search, int &budget) {
        Mask unused=bit(n)-1;
        for (int i=0;i<n;++i) if (s.p[i]<n) unused^=bit(s.p[i]);
        while (remaining) {
            int fewest=n+1, a=-1;
            Mask vars=remaining, choices=0;
            while (vars) {
                int i=countr_zero(vars);
                vars&=vars-1;
                Mask opts=unused&dom[i];
                int cnt=popcount(opts);
                if (cnt<fewest) fewest=cnt,a=i,choices=opts;
            }
            if (!choices) return false;
            remaining^=bit(a);
            s.mass*=fewest;
            if (search && fewest>1) {
                while (choices) {
                    if (--budget<0) return false;
                    int v=countr_zero(choices);
                    choices&=choices-1;
                    Row tmp=s;
                    tmp.p[a]=v;
                    Domains restricted=dom;
                    for (auto [j,g]:neighbors[a]) restricted[j]&=compatible[v][g];
                    if (draw(restricted,remaining,tmp,true,budget)) {
                        s=tmp;
                        return true;
                    }
                }
                return false;
            }
            if (fewest>1) {
                int skip=uniform_int_distribution<int>(0,fewest-1)(rng);
                while (skip--) choices&=choices-1;
            }
            int v=countr_zero(choices);
            s.p[a]=v;
            unused^=bit(v);
            for (auto [j,g]:neighbors[a]) dom[j]&=compatible[v][g];
        }
        array<int,K> values;
        int cnt=0;
        while (unused) {
            values[cnt++]=countr_zero(unused);
            unused&=unused-1;
        }
        s.mass*=factorial[cnt];
        shuffle(values.begin(),values.begin()+cnt,rng);
        for (int i=0;i<n;++i) if (s.p[i]>=n) s.p[i]=values[--cnt];
        return true;
    }

    void report(const vector<int> &live) {
        if (live.size()<=2) answer(names[live.front()],names[live.back()]);
    }

    // A bounded exhaustive search can certify an impossible zero position.
    // Random completions only rank queries. Their weights compensate for
    // drawing uniformly among the remaining choices at each assignment.
    void sampled(vector<Row> &rows) {
        vector<int> live;
        Mask touched=0;
        for (int i=0;i<n;++i) if (degree[i]) touched|=bit(i);
        for (int z=0;z<n;++z) {
            Domains dom;
            if (!initial(z,dom)) continue;
            Row base;
            base.zero=z;
            base.p.fill(255);
            Mask vars=touched|bit(z);
            int budget=400;
            Row example=base;
            bool feasible=draw(dom,vars,example,true,budget);
            if (!feasible && budget>=0) continue;
            live.push_back(z);
            size_t start=rows.size();
            for (int t=0;t<160;++t) {
                Row s=base;
                if (draw(dom,vars,s,false,budget)) rows.push_back(s);
            }
            if (start==rows.size() && feasible) {
                example.mass*=1.e-4;
                rows.push_back(example);
            }
        }
        report(live);
    }

public:
    Planner(int size, mt19937 &gen, vector<int> permutation)
        : n(size),rng(gen),names(move(permutation)) {
        factorial[0]=1;
        for (int i=1;i<=n;++i) factorial[i]=factorial[i-1]*i;
        for (int i=0;i<n;++i) for (int j=0;j<n;++j) {
            table[i][j]=gcd(i,j);
            if (i!=j) compatible[i][table[i][j]]|=bit(j);
        }
    }

    void resumeSearch() const {
        vector<int> fresh;
        for (int i=0;i<n;++i) if (!degree[i]) fresh.push_back(names[i]);
        Search solver(n,move(fresh));
        for (int i=0;i<n;++i) for (auto [j,g]:neighbors[i]) if (i<j)
            solver.learn(names[i],names[j],g);
        solver.run();
    }

    void run() {
        vector<Row> rows;
        clock_t start=clock();
        int a=0,b=1;
        while (true) {
            int g=ask(names[a],names[b]);
            if (2*g>=n) answer(names[a],names[b]);
            record(a,b,g);
            if constexpr (K==10) {
                if (rows.empty()) {
                    Row s;
                    iota(s.p.begin(),s.p.begin()+n,0);
                    do {
                        if (table[s.p[a]][s.p[b]]!=g) continue;
                        s.zero=find(s.p.begin(),s.p.begin()+n,0)-s.p.begin();
                        rows.push_back(s);
                    } while (next_permutation(s.p.begin(),s.p.begin()+n));
                } else {
                    erase_if(rows,[&](auto &s){return table[s.p[a]][s.p[b]]!=g;});
                }
                array<bool,K> seen{};
                vector<int> live;
                for (auto &s:rows) if (!seen[s.zero]) {
                    seen[s.zero]=true;
                    live.push_back(s.zero);
                }
                report(live);
            } else {
                if (clock()-start>2*CLOCKS_PER_SEC) resumeSearch();
                rows.clear();
                sampled(rows);
            }
            int depth=0;
            if constexpr (K==10) {
                if (rows.size()<=45000) depth=1;
            }
            auto pick=choose(rows,depth);
            if (pick.a<0) resumeSearch();
            a=pick.a;
            b=pick.b;
        }
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    int n;
    if (!(cin>>n) || n<2) return 0;
    if (n==2) answer(0,1);
    mt19937 rng(random_device{}());
    vector<int> order(n);
    iota(order.begin(),order.end(),0);
    shuffle(order.begin(),order.end(),rng);
    if (n<=8) ExactTree(n).run(order);
    else if (n<=10) Planner<10>(n,rng,move(order)).run();
    else if (n<=42) Planner<42>(n,rng,move(order)).run();
    else Search(n,move(order)).run();
}
