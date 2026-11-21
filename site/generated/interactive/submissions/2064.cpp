#include <bits/stdc++.h>
using namespace std;

static int n;
static vector<vector<int>> adj, direction;
static vector<int> order, parentNode, pos, depth;
static vector<vector<short>> dist;
static mt19937 rng(728341);

static int query(const vector<int>& q) {
    int k = accumulate(q.begin(), q.end(), 0);
    cout << "? 1 " << k;
    for (int v = 0; v < n; ++v)
        for (int j = 0; j < q[v]; ++j) cout << ' ' << v + 1;
    cout << endl;
    int result;
    if (!(cin >> result)) exit(0);
    return result;
}
static void toggle(int v) {
    cout << "? 2 " << v + 1 << endl;
}
static void answer(const vector<int>& a) {
    cout << '!';
    for (int x : a) cout << ' ' << x;
    cout << endl;
}
static void prepareTree() {
    direction.assign(n, vector<int>(n));
    dist.assign(n,vector<short>(n));
    int start = 0, longest = -1;
    for (int p = 0; p < n; ++p) {
        direction[p][p] = p;
        auto dfs = [&](auto&& self, int v, int par, int first, int d) -> void {
            direction[p][v] = first; dist[p][v]=d;
            if (p == 0 && d > longest) longest = d, start = v;
            for (int u : adj[v]) if (u != par) self(self, u, v, first, d + 1);
        };
        for (int v : adj[p]) dfs(dfs,v,p,v,1);
    }
    parentNode.resize(n); depth.assign(n,0); pos.resize(n);
    vector<int> height(n);
    auto sizes = [&](auto&& self, int v, int p) -> void {
        parentNode[v] = p;
        for (int u : adj[v]) if (u != p) {
            depth[u] = depth[v] + 1;
            self(self,u,v);
            height[v] = max(height[v], height[u] + 1);
        }
        sort(adj[v].begin(), adj[v].end(), [&](int a, int b) {
            return height[a] > height[b];
        });
    };
    sizes(sizes,start,-1);
    auto visit = [&](auto&& self,int v) -> void {
        pos[v] = order.size(); order.push_back(v);
        for (int u : adj[v]) if (u != parentNode[v]) self(self,u);
    };
    visit(visit,start);
}

// For small trees keep the full set of compatible states.
template<int K> struct SmallState {
    int root;
    uint64_t mask;
    array<signed char,K> f;
};
template<int K>
static void solveStates(vector<SmallState<K>>& states,const vector<int>& focus,
                        const vector<int>& packed={}) {
    vector<vector<int>> choices;
    if (n <= 6) {
        vector<int> a(n);
        auto gen = [&](auto&& self,int i,int rem) -> void {
            if (i == n) {
                if (rem != n) choices.push_back(a);
                return;
            }
            for (a[i]=0; a[i]<=rem; ++a[i]) self(self,i+1,rem-a[i]);
        };
        gen(gen,0,n);
    } else {
        set<vector<int>> unique;
        for (int v=0;v<n;++v) {
            vector<int> a(n); a[v]=1; unique.insert(a);
        }
        for (int i=0;i<260;++i) {
            vector<int> a(n);
            int k = n - (i%4==0 ? rng()%n : 0);
            while (k) {
                int w = i%3 ? 1+rng()%k : 1;
                a[rng()%n] += w; k-=w;
            }
            unique.insert(a);
        }
        if (!packed.empty()) unique.insert(packed);
        if (!focus.empty()) for (int i=0;i<250;++i) {
            vector<int> a(n),vs=focus;
            shuffle(vs.begin(),vs.end(),rng);
            int k=n;
            for (int v:vs) {
                int w=i%3 ? 1+rng()%k : min(k,1<<(rng()%bit_width((unsigned)n)));
                a[v]+=w; k-=w;
                if (!k) break;
            }
            unique.insert(a);
        }
        choices.assign(unique.begin(),unique.end());
    }
    vector<vector<pair<int,int>>> terms;
    for (const auto& q:choices) {
        terms.emplace_back();
        for (int v=0;v<n;++v) if (q[v]) terms.back().emplace_back(v,q[v]);
    }
    vector<double> delta(states.size()+1);
    for (int i=1;i<(int)delta.size();++i)
        delta[i]=i*log2(i)-(i==1?0:(i-1)*log2(i-1));
    for (;;) {
        uint64_t mask=states[0].mask;
        if (all_of(states.begin(),states.end(),[&](const auto& s){return s.mask==mask;})) {
            vector<int> a(n);
            for (int v=0;v<n;++v) a[v]=(mask>>v&1)?1:-1;
            answer(a); return;
        }
        auto applyFlip = [&](SmallState<K>& s,int p) {
            int change=(s.mask>>p&1)?-2:2;
            for (int v=0;v<n;++v)
                if (s.root==p || direction[p][v]!=direction[p][s.root]) s.f[v]+=change;
            s.mask ^= 1ULL<<p;
        };
        int bestQ=-1,bestP=-1;
        double best=-1;
        const int bound=n*n, bins=2*bound+1;
        vector<int> hist(bins),within(bins),stamp(bins,-1);
        int tick=0;
        auto evaluate = [&](int p) {
            // Equal masks remain consecutive, including after a common flip.
            for (int qi=0; qi<(int)choices.size(); ++qi) {
                const auto& items=terms[qi];
                fill(hist.begin(),hist.end(),0);
                double total=0, conditional=0, maskTotal=0;
                uint64_t prev=UINT64_MAX;
                int count=0;
                for (const SmallState<K>& s:states) {
                    int y=bound;
                    for (auto [v,w]:items) y+=w*s.f[v];
                    if (prev!=s.mask) {++tick; prev=s.mask; count=0;}
                    if (stamp[y]!=tick) {stamp[y]=tick; within[y]=0;}
                    total+=delta[++hist[y]];
                    conditional+=delta[++within[y]];
                    maskTotal+=delta[++count];
                }
                double ent=log2(states.size())-total/states.size();
                if (ent<1e-8) continue;
                double info=ent-(maskTotal-conditional)/states.size();
                double score=(info+0.025*ent)/(p<0?1:2);
                // Prefer a terminal measurement over a nonterminal split.
                double labelEntropy=log2(states.size())-maskTotal/states.size();
                if (abs(info-labelEntropy)<1e-8 && p<0) score+=100;
                if (score>best+1e-9) {best=score; bestP=p; bestQ=qi;}
            }
        };
        evaluate(-1);
        if (best<0 || (states.size()<=70 && best<100)) {
            for (int p=0;p<n;++p) {
                for (auto& s:states) applyFlip(s,p);
                evaluate(p);
                for (auto& s:states) applyFlip(s,p);
            }
        }
        if (bestP>=0) {
            toggle(bestP);
            for (auto& s:states) applyFlip(s,bestP);
        }
        const auto& a=choices[bestQ];
        int y=query(a);
        erase_if(states,[&](const SmallState<K>& s) {
            int t=0; for (int v=0;v<n;++v) t+=s.f[v]*a[v];
            return t!=y;
        });
        if (states.empty()) exit(0);
    }
}

static void smallTree() {
    vector<SmallState<12>> states;
    for (int mask = 0; mask < (1<<n); ++mask) for (int r = 0; r < n; ++r) {
        SmallState<12> st{r,(uint64_t)mask,{}};
        auto dfs = [&](auto&& self, int v, int p, int sum) -> void {
            sum += (mask >> v & 1) ? 1 : -1;
            st.f[v] = sum;
            for (int u : adj[v]) if (u != p) self(self,u,v,sum);
        };
        dfs(dfs,r,-1,0);
        states.push_back(st);
    }
    solveStates(states,{});
}

constexpr int B=12, S=1024;
static constexpr int U[]={0,1,2,4,7,13,24,44,84,161,309,594,1164};
struct Quality {
    int duplicates=INT_MAX, c=0;
    bool operator<(const Quality& o) const {
        return tie(duplicates,c)<tie(o.duplicates,o.c);
    }
};
static Quality quality[B+1][2*S];
static void buildCodes() {
    // Among weights c-U[i], keep codes with at most two preimages per sum.
    for (int h=2;h<=B;++h) {
        for (int c=U[h-1]+1;c<=n;++c) {
            bitset<B*1000+1> once,twice;
            once[0]=1;
            bool ok=true;
            for (int i=0;i<h;++i) {
                int w=c-U[i];
                auto shifted=once<<w;
                if ((twice&shifted).any() || ((twice<<w)&once).any()) {ok=false;break;}
                twice|=(twice<<w)|(once&shifted);
                once|=shifted;
            }
            if (ok) quality[h][S+c]={(int)twice.count(),c};
        }
        for (int i=S-1;i;--i) quality[h][i]=min(quality[h][2*i],quality[h][2*i+1]);
    }
}
static Quality bestCode(int h,int lo,int hi) {
    Quality result;
    for (lo+=S,hi+=S+1;lo<hi;lo/=2,hi/=2) {
        if (lo&1) result=min(result,quality[h][lo++]);
        if (hi&1) result=min(result,quality[h][--hi]);
    }
    return result;
}

struct Pack {
    int m=0;
    array<int,B> weight{};
    double cost=1e20;
};
static vector<array<double,2>> dp;
static vector<array<Pack,2>> plans;

// An unknown edge has coefficient equal to the number of query vertices below it.
// Subtract the child coefficients to obtain nonnegative query multiplicities.
static Pack choosePack(int at,int cap) {
    Pack best;
    int par[B];
    for (int i=0;i<min(B,n-at);++i) {
        int p=parentNode[order[at+i]];
        par[i]=pos[p]-at;
    }
    for (int m=1;m<=min(B,n-at);++m) {
        for (int h=1;h<=m;++h) {
            int low=m-h,scale=1<<low;
            if (h>=2 && (U[h-1]+1)*scale>cap) continue;
            int a[B],b[B],da[B],db[B];
            for (int i=0;i<m;++i) {
                if (i<h) {a[i]=h==1?1:scale; b[i]=-U[i]*scale;}
                else {a[i]=0; b[i]=1<<(m-i-1);}
                da[i]=a[i]; db[i]=b[i];
            }
            int topA=0,topB=0;
            for (int i=0;i<m;++i) {
                if (par[i]<0) {topA+=a[i]; topB+=b[i];}
                else {da[par[i]]-=a[i]; db[par[i]]-=b[i];}
            }
            int lo=h==1?1:U[h-1]+1;
            int hi=min(h==1?scale:cap,(cap-topB)/topA);
            for (int i=0;i<m && lo<=hi;++i) {
                if (da[i]>0 && db[i]<0) lo=max(lo,(-db[i]+da[i]-1)/da[i]);
                if (da[i]<0) {
                    if (db[i]<0) hi=0; else hi=min(hi,db[i]/(-da[i]));
                }
                if (!da[i] && db[i]<0) hi=0;
            }
            if (lo>hi) continue;
            int c;
            double amb;
            if (h==1) {c=hi; amb=1.0-(double)c/scale;}
            else {
                auto q=bestCode(h,lo,hi);
                if (!q.c) continue;
                c=q.c; amb=(double)(2*q.duplicates)/(1<<h);
            }
            double cost=1+dp[at+m][0]+amb*(dp[at+m][1]-dp[at+m][0]);
            if (cost<best.cost-1e-10 || (abs(cost-best.cost)<1e-10 && m>best.m)) {
                best.cost=cost; best.m=m;
                for (int i=0;i<m;++i) best.weight[i]=a[i]*c+b[i];
            }
        }
    }
    return best;
}

class Solver {
    int at=1;
    vector<vector<int>> f;
    vector<int> offset, flipped, possible, label;
    vector<double> change;

    array<int,4> measured{-1,-1,-1,-1};
    vector<int> prediction;
    bool checkedPrediction=false;
    int guess=-1, guessSign=1;
    int pattern(int v,int kind) const {
        if (kind==3) return prediction[v];
        if (kind==2) return direction[v][guess]==parentNode[v]?guessSign:-guessSign;
        return kind && depth[v]%2 ? -1 : 1;
    }
    int knownErrors(int kind,const vector<int>& pot,int end) const {
        int errors=0;
        for (int i=1;i<end;++i) {
            int v=order[i];
            errors+=pot[v]-pot[parentNode[v]]!=pattern(v,kind);
        }
        return errors;
    }
    // Each edge contributes one signed copy of each endpoint. After cancellation
    // the positive and negative parts each use at most one copy per selected edge.
    int measureErrors(int end,int kind,int sign) {
        vector<int> c(n),q(n);
        for (int i=at;i<end;++i) {
            int v=order[i],w=sign*pattern(v,kind);
            c[v]+=w; c[parentNode[v]]-=w;
        }
        int dot=0;
        for (int i=0;i<at;++i) {
            int v=order[i]; dot+=c[v]*f[0][v]; c[v]=0;
        }
        for (int polarity:{1,-1}) {
            bool used=false;
            for (int v=0;v<n;++v) {
                q[v]=max(0,polarity*c[v]); used|=q[v]>0;
            }
            if (used) dot+=polarity*(query(q)-inner_product(q.begin(),q.end(),offset.begin(),0));
        }
        return (end-at-dot)/2;
    }
    // With one wrong predicted edge, a query changes by twice the number of
    // queried vertices in that edge's subtree, with a known sign.
    int locateError(int end,int kind,int sign) {
        int m=end-at;
        vector<int> pred=f[0], candidates(m);
        iota(candidates.begin(),candidates.end(),at);
        for (int i=at;i<end;++i) {
            int v=order[i]; pred[v]=pred[parentNode[v]]+sign*pattern(v,kind);
        }
        while (candidates.size()>1) {
            vector<char> active(n);
            for (int i:candidates) active[order[i]]=1;
            vector<int> q(n),weight(n);
            auto design=[&](bool exact) {
                fill(q.begin(),q.end(),0); fill(weight.begin(),weight.end(),0);
                map<int,int> freq;
                int spent=0,step=max(1,2*n/(int)candidates.size());
                for (int i=end-1;i>=at;--i) {
                    int v=order[i],sgn=sign*pattern(v,kind);
                    if (active[v]) {
                        int w=weight[v];
                        if (exact) {
                            while (freq.count(sgn*w)) ++w;
                        } else {
                            int bound=w+min(step,n-spent);
                            for (int j=w+1;j<=bound && freq[sgn*w]>0;++j)
                                if (freq[sgn*j]<freq[sgn*w]) w=j;
                        }
                        ++freq[sgn*w]; q[v]=w-weight[v]; spent+=q[v]; weight[v]=w;
                        if (spent>n) return false;
                    }
                    weight[parentNode[v]]+=weight[v];
                }
                return spent>0;
            };
            if (!design(true)) design(false);
            auto shift=[&](int i) {int v=order[i];return -2*sign*pattern(v,kind)*weight[v];};
            if (all_of(candidates.begin(),candidates.end(),[&](int i){return shift(i)==shift(candidates[0]);})) {
                fill(q.begin(),q.end(),0); fill(weight.begin(),weight.end(),0);
                int v=order[candidates[0]]; q[v]=1; weight[v]=1;
            }
            int y=query(q);
            for (int v=0;v<n;++v) y-=q[v]*(pred[v]+offset[v]);
            erase_if(candidates,[&](int i){return shift(i)!=y;});
            if (candidates.empty()) exit(0);
        }
        return candidates[0];
    }
    void sparseRange(int end,int kind,int sign,int errors) {
        int m=end-at;
        if (errors>m/2) {sign=-sign; errors=m-errors;}
        if (errors<=1) {
            int bad=errors?locateError(end,kind,sign):-1;
            while (at<end) {
                int v=order[at];
                f[0][v]=f[0][parentNode[v]]+(at==bad?-sign:sign)*pattern(v,kind);
                ++at;
            }
        } else {
            int mid=at+m/2, left=measureErrors(mid,kind,sign);
            sparseRange(mid,kind,sign,left);
            sparseRange(end,kind,sign,errors-left);
        }
    }
    bool uniformFits(int minus) const {
        int x=f[0][order[0]];
        if (!x) return false;
        int drops=x>0?minus:at-1-minus;
        if (drops>abs(x)-1) return false;
        vector<int> anc(n);
        for (int v:order) {
            anc[v]=pos[v]<at?v:anc[parentNode[v]];
            if (depth[v]!=abs(x)-1) continue;
            int u=anc[v];
            if (depth[u]==drops && abs(x)-(x>0?f[0][u]:-f[0][u])==drops)
                return true;
        }
        return false;
    }
    // The sum of (2-degree(v))*f(v) equals sum(a[v])+a[root].
    // A small deficit from its maximum bounds the error in every path length.
    void approximateRoot(int sign,int deficit) {
        vector<int> roots=possible, value(n,INT_MAX);
        auto filter=[&](int v) {
            erase_if(roots,[&](int r) {
                int d=dist[v][r]+1-sign*value[v];
                return d<0 || d>2*deficit || d%2;
            });
        };
        for (int i=0;i<at;++i) {
            int v=order[i]; value[v]=f[0][v]; filter(v);
        }
        if (roots.empty()) exit(0);
        auto readValue=[&](int v) {
            if (value[v]==INT_MAX) {
                vector<int> q(n); q[v]=1;
                value[v]=query(q)-offset[v]; filter(v);
            }
        };
        auto farthest=[&](int v) {
            return *max_element(roots.begin(),roots.end(),[&](int a,int b) {
                return dist[v][a]<dist[v][b];
            });
        };
        for (int i=0;i<4;++i) {
            int a=farthest(roots.front()), b=farthest(a), d=dist[a][b];
            if (d<=2*deficit+3 || i==3) {
                guess=a;
                for (int j=0;j<d/2;++j) guess=direction[guess][b];
                readValue(guess);
                break;
            }
            // Prefer a landmark whose possible readings separate many roots.
            int best=-1;
            long long rootCount=roots.size();
            long long bestPairs=rootCount*rootCount;
            vector<int> hist(n);
            for (int v=0;v<n;++v) if (value[v]==INT_MAX) {
                fill(hist.begin(),hist.end(),0);
                for (int r:roots) ++hist[dist[v][r]];
                int near=0;
                long long pairs=0;
                for (int j=0;j<n;++j) {
                    if (j>2*deficit) near-=hist[j-2*deficit-1];
                    pairs+=(long long)hist[j]*(hist[j]+2*near);
                    near+=hist[j];
                }
                if (pairs<bestPairs) {bestPairs=pairs; best=v;}
            }
            if (best<0 || bestPairs*5>4*rootCount*rootCount) {
                guess=a;
                for (int j=0;j<d/2;++j) guess=direction[guess][b];
                readValue(guess);
                break;
            }
            readValue(best);
            if (sign*value[best]<=2) {guess=best; break;}
        }
        guessSign=sign;
        measured[2]=deficit-1+sign*value[guess];
        // All observations are original potentials, so these exclusions persist.
        possible=move(roots);
    }
    // Fit simple periodic assignments to the observed values. They only guide
    // the next measurement: the error count is always established by the judge.
    void predictSuffix() {
        checkedPrediction=true;
        vector<vector<int>> features;
        for (int period=2;period<=6;++period) {
            vector<int> byLabel(n),byDepth(n);
            for (int v=0;v<n;++v) {
                byLabel[v]=v%period; byDepth[v]=depth[v]%period;
            }
            features.push_back(move(byLabel)); features.push_back(move(byDepth));
        }
        for (int cap:{2,3,4}) {
            vector<int> degree(n);
            for (int v=0;v<n;++v) degree[v]=min(cap,(int)adj[v].size())-1;
            features.push_back(move(degree));
        }
        set<vector<int>> models;
        vector<int> val(at);
        for (int r:possible) {
            if (pos[r]<at && abs(f[0][r])!=1) continue;
            for (int i=0;i<at;++i) {
                int v=order[i],d=direction[v][r];
                val[i]=r==v?f[0][v]:(pos[d]<at?f[0][v]-f[0][d]:0);
            }
            for (const auto& feat:features) {
                array<int,6> mapping{};
                bool fits=true;
                for (int i=0;i<at && fits;++i) if (val[i]) {
                    int& a=mapping[feat[order[i]]];
                    if (a && a!=val[i]) fits=false;
                    a=val[i];
                }
                if (!fits) continue;
                vector<int> b(n);
                for (int v=0;v<n;++v) b[v]=mapping[feat[v]];
                auto [mn,mx]=minmax_element(b.begin(),b.end());
                if (*mn!=-1 || *mx!=1 || count(b.begin(),b.end(),0)) continue;
                models.insert(move(b));
            }
        }
        double best=max(3.0,(n-at)*0.045);
        for (const auto& b:models) {
            vector<int> err(n),path(n),below(n);
            int ref=order[0]; path[ref]=b[ref];
            for (int i=1;i<at;++i) {
                int v=order[i];
                err[ref]+=f[0][v]-f[0][parentNode[v]]!=b[v];
            }
            for (int i=1;i<n;++i) {
                int v=order[i],p=parentNode[v];
                path[v]=path[p]+b[v]; err[v]=err[p];
                if (i<at) {
                    int d=f[0][v]-f[0][p];
                    err[v]+=(d!=-b[p])-(d!=b[v]);
                }
            }
            int count=0;
            for (int r:possible) if (!err[r] && path[r]==f[0][ref]) {
                ++count; below[r]=1;
            }
            if (!count) continue;
            for (int i=n-1;i>0;--i) below[parentNode[order[i]]]+=below[order[i]];
            vector<int> trial(n);
            long long cost=0;
            for (int i=1;i<n;++i) {
                int v=order[i],p=parentNode[v];
                trial[v]=2*below[v]>count?-b[p]:b[v];
                if (b[p]==b[v]) cost+=min(below[v],count-below[v]);
            }
            if ((double)cost/count<best) {best=(double)cost/count; prediction=move(trial);}
        }
    }
    bool tryPatterns() {
        for (int kind=0;kind<4;++kind) {
            if (kind==3 && prediction.empty()) {
                if (!checkedPrediction && at>=18 && n-at>=30) predictSuffix();
                if (prediction.empty()) continue;
                measured[3]=knownErrors(3,f[0],at)+measureErrors(n,3,1);
            }
            if (kind==2 && guess<0) continue;
            int seen=knownErrors(kind,f[0],at);
            int m=n-at;
            if (measured[kind]<0) {
                int fit=min(seen,at-1-seen);
                bool uniform=!kind && uniformFits(seen);
                if (m<7 || at<4 || (!uniform && fit>(at-1)/22)) continue;
                if (kind && measured[0]>=0) {
                    bool same=true;
                    for (int i=at+1;i<n;++i)
                        if (pattern(order[i],kind)!=pattern(order[at],kind)) {same=false;break;}
                    if (same) {
                        int left=measured[0]-knownErrors(0,f[0],at);
                        measured[1]=seen+(pattern(order[at],kind)>0?left:m-left);
                    }
                }
                if (measured[kind]<0) measured[kind]=seen+measureErrors(n,kind,1);
            }
            if (!kind) {
                int x=f[0][order[0]], drops=x>0?measured[0]:n-1-measured[0];
                // To reach a root potential of +/-1, all these decreasing
                // edges must form exactly the path from the anchor to the root.
                if (x && drops==abs(x)-1) {
                    vector<int> a(n,x>0?1:-1);
                    for (int v=0;v<n;++v) if (flipped[v]) a[v]=-a[v];
                    answer(a); return true;
                }
            }
            int errors=measured[kind]-seen;
            int few=min(errors,m-errors);
            double sparseCost=4.5*few+log2(m);
            if (!kind && guess<0 && few>2 && sparseCost>dp[at][0]*0.6) {
                int ePlus=measured[0]-f[0][order[0]]+1;
                int eMinus=n+1-ePlus;
                int e=min(ePlus,eMinus);
                if (e>0 && e<=16 && 3.0*e+4<dp[at][0])
                    approximateRoot(ePlus<eMinus?1:-1,e);
            }
            if (few<=1 || sparseCost<dp[at][0]) {
                sparseRange(n,kind,1,errors);
                break;
            }
        }
        return false;
    }
    vector<int> multiplicities(const Pack& pack) const {
        vector<int> q(n);
        for (int i=0;i<pack.m;++i) {
            int v=order[at+i], p=parentNode[v];
            q[v]+=pack.weight[i];
            if (pos[p]>=at) q[p]-=pack.weight[i];
        }
        return q;
    }
    int base(const vector<int>& pot,const Pack& pack) const {
        int total=0;
        for (int i=0;i<pack.m;++i) {
            int p=parentNode[order[at+i]];
            if (pos[p]<at) total+=pack.weight[i]*pot[p];
        }
        return total;
    }
    void decode(const Pack& pack,int y) {
        int sum=accumulate(pack.weight.begin(),pack.weight.begin()+pack.m,0);
        int twice=y-base(f[0],pack)+sum;
        if (twice%2) exit(0);
        int target=twice/2;
        vector<vector<int>> next;
        int val=0, prev=0;
        for (int i=0;i<(1<<pack.m);++i) {
            int mask=i^(i>>1);
            if (i) {
                int k=countr_zero((unsigned)(mask^prev));
                val+=(mask>>k&1)?pack.weight[k]:-pack.weight[k];
            }
            prev=mask;
            if (val!=target) continue;
            next.push_back(f[0]);
            auto& pot=next.back();
            for (int j=0;j<pack.m;++j) {
                int v=order[at+j];
                pot[v]=pot[parentNode[v]]+((mask>>j&1)?1:-1);
            }
        }
        for (int kind=0;kind<4;++kind) if (measured[kind]>=0) {
            erase_if(next,[&](const auto& pot) {
                int left=measured[kind]-knownErrors(kind,pot,at+pack.m);
                return left<0 || left>n-at-pack.m;
            });
        }
        if (next.empty()) exit(0);
        f=move(next); at+=pack.m;
    }
    int witness() const {
        for (int i=0;i<at;++i) {
            int v=order[i];
            if ((f[0][v]-f[1][v])%4) return v;
        }
        exit(0);
    }
    void normalPack() {
        int carry=(f.size()==2);
        Pack pack=plans[at][carry];
        vector<int> q=multiplicities(pack);
        int factor=carry+1, w=-1;
        for (int& x:q) x*=factor;
        if (carry) {w=witness(); ++q[w];}
        int y=query(q)-inner_product(q.begin(),q.end(),offset.begin(),0);
        if (carry) {
            int sum=accumulate(pack.weight.begin(),pack.weight.begin()+pack.m,0);
            if ((y-f[0][w]-2*(base(f[0],pack)+sum))%4) swap(f[0],f[1]);
            y=(y-f[0][w])/2;
            f.resize(1);
        }
        decode(pack,y);
    }
    // Near the end of a small case, search the remaining joint possibilities for
    // both the values and the root. A single query can then settle both questions.
    void solveTail() {
        vector<SmallState<48>> states;
        int m=n-at;
        for (auto pot:f) for (int mask=0;mask<(1<<m);++mask) {
            for (int j=0;j<m;++j) {
                int v=order[at+j];
                pot[v]=pot[parentNode[v]]+((mask>>j&1)?1:-1);
            }
            bool ok=true;
            for (int k=0;k<4;++k)
                if (measured[k]>=0 && measured[k]!=knownErrors(k,pot,n)) ok=false;
            if (!ok) continue;
            SmallState<48> st{};
            for (int v=0;v<n;++v) st.f[v]=pot[v]+offset[v];
            for (int r:possible) if (abs(pot[r])==1) {
                st.root=r; st.mask=0;
                for (int v=0;v<n;++v) {
                    int a=pot[v]-(v==r?0:pot[direction[v][r]]);
                    if ((a==1)!=bool(flipped[v])) st.mask|=1ULL<<v;
                }
                states.push_back(st);
            }
        }
        if (states.empty()) exit(0);
        sort(states.begin(),states.end(),[](const auto& a,const auto& b){return a.mask<b.mask;});
        vector<int> focus(order.begin()+at,order.end());
        vector<int> q=multiplicities(plans[at][f.size()-1]);
        if (f.size()==2) {
            for (int& w:q) w*=2;
            ++q[witness()];
        }
        solveStates(states,focus,q);
    }
    void initialPack() {
        int k=(n%2?n:n-1);
        Pack pack;
        for (int m=1;m<n && (1<<m)<=k;++m) {
            if (dp[1+m][0]<=dp[1+pack.m][0]+1e-9) pack.m=m;
        }
        for (int i=0;i<pack.m;++i) pack.weight[i]=1<<(pack.m-1-i);
        vector<int> q=multiplicities(pack);
        q[order[0]]=k-accumulate(q.begin(),q.end(),0);
        int y=query(q), sum=(1<<pack.m)-1;
        for (int x=-n;x<=n;++x) {
            if (abs(y-k*x)<=sum && (y-k*x-sum)%2==0) {
                f[0][order[0]]=x;
                // The coefficient of the anchor, including all its descendants, is k.
                decode(pack,y-k*x+base(f[0],pack));
                return;
            }
        }
        exit(0);
    }

    // Candidate roots inside the known prefix have identical unknown suffix values
    // whenever their values agree on the prefix. Use exact bit vectors for this test.
    int classify() {
        vector<int> anc(n);
        for (int v:order) anc[v]=pos[v]<at?v:anc[parentNode[v]];
        auto& pot=f[0];
        erase_if(possible,[&](int r) {
            int d=depth[r]-depth[anc[r]], x=pot[anc[r]];
            return (x+d-1)%2!=0 || abs(abs(x)-1)>d;
        });
        if (possible.empty()) exit(0);
        using Bits=array<unsigned long long,16>;
        vector<Bits> keys(at);
        for (int i=1;i<at;++i) {
            int v=order[i],p=parentNode[v];
            keys[i]=keys[pos[p]];
            auto flipBit=[&](int u) { keys[i][u/64]^=1ULL<<(u%64); };
            if ((pot[p]==1)!=(pot[p]-pot[v]==1)) flipBit(p);
            if ((pot[v]==1)!=(pot[v]-pot[p]==1)) flipBit(v);
        }
        map<Bits,int> classes;
        int cnt=0;
        for (int r:possible) {
            if (pos[r]>=at) label[r]=cnt++;
            else {
                auto [it,fresh]=classes.emplace(keys[pos[r]],cnt);
                if (fresh) ++cnt;
                label[r]=it->second;
            }
        }
        return cnt;
    }
    int pivot() const {
        vector<int> sorted=possible;
        sort(sorted.begin(),sorted.end(),[&](int a,int b){return label[a]<label[b];});
        double best=1e100; int pBest=-1;
        vector<int> count(n), joint(n), last(n);
        for (int p=0;p<n;++p) {
            fill(count.begin(),count.end(),0); fill(last.begin(),last.end(),-1);
            double score=0;
            for (int r:sorted) {
                int d=direction[p][r],l=label[r];
                if (last[d]!=l) {last[d]=l; joint[d]=0;}
                score+=change[++count[d]]-change[++joint[d]];
            }
            if (score<best-1e-8) {best=score;pBest=p;}
        }
        return pBest;
    }
    int halfShift(int p,int d) const {
        int val=f[0][p]+offset[p];
        if (d!=p) val-=f[0][d]+offset[d];
        return -val;
    }
    bool affected(int p,int d,int v) const {
        return d==p || direction[p][v]!=d;
    }
    void applyOffset(int p,int d,int z) {
        for (int v=0;v<n;++v) if (affected(p,d,v)) offset[v]+=2*z;
    }
    // Encode a flip observation in the low digit while packing new edges in
    // higher digits. All root directions then have distinct residues.
    bool integratedFlip(int p) {
        if (pos[p]>=at) return false;
        map<int,int> ids;
        for (int r:possible) {
            int d=direction[p][r];
            if (pos[d]>=at) return false;
            ids[d]=0;
        }
        int radix=ids.size();
        if (radix<2 || radix>8) return false;
        int id=ids.count(p);
        for (auto& [d,j]:ids) if (d!=p) j=id++;
        vector<int> probe(n);
        for (auto [d,j]:ids) if (d!=p)
            probe[d]=(radix-halfShift(p,d)*j%radix)%radix;
        int k=accumulate(probe.begin(),probe.end(),0);
        probe[p]=(radix-k%radix)%radix; k+=probe[p];
        int cap=(n-k)/radix;
        if (cap<1) return false;
        Pack pack=choosePack(at,cap);
        if (!pack.m) return false;
        vector<int> q=multiplicities(pack);
        for (int v=0;v<n;++v) q[v]=radix*q[v]+probe[v];
        toggle(p); flipped[p]^=1;
        int y=query(q)-inner_product(q.begin(),q.end(),offset.begin(),0);
        int probeBase=inner_product(probe.begin(),probe.end(),f[0].begin(),0);
        int parity=base(f[0],pack)+accumulate(pack.weight.begin(),pack.weight.begin()+pack.m,0);
        int raw=y-probeBase-radix*parity;
        if (raw%2) exit(0);
        int j=(raw/2%radix+radix)%radix;
        int d=-1;
        for (auto [dir,num]:ids) if (num==j) d=dir;
        if (d==-1) exit(0);
        int z=halfShift(p,d),affectedCount=0;
        for (int v=0;v<n;++v) if (affected(p,d,v)) affectedCount+=q[v];
        erase_if(possible,[&](int r){return direction[p][r]!=d;});
        applyOffset(p,d,z);
        decode(pack,(y-probeBase-2*z*affectedCount)/radix);
        return true;
    }
    bool oneClass() const {
        return all_of(possible.begin(),possible.end(),[&](int r){return label[r]==label[possible[0]];});
    }

    void finishFlip(int p) {
        toggle(p); flipped[p]^=1;
        for (;;) {
            map<int,vector<int>> byDir;
            for (int r:possible) byDir[direction[p][r]].push_back(r);
            // Directions giving the same final answer can share a weight.
            struct Group { vector<int> dirs; bool center=false; };
            array<vector<Group>,2> groups;
            map<pair<int,int>,int> where;
            for (const auto& [d,rs]:byDir) {
                int key=label[rs[0]];
                for (int r:rs) if (label[r]!=key) {key=-d-1; break;}
                int sign=halfShift(p,d)>0;
                auto [it,fresh]=where.emplace(make_pair(sign,key),groups[sign].size());
                if (fresh) groups[sign].push_back({});
                auto& g=groups[sign][it->second]; g.dirs.push_back(d); g.center|=d==p;
            }
            for (auto& side:groups) sort(side.begin(),side.end(),[](const Group& a,const Group& b){
                if (a.center!=b.center) return a.center;
                return a.dirs.size()>b.dirs.size();
            });
            int limit=max(groups[0].size(),groups[1].size());
            vector<int> q(n);
            for (int radix=limit;radix>=1;--radix) {
                fill(q.begin(),q.end(),0);
                q[p]=1; // Makes both signs distinguishable even for a single nonzero weight.
                for (auto& side:groups) for (int i=0;i<(int)side.size();++i) {
                    for (int d:side[i].dirs) if (d!=p) q[d]=i%radix;
                }
                if (accumulate(q.begin(),q.end(),0)<=n) break;
            }
            int k=accumulate(q.begin(),q.end(),0);
            int y=query(q);
            for (int v=0;v<n;++v) y-=q[v]*(f[0][v]+offset[v]);
            erase_if(possible,[&](int r){
                int d=direction[p][r];
                return y!=2*halfShift(p,d)*(k-(d==p?0:q[d]));
            });
            if (possible.empty()) exit(0);
            if (oneClass()) return;
            int d=direction[p][possible.front()];
            if (all_of(possible.begin(),possible.end(),[&](int r){return direction[p][r]==d;})) {
                applyOffset(p,d,halfShift(p,d));
                return;
            }
        }
    }
public:
    Solver():f(1,vector<int>(n)),offset(n),flipped(n),possible(n),label(n),change(n+1) {
        iota(possible.begin(),possible.end(),0);
        for (int i=1;i<=n;++i) change[i]=i*log2(i)-(i==1?0:(i-1)*log2(i-1));
    }
    void run() {
        buildCodes();
        dp.resize(n+1); plans.resize(n+1); dp[n]={0,1};
        for (int i=n-1;i>=1;--i) for (int carry=0;carry<=1;++carry) {
            plans[i][carry]=choosePack(i,(n-carry)/(1+carry));
            dp[i][carry]=plans[i][carry].cost;
        }
        initialPack();
        int window=pow(log2(n)+1,2);
        while (at<n) {
            if (f.size()==1 && tryPatterns()) return;
            if (at==n) break;
            if (n<=48 && n-at<=9) {solveTail(); return;}
            if (f.size()==1 && at>=n/2 && n-at<=window && possible.size()>1) {
                if (classify()>1) {
                    set<int> known;
                    for (int r:possible) if (pos[r]<at) known.insert(label[r]);
                    if (known.size()>=2 && integratedFlip(pivot())) continue;
                }
            }
            normalPack();
        }
        if (f.size()==2) {
            int w=witness();
            vector<int> q(n); q[w]=1;
            int y=query(q)-offset[w];
            if (y!=f[0][w]) swap(f[0],f[1]);
            f.resize(1);
        }
        classify();
        while (!oneClass()) finishFlip(pivot());
        int r=possible[0];
        vector<int> a(n);
        for (int v=0;v<n;++v) {
            a[v]=f[0][v]-(v==r?0:f[0][direction[v][r]]);
            if (flipped[v]) a[v]=-a[v];
        }
        answer(a);
    }
};
int main() {
    ios::sync_with_stdio(false); cin.tie(nullptr);
    if (!(cin>>n)) return 0;
    adj.resize(n);
    for (int i=1;i<n;++i) {
        int a,b; if (!(cin>>a>>b)) return 0;
        --a;--b; adj[a].push_back(b);adj[b].push_back(a);
    }
    prepareTree();
    if (n<=12) smallTree();
    else Solver().run();
}
