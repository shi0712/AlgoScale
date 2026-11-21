#include <bits/stdc++.h>
using namespace std;

const int M = 505;
int n, m, depthv[M], usedv[M];
string basebits;
mt19937 rng(71836491);

int ask(int v, const string &s) {
    cout << "? " << v << ' ' << s << endl;
    int x;
    if (!(cin >> x) || x < 0) exit(0);
    return x;
}

// A pool of edges with a known number of marked members.
// measure(S) returns the number of marked members in S.
using Measure = function<int(const vector<int>&)>;
double C[M][M], costv[M][M];
int cutv[M][M], denseCost[M], matK[M];

void prepare() {
    for (int a = 0; a <= m; ++a) {
        C[a][0] = C[a][a] = 1;
        for (int b = 1; b < a; ++b)
            C[a][b] = C[a-1][b-1] + C[a-1][b];
    }
    for (int a = 1; a <= m; ++a) {
        denseCost[a] = 100000;
        for (int k = 1; k <= 8; ++k) {
            int cap = k * (1 << (k-1));
            int val = (1<<k)-1 + denseCost[max(0,a-cap)];
            if (val < denseCost[a] || (val == denseCost[a] && cap <= a)) {
                denseCost[a] = val;
                matK[a] = k;
            }
        }
    }
    for (int a = 2; a <= m; ++a) {
        for (int b = 1; b <= a/2; ++b) {
            double best = denseCost[a-1];
            int cut = 0;
            for (int t : {a/2, (a+2)/3}) {
                double val = 1;
                for (int i = max(0,b-(a-t)); i <= min(t,b); ++i)
                    val += C[t][i]*C[a-t][b-i]/C[a][b] *
                           (costv[t][i]+costv[a-t][b-i]);
                if (val <= best + 1e-10) best = val, cut = t;
            }
            costv[a][b] = costv[a][a-b] = best;
            cutv[a][b] = cutv[a][a-b] = cut;
        }
    }
}

// Column (s,j) depends only on coordinates in s. On these coordinates
// its support consists of 2^j odd-parity patterns. Its Walsh coefficient
// at s thus encodes a binary place, and coefficients outside subsets of
// s vanish.
void dense(vector<int> a, int b, const Measure &measure, vector<int> &out) {
    while (b && b < (int)a.size()) {
        int k = matK[a.size()-1], R = 1<<k;
        int len = min((int)a.size()-1,k*R/2);
        vector<vector<int>> rows(R);
        vector<vector<pair<int,vector<int>>>> cols(R);
        int cnt = 0;
        for (int s = 1; s < R && cnt < len; ++s) {
            int d = __builtin_popcount((unsigned)s);
            vector<int> odd;
            for (int p = s; p; p = (p-1)&s)
                if (__builtin_parity((unsigned)p)) odd.push_back(p);
            for (int j = 0; j < d && cnt < len; ++j) {
                vector<int> hits;
                for (int h = 0; h < (1<<j); ++h) {
                    int rem = R-1-s;
                    for (int q = rem;; q = (q-1)&rem) {
                        int x = odd[h]|q;
                        hits.push_back(x);
                        rows[x].push_back(a[cnt]);
                        if (!q) break;
                    }
                }
                cols[s].push_back({a[cnt++],move(hits)});
            }
        }
        vector<int> y(R);
        for (int x = 1; x < R; ++x)
            if (!rows[x].empty()) y[x] = measure(rows[x]);
        for (int s = R-1; s; --s) if (!cols[s].empty()) {
            int z = 0;
            for (int x = 0; x < R; ++x)
                z += __builtin_parity((unsigned)(x&s)) ? y[x] : -y[x];
            z /= R >> __builtin_popcount((unsigned)s);
            for (auto &[e,hits] : cols[s]) {
                if (z&1) {
                    out.push_back(e);
                    --b;
                    for (int x : hits) --y[x];
                }
                z >>= 1;
            }
        }
        a.erase(a.begin(),a.begin()+len);
    }
    if (b) out.insert(out.end(),a.begin(),a.end());
}

void recover(const vector<int>& a, int b, const Measure &measure, vector<int>& out) {
    if (!b) return;
    int z = a.size();
    if (b == z) {
        out.insert(out.end(),a.begin(),a.end());
        return;
    }
    int t = cutv[z][b];
    if (!t) {
        dense(a,b,measure,out);
        return;
    }
    vector<int> l(a.begin(),a.begin()+t), r(a.begin()+t,a.end());
    int x = measure(l);
    recover(l,x,measure,out);
    recover(r,b-x,measure,out);
}

struct Segment { int u,v; vector<int> e; };
struct Loc { int s,h,k,g; }; // segment, offset, attachment depth, free-edge pool
vector<Segment> seg;
int dep[M], lab[M], nodes=1;
int node(int d) { dep[++nodes]=d; return nodes; }

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    if (!(cin>>n)) return 0;
    m=n-1;
    prepare();
    vector<int> ids(m);
    iota(ids.begin(),ids.end(),0);
    string zeros(m,'0');
    int z=ask(1,zeros);
    vector<int> zeroToward;
    recover(ids,z,[&](const vector<int>& a){
        string s=zeros;
        for(int e:a) s[e]='1';
        return (z+(int)a.size()-ask(1,s))/2;
    },zeroToward);
    basebits=string(m,'1');
    for(int e:zeroToward) basebits[e]='0';

    int levels[M]={}, mx=0;
    lab[1]=1; usedv[1]=1; levels[0]=1;
    for(int v=2;v<=n;++v) {
        int d=-1;
        if(v==n) {
            if(!mx) d=1;
            for(int h=1;h<mx;++h) if(!levels[h]) d=h;
        }
        if(d<0) d=m-ask(v,basebits);
        depthv[v]=d; ++levels[d]; mx=max(mx,d);
    }
    vector<int> freeE=ids;
    for(int step=0;step<m;++step) {
        sort(seg.begin(),seg.end(),[](const Segment&a,const Segment&b){
            return dep[a.u]<dep[b.u];
        });
        int cover[M]={}, balance[M]={};
        cover[0]=1;
        for(auto &s:seg) for(int d=dep[s.u]+1;d<=dep[s.v];++d) {
            ++cover[d];
            balance[d]=max(balance[d],min(d-dep[s.u],dep[s.v]-d));
        }
        int v=-1, best=-1;
        for(int w=2;w<=n;++w) if(!usedv[w]) {
            int d=depthv[w], sc=cover[d]<levels[d]?1000+d:balance[d];
            if(sc>best) best=sc,v=w;
        }
        int d=depthv[v];
        Measure measure=[&](const vector<int>& a) {
            string s=basebits;
            for(int e:a) s[e]^=1;
            return (ask(v,s)-m+d+(int)a.size())/2;
        };
        // The explored union of root paths is represented by segments.
        // Inside each segment the edge set is known but its order is not.
        int low=0;
        for(int h=1;h<=d;++h) if(cover[h]==levels[h]) low=h;
        vector<Loc> cand;
        if(!low) cand.push_back({-1,0,0,0});
        for(int j=0;j<(int)seg.size();++j) {
            auto &s=seg[j];
            for(int h=1;h<=(int)s.e.size();++h) {
                int k=dep[s.u]+h;
                if(k<low || k>d) continue;
                if(k==d && k==dep[s.v] && lab[s.v]) continue;
                cand.push_back({j,h,k,0});
            }
        }
        bool joint=true;
        for(auto p:cand) if(d-p.k>1) joint=false;
        vector<vector<int>> pools(1,freeE);
        while(cand.size()>1) {
            // Color entire segments so the answer does not depend on
            // their unknown internal orders. For singleton extensions,
            // also obtain information about the new edge in the same query.
            vector<int> bestCol, bestVal, bestTake;
            double quality=-1;
            int J=seg.size();
            for(int trial=0;;++trial) {
                vector<int> col(J),sum(nodes+1),val, take(pools.size());
                if(trial<20) {
                    for(int &x:col) x=rng()&1;
                } else if(trial-20<J) col[trial-20]=1;
                int fc=trial<20?(rng()&1):0;
                for(int g=0;g<(int)pools.size();++g)
                    take[g]=joint && trial<20 && trial%4!=0 ?
                        (pools[g].size()+int(rng()&1))/2 : pools[g].size()*fc;
                for(int j=0;j<J;++j)
                    sum[seg[j].v]=sum[seg[j].u]+col[j]*(int)seg[j].e.size();
                double hist[M]={};
                for(auto p:cand) {
                    int x=p.s<0?0:sum[seg[p.s].u]+col[p.s]*p.h;
                    if(!joint) x+=(d-p.k)*fc;
                    val.push_back(x);
                    if(joint && d>p.k) {
                        double f=(double)take[p.g]/pools[p.g].size();
                        hist[x]+=1-f; hist[x+1]+=f;
                    } else ++hist[x];
                }
                double q=0;
                for(int x=0;x<=d;++x) if(hist[x]>0)
                    q-=hist[x]*log(hist[x]/cand.size());
                if(q>quality) {
                    quality=q; bestCol=move(col); bestVal=move(val); bestTake=move(take);
                }
                if(trial>=19 && quality>1e-8) break;
            }
            vector<int> test;
            for(int j=0;j<J;++j) if(bestCol[j])
                test.insert(test.end(),seg[j].e.begin(),seg[j].e.end());
            for(int g=0;g<(int)pools.size();++g)
                test.insert(test.end(),pools[g].begin(),pools[g].begin()+bestTake[g]);
            int ans=measure(test);
            vector<Loc> keep;
            vector<vector<int>> next;
            vector<array<int,2>> idx(pools.size(),array<int,2>{-1,-1});
            for(int i=0;i<(int)cand.size();++i) {
                auto p=cand[i];
                int delta=ans-bestVal[i];
                if(joint && d>p.k) {
                    if(delta<0 || delta>1) continue;
                    int g=p.g;
                    auto mid=pools[g].begin()+bestTake[g];
                    auto l=delta?pools[g].begin():mid;
                    auto r=delta?mid:pools[g].end();
                    if(l==r) continue;
                    if(idx[g][delta]<0) {
                        idx[g][delta]=next.size();
                        next.emplace_back(l,r);
                    }
                    p.g=idx[g][delta];
                } else if(delta) continue;
                keep.push_back(p);
            }
            cand.swap(keep);
            if(joint) pools.swap(next);
        }
        Loc p=cand[0];
        int at=1;
        if(p.s>=0) {
            auto &s=seg[p.s];
            if(p.h==(int)s.e.size()) at=s.v;
            else {
                vector<int> pref,rest;
                recover(s.e,p.h,measure,pref);
                bool sel[M]={};
                for(int e:pref) sel[e]=true;
                for(int e:s.e) if(!sel[e]) rest.push_back(e);
                int old=s.v;
                at=node(p.k); s.v=at; s.e=move(pref);
                seg.push_back({at,old,move(rest)});
            }
        }
        if(d>p.k) {
            vector<int> ext;
            recover(joint?pools[p.g]:freeE,d-p.k,measure,ext);
            bool sel[M]={};
            for(int e:ext) sel[e]=true;
            erase_if(freeE,[&](int e){return sel[e];});
            int w=node(d);
            seg.push_back({at,w,move(ext)});
            at=w;
        }
        lab[at]=v;
        usedv[v]=1;
    }
    vector<pair<int,int>> edges(m);
    for(auto &s:seg) {
        int e=s.e[0], a=lab[s.v], b=lab[s.u];
        if(basebits[e]=='0') swap(a,b);
        edges[e]={a,b};
    }
    cout<<"!";
    for(auto [a,b]:edges) cout<<' '<<a<<' '<<b;
    cout<<endl;
    return 0;
}
