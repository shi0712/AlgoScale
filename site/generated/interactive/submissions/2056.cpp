#include <bits/stdc++.h>
using namespace std;

// At step i the still available values <= i+2 (one-based) occupy the
// next three slots. The emitted value is one of their three order
// statistics. Equivalently its number of smaller entries to the right
// is 0, 1 or 2. These digits specify the permutation by reverse insertion.
//
// After sorting a suffix, only its two smallest entries can interact
// with the preceding positions. Every other suffix rank is final.
constexpr int W = 6;
constexpr int BLOCK = 60;
using Order = array<unsigned char, W>;
using Bits = uint64_t;

struct Mask {
    array<Bits, 3> bits{};
    bool operator==(const Mask&) const = default;
    Mask operator&(const Mask& b) const {
        return {{bits[0]&b.bits[0], bits[1]&b.bits[1], bits[2]&b.bits[2]}};
    }
    Mask operator^(const Mask& b) const {
        return {{bits[0]^b.bits[0], bits[1]^b.bits[1], bits[2]^b.bits[2]}};
    }
    void put(int i) { bits[i/64] |= Bits(1) << (i%64); }
    int size() const { return popcount(bits[0])+popcount(bits[1])+popcount(bits[2]); }
    template<class F> void each(F f) const {
        for (int i=0; i<3; ++i)
            for (Bits b=bits[i]; b; b &= b-1)
                f(64*i+countr_zero(b));
    }
};
struct MaskHash {
    size_t operator()(const Mask& m) const {
        Bits h=0;
        for (Bits x : m.bits) {
            x ^= x >> 30;
            x *= 0xbf58476d1ce4e5b9ULL;
            h = rotl(h, 23) ^ x ^ (x >> 27);
        }
        return h;
    }
};

struct Edge {
    int x,y,lt,gt;
};
struct State {
    Mask set;
    vector<Edge> edges;
    int size, largest=-1, next=-1;
};

// All feasible orders of a window. Once the maximum is determined, drop
// it and introduce the next position at one of the three smallest ranks.
using Distribution=array<double,3>;
const vector<Distribution> priors=[] {
    vector<Distribution> p{{1./3,1./3,1./3}};
    for (double major : {0.004,0.13,0.65,0.84,0.98}) {
        for (int i=0;i<3;++i) {
            Distribution q;
            q.fill((1-major)/2);
            q[i]=major;
            p.push_back(q);
        }
    }
    return p;
}();

struct Window {
    int w, start;
    vector<Order> orders, digits;
    vector<State> states;
    vector<vector<int>> worst;
    vector<int> bySize;
    map<int,vector<vector<double>>> redundancy;
    explicit Window(int width, int maxHorizon) : w(width) {
        Order p{};
        iota(p.begin(),p.begin()+w,0);
        map<Order,int> index;
        do {
            Order code{};
            for (int j=0;j<w;++j)
                for (int i=0;i<j;++i)
                    code[p[j]] += p[i]>p[j];
            if (*max_element(code.begin(),code.end())<=2) {
                index[p]=orders.size();
                orders.push_back(p);
                digits.push_back(code);
            }
        } while (next_permutation(p.begin(),p.begin()+w));

        Mask all, less[W][W];
        vector<array<int,3>> expand(orders.size());
        for (int k=0;k<(int)orders.size();++k) {
            all.put(k);
            auto s=orders[k];
            for (int i=0;i<w;++i)
                for (int j=i+1;j<w;++j) less[s[i]][s[j]].put(k);
            int largest=s[w-1];
            for (int rank=0;rank<3;++rank) {
                Order t{};
                for (int i=0;i<w-1;++i)
                    t[i+(i>=rank)] = 1+s[i]-(s[i]>largest);
                expand[k][rank]=index.at(t);
            }
        }

        unordered_map<Mask,int,MaskHash> ids;
        auto intern = [&](Mask s) {
            auto [it,fresh]=ids.emplace(s,(int)states.size());
            if (fresh) states.push_back({s,{},s.size()});
            return it->second;
        };
        intern(all);
        start=intern(less[w-2][w-1]);
        for (int i=0;i<(int)states.size();++i) {
            Mask s=states[i].set, expanded;
            int largest=-1;
            s.each([&](int k) {
                int x=orders[k][w-1];
                if (largest==-1) largest=x;
                else if (x!=largest) largest=-2;
            });
            if (largest>=0) {
                s.each([&](int k) {
                    for (int t:expand[k]) expanded.put(t);
                });
                int nxt=intern(expanded);
                states[i].largest=largest;
                states[i].next=nxt;
            }
            for (int x=0;x<w;++x) for (int y=x+1;y<w;++y) {
                Mask lo=s&less[x][y];
                if (lo==Mask{} || lo==s) continue;
                int a=intern(lo), b=intern(s^lo);
                states[i].edges.push_back({x,y,a,b});
            }
        }

        bySize.resize(states.size());
        iota(bySize.begin(),bySize.end(),0);
        sort(bySize.begin(),bySize.end(),[&](int a,int b) {
            return states[a].size<states[b].size;
        });
        for (int d=0;d<=maxHorizon;++d) {
            auto& h=worst.emplace_back(states.size());
            for (int i:bySize) {
                const auto& s=states[i];
                if (d && s.next>=0) {
                    h[i]=worst[d-1][s.next];
                } else if (s.size>1) {
                    int depth=INT_MAX;
                    for (auto [x,y,a,b]:s.edges) {
                        depth=min(depth,1+max(h[a],h[b]));
                    }
                    h[i]=depth;
                }
            }
        }
    }
    // Expected excess over the entropy bound. The cost of a comparison
    // with branch probability p is one bit minus its binary entropy.
    const vector<vector<double>>& costs(int prior) {
        auto [it,fresh]=redundancy.try_emplace(prior);
        auto& rows=it->second;
        if (!fresh) return rows;
        vector<double> prob(orders.size()), mass(states.size());
        for (int t=0;t<(int)orders.size();++t) {
            prob[t]=1;
            for (int j=0;j<w;++j) prob[t]*=priors[prior][digits[t][j]];
        }
        struct Split { double p,q,loss; };
        vector<vector<Split>> split(states.size());
        for (int i=0;i<(int)states.size();++i)
            states[i].set.each([&](int t){mass[i]+=prob[t];});
        for (int i=0;i<(int)states.size();++i) for (const Edge& e:states[i].edges) {
            double p=mass[e.lt]/mass[i], q=mass[e.gt]/mass[i];
            split[i].push_back({p,q,1+p*log2(p)+q*log2(q)});
        }
        for (int d=0;d<(int)worst.size();++d) {
            auto& v=rows.emplace_back(states.size());
            for (int i:bySize) {
                const State& s=states[i];
                if (d && s.next>=0) v[i]=rows[d-1][s.next];
                else if (s.size>1) {
                    double best=1e100;
                    for (int j=0;j<(int)s.edges.size();++j) {
                        auto [p,q,loss]=split[i][j];
                        const Edge& e=s.edges[j];
                        best=min(best,loss+p*v[e.lt]+q*v[e.gt]);
                    }
                    v[i]=best;
                }
            }
        }
        return rows;
    }
};

int questions;
unordered_map<uint64_t,bool> recorded;
bool comparePositions(int x,int y) {
    int lo=min(x,y),hi=max(x,y);
    uint64_t key=(uint64_t(lo)<<32)|unsigned(hi);
    auto it=recorded.find(key);
    if (it!=recorded.end()) return it->second^(x!=lo);
    cout << "? " << x+1 << ' ' << y+1 << endl;
    string reply;
    if (!(cin>>reply) || (reply!="<" && reply!=">")) exit(0);
    ++questions;
    bool smaller=reply=="<";
    recorded.emplace(key,smaller^(x!=lo));
    return smaller;
}

constexpr int DEPTH=4, CONTEXTS=121;
struct Model {
    struct Node {
        Distribution count{};
        double evidence=0;
    };
    array<Node,CONTEXTS> nodes;
    array<Distribution,CONTEXTS> prediction;

    Distribution local(int id) const {
        auto c=nodes[id].count;
        double sum=c[0]+c[1]+c[2]+1.5;
        for (double& x:c) x=(x+0.5)/sum;
        return c;
    }
    Distribution blend(int id,Distribution base,Distribution alt) const {
        double mix=1/(1+exp(1.7-nodes[id].evidence));
        for (int j=0;j<3;++j) base[j]+=mix*(alt[j]-base[j]);
        return base;
    }
    void prepare() {
        for (int id=0;id<CONTEXTS;++id) {
            Distribution base=id ? prediction[(id-1)/3] : Distribution{1./3,1./3,1./3};
            prediction[id]=blend(id,base,local(id));
        }
    }
    int prior() const {
        int best=0;
        double high=-1e30;
        for (int i=0;i<(int)priors.size();++i) {
            double score=0;
            for (int j=0;j<3;++j) score+=prediction[0][j]*log(priors[i][j]);
            if (score>high) { high=score; best=i; }
        }
        return best;
    }
    static int context(const vector<int>& r,int at) {
        int id=0;
        for (int j=at+1;j<(int)r.size() && j<=at+DEPTH;++j)
            id=3*id+1+r[j];
        return id;
    }
    void train(const vector<int>& r,int at) {
        int id=0,out=r[at];
        Distribution base{1./3,1./3,1./3};
        for (int depth=0;;++depth) {
            auto alt=local(id);
            auto mixed=blend(id,base,alt);
            auto& node=nodes[id];
            node.evidence=clamp(0.985*node.evidence+log(alt[out]/base[out]),-12.,14.);
            for (double& x:node.count) x*=0.985;
            node.count[out]+=1;
            base=mixed;
            if (depth==DEPTH || at+depth+1>=(int)r.size()) break;
            id=3*id+1+r[at+depth+1];
        }
    }
};

struct Block {
    vector<int> pos;
    vector<Bits> above;
    Window& win;
    Model& model;
    vector<int>& ranks;
    int begin, k;
    Block(vector<int> positions, Window& w, Model& m,vector<int>& r)
        : pos(move(positions)),above(pos.size()),win(w),model(m),ranks(r),
          begin(pos[0]),k(pos.size()-2) {
        relate(k,k+1);
        for (int x=0;x<k+2;++x) for (int y=x+1;y<k+2;++y) {
            int a=pos[x],b=pos[y];
            uint64_t key=(uint64_t(min(a,b))<<32)|unsigned(max(a,b));
            auto it=recorded.find(key);
            if (it!=recorded.end()) {
                if (it->second^(a>b)) relate(x,y);
                else relate(y,x);
            }
        }
    }
    void relate(int x,int y) {
        Bits up=above[y]|(Bits(1)<<y), from=Bits(1)<<x;
        for (int i=0;i<(int)pos.size();++i)
            if (i==x || (above[i]&from)) above[i]|=up;
    }
    int relation(int x,int y) const {
        if ((above[x]>>y)&1) return -1;
        if ((above[y]>>x)&1) return 1;
        return 0;
    }
    bool ask(int x,int y) {
        int c=relation(x,y);
        if (c) return c<0;
        bool lt=comparePositions(pos[x],pos[y]);
        if (lt) relate(x,y); else relate(y,x);
        return lt;
    }

    vector<int> sortWindow(int stop) {
        const auto& cost=win.costs(model.prior());
        const int w=win.w, size=k+2;
        int d=size-w, id=win.start;
        Order slot{};
        iota(slot.begin(),slot.begin()+w,d);
        vector<int> sorted(size);
        while (true) {
            const State& s=win.states[id];
            if (d && s.next>=0) {
                int x=s.largest, old=slot[x];
                sorted[d+w-1]=old;
                if (old<k) ranks[begin+old]=w-1-x;
                for (int j=x;j>0;--j) slot[j]=slot[j-1];
                slot[0]=--d;
                id=s.next;
                continue;
            }
            if (s.size==1) {
                s.set.each([&](int a) {
                    for (int j=0;j<w;++j) sorted[j]=slot[win.orders[a][j]];
                });
                break;
            }
            int known=-1;
            for (const Edge& e:s.edges) {
                int cmp=relation(slot[e.x],slot[e.y]);
                if (cmp) { known=cmp<0?e.lt:e.gt; break; }
            }
            if (known>=0) { id=known; continue; }

            // Posterior weights for the possible local orders. Factors
            // whose digit and right context are already fixed cancel out.
            Bits affected=0;
            for (int j=0;j<w && slot[j]<k;++j) {
                int a=slot[j];
                for (int b=max(d,a-DEPTH);b<=a;++b) affected|=Bits(1)<<b;
            }
            array<double,162> wt;
            double sum=0;
            s.set.each([&](int t) {
                for (int j=0;j<w && slot[j]<k;++j)
                    ranks[begin+slot[j]]=win.digits[t][j];
                double p=1;
                for (Bits b=affected;b;b&=b-1) {
                    int a=begin+countr_zero(b);
                    p*=model.prediction[Model::context(ranks,a)][ranks[a]];
                }
                wt[t]=p;
                sum+=p;
            });
            const Edge* best=nullptr;
            double bestScore=1e100;
            int remaining=stop-questions;
            for (const Edge& e:s.edges) {
                int a=e.lt,b=e.gt;
                if (1+max(win.worst[d][a],win.worst[d][b])>remaining) continue;
                const State& lo=win.states[a];
                const State& hi=win.states[b];
                bool smaller=lo.size<=hi.size;
                double mass=0;
                (smaller?lo:hi).set.each([&](int t){ mass+=wt[t]; });
                double p=smaller?mass/sum:1-mass/sum;
                p=0.997*p+0.003*lo.size/s.size;
                double q=1-p;
                double score=p*(cost[d][a]+log2(p))+q*(cost[d][b]+log2(q));
                if (score<bestScore-1e-10) { bestScore=score; best=&e; }
            }
            // Inferred comparisons can only improve the remaining worst
            // cost, but the finite-window policy may advance at a different
            // time after receiving free information. Always allow progress.
            if (!best) {
                best=&*min_element(s.edges.begin(),s.edges.end(),[&](const Edge& a,const Edge& b) {
                    return max(win.worst[d][a.lt],win.worst[d][a.gt])
                          <max(win.worst[d][b.lt],win.worst[d][b.gt]);
                });
            }
            id=ask(slot[best->x],slot[best->y])?best->lt:best->gt;
        }
        return sorted;
    }
};

// Extrapolate a repeated suffix of the already decoded rank digits.
// The comparison certificate below checks every extrapolation.
pair<int,int> repetition(const vector<int>& ranks,int start) {
    int available=(int)ranks.size()-2-start;
    pair<int,int> best{};
    for (int period=1;period<=64 && 2*period<=available;++period) {
        int needed=period+max(period,period==1?5:8);
        if (needed>available) continue;
        int len=period, cap=min(available,2048);
        while (len<cap && ranks[start+len]==ranks[start+len-period]) ++len;
        if (len>=needed && len-period>best.second-best.first) {
            best={period,len};
            if (len==cap) break;
        }
    }
    return best;
}

bool verifyPeriod(int first,int count,int period,vector<int>& ranks,
                  int& small,int& big,vector<int>& answer) {
    int k=count-first;
    vector<int> order(k+2);
    order[k]=small; order[k+1]=big;
    for (int i=k-1;i>=0;--i) {
        int digit=ranks[first+i+period];
        ranks[first+i]=digit;
        for (int j=0;j<digit;++j) order[i+j]=order[i+j+1];
        order[i+digit]=first+i;
    }

    // Partition the predicted value order just after each key attaining
    // its rank bound. In each part, verify its internal increasing order.
    // By induction all lower parts occupy their exact ranks, and the last
    // key's upper bound leaves precisely enough ranks for this entire part.
    vector<pair<int,int>> checks;
    for (int i=0;i<k+1;++i) {
        int x=order[i], y=order[i+1];
        if (x==small && y==big) continue;
        if (x<count && x-first+2==i) continue;
        checks.emplace_back(x,y);
    }
    // Sample across the interval first, so a changed pattern can be
    // discovered before spending many comparisons on the prediction.
    int samples=min(8,(int)checks.size());
    for (int j=0;j<samples;++j) {
        int at=j+((int)checks.size()-j)*j/samples;
        swap(checks[j],checks[at]);
    }
    for (auto [x,y]:checks) if (!comparePositions(x,y)) return false;
    small=order[0];
    big=order[1];
    for (int i=2;i<k+2;++i) answer[order[i]]=first+i+1;
    return true;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    int n;
    if (!(cin>>n) || n<1 || n>30000) return 0;
    vector<int> answer(n),ranks(n);
    if (n==1) {
        cout << "! 1" << endl;
        return 0;
    }
    int small=n-2,big=n-1;
    if (!comparePositions(small,big)) swap(small,big);
    ranks[n-2]=(small==n-1);
    int count=n-2, bank=n<14?0:5, done=0, twos=0;
    Model model;
    array<unique_ptr<Window>,W+1> windows;
    auto takeTwos = [&](int length) {
        for (int j=0;j<length;++j) {
            --count;
            answer[count]=count+3;
            ranks[count]=2;
            model.train(ranks,count);
        }
        done+=length;
        twos+=length;
    };
    if (count>=12 && comparePositions(big,0)) takeTwos(count);
    if (count>=24 && comparePositions(big,count-6)) {
        takeTwos(6);
        bank+=9;
    }
    while (count) {
        bool changed=false;
        auto [period,matched]=repetition(ranks,count);
        if (period && (period!=1 || ranks[count]!=2) && bank>=4) {
            int span=min({count,bank-1,2*matched});
            int before=questions;
            if (verifyPeriod(count-span,count,period,ranks,small,big,answer)) {
                for (int i=count-1;i>=count-span;--i) {
                    model.train(ranks,i);
                    twos=ranks[i]==2?twos+1:0;
                }
                bank=min(8192,bank+5*span/3-(questions-before));
                count-=span;
                done+=span;
                continue;
            }
            bank-=questions-before;
            changed=true;
        }
        if (twos>=4 && count>=6 && period<=1 && bank>0) {
            // a[i] exceeds both survivors exactly when the whole interval
            // from i to the current boundary consists of rank-two digits.
            int before=questions, lo=0,hi=min(count,max(6,twos));
            while (comparePositions(big,count-hi)) {
                lo=hi;
                if (lo==count) break;
                hi=min(count,2*hi);
            }
            while (lo+1<hi && questions-before<bank+5*lo/3) {
                int mid=lo+(hi-lo)/2;
                if (comparePositions(big,count-mid)) lo=mid; else hi=mid;
            }
            takeTwos(lo);
            bank=clamp(bank+(5*lo)/3-(questions-before),0,8192);
            if (!count) break;
            if (lo+1==hi) {
                // The search also established that the next digit is 0 or 1.
                int x=--count;
                bool first=comparePositions(x,small);
                answer[big]=count+3;
                big=first?small:x;
                if (first) small=x;
                ranks[x]=first?0:1;
                model.train(ranks,x);
                twos=0;
                ++done;
                changed=true;
                if (!count) break;
            }
        }

        model.prepare();
        int length=min(count,done<6?6:done<18?12:done<42?24:BLOCK);
        if (changed) length=min(length,6);
        auto nextProb=model.prediction[Model::context(ranks,count-1)];
        if (*max_element(nextProb.begin(),nextProb.end())>0.82)
            length=min(length,max(6,3*((bank-1)/3)));
        if ((count<=length+2 || n<14) && count<=BLOCK) length=count;
        int w=min(W,length+2);
        if (!windows[w])
            windows[w]=make_unique<Window>(w,w==W?min(BLOCK,n-2)+2-w:0);
        Window& win=*windows[w];
        int first=count-length, d=length+2-w;
        vector<int> positions(length+2);
        iota(positions.begin(),positions.begin()+length,first);
        positions[length]=small;
        positions[length+1]=big;
        Block block(move(positions),win,model,ranks);
        int before=questions, base=win.worst[d][win.start];
        auto order=block.sortWindow(before+base+bank);

        // All unintroduced positions are smaller than the third element
        // of this sorted block. Only the first two carry into the next one.
        Bits earlier=0;
        for (int j=0;j<length+2;++j) {
            int idx=order[j];
            if (idx<length) ranks[first+idx]=popcount(earlier>>(idx+1));
            earlier|=Bits(1)<<idx;
            if (j>=2) answer[block.pos[idx]]=first+j+1;
        }
        small=block.pos[order[0]];
        big=block.pos[order[1]];
        for (int i=count-1;i>=first;--i) {
            model.train(ranks,i);
            twos=(ranks[i]==2?twos+1:0);
        }
        done+=length;
        count=first;
        bank=clamp(bank+base-(questions-before),0,8192);
    }
    answer[small]=1;
    answer[big]=2;
    cout << '!';
    for (int a:answer) cout << ' ' << a;
    cout << endl;
}
