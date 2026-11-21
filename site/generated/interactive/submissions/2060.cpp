#include <bits/stdc++.h>
using namespace std;

// capacity[d][b] is the maximum number of free candidates that can
// coexist with b forced candidates and be solved in d more questions.
using Row = vector<int>;

class NextCapacity {
    const Row &f;
    static constexpr int scale = 10946, slope = 6765;
    vector<vector<int>> residual;

    int maximum(int l, int r) const {
        int k = __lg(r - l + 1);
        return max(residual[k][l], residual[k][r - (1 << k) + 1]);
    }

    void search(int a, int l, int r, int &best) const {
        // The slope is only for pruning. The bound is exact for any slope.
        long long bound = (long long)maximum(l, r) + maximum(a-r, a-l);
        if (bound < (long long)(best+1)*scale + (long long)a*slope) return;
        if (r-l <= 24) {
            for (int i=l; i<=r; ++i) best = max(best, f[i]+f[a-i]);
        } else {
            int mid = (l+r)/2;
            best = max(best, f[mid]+f[a-mid]);
            search(a, l, mid, best);
            search(a, mid+1, r, best);
        }
    }

public:
    explicit NextCapacity(const Row &previous) : f(previous) {
        int m = f.size();
        residual.emplace_back(m);
        for (int i=0; i<m; ++i) residual[0][i] = scale*f[i]+slope*i;
        for (int step=1; 2*step<=m; step*=2) {
            vector<int> level(m-2*step+1);
            const auto &last = residual.back();
            for (int i=0; i<(int)level.size(); ++i)
                level[i] = max(last[i], last[i+step]);
            residual.push_back(move(level));
        }
    }

    Row build(int n) const {
        Row next(min(n, 2*f[0])+1, -1);
        int l=0;
        for (int a=0; a<=n; ++a) {
            // The a free candidates repeat on exactly one branch each.
            // Split their repeat counts as i and a-i. The opposite group
            // switches on each branch, consuming a-i and i free slots.
            l = max(l, a-(int)f.size()+1);
            while (l<=a/2 && f[l]+l<a) ++l;
            if (l>a/2) break;
            int r=a/2;
            int best=max(f[l]+f[a-l], f[r]+f[a-r]);
            search(a, l, r, best);
            int b=min(n, best-a);
            next[b]=a;
        }
        for (int b=(int)next.size()-2; b>=0; --b)
            next[b]=max(next[b], next[b+1]);
        return next;
    }
};

constexpr double invPhi = 0.6180339887498948482;

bool fits(const Row &f, int a, int b) {
    return b < (int)f.size() && a <= f[b];
}

class Costs {
    static constexpr int limit = 64;
    double small[limit+1][limit+1]{};
    double factor[100]{};
public:
    vector<Row> capacity{{2, 1, 0}};

    explicit Costs(int n) {
        while (capacity.back()[0]<n)
            capacity.push_back(NextCapacity(capacity.back()).build(n));
        factor[0]=1;
        for (int i=1; i<100; ++i) factor[i]=factor[i-1]*(1+invPhi)/2;

        // Break minimax ties by expected length under the maximum-entropy
        // legal honesty process. A free state has mass 1, a forced one 1/phi.
        for (int d=1; d<(int)capacity.size(); ++d) {
            const Row &f=capacity[d-1];
            for (int a=0; a<=min(limit,n); ++a) {
                for (int b=0; a+b<=min(limit,n); ++b) {
                    if (a+b<=2 || fits(f,a,b) || !fits(capacity[d],a,b)) continue;
                    if (a+b==3 && b>=2) {
                        small[a][b]=1-(a ? 1/(a+b*invPhi) : 1.0/3);
                        continue;
                    }
                    double best=1e10;
                    for (int x=0; x<=a; ++x) {
                        if (a-x >= (int)f.size() || x >= (int)f.size()) continue;
                        int low=max(0, a+b-x-f[x]);
                        int high=min(b, f[a-x]-x);
                        for (int y=low; y<=high; ++y) {
                            double z=(x+y+(a-x)*invPhi)*small[x+y][a-x];
                            z+=(a+b-x-y+x*invPhi)*small[a+b-x-y][x];
                            best=min(best,z);
                        }
                    }
                    small[a][b]=1+best/((1+invPhi)*(a+b*invPhi));
                }
            }
        }
    }

    int depth(int a, int b) const {
        int lo=0, hi=(int)capacity.size()-1;
        while (lo<hi) {
            int mid=(lo+hi)/2;
            if (fits(capacity[mid],a,b)) hi=mid;
            else lo=mid+1;
        }
        return lo;
    }

    double estimate(int a, int b) const {
        if (a+b<=limit) return small[a][b];
        // Rescale a large state to the endgame table. Each balanced question
        // reduces its mass by the same factor used here.
        int t=lower_bound(factor, factor+99, (limit-2.0)/(a+b), greater<double>())-factor;
        double u=a*factor[t], v=b*factor[t];
        int i=(int)u, j=(int)v;
        u-=i; v-=j;
        return t+(1-u)*((1-v)*small[i][j]+v*small[i][j+1])
                +u*((1-v)*small[i+1][j]+v*small[i+1][j+1]);
    }
};

// Alongside a distribution giving every legal transcript positive weight,
// track repeat rates and short periodic honesty sequences. These affect only
// the choice among questions that meet the minimax bound.
struct Cycles {
    uint64_t lies[64]{}, all=0;
    Cycles() {
        int k=0;
        for (int p=2; p<=6; ++p) {
            for (int m=0; m<(1<<p); ++m) {
                auto bit=[&](int i){return (m>>(i%p))&1;};
                bool valid=true;
                for (int i=0; i<p; ++i)
                    if (bit(i)==bit(i+1) && bit(i)==bit(i+2)) valid=false;
                for (int s=1; s<p; ++s) if (p%s==0) {
                    bool same=true;
                    for (int i=s; i<p; ++i) if (bit(i)!=bit(i-s)) same=false;
                    if (same) valid=false;
                }
                if (!valid) continue;
                uint64_t flag=1ULL<<k++;
                for (int t=0; t<64; ++t) if (bit(t)) lies[t]|=flag;
                all|=flag;
            }
        }
    }
};

struct Candidate {
    int id;
    bool lie=false, forced=false;
    unsigned char repeats[2]{}, choices[2]{};
    double fixed=0.2, learned=0.2;
    uint64_t cycles;
    double repeatChance() const {
        return (repeats[lie]+0.5)/(choices[lie]+1.0);
    }
};

struct Prediction {
    double total, lie;
};

Prediction predict(const Candidate &c, const Cycles &cycles, int t) {
    double unit=0.2/__builtin_popcountll(cycles.all);
    double total=c.fixed+c.learned+unit*__builtin_popcountll(c.cycles);
    double repeat=c.forced ? 0 : (1-invPhi)*c.fixed+c.repeatChance()*c.learned;
    double lie=c.lie ? repeat : c.fixed+c.learned-repeat;
    lie+=unit*__builtin_popcountll(c.cycles & cycles.lies[t]);
    return {total,lie};
}

// Track reply policies as well as per-student honesty histories. A policy's
// influence is proportional to the likelihood of the replies seen so far.
struct ReplyModels {
    array<double,3> mass;
    explicit ReplyModels(int n) : mass{0.02*n,0.02*n,0.04*n} {}

    static array<double,3> chances(int free0,int forced0,int free1,int forced1) {
        if (free0+forced0==0) return {0,0,0};
        if (free1+forced1==0) return {1,1,1};
        int count=free0+forced0-free1-forced1;
        double weight=(free0-free1)+invPhi*(forced0-forced1);
        auto prefer=[](double advantage) {
            return advantage>0 ? 0.98 : advantage<0 ? 0.02 : 0.5;
        };
        return {prefer(count ? count : weight),prefer(weight),0.5};
    }

    void observe(const array<double,3> &p,int reply) {
        for (int k=0; k<3; ++k) mass[k]*=reply ? 1-p[k] : p[k];
    }
};

bool advance(Candidate &s,bool lie,const Cycles &cycles,int t) {
    bool repeat=(lie==s.lie);
    if (repeat && s.forced) return false;
    if (!s.forced) {
        s.fixed*=repeat ? 1-invPhi : invPhi;
        double probability=s.repeatChance();
        s.learned*=repeat ? probability : 1-probability;
        ++s.choices[s.lie];
        s.repeats[s.lie]+=repeat;
    }
    s.cycles&=lie ? cycles.lies[t] : cycles.all^cycles.lies[t];
    s.forced=repeat;
    s.lie=lie;
    return true;
}

struct Cut { int x=0,y=0; double probability=0; };
struct Run { int start,end; bool lie; };

// The previous two lie sets are circular intervals on the surviving order.
// A circular question is legal up to complement, which swaps its replies.
// To see why all aggregate splits (x,y) occur, consider the space of circular
// intervals. Moving an endpoint flips one candidate. The two endpoint indices
// form a square grid on a cylinder, capped by the empty and full intervals.
// Its image in (x,y) moves by one along a coordinate at each flip. The last
// two lie sets and their complements give the four corners of [0,a] x [0,b].
// Join successive corners by flipping only differing bits (shrink to the
// intersection, or grow to the union if the intersection is disconnected).
// The resulting loop maps around the rectangle and bounds a disk on the
// capped cylinder. Its grid faces map to points, edges or unit squares, so
// every lattice point of the rectangle is the image of some interval.
class Questions {
    const vector<Candidate> &c;
    const Costs &cost;
    const ReplyModels &models;
    int m, a=0, b=0, x0=0, y0=0;
    double outside=0, total=0;
    vector<Cut> prefix;
    vector<Run> runs;
    vector<double> cumulative;
    pair<int,int> chosen{-1,-1};
    double best=1e100;
    array<double,3> chosenChances{};
    vector<int> significant;
    vector<Prediction> endgame;

    const Cycles &history;
    int turn;
    bool lookAhead=false;
    struct Option {
        double score=1e100;
        int first=-1,last=-1;
        array<double,3> probabilities{};
    };
    array<Option,12> options;

    void tryInterval(int i,int j,int x,int y) {
        double world0=clamp(outside+prefix[j].probability-prefix[i].probability,0.0,total);
        int free0=x+y,forced0=a-x,free1=a+b-x-y,forced1=x;
        auto probabilities=ReplyModels::chances(free0,forced0,free1,forced1);
        double mass0=world0,mass1=total-world0;
        for (int k=0; k<3; ++k) {
            mass0+=models.mass[k]*probabilities[k];
            mass1+=models.mass[k]*(1-probabilities[k]);
        }
        double left=cost.estimate(free0,forced0),right=cost.estimate(free1,forced1);
        double score=mass0*left+mass1*right;
        if (!endgame.empty()) {
            for (int z=0; z<2; ++z) {
                int free=z ? free1 : free0, forced=z ? forced1 : forced0;
                if (free+forced!=3 || forced<2) continue;
                double guess=0;
                for (int k=0; k<m; ++k) {
                    bool lie=(i<=k && k<j)^z;
                    bool repeat=lie==c[k].lie;
                    if (repeat && (c[k].forced || free==1)) continue;
                    guess=max(guess,lie ? endgame[k].lie : endgame[k].total-endgame[k].lie);
                }
                // A reply policy can decline the first nomination. Under
                // the student models its success saves the final question.
                score+=(z ? mass1 : mass0)*(1-(z ? right : left))-guess;
            }
        }
        if (lookAhead) {
            // Retain several different splits for a second question of
            // lookahead. Reflecting a split exchanges its branches.
            int u=x,v=y;
            if (2*u>a || (2*u==a && 2*v>b)) {u=a-u; v=b-v;}
            int bin=min(3,8*u/max(1,a))*3+min(2,3*v/max(1,b));
            if (score<options[bin].score) options[bin]={score,i,j,probabilities};
        }
        if (score<best) {
            best=score;
            chosenChances=probabilities;
            chosen={i,j};
        }
    }

    void tryStart(int i,const Row &cap,bool exhaustive) {
        int baseX=x0-prefix[i].x, baseY=y0-prefix[i].y;
        for (auto [s,e,lie]:runs) {
            if (e<i) continue;
            int l=max(s,i),r=e;
            auto allowed=[&](int j,bool second) {
                int x=baseX+prefix[j].x, y=baseY+prefix[j].y;
                if (second) return fits(cap,a+b-x-y,x);
                return fits(cap,x+y,a-x);
            };
            // As the end moves through one run, one branch becomes easier
            // and the other harder. Find the entire admissible range of cuts.
            bool increasing=!lie;
            if (!allowed(r,increasing) || !allowed(l,!increasing)) continue;
            if (!allowed(l,increasing)) {
                int lo=l,hi=r;
                while (hi-lo>1) {
                    int mid=(lo+hi)/2;
                    if (allowed(mid,increasing)) hi=mid;
                    else lo=mid;
                }
                l=hi;
            }
            if (!allowed(l,!increasing)) continue;
            if (!allowed(r,!increasing)) {
                int lo=l,hi=r;
                while (hi-lo>1) {
                    int mid=(lo+hi)/2;
                    if (allowed(mid,!increasing)) lo=mid;
                    else hi=mid;
                }
                r=lo;
            }
            auto evaluate=[&](int j) {
                tryInterval(i,j,baseX+prefix[j].x,baseY+prefix[j].y);
            };
            if (!exhaustive) {
                for (int j:significant) if (l<=j && j<=r) evaluate(j);
                // Include both sides of a count tie and a future-mass tie.
                bool forced=c[e-1].forced;
                int x=baseX+prefix[l].x,y=baseY+prefix[l].y;
                int sign=lie ? -1 : 1;
                double weight=(2*x-a)*(1-invPhi)+(2*y-b);
                double tie=l-sign*weight/(2*(forced ? 1 : 1-invPhi));
                for (double center : {tie, forced ? l+sign*(b*0.5-y) : double(l)}) {
                    int k=(int)floor(center);
                    for (int j=max(l,k-1); j<=min(r,k+2); ++j) evaluate(j);
                }
            }
            int samples=exhaustive ? r-l : min(r-l,12);
            for (int k=0; k<=samples; ++k) {
                int j=samples ? l+(r-l)*k/samples : l;
                evaluate(j);
            }
        }
    }

public:
    Questions(const vector<Candidate> &active,const Costs &costs,
              const Cycles &cycles,int t,const ReplyModels &replyModels)
        : c(active),cost(costs),models(replyModels),m(c.size()),prefix(m+1),cumulative(m+1),history(cycles),turn(t) {
        for (int i=0; i<m; ++i) {
            const Candidate &s=c[i];
            int &count=s.forced ? b : a;
            int &base=s.forced ? y0 : x0;
            ++count;
            base+=s.lie;
            prefix[i+1]=prefix[i];
            (s.forced ? prefix[i+1].y : prefix[i+1].x)+=s.lie ? -1 : 1;
            auto p=predict(s,cycles,t);
            if (m<=8) endgame.push_back(p);
            outside+=p.total-p.lie;
            total+=p.total;
            cumulative[i+1]=total;
            prefix[i+1].probability+=2*p.lie-p.total;
            if (!i || s.forced!=c[i-1].forced || s.lie!=c[i-1].lie)
                runs.push_back({i,i+1,s.lie});
            else runs.back().end=i+1;
        }
    }

    void generate(int remaining,bool probe) {
        lookAhead=!probe;
        const Row &cap=cost.capacity[remaining-1];
        if (m<=80) {
            for (int i=0; i<=m; ++i) tryStart(i,cap,true);
        } else {
            vector<int> starts;
            for (auto [l,r,lie]:runs) {
                int steps=min(r-l,probe ? 12 : 40);
                for (int k=0; k<=steps; ++k) starts.push_back(l+(r-l)*k/steps);
            }
            for (int k=1; k<48; ++k) {
                int i=lower_bound(cumulative.begin(),cumulative.end(),total*k/48)-cumulative.begin();
                if (i && cumulative[i]-cumulative[i-1]>total/64) {
                    if (significant.empty() || significant.back()!=i) {
                        significant.push_back(i-1);
                        significant.push_back(i);
                    }
                }
                if (i) starts.push_back(i-1);
                starts.push_back(i);
            }
            sort(starts.begin(),starts.end());
            starts.erase(unique(starts.begin(),starts.end()),starts.end());
            for (int i:starts) tryStart(i,cap,false);
            if (chosen.first<0)
                for (int i=0; i<=m && chosen.first<0; ++i) tryStart(i,cap,false);
        }
    }

    double continuation(const Option &option,int remaining) const {
        double value=0;
        vector<Candidate> next;
        next.reserve(m);
        for (int z=0; z<=1; ++z) {
            next.clear();
            for (int k=0; k<m; ++k) {
                bool lie=(option.first<=k && k<option.last)^z;
                if (c[k].forced && c[k].lie==lie) continue;
                next.push_back(c[k]);
                advance(next.back(),lie,history,turn);
            }
            if (next.size()<=2) continue;
            ReplyModels nextModels=models;
            nextModels.observe(option.probabilities,z);
            double policyMass=accumulate(nextModels.mass.begin(),nextModels.mass.end(),0.0);
            Questions child(next,cost,history,turn+1,nextModels);
            if (next.size()==3 && child.b>=2) {
                double guess=0;
                for (int k=0; k<3; ++k)
                    if (child.b==3 || !next[k].forced) guess=max(guess,child.endgame[k].total);
                value+=policyMass+child.total-guess;
            } else {
                child.generate(remaining-1,true);
                value+=policyMass+child.total+child.best;
            }
        }
        return value;
    }

    pair<int,int> select(int n,int remaining,array<double,3> &p) {
        generate(remaining,false);
        sort(options.begin(),options.end(),[](const Option &u,const Option &v) {
            return u.score<v.score;
        });
        double bestFuture=1e100;
        for (int k=0; k<5 && options[k].first>=0; ++k) {
            double future=continuation(options[k],remaining);
            if (future<bestFuture) {
                bestFuture=future;
                chosen={options[k].first,options[k].last};
                chosenChances=options[k].probabilities;
            }
        }
        auto [i,j]=chosen;
        // Complementing an empty selection just interchanges the two replies.
        p=chosenChances;
        if (i==j) {
            for (double &z:p) z=1-z;
            return {1,n};
        }
        return {c[i].id,c[j-1].id};
    }
};

int ask(int l,int r) {
    cout<<"? "<<l<<' '<<r<<endl;
    int reply;
    if (!(cin>>reply)) exit(0);
    if (reply==r-l) return 1;
    if (reply==r-l+1) return 0;
    exit(0);
}

void nominate(int id) {
    cout<<"! "<<id<<endl;
    int reply;
    if (!(cin>>reply) || (reply!=0 && reply!=1)) exit(0);
    if (reply==1) {
        cout<<"#"<<endl;
        exit(0);
    }
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    int n;
    if (!(cin>>n) || n<3 || n>100000) return 0;
    Costs costs(n);
    Cycles cycles;
    ReplyModels models(n);
    vector<Candidate> active;
    active.reserve(n);
    int l=1,r=n/2;
    int answer=ask(l,r);
    for (int id=1; id<=n; ++id) {
        Candidate s{};
        s.id=id;
        s.lie=answer^(id<=r);
        s.cycles=s.lie ? cycles.lies[0] : cycles.all^cycles.lies[0];
        active.push_back(s);
    }
    int deadline=1+costs.depth(n,0);
    for (int t=1; ; ++t) {
        int m=active.size();
        int forced=count_if(active.begin(),active.end(),[](const Candidate &s){return s.forced;});
        if (m<=2 || (m==3 && forced>=2)) {
            int pick=-1;
            double best=-1;
            for (int i=0; i<m; ++i) {
                if (m==3 && forced==2 && active[i].forced) continue;
                double p=predict(active[i],cycles,t).total;
                if (p>best) {best=p; pick=i;}
            }
            nominate(active[pick].id);
            active.erase(active.begin()+pick);
            if (active.size()==2) {
                // Both remaining candidates are forced. Make them predict
                // opposite replies, using either a singleton or the full class.
                l=1; r=n;
                if (active[0].lie==active[1].lie) l=r=active[0].id;
                int z=ask(l,r);
                bool lie=z^(l<=active[0].id && active[0].id<=r);
                pick=(lie==active[0].lie);
            } else if (active.empty()) return 0;
            else pick=0;
            nominate(active[pick].id);
            return 0;
        }
        array<double,3> p;
        tie(l,r)=Questions(active,costs,cycles,t,models).select(n,deadline-t,p);
        answer=ask(l,r);
        models.observe(p,answer);
        int kept=0;
        for (int i=0; i<m; ++i) {
            auto &s=active[i];
            bool lie=answer^(l<=s.id && s.id<=r);
            if (!advance(s,lie,cycles,t)) continue;
            if (kept!=i) active[kept]=s;
            ++kept;
        }
        if (!kept) return 0;
        active.resize(kept);
    }
}
