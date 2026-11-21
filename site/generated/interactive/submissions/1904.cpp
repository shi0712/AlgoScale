#include <bits/stdc++.h>
using namespace std;

using V = vector<int>;

int ask(const V& w) {
    int k = accumulate(w.begin(), w.end(), 0);
    cout << "? 1 " << k;
    for (int i = 0; i < (int)w.size(); ++i)
        for (int j = 0; j < w[i]; ++j) cout << ' ' << i + 1;
    cout << endl;
    int y;
    if (!(cin >> y) || abs(y) > 3 * k) exit(0);
    return y; // -1 is also a legitimate path-sum response.
}

void flip() { cout << "? 2 1" << endl; }

void finish(int center, const V& bits) {
    cout << "! " << center;
    for (int b : bits) cout << ' ' << 2*b-1;
    cout << endl;
    exit(0);
}

// Binary weights with an optional truncated most significant weight.
// At most two subsets can have any given sum.
V weights(int cap, int limit) {
    V a;
    for (int v = 1; cap && (int)a.size() < limit; v *= 2) {
        a.push_back(min(cap, v));
        cap -= a.back();
    }
    return a;
}

vector<V> decode(const V& a, int target) {
    vector<V> ans;
    if (a.empty()) {
        if (!target) ans.emplace_back();
        return ans;
    }
    int h = (int)a.size()-1;
    for (int b : {0,1}) {
        int s = target-b*a[h];
        if (s < 0 || s >= (1<<h)) continue;
        V x(h+1);
        for (int i=0;i<h;++i) x[i]=(s>>i)&1;
        x[h]=b;
        ans.push_back(x);
    }
    return ans;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    int n;
    if (!(cin >> n)) return 0;
    int L=n-1;
    for (int i=0,u,v;i<L;++i) if (!(cin>>u>>v)) return 0;

    // c=f(1); the bit at each leaf represents (f(leaf)-c+1)/2.
    // Odd total multiplicity reveals c's parity. Positive padding
    // at the center separates the intervals for c's of equal parity.
    int k=(n%2?n:n-1);
    V a=weights(k-1,L), w(n);
    int W=accumulate(a.begin(),a.end(),0);
    w[0]=k-W;
    copy(a.begin(),a.end(),w.begin()+1);
    int y=ask(w), c=0;
    vector<V> p;
    for (int t=-2;t<=2;++t) {
        int z=y-k*t+W;
        if (z%2) continue;
        auto q=decode(a,z/2);
        if (!q.empty()) { c=t; p=move(q); break; }
    }
    if (p.empty()) return 0;
    int g=a.size(), center=c;
    bool flipped=false;
    if (abs(c)==2) {
        int signbit=(c>0);
        // Try to finish without a flip if all root choices and
        // remaining leaf assignments are already forced.
        if (g>=L-1) {
            set<V> answers;
            for (V x:p) for (int t=0;t<(g==L?1:2);++t) {
                V v=x;
                if (g<L) v.push_back(t);
                for (int i=0;i<L;++i) if (v[i]!=signbit) {
                    v[i]^=1; answers.insert(v); v[i]^=1;
                }
            }
            if (answers.size()==1) finish(c/2,*answers.begin());
        }
        // After the flip c becomes zero and every difference bit
        // represents the actual leaf value, even at the root.
        flip();
        flipped=true;
        center=-c/2;
        set<V> next;
        for (V x:p) {
            if (g<L) next.insert(x); // root outside prefix
            for (int i=0;i<g;++i) if (x[i]!=signbit) {
                x[i]^=1; next.insert(x); x[i]^=1;
            }
        }
        p.assign(next.begin(),next.end());
        c=0;
    }

    mt19937 rng(102729);
    while (true) {
        if (flipped && g>=L-1) {
            set<V> all;
            for (V x:p) for (int t=0;t<(g==L?1:2);++t) {
                V v=x;
                if (g<L) v.push_back(t);
                if (count(v.begin(),v.end(),int(center<0))) all.insert(v);
            }
            g=L; p.assign(all.begin(),all.end());
        }
        if (p.empty()) return 0;
        if (g==L && p.size()==1) break;

        // Encode the known prefix alternatives by distinct residues
        // modulo d, then pack new binary digits above that residue.
        V coef(g,0);
        int d=1;
        if (p.size()==2) {
            int i=0; while (p[0][i]==p[1][i]) ++i;
            coef[i]=1; d=2;
        } else if (p.size()>2) {
            // This case only occurs on the short prefix affected by
            // the center flip (possibly extended by one forced bit).
            // Find small nonnegative separating weights.
            double best=-1;
            auto test=[&](const V& v) {
                int s=accumulate(v.begin(),v.end(),0);
                if (s>n) return;
                V codes;
                for (const V& x:p)
                    codes.push_back(inner_product(x.begin(),x.end(),v.begin(),0));
                sort(codes.begin(),codes.end());
                if (adjacent_find(codes.begin(),codes.end())!=codes.end()) return;
                for (int mod=p.size();mod<=s+1;++mod) {
                    V seen(mod);
                    bool ok=true;
                    for (int z:codes) if (seen[z%mod]++) {ok=false;break;}
                    if (!ok) continue;
                    double score=(n-s+0.5)/mod;
                    if (score>best) {best=score; d=mod; coef=v;}
                    break;
                }
            };
            V v(g);
            for (int i=0;i<g;++i) v[i]=1<<i;
            test(v);
            for (int it=0;it<600;++it) {
                iota(v.begin(),v.end(),it%2);
                shuffle(v.begin(),v.end(),rng);
                if (it%3==0) for(int& z:v) z=rng()%(g+2);
                test(v);
            }
            if (best<0) {
                // If the query budget cannot separate the entire
                // prefix, split its alternatives with one bit.
                int id=0, balance=0;
                for (int i=0;i<g;++i) {
                    int ones=0;
                    for(auto& x:p) ones+=x[i];
                    int b=min(ones,(int)p.size()-ones);
                    if(b>balance) balance=b,id=i;
                }
                w.assign(n,0); w[id+1]=1;
                int b=(ask(w)-c+1)/2;
                erase_if(p,[&](const V& x){return x[id]!=b;});
                continue;
            }
        }
        int cost=accumulate(coef.begin(),coef.end(),0);
        a=weights((n-cost)/d,L-g);
        w.assign(n,0);
        copy(coef.begin(),coef.end(),w.begin()+1);
        for(int i=0;i<(int)a.size();++i) w[g+i+1]=a[i]*d;
        k=accumulate(w.begin(),w.end(),0);
        int code=(ask(w)-k*c+k)/2;
        vector<V> next;
        for(const V& x:p) {
            int delta=code-inner_product(x.begin(),x.end(),coef.begin(),0);
            if(delta<0 || delta%d) continue;
            for(const V& suffix:decode(a,delta/d)) {
                V v=x;
                v.insert(v.end(),suffix.begin(),suffix.end());
                next.push_back(move(v));
            }
        }
        g+=a.size();
        p=move(next);
    }

    V x=p.front();
    if (center) finish(center,x);
    // Initial c=0: all leaf signs are known. The original center
    // sign is the negative of the (unknown) root leaf's sign.
    if (count(x.begin(),x.end(),x[0])==L) finish(1-2*x[0],x);
    flip();
    // The change on a weighted query is twice the root sign
    // times (k - root multiplicity). Encode as many distinct
    // multiplicities within each sign class as the budget permits.
    V group[2];
    for (int i=0;i<L;++i) group[x[i]].push_back(i);
    w.assign(n,0);
    int used=0;
    for(int j=1;j<L;++j) {
        for(int b:{0,1}) if(j<(int)group[b].size() && used+j<=n) {
            w[group[b][j]+1]=j; used+=j;
        }
        if(used+j+1>n) break;
    }
    int mx=*max_element(w.begin(),w.end());
    if (used==mx) { w[0]=1; ++used; }
    int old=0;
    for(int i=0;i<L;++i) old+=w[i+1]*(2*x[i]-1);
    int delta=(ask(w)-old)/2;
    int b=delta>0, rootWeight=used-abs(delta);
    V possible;
    for(int i:group[b]) if(w[i+1]==rootWeight) possible.push_back(i);
    if(possible.size()==1) finish(2*b-1,x);
    flip();
    finish(1-2*b,x);
}
