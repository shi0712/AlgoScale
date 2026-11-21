#include <bits/stdc++.h>
using namespace std;

constexpr int OFFSET = 2001;
constexpr int LIMIT = 2*OFFSET+1;
constexpr int HALF = 2000;
int opposite(int v) {
    if (v==0) return 0;
    return v>OFFSET ? v-OFFSET : v+OFFSET;
}
using Bits = bitset<LIMIT>;

// The graph records weak numerical inequalities, including those involving
// erased labels. Label zero represents the mathematical constant zero.
Bits above[LIMIT], below[LIMIT];
int leftPart[LIMIT], rightPart[LIMIT], lastLabel, n;
mt19937 rng(73819037);

struct UseSortedBatches {};
int comparisonCount;
bool inFallback;

int message(char op, int a, int b) {
    if(op=='?' && !inFallback && ++comparisonCount>7*n+10*(lastLabel-n)+64)
        throw UseSortedBatches{};
    cout << op << ' ' << a << ' ' << b << endl;
    int r;
    if (!(cin >> r) || r == -1) exit(0);
    return r;
}

bool le(int a, int b) { return above[a][b]; }

void oneEdge(int a, int b) {
    if (le(a,b)) return;
    Bits up = above[b], down = below[a];
    Bits change = down & ~below[b];
    for (size_t i = change._Find_first(); i < LIMIT; i = change._Find_next(i))
        above[i] |= up;
    for (size_t i = up._Find_first(); i < LIMIT; i = up._Find_next(i))
        if (!below[i][a]) below[i] |= down;
}

void linkValues(int a,int b) {
    oneEdge(a,b);
    oneEdge(opposite(b),opposite(a));
}

vector<int> compatible[LIMIT];
vector<pair<int,int>> triples;
void certificate(int a,int b) {
    compatible[a].push_back(b);
    compatible[b].push_back(a);
    compatible[opposite(a)].push_back(opposite(b));
    compatible[opposite(b)].push_back(opposite(a));
    triples.emplace_back(a,b);
}

void introduce(int x) {
    for (int v : {x,opposite(x)}) { above[v].set(v); below[v].set(v); }
}

// Both answers are acceptable for equal numbers.
bool ordered(int a, int b) {
    if (le(a,b)) return true;
    if (le(b,a)) return false;
    if (leftPart[a] && leftPart[b]) {
        int x = leftPart[a], y = rightPart[a], u = leftPart[b], v = rightPart[b];
        if ((le(x,u) && le(y,v)) || (le(x,v) && le(y,u))) {
            linkValues(a,b);
            return true;
        }
        if ((le(u,x) && le(v,y)) || (le(v,x) && le(u,y))) {
            linkValues(b,a);
            return false;
        }
    }
    // Ties among fresh values follow the label order.
    bool first = a < b ? !message('?',b,a) : message('?',a,b);
    if (first) linkValues(a,b);
    else linkValues(b,a);
    return first;
}

vector<int> board;
int addPair(int a, int b) {
    int s = message('+',a,b);
    if (s != lastLabel+1) exit(0);
    lastLabel = s;
    leftPart[s] = a;
    rightPart[s] = b;
    introduce(s);
    certificate(a,b);
    certificate(s,opposite(a));
    certificate(s,opposite(b));
    board.erase(remove_if(board.begin(),board.end(),[&](int v){return v==a || v==b;}),board.end());
    board.push_back(s);
    return s;
}

// For s = a+b, the signs of b and s-a agree. Apply this in both
// directions, so that later comparisons can give old operands a sign.
void infer() {
    bool changed;
    do {
        changed = false;
        auto imply = [&](int a,int b) {
            if (!le(a,b)) { linkValues(a,b); changed = true; }
        };
        for (int s = n+1; s <= lastLabel; ++s) {
            int x = leftPart[s], y = rightPart[s];
            // The three comparisons against zero are equivalent to
            // comparisons of the other two terms in a+b-s=0.
            auto sign = [&](int term,int u,int v) {
                if (le(0,term)) imply(v,u);
                if (le(term,0)) imply(u,v);
                if (le(v,u)) imply(0,term);
                if (le(u,v)) imply(term,0);
            };
            sign(y,s,x);
            sign(x,s,y);
            sign(s,x,opposite(y));
        }
        for(int v=1;v<=lastLabel;++v) {
            if(le(v,opposite(v))) imply(v,0);
            if(le(opposite(v),v)) imply(0,v);
        }
        for(auto [u,v]:triples) {
            if(le(u,v)) { imply(u,HALF); imply(opposite(HALF),v); }
            if(le(v,u)) { imply(v,HALF); imply(opposite(HALF),u); }
        }
    } while (changed);
}

int orderCount(int v, bool upward) {
    return upward ? above[v].count() : below[v].count();
}

int best(const vector<int>& values, bool small) {
    int ans = -1, score = -1;
    for (int v : values) {
        int t = orderCount(v,small);
        if (t > score) { score = t; ans = v; }
    }
    return ans;
}

// Partition only the portions needed to certify the two ranks.
struct Selection {
    vector<int> v;
    bool reversed;
    Selection(const vector<int>& data,bool reverse): v(data),reversed(reverse) {}
    bool before(int a,int b) const {return reversed?ordered(b,a):ordered(a,b);}
    int split(int l,int r,int p) {
        int i=l;
        for (int j=l;j<r;++j)
            if (v[j]!=p && before(v[j],p)) swap(v[i++],v[j]);
        auto it = find(v.begin()+i,v.begin()+r,p);
        iter_swap(v.begin()+i,it);
        return i;
    }
    int pivot(int l,int r,int target) {
        int m=r-l;
        // Previously established cuts often survive many additions.
        Bits active;
        for (int i=l;i<r;++i) active.set(v[i]);
        int winner=-1;
        long long saving=0;
        for (int i=l;i<r;++i) {
            int x=v[i];
            int lo=(below[x]&active).count()-1;
            int hi=(above[x]&active).count()-1;
            if(reversed) swap(lo,hi);
            int ranked=lo+hi;
            if(!ranked) continue;
            int estimate=lo*(m-1)/ranked;
            int distance=abs((target-l)-estimate);
            long long gain=min(m-1,ranked)-3LL*distance;
            if (gain>saving) {saving=gain;winner=x;}
        }
        if (winner!=-1 && saving*3>=m*2) return winner;
        int k=min(m, m>=80 ? 13 : (m>=18 ? 5 : 3));
        vector<int> sample;
        for (int i=0;i<k;++i) {
            int at=uniform_int_distribution<int>(l+i,r-1)(rng);
            swap(v[l+i],v[at]);
            int x=v[l+i], start=0, end=sample.size();
            while(start<end) {
                int middle=(start+end)/2;
                if(before(x,sample[middle])) end=middle;
                else start=middle+1;
            }
            sample.insert(sample.begin()+start,x);
        }
        return sample[min(k-1,(target-l)*k/m)];
    }
    int place(int l,int r,int target) {
        while(r-l>1) {
            if(target==l || target==r-1) {
                bool wantMin=target==l;
                int w=l;
                for(int i=l+1;i<r;++i)
                    if(wantMin?before(v[i],v[w]):before(v[w],v[i])) w=i;
                swap(v[w],v[target]);
                break;
            }
            int p=split(l,r,pivot(l,r,target));
            if(p==target) break;
            if(p<target) l=p+1;
            else r=p;
        }
        return v[target];
    }
};

int witness(int ref, bool higher, const vector<int>& pool) {
    int w=-1, weight=-1;
    for (int v : pool) {
        if (v==ref || !(higher ? le(ref,v) : le(v,ref))) continue;
        int score=orderCount(v,higher);
        if (w==-1 || (higher ? le(v,w) : le(w,v)) ||
            (!(higher ? le(w,v) : le(v,w)) && score>weight)) {
            w=v;
            weight=score;
        }
    }
    return w;
}

void testSign(int s,int operand,int partner,bool tryPositive) {
    // Compare with a surviving upper/lower bound for the erased partner.
    for(int i=0;i<2 && !le(operand,0) && !le(0,operand);++i) {
        bool higher=i==0 ? tryPositive : !tryPositive;
        int w=witness(partner,higher,board);
        if(w<0 || w==s) continue;
        ordered(s,w);
        if(le(w,s) && le(partner,w)) linkValues(0,operand);
        if(le(s,w) && le(w,partner)) linkValues(operand,0);
    }
}

// The certificate |u+v| <= R gives an upper bound for every
// componentwise smaller pair and a lower bound for every larger pair.
Bits partners(int x,bool upper,const Bits& active) {
    if (upper ? le(x,0) : le(0,x)) return active;
    const Bits& outer=upper ? above[x] : below[x];
    Bits found=upper ? below[0] : above[0];
    for(size_t u=outer._Find_first();u<LIMIT;u=outer._Find_next(u)) {
        for(int v:compatible[u]) found |= upper ? below[v] : above[v];
    }
    found.reset(x);
    return found & active;
}

struct TotalBound {
    Bits live, canBeNegative[2], beyondHalf[2];
    TotalBound() {
        for(int x:board) live.set(x);
        canBeNegative[0]=live & ~above[0];
        canBeNegative[1]=live & ~below[0];
        beyondHalf[0]=live & ~above[opposite(HALF)];
        beyondHalf[1]=live & ~below[HALF];
    }

    bool within(int a,int b,int inv) const {
        // For the lower bound, use the same calculation on negated values.
        int x=inv?opposite(a):a, y=inv?opposite(b):b;
        if(le(x,0) || le(y,0) || (le(x,HALF) && le(y,HALF))) return true;
        const Bits& gx=inv?below[a]:above[a];
        const Bits& gy=inv?below[b]:above[b];
        Bits covered=gx|gy;
        int need=2+(canBeNegative[inv]&~covered).count()+
                    (beyondHalf[inv]&~covered).count();
        if((int)(covered&live).count()<need) return false;
        if(!le(y,x) && !le(y,HALF) && 2*(int)(gy&live).count()<need) return false;
        if(!le(x,y) && !le(x,HALF) && 2*(int)(gx&live).count()<need) return false;
        // For x <= y and t=x+y>R, the covered elements contribute at
        // least min(|gy|, |covered|/2)*t: minimize the linear bound
        // |gy|*y + (|covered|-|gy|)*x for x between 0 and t/2.
        // The tests above also cover the reverse ordering when possible.
        return true;
    }
};

int partnerFor(int x,const TotalBound& bound,const vector<int>& candidates,int budget) {
    Bits up=partners(x,true,bound.live), down=partners(x,false,bound.live);
    Bits both=up&down;both.reset(x);
    if(both.any()) return both._Find_first();
    int m=candidates.size();
    for(int i=0;i<min(m,budget);++i) {
        int y=candidates[i*m/min(m,budget)];
        if(x!=y && (up[y] || bound.within(x,y,0)) &&
                   (down[y] || bound.within(x,y,1))) return y;
    }
    return -1;
}

pair<int,int> safePair(const vector<int>& unknown,const vector<int>& same) {
    TotalBound bound;
    for(int pass=0;pass<2;++pass) {
        const auto& candidates=pass ? same : unknown;
        int tries=min(12,(int)candidates.size());
        for(int i=0;i<tries;++i) {
            int x=candidates[i*candidates.size()/tries];
            int y=partnerFor(x,bound,board,24);
            if(y!=-1) return {x,y};
        }
    }
    return {-1,-1};
}

void rankProbe(int known,bool reverse) {
    int m=board.size(), cap=(m+1)/3;
    if(m==3) {
        int a=board[0],b=board[1],c=board[2];
        if(!ordered(a,b)) swap(a,b);
        if(ordered(b,c)) addPair(a,c);
        else if(ordered(a,c)) addPair(a,b);
        else addPair(c,b);
        return;
    }
    if(m<=12) {
        int small=board[0],large=small, i=1;
        if(m%2==0) {
            large=board[1]; i=2;
            if(!ordered(small,large)) swap(small,large);
        }
        for(;i<m;i+=2) {
            int a=board[i],b=board[i+1];
            if(!ordered(a,b)) swap(a,b);
            if(ordered(a,small)) small=a;
            if(ordered(large,b)) large=b;
        }
        if(small==large) large=board[1];
        addPair(small,large);
        infer();
        return;
    }
    Selection q(board,reverse);
    int target=max(0,min((m*26)/100,(m-known)/2));
    target=min(target,cap-1);
    int lo=0,hi=m,k;
    // For 1-based rank t with 3t <= m+1, pair x[t] with x[m+1-t].
    // If their sum exceeds R, both are positive. The largest t
    // elements and t others >= x[t] form t pairs each exceeding R.
    // Only t-1 remaining elements can be negative, so the board total
    // would exceed R. Negating all values proves the lower bound.
    // Any rank in the outer third suffices. Accept a sampled pivot as
    // soon as its actual rank gives a substantial certified tail.
    while(true) {
        int p=q.pivot(lo,hi,target);
        k=q.split(lo,hi,p);
        if(k==target || (k<cap && k>=target/2 && k<=target+max(2,target/2))) break;
        if(k<target) lo=k+1;
        else hi=k;
    }
    int a=q.v[k], b=q.place(k+1,m,m-1-k);
    if(reverse) swap(a,b);
    int s=addPair(a,b);
    int mid=q.v[(m-1)/2];
    if(mid!=a && mid!=b) ordered(s,mid);
    if(le(a,s)) linkValues(0,b);
    if(le(s,b)) linkValues(a,0);
    testSign(s,a,b,true);
    testSign(s,b,a,false);
    infer();
}

bool confirmLE(int a,int b) {
    if(le(a,b)) return true;
    if(!message('?',b,a)) { linkValues(a,b); return true; }
    linkValues(b,a);
    return false;
}

// Sample the input for monotone order or frequent equal values. Equality
// witnesses remain useful even after the original representative is erased.
bool inspectInput() {
    if(n<4) return false;
    for(int dir=0;dir<2;++dir) {
        auto monotone=[&](int a,int b){return dir?confirmLE(b,a):confirmLE(a,b);};
        bool ok=true;
        for(int i=1;i<n && ok;++i) ok=monotone(i,i+1);
        if(ok) return monotone(n,1);
    }
    if(n<24) return false;
    vector<int> sample;
    int size = n<80 ? 7 : 13;
    for(int i=0;i<size;++i) sample.push_back(1+i*(n-1)/(size-1));
    for(int i=1;i<size;++i) {
        int v=sample[i], j=i;
        while(j && ordered(v,sample[j-1])) {sample[j]=sample[j-1];--j;}
        sample[j]=v;
    }
    vector<int> reps;
    for(int v:sample) {
        if(reps.empty() || !confirmLE(v,reps.back())) reps.push_back(v);
    }
    vector<int> frequent=reps;
    if(reps.size()*4>sample.size()*3 || frequent.empty()) return false;
    vector<vector<int>> bins(frequent.size());
    for(int v:board) {
        int l=0,r=(int)frequent.size()-1;
        while(l<r) {
            int mid=(l+r)/2;
            if(confirmLE(v,frequent[mid])) r=mid;
            else l=mid+1;
        }
        bins[l].push_back(v);
    }
    vector<int> unresolved;
    for(int b=0;b<(int)bins.size();++b) {
        int rep=frequent[b];
        const auto& bin=bins[b];
        for(int l=0;l<(int)bin.size();l+=16) {
            int end=min(l+16,(int)bin.size()),prev=rep;
            bool ok=true;
            for(int j=l;j<end && ok;++j) {
                ok=confirmLE(prev,bin[j]);
                prev=bin[j];
            }
            if(!ok || !confirmLE(prev,rep))
                unresolved.insert(unresolved.end(),bin.begin()+l,bin.begin()+end);
        }
    }
    for(int v:unresolved) {
        bool matched=false;
        int l=0,r=frequent.size();
        while(l<r) {
            int m=(l+r)/2,p=frequent[m];
            if(!confirmLE(v,p)) l=m+1;
            else if(!confirmLE(p,v)) r=m;
            else {matched=true;break;}
        }
        if(!matched && frequent.size()<8) frequent.insert(frequent.begin()+l,v);
    }
    return false;
}

bool groupReduction() {
    vector<vector<int>> groups;
    for(int v:board) {
        auto it=find_if(groups.begin(),groups.end(),[&](const auto& g){
            return le(v,g.back()) && le(g.back(),v);
        });
        if(it!=groups.end()) it->push_back(v);
        else {
            groups.push_back({v});
            if(groups.size()>8) return false;
        }
    }
    // With just two distinct groups their representatives are the two
    // extremes in some order. All copies of that addition are safe.
    while(groups.size()>1) {
        int lo=0,hi=1;
        if(groups.size()>2) {
            if(!ordered(groups[lo].back(),groups[hi].back())) swap(lo,hi);
            for(int i=2;i<(int)groups.size();++i) {
                if(ordered(groups[i].back(),groups[lo].back())) lo=i;
                else if(ordered(groups[hi].back(),groups[i].back())) hi=i;
            }
        }
        vector<int> sums;
        while(!groups[lo].empty() && !groups[hi].empty()) {
            int a=groups[lo].back(),b=groups[hi].back();
            groups[lo].pop_back();groups[hi].pop_back();
            sums.push_back(addPair(a,b));
        }
        groups.erase(remove_if(groups.begin(),groups.end(),[](const auto& g){return g.empty();}),groups.end());
        groups.push_back(move(sums));
    }
    while(board.size()>1) addPair(board.front(),board.back());
    return true;
}

void cardinalBounds() {
    Bits lowerHalf,upperHalf;
    for(auto [u,v]:triples) {
        lowerHalf |= above[u]&above[v];
        upperHalf |= below[u]&below[v];
    }
    upperHalf &= ~below[HALF];
    lowerHalf &= ~above[opposite(HALF)];
    for(size_t v=upperHalf._Find_first();v<LIMIT;v=upperHalf._Find_next(v))
        linkValues(v,HALF);
    for(size_t v=lowerHalf._Find_first();v<LIMIT;v=lowerHalf._Find_next(v))
        linkValues(opposite(HALF),v);

    Bits active;
    for(int x:board) active.set(x);
    for(int sign=0;sign<2;++sign) {
        // If x > R/2, every successor contributes more than R/2.
        // Use the best known sign or half-range bound for the others.
        Bits nonnegative=active & (sign?below[0]:above[0]);
        Bits moderate=active & (sign?below[HALF]:above[opposite(HALF)]);
        Bits low=active & ~moderate;
        int base=-(int)low.count()-(int)(active & ~nonnegative).count();
        for(int x:board) {
            int v=sign?opposite(x):x;
            if(le(v,HALF)) continue;
            const Bits& higher=sign?below[x]:above[x];
            int increase=(higher&active).count()+
                         (higher & active & ~nonnegative).count()+(higher&low).count();
            if(base+increase>=2) linkValues(v,HALF);
        }
    }
}

void solve() {
    if(inspectInput()) {
        while(board.size()>1) addPair(board.front(),board.back());
        cout << '!' << endl;
        return;
    }
    if(n>=16 && groupReduction()) {cout << '!' << endl;return;}
    int newest=-1, lastBounds=2*n;
    while(board.size()>1) {
        if(board.size()==2) { addPair(board[0],board[1]); break; }
        vector<int> pos,neg,unknown;
        int zero=-1;
        for(int v:board) {
            if(le(0,v) && le(v,0)) zero=v;
            else if(le(0,v)) pos.push_back(v);
            else if(le(v,0)) neg.push_back(v);
            else unknown.push_back(v);
        }
        if(zero!=-1) {
            bool merged=false;
            for(int v:board) if(v!=zero && le(v,0) && le(0,v)) {
                int s=addPair(zero,v);
                linkValues(0,s);linkValues(s,0);
                merged=true;break;
            }
            if(merged) continue;
            if(!unknown.empty()) {ordered(unknown.back(),zero);continue;}
            if(!neg.empty() && !pos.empty()) {
                newest=addPair(pos.back(),neg.back());infer();continue;
            }
            while(board.size()>1) addPair(board.front(),board.back());
            break;
        }
        if(max(pos.size(),neg.size())+1>=board.size()) {
            bool positive=pos.size()>neg.size();
            int a=board.front();
            for(int v:board) if(!(positive?le(0,v):le(v,0))) a=v;
            while(board.size()>1) a=addPair(a,board[0]==a?board[1]:board[0]);
            break;
        }

        // Reuse near-zero sign witnesses to classify new sums.
        if(newest>0 && !le(0,newest) && !le(newest,0)) {
            int p=best(pos,true), q=best(neg,false);
            if(p!=-1 && !le(newest,p)) ordered(p,newest);
            if(q!=-1 && !le(0,newest) && !le(q,newest)) ordered(newest,q);
            newest=-1;
            infer();
            continue;
        }
        newest=-1;
        if(!pos.empty() && !neg.empty()) {
            int p=best(pos,true);
            int a=best(pos,false), b=best(neg,true);
            if(pos.size()>2) {
                swap(*find(pos.begin(),pos.end(),p),pos.back());
                pos.pop_back();
                a=best(pos,true);
            }
            newest=addPair(a,b);
            infer();
            continue;
        }

        bool positive=!pos.empty();
        const auto& same=positive?pos:neg;
        if(lastBounds-(int)board.size()>=max(1,(int)board.size()/15)) {
            cardinalBounds(); lastBounds=board.size();
        }
        auto [u,v]=safePair(unknown,same);
        if (u!=-1) {
            newest=addPair(u,v);
            infer();
            if(!le(0,u) && !le(u,0)) testSign(newest,u,v,true);
            if(!le(0,v) && !le(v,0)) testSign(newest,v,u,false);
            infer();
            continue;
        }
        if(!same.empty() && (unknown.size()<=5 || unknown.size()*5<=board.size())) {
            // If the least unknown is positive, all other live values
            // are nonnegative. Otherwise it can absorb any positive.
            int a=unknown[0];
            for(int v:unknown) if(positive?ordered(v,a):ordered(a,v)) a=v;
            int b=best(same,!positive);
            int neighbor=witness(b,!positive,same);
            if(neighbor!=-1) b=neighbor;
            newest=addPair(a,b);
            testSign(newest,a,b,positive);
            infer();
        } else {
            rankProbe(same.size(),!neg.empty());
        }
    }
    cout << '!' << endl;
}

vector<int> mergeValues(const vector<int>& a,const vector<int>& b) {
    vector<int> result;
    size_t i=0,j=0;
    while(i<a.size() && j<b.size()) {
        if(ordered(a[i],b[j])) result.push_back(a[i++]);
        else result.push_back(b[j++]);
    }
    result.insert(result.end(),a.begin()+i,a.end());
    result.insert(result.end(),b.begin()+j,b.end());
    return result;
}

vector<int> sortValues(vector<int> values) {
    if(values.size()<2) return values;
    auto mid=values.begin()+values.size()/2;
    return mergeValues(sortValues({values.begin(),mid}),sortValues({mid,values.end()}));
}

void sortedBatches() {
    // Bound the query cost even when successive rank probes give very
    // little sign information. Within a batch the pairs are disjoint
    // and each is certified by the ordering at the start of the batch.
    inFallback=true;
    vector<int> values=sortValues(board);
    while(values.size()>1) {
        int m=values.size(), k=(m+1)/3;
        vector<int> sums;
        for(int i=0;i<k;++i) sums.push_back(addPair(values[i],values[m-1-i]));
        vector<int> middle(values.begin()+k,values.end()-k);
        values=mergeValues(middle,sortValues(sums));
    }
    cout << '!' << endl;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    if(!(cin>>n) || n<2 || n>1000) return 0;
    lastLabel=n;
    for(int i=0;i<=n;++i) {
        introduce(i);
        if(i) board.push_back(i);
    }
    introduce(HALF); linkValues(0,HALF); certificate(HALF,HALF);
    try { solve(); }
    catch(const UseSortedBatches&) { sortedBatches(); }
}
