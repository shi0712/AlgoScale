#include <bits/stdc++.h>
using namespace std;

int n;
vector<vector<int>> g, nb, dista;
vector<int> ord, par, posi, height_;
using V = vector<int>;

int ask(const V &w) {
    int k = accumulate(w.begin(), w.end(), 0);
    cout << "? 1 " << k;
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < w[i]; ++j) cout << ' ' << i + 1;
    cout << endl;
    int x;
    if (!(cin >> x)) exit(0);
    // Negative sums, including -1, are legitimate replies.
    return x;
}

void layout(int u, int p) {
    par[u] = p;
    posi[u] = (int)ord.size();
    ord.push_back(u);
    for (int v : g[u]) if (v != p) layout(v, u);
}

// Can this connected prefix of path sums be extended to a valid tree?
bool feasible(const V &f, int len) {
    for (int i = 0; i < len; ++i)
        if (abs(f[ord[i]]) == 1) return true;
    for (int i = len; i < n; ++i) {
        int u = ord[i], p = par[u];
        if (posi[p] >= len) continue;
        int d = min(abs(f[p] - 1), abs(f[p] + 1));
        if (d > 0 && d <= height_[u] + 1) return true;
    }
    return false;
}

// To obtain coefficients c on the signed parent-edge differences,
// query a vertex c[i] minus the coefficients of its included children times.
V weights(int start, const V &c) {
    V w = c;
    for (int i = 0; i < (int)c.size(); ++i) {
        int p = par[ord[start+i]];
        if (posi[p] >= start) w[posi[p]-start] -= c[i];
    }
    return w;
}

struct Cand {
    int r, id, h;
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    if (!(cin >> n)) return 0;
    g.resize(n);
    for (int i = 1, a, b; i < n; ++i) {
        if (!(cin >> a >> b)) return 0;
        --a; --b;
        g[a].push_back(b);
        g[b].push_back(a);
    }

    dista.assign(n,V(n));
    nb.assign(n,V(n,-1));
    for (int r = 0; r < n; ++r) {
        function<void(int,int)> dfs = [&](int u, int p) {
            for (int v : g[u]) if (v != p) {
                nb[r][v] = u == r ? v : nb[r][u];
                dista[r][v] = dista[r][u]+1;
                dfs(v,u);
            }
        };
        dfs(r,-1);
    }
    int a = max_element(dista[0].begin(),dista[0].end())-dista[0].begin();
    height_.assign(n,0);
    function<void(int,int)> prep = [&](int u,int p) {
        for (int v : g[u]) if (v != p) {
            prep(v,u);
            height_[u] = max(height_[u],1+height_[v]);
        }
        sort(g[u].begin(),g[u].end(),[&](int x,int y){
            return height_[x]>height_[y];
        });
    };
    prep(a,-1);
    par.resize(n); posi.resize(n);
    layout(a,-1);

    // First packet also measures the unknown additive offset f(a).
    int k = (n%2 ? n : n-1);
    V c, w;
    for (int b = 1; b < n && (1<<b) <= 2*k; ++b) {
        V t(b);
        for (int i=0;i<b;++i) t[i]=1<<(b-1-i);
        V z=weights(1,t);
        if (accumulate(z.begin(),z.end(),0)>k) break;
        c=t; w=z;
    }
    int b=c.size();
    V q(n);
    for(int i=0;i<b;++i) q[ord[1+i]]=w[i];
    q[a]=k-accumulate(w.begin(),w.end(),0);
    int reply=ask(q), len=1+b;
    vector<V> states;
    for(int mask=0;mask<(1<<b);++mask){
        int rem=reply-(2*mask-((1<<b)-1));
        if(rem%k) continue;
        V f(n); f[a]=rem/k;
        for(int i=0;i<b;++i){
            int u=ord[1+i];
            f[u]=f[par[u]]+((mask&c[i])?1:-1);
        }
        if(feasible(f,len)) states.push_back(f);
    }

    while(len<n){
        if(states.empty()) return 0;
        int u=ord[len];
        vector<V> forced;
        for(auto f:states) for(int s:{-1,1}){
            f[u]=f[par[u]]+s;
            if(feasible(f,len+1)) forced.push_back(f);
        }
        if(forced.size()==1){
            states=move(forced); ++len; continue;
        }

        // If the last packet was ambiguous, one vertex separates its
        // two interpretations modulo 4. Use even multiplicities elsewhere.
        int wit=-1, scale=1;
        if(states.size()>1){
            for(int v:ord) if(abs(states[0][v]-states[1][v])==2){
                wit=v; break;
            }
            if(wit<0) return 0;
            scale=2;
        }
        int cap=(n-(wit!=-1))/scale;
        c.clear(); w.clear();
        for(int m=1; len+m<=n && (1<<max(0,m-2))<=cap; ++m){
            V t(m);
            for(int i=0;i<m;++i)t[i]=1<<(m-1-i);
            V z=weights(len,t);
            int excess=max(0,accumulate(z.begin(),z.end(),0)-cap);
            t[0]-=excess; z[0]-=excess;
            if(t[0]<=0 || z[0]<0) continue;
            c=t; w=z;
        }
        q.assign(n,0);
        if(wit>=0) q[wit]=1;
        b=c.size();
        for(int i=0;i<b;++i) q[ord[len+i]]=scale*w[i];
        reply=ask(q);
        vector<V> next;
        for(auto &f:states){
            int rem=reply-(wit>=0?f[wit]:0);
            for(int i=0;i<b;++i){
                int v=ord[len+i],p=par[v];
                if(posi[p]<len) rem-=scale*c[i]*f[p];
            }
            if(rem%scale) continue;
            rem/=scale;
            if(!b){
                if(!rem) next.push_back(f);
                continue;
            }
            int low=(1<<(b-1))-1;
            for(int s:{-1,1}){
                int h=rem-s*c[0]+low;
                if(h<0 || h>2*low || h%2) continue;
                int mask=h/2;
                V nf=f;
                for(int i=0;i<b;++i){
                    int v=ord[len+i];
                    nf[v]=nf[par[v]]+(i==0?s:((mask&c[i])?1:-1));
                }
                if(feasible(nf,len+b)) next.push_back(move(nf));
            }
        }
        states=move(next);
        len+=b;
    }

    map<V,int> ids;
    vector<V> vals;
    vector<Cand> cs;
    for(int h=0;h<(int)states.size();++h){
        const V &f=states[h];
        for(int r=0;r<n;++r) if(abs(f[r])==1){
            V v(n);
            for(int i=0;i<n;++i) v[i]=i==r?f[r]:f[i]-f[nb[i][r]];
            auto [it,ok]=ids.emplace(v,vals.size());
            if(ok) vals.push_back(v);
            cs.push_back({r,it->second,h});
        }
    }
    V flip(n,1);
    auto done=[&](){
        if(cs.empty()) exit(0);
        for(auto x:cs) if(x.id!=cs[0].id) return false;
        cout<<"!";
        for(int i=0;i<n;++i) cout<<' '<<vals[cs[0].id][i]*flip[i];
        cout<<endl;
        return true;
    };
    if(done()) return 0;
    if(cs.front().h!=cs.back().h){
        int u=0;
        while(states[0][u]==states[1][u]) ++u;
        q.assign(n,0); q[u]=1;
        int x=ask(q);
        erase_if(cs,[&](Cand e){return states[e.h][u]!=x;});
    }
    if(cs.empty()) return 0;
    V f=states[cs[0].h];

    while(!done()){
        // Balance the remaining root candidates. Prefer a pivot whose
        // sign alone completely determines the answer, when available.
        int p=-1;
        pair<int,int> best={INT_MAX,INT_MAX};
        for(int i=0;i<n;++i){
            V cnt(n);
            int mx=0,sum=0,sg[2]={-1,-1};
            bool easy=true;
            for(auto e:cs){
                int j=nb[i][e.r]; if(j<0) j=i;
                sum+=2*cnt[j]+1;
                mx=max(mx,++cnt[j]);
                int s=vals[e.id][i]>0;
                if(sg[s]!=-1 && sg[s]!=e.id) easy=false;
                sg[s]=e.id;
            }
            pair<int,int> score={easy?0:mx,sum};
            if(score<best){best=score;p=i;}
        }
        cout<<"? 2 "<<p+1<<endl;
        int old=flip[p]; flip[p]*=-1;
        auto branch=[&](Cand e){
            int j=nb[p][e.r];
            return j<0?p:j;
        };
        auto delta=[&](Cand e){return -2*old*vals[e.id][p];};

        for(;;){
            // Branches with the same sign are distinguished by their
            // queried multiplicities. Merge equivalent terminal answers.
            V id(n,-1),sign(n);
            for(auto e:cs){
                int j=branch(e);
                sign[j]=delta(e)>0;
                if(id[j]==-1) id[j]=e.id;
                else if(id[j]!=e.id) id[j]=-2;
            }
            vector<V> groups[2];
            map<pair<int,int>,int> loc;
            for(int t=0;t<=n;++t){
                int j=t==0?p:t-1;
                if((t && j==p)||id[j]==-1) continue;
                int s=sign[j], key=id[j]>=0?id[j]:-1-j;
                auto [it,ok]=loc.emplace(make_pair(s,key),groups[s].size());
                if(ok) groups[s].push_back({});
                groups[s][it->second].push_back(j);
            }
            int ng=max(groups[0].size(),groups[1].size());
            auto makeq=[&](int digits){
                V z(n);
                for(auto &gs:groups)
                    for(int i=0;i<(int)gs.size();++i)
                        for(int j:gs[i]) z[j]=i%digits;
                return z;
            };
            for(int d=ng;d>=1;--d){
                q=makeq(d);
                if(accumulate(q.begin(),q.end(),0)<n) break;
            }
            ++q[p]; // guarantees a nonzero change and hence reveals sign
            int total=accumulate(q.begin(),q.end(),0);
            int diff=ask(q)-inner_product(q.begin(),q.end(),f.begin(),0);
            erase_if(cs,[&](Cand e){
                int j=branch(e);
                return diff!=delta(e)*(total-(j==p?0:q[j]));
            });
            if(done()) return 0;
            int j=branch(cs[0]);
            if(all_of(cs.begin(),cs.end(),[&](Cand e){return branch(e)==j;})){
                int change=delta(cs[0]);
                for(int v=0;v<n;++v)
                    if(j==p || nb[p][v]!=j) f[v]+=change;
                break;
            }
        }
    }
    return 0;
}
