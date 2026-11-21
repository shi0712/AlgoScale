#include <bits/stdc++.h>
using namespace std;

int n, lim, r;
mt19937 rng(782153);
int ask(int i, int j) {
    cout << "? " << i << ' ' << j << endl;
    int z;
    if (!(cin >> z) || z < 0 || z >= lim) exit(0);
    return z;
}
[[noreturn]] void finish(int c, const vector<int>& p) {
    cout << "!" << endl << c << endl;
    for (int i = 0; i < n; ++i) {
        if (i) cout << ' ';
        cout << p[i];
    }
    cout << endl;
    exit(0);
}

// Enumerate a small residual matching. One known endpoint on each side
// suffices to represent every query involving a known entry.
void small(vector<int> p, vector<int> U, vector<int> V) {
    vector<int> I = U, J = V;
    for (int i = 0; i < n; ++i) if (p[i] >= 0) {
        I.push_back(i); J.push_back(p[i]); break;
    }
    vector<array<int, 9>> pp, bb;
    sort(V.begin(), V.end());
    do {
        array<int, 9> a{}, b{};
        for (int k = 0; k < (int)U.size(); ++k) p[U[k]] = V[k];
        for (int k = 0; k < (int)I.size(); ++k) {
            a[k] = p[I[k]];
            int t = find(J.begin(), J.end(), a[k]) - J.begin();
            b[t] = I[k];
        }
        pp.push_back(a); bb.push_back(b);
    } while (next_permutation(V.begin(), V.end()));
    vector<int> live(pp.size()), freq(lim);
    iota(live.begin(), live.end(), 0);
    while (true) {
        long long best = 1LL * live.size() * live.size();
        int s = -1, t = -1;
        for (int i = 0; i < (int)I.size(); ++i)
            for (int j = 0; j < (int)J.size(); ++j) {
                fill(freq.begin(), freq.end(), 0);
                long long score = 0;
                for (int k : live)
                    score += 2 * freq[pp[k][i] ^ bb[k][j]]++ + 1;
                if (score < best) best = score, s = i, t = j;
            }
        if (s < 0) {
            for (int k = 0; k < (int)I.size(); ++k)
                p[I[k]] = pp[live[0]][k];
            finish(live.size(), p);
        }
        int z = ask(I[s], J[t]);
        erase_if(live, [&](int k){ return (pp[k][s] ^ bb[k][t]) != z; });
    }
}

vector<int> A,T,ia,it,live,leftcnt,mi,mv;
// A_i = p_i xor b_0, T_j = b_j xor p_0.
int P(int x, int i) {
    if (A[i] >= 0) return A[i]^x;
    int v = it[i^x^r];
    if (v >= 0) return v;
    return leftcnt[x]==1 ? mv[x] : -1;
}
int B(int x, int j) {
    if (T[j] >= 0) return T[j]^x^r;
    int i = ia[j^x];
    if (i >= 0) return i;
    return leftcnt[x]==1 ? mi[x] : -1;
}
void add(bool row, int k, int z) {
    erase_if(live,[&](int x){
        int v = z ^ x ^ (row?r:0);
        if (v >= n) return true;
        int old = row ? B(x,k) : P(x,k);
        int rev = row ? P(x,v) : B(x,v);
        if ((old>=0 && old!=v) || (rev>=0 && rev!=k)) return true;
        bool overlap = row ? A[v]>=0 : T[v]>=0;
        if (!overlap) {
            --leftcnt[x]; mi[x] ^= row?v:k; mv[x] ^= row?k:v;
        }
        return false;
    });
    (row?T:A)[k]=z;
    (row?it:ia)[z]=k;
}

// With the offset known, use a star of XORs entirely inside the
// remaining matching. Besides its queried inverse entries, a star can
// recover its center for free.
void known(int x) {
    vector<int> p(n), b(n,-1);
    for (int i=0;i<n;++i) {
        p[i]=P(x,i);
        if (p[i]>=0) b[p[i]]=i;
    }
    while (true) {
        vector<int> u,v;
        for(int k=0;k<n;++k) {
            if(p[k]<0) u.push_back(k);
            if(b[k]<0) v.push_back(k);
        }
        if(u.size()<=7) small(p,u,v);
        shuffle(v.begin(),v.end(),rng);
        int i=u[rng()%u.size()];
        vector<int> cand=v, js,zs;
        vector<char> possible(n,1);
        while(cand.size()>1) {
            int h=-1;
            // Prefer a spoke that cannot be the image of the center.
            for(int k=(int)v.size()-1;k>=0;--k)
                if(!possible[v[k]]) {h=k;break;}
            if(h<0 && !js.empty() &&
               (v.size()<=1 || js.size()>=8)) {
                int val=ask(i,0)^x;
                cand={val}; break;
            }
            if(h<0) h=(int)v.size()-1;
            int j=v[h]; swap(v[h],v.back()); v.pop_back();
            int z=ask(i,j);
            js.push_back(j); zs.push_back(z);
            erase_if(cand,[&](int a){
                int k=z^a;
                if(k>=n || p[k]>=0 || ((k==i)!=(a==j))) {
                    possible[a]=0; return true;
                }
                return false;
            });
        }
        int a=cand[0];
        p[i]=a; b[a]=i;
        for(int k=0;k<(int)js.size();++k) {
            int idx=zs[k]^a;
            p[idx]=js[k]; b[js[k]]=idx;
        }
    }
}

void complete_candidates() {
    vector<char> fixedA(n),fixedT(n);
    vector<int> freq(lim);
    while(true) {
        int x0=live[0];
        vector<int> I,J;
        for(int side=0;side<2;++side) {
            auto &fixed=side?fixedT:fixedA;
            auto &lst=side?J:I;
            for(int k=0;k<n && lst.size()<6;++k) {
                if(fixed[k]) continue;
                int first=side?(B(x0,k)^x0):(P(x0,k)^x0);
                bool diff=false;
                if((side?T:A)[k]<0)
                    for(int x:live)
                        if(((side?B(x,k):P(x,k))^x)!=first) {
                            diff=true; break;
                        }
                if(diff) lst.push_back(k);
                else fixed[k]=1;
            }
        }
        if(I.empty() && J.empty()) {
            vector<int> p(n);
            for(int k=0;k<n;++k) p[k]=P(x0,k);
            finish(live.size(),p);
        }
        long long best=LLONG_MAX;
        int s=0,t=0;
        auto test=[&](int i,int j){
            fill(freq.begin(),freq.end(),0);
            long long sum=0;
            for(int x:live) sum+=2*freq[P(x,i)^B(x,j)]+++1;
            if(sum<best) best=sum,s=i,t=j;
        };
        for(int i:I) test(i,0);
        for(int j:J) test(0,j);
        if(!J.empty()) for(int h=0;h<(int)I.size();++h)
            test(I[h],J[h%J.size()]);
        int z=ask(s,t);
        erase_if(live,[&](int x){return (P(x,s)^B(x,t))!=z;});
    }
}

int main(){
    ios::sync_with_stdio(false); cin.tie(nullptr);
    if(!(cin>>n)) return 0;
    lim=1; while(lim<n) lim*=2;
    vector<int> order(n); iota(order.begin(),order.end(),0);
    if(n<=8) small(vector<int>(n,-1),order,order);
    r=ask(0,0);
    A.assign(lim,-1); T=ia=it=A;
    int total=0;
    for(int i:order) {
        total^=i;
        if((i^r)<n) live.push_back(i);
    }
    leftcnt.assign(n,n); mi.assign(n,total); mv=mi;
    add(false,0,r); add(true,0,r);
    vector<int> rows=order;
    shuffle(order.begin()+1,order.end(),rng);
    shuffle(rows.begin()+1,rows.end(),rng);
    int c=1,d=1;
    bool translation=true;
    while(true) {
        if(live.size()==1) known(live[0]);
        if(all_of(live.begin(),live.end(),[](int x){return leftcnt[x]<=1;}))
            complete_candidates();
        bool row=!translation && c>=sqrt(n) && d<n &&
            d<min(4+(int)log2(n),1+c/4);
        int k=row?rows[d++]:order[c++];
        int z=-1;
        for(int x:live) {
            int v=row?B(x,k):P(x,k);
            if(v<0) { z=-1; break; }
            v^=x^(row?r:0);
            if(z>=0 && z!=v) {z=-1;break;}
            z=v;
        }
        if(z<0) z=row?ask(0,k):ask(k,0);
        if(!row && (z^k)!=r) translation=false;
        add(row,k,z);
    }
}
