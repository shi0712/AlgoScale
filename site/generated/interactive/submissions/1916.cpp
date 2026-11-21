#include <bits/stdc++.h>
using namespace std;

int n, total;
vector<int> val;
int unknowns;
vector<pair<vector<int>,int>> observations;

int ask(const string& s) {
    cout << s << '\n' << flush;
    int r;
    if (!(cin >> r) || r < 0 || r == n) exit(0);
    if (r == 0) {
        string t = s;
        for (char& c : t) c = (c == 'T' ? 'F' : 'T');
        cout << t << '\n' << flush;
        cin >> r;
        exit(0);
    }
    return r;
}

void finish() {
    string s(n,'F');
    for (int i=0;i<n;i++) if (val[i]==1) s[i]='T';
    ask(s);
    exit(0);
}

// Return the number of true bits in the indicated subset.
int weigh(const vector<int>& subset) {
    string s(n,'F');
    int knownOnes=0, insideKnown=0, cnt=0;
    for (int i=0;i<n;i++) if (val[i]==1) s[i]='T', ++knownOnes;
    for (int i:subset) {
        if (val[i]==1) ++insideKnown;
        if (val[i]<0) s[i]='T', ++cnt;
    }
    int t=total-knownOnes;
    if (!cnt || !t) return insideKnown;
    if (cnt==unknowns) return insideKnown+t;
    if (t==unknowns) return insideKnown+cnt;
    bool inv=abs(unknowns-cnt-t)<abs(cnt-t);
    if (inv) for (int i=0;i<n;i++) if (val[i]<0)
        s[i]=(s[i]=='T'?'F':'T');
    int used=inv?unknowns-cnt:cnt;
    int r=ask(s);
    int hits=(r-n+used+t)/2;
    return insideKnown+(inv?t-hits:hits);
}

struct Eq {vector<pair<int,int>> a; int b; bool queued=false;};
vector<Eq> eqs;
vector<vector<int>> adj;
queue<int> pending;
void enqueue(int j) {
    if (!eqs[j].queued) eqs[j].queued=true,pending.push(j);
}
void fixbit(int i,int v) {
    if (val[i]>=0) return;
    val[i]=v; --unknowns;
    for (int j:adj[i]) enqueue(j);
}
void equation(vector<pair<int,int>> a,int b) {
    int j=eqs.size();
    eqs.push_back({move(a),b});
    for (auto [i,w]:eqs.back().a) if (val[i]<0) adj[i].push_back(j);
    enqueue(j);
}
void record(const vector<int>& a,int b) {
    observations.push_back({a,b});
    vector<pair<int,int>> e;
    for (int i:a) e.emplace_back(i,1);
    equation(move(e),b);
}

// All coefficient magnitudes are powers of two. After complementing
// negatively weighted bits, propagate bounds on the binary carries.
void propagate() {
    while (!pending.empty()) {
        int j=pending.front(); pending.pop();
        auto &e=eqs[j]; e.queued=false;
        int b=e.b, cnt[12]={}, top=-1;
        for (auto [i,w]:e.a) {
            if (val[i]>=0) b-=w*val[i];
            else {
                if (w<0) b-=w;
                int p=__builtin_ctz((unsigned)abs(w));
                ++cnt[p]; top=max(top,p);
            }
        }
        if (top<0) continue;
        int l[13],r[13],forced[12]; bool any=false;
        l[0]=r[0]=b;
        for (int p=0;p<=top;p++) {
            l[p+1]=max(0,(l[p]-cnt[p]+1)/2);
            r[p+1]=r[p]/2;
        }
        int lo=0,hi=0;
        for (int p=top;p>=0;p--) {
            int mn=max(0,l[p]-2*hi), mx=min(cnt[p],r[p]-2*lo);
            forced[p]=(mx==0?0:mn==cnt[p]?1:-1);
            any |= cnt[p] && forced[p]>=0;
            lo=max(l[p],2*lo);
            hi=min(r[p],2*hi+cnt[p]);
        }
        if (any) for (auto [i,w]:e.a) if (val[i]<0) {
            int v=forced[__builtin_ctz((unsigned)abs(w))];
            if (v>=0) fixbit(i,v^(w<0));
        }
    }
}

void smallSolve() {
    vector<int> ids,idx(n,-1);
    string base(n,'F');
    int known=0;
    for (int i=0;i<n;i++) {
        if (val[i]<0) idx[i]=ids.size(),ids.push_back(i);
        else if (val[i]) base[i]='T',++known;
    }
    int k=ids.size(), lim=1<<k;
    vector<int> cand;
    for (int m=0;m<lim;m++) if (__builtin_popcount((unsigned)m)==total-known)
        cand.push_back(m);
    for (auto &[a,b]:observations) {
        int mask=0,rem=b;
        for (int i:a) {
            if (val[i]<0) mask|=1<<idx[i];
            else rem-=val[i];
        }
        erase_if(cand,[&](int x){return __builtin_popcount((unsigned)(x&mask))!=rem;});
    }
    mt19937 rng(872371);
    while (true) {
        int best=cand[0];
        if (cand.size()>1) {
            vector<int> tries;
            if (lim<=512) for (int m=0;m<lim;m++) tries.push_back(m);
            else {
                for (int j=0;j<128;j++) {
                    tries.push_back(rng()%lim);
                    tries.push_back(cand[rng()%cand.size()]);
                }
                tries.push_back(best);
            }
            long long opt=LLONG_MAX;
            for (int m:tries) {
                int h[16]={};
                for (int x:cand) ++h[__builtin_popcount((unsigned)(x^m))];
                long long cost=0;
                for (int d=1;d<=k;d++) cost+=1LL*h[d]*h[d];
                if (cost<opt) opt=cost,best=m;
            }
        }
        string s=base;
        for (int j=0;j<k;j++) if ((best>>j)&1) s[ids[j]]='T';
        int dist=n-ask(s);
        erase_if(cand,[&](int x){return __builtin_popcount((unsigned)(x^best))!=dist;});
    }
}

void sparseSolve(vector<int> a,int t) {
    if (t==0 || t==(int)a.size()) {
        for (int i:a) fixbit(i,t!=0);
        return;
    }
    if (unknowns<=14) smallSolve();
    int h=a.size()/2;
    vector<int> b(a.begin()+h,a.end());
    a.resize(h);
    int x=weigh(a);
    observations.push_back({a,x});
    sparseSolve(move(a),x);
    sparseSolve(move(b),t-x);
}

int main() {
    ios::sync_with_stdio(false); cin.tie(nullptr);
    if (!(cin>>n)) return 0;
    total=n-ask(string(n,'F'));
    val.assign(n,-1); unknowns=n; adj.resize(n);
    vector<int> all(n); iota(all.begin(),all.end(),0);
    record(all,total);

    // A column on support S is AND(fixed bits) * parity(free bits).
    // Its top multilinear coefficient is (-2)^(number of free bits - 1).
    // One zero column is recoverable using the globally known total.
    vector<int> fixedMask(n), freeMask(n);
    int used=0, last=0;
    for (int s=1;used<n-1;s++) {
        int f=0;
        for (int z=s;z && used<n-1;z&=z-1) {
            f|=z&-z;
            fixedMask[used]=s^f; freeMask[used]=f;
            used++;
        }
        last=s;
    }
    vector<int> coef(last+1);
    for (int s=1;s<=last;s++) {
        propagate();
        if (!unknowns) finish();
        if (unknowns<=14) smallSolve();
        int t=total-count(val.begin(),val.end(),1);
        int rare=min(t,unknowns-t), bound=0;
        for (int g=1;g<unknowns;g*=2) bound+=min(g,rare);
        if (bound<last-s+1) {
            vector<int> a;
            for (int i=0;i<n;i++) if (val[i]<0) a.push_back(i);
            sparseSolve(move(a),t);
            finish();
        }

        vector<pair<int,int>> e;
        vector<int> row;
        int inferred=0;
        bool determined=true;
        for (int i=0;i<used;i++) {
            int h=fixedMask[i],f=freeMask[i];
            if ((s&h)!=h) continue;
            int k=__builtin_popcount((unsigned)(s&f));
            if (k&1) row.push_back(i);
            if (k && !(s&~(h|f))) {
                int w=(1<<(k-1))*(k%2?1:-1);
                e.emplace_back(i,w);
                if (val[i]<0) determined=false;
                else inferred+=w*val[i];
            }
        }
        int lower=0;
        for (int z=(s-1)&s;z;z=(z-1)&s) lower+=coef[z];
        int b=determined?lower+inferred:weigh(row);
        coef[s]=b-lower;
        record(row,b);
        equation(move(e),coef[s]);
    }
    propagate();
    finish();
}
