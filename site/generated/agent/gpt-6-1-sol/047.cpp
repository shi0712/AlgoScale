#include <bits/stdc++.h>
using namespace std;

struct Item { array<int,4> p; int n; };

// A bounded-variable revised simplex. There is one assignment equation per
// distinct position and only four other rows, so its dense basis is 4 by 4.
struct LP {
    int n, nv;
    vector<Item>& a;
    array<int,4> cap;
    vector<int> lo,hi;
    vector<long double> val,upper,cost;
    vector<int> basic, key, extra;
    vector<unsigned char> in;
    long double inv[4][4], pi[4];
    bool phase2=false;
    LP(vector<Item>& a_,array<int,4> c,const vector<int>& l,const vector<int>& h):n(a_.size()),nv(5*n+4),a(a_),cap(c),lo(l),hi(h),val(nv),upper(nv),cost(nv),key(n),in(nv) {
        for(int i=0;i<n;i++) {
            for(int j=0;j<4;j++) upper[5*i+j]=hi[4*i+j]-lo[4*i+j];
            upper[5*i+4]=a[i].n;
            cost[5*i+4]=1;
            basic.push_back(5*i+4);in[5*i+4]=1;
        }
        for(int j=0;j<4;j++) {upper[5*n+j]=1e100;basic.push_back(5*n+j);in[5*n+j]=1;}
    }
    void col(int v,long double* d) {
        fill(d,d+4,0.);
        if(v>=5*n) {d[v-5*n]=1;return;}
        int i=v/5,j=v%5;
        if(j<4)d[j]=a[i].p[j];
    }
    bool rebuild() {
        fill(key.begin(),key.end(),-1);extra.clear();
        for(int v:basic) {
            if(v<5*n && key[v/5]<0)key[v/5]=v;
            else extra.push_back(v);
        }
        long double rhs[4];for(int j=0;j<4;j++)rhs[j]=cap[j];
        vector<long double> rem(n);
        for(int i=0;i<n;i++) {
            rem[i]=a[i].n;
            for(int j=0;j<4;j++) {rem[i]-=lo[4*i+j];rhs[j]-=static_cast<long double>(a[i].p[j])*lo[4*i+j];}
            for(int j=0;j<5;j++)if(!in[5*i+j]) {
                rem[i]-=val[5*i+j];if(j<4)rhs[j]-=val[5*i+j]*a[i].p[j];
            }
            int j=key[i]%5;if(j<4)rhs[j]-=rem[i]*a[i].p[j];
        }
        for(int j=0;j<4;j++)if(!in[5*n+j])rhs[j]-=val[5*n+j];
        long double mat[4][8]={};
        long double ec[4];
        for(int k=0;k<4;k++) {
            int v=extra[k];long double d[4];col(v,d);ec[k]=cost[v];
            if(v<5*n) {
                long double b[4];col(key[v/5],b);for(int j=0;j<4;j++)d[j]-=b[j];
                ec[k]-=cost[key[v/5]];
            }
            for(int j=0;j<4;j++)mat[j][k]=d[j];
        }
        for(int j=0;j<4;j++)mat[j][4+j]=1;
        for(int k=0;k<4;k++) {
            int r=k;for(int j=k+1;j<4;j++)if(abs(mat[j][k])>abs(mat[r][k]))r=j;
            if(abs(mat[r][k])<1e-12)return false;
            for(int j=0;j<8;j++)swap(mat[k][j],mat[r][j]);
            long double z=mat[k][k];for(int j=0;j<8;j++)mat[k][j]/=z;
            for(int r2=0;r2<4;r2++)if(r2!=k) {
                z=mat[r2][k];for(int j=0;j<8;j++)mat[r2][j]-=z*mat[k][j];
            }
        }
        for(int j=0;j<4;j++) {pi[j]=0;for(int k=0;k<4;k++){inv[k][j]=mat[k][4+j];pi[j]+=ec[k]*inv[k][j];}}
        for(int i=0;i<n;i++)val[key[i]]=rem[i];
        for(int k=0;k<4;k++) {
            long double z=0;for(int j=0;j<4;j++)z+=inv[k][j]*rhs[j];
            val[extra[k]]=z;
            if(extra[k]<5*n)val[key[extra[k]/5]]-=z;
        }
        return true;
    }
    bool solve(vector<long double>& answer) {
        int iter=0;
        while(true) {
            if(!rebuild())return false;
            int ent=-1;long double best=-1e-8,sgn=1;
            for(int v=0;v<nv;v++)if(!in[v] && upper[v]>1e-9) {
                if(phase2 && v<5*n && v%5==4)continue;
                long double rc=cost[v];
                if(v>=5*n)rc-=pi[v-5*n];
                else {
                    int i=v/5,j=v%5,k=key[i]%5;
                    rc-=cost[key[i]];
                    if(j<4)rc-=pi[j]*a[i].p[j];
                    if(k<4)rc+=pi[k]*a[i].p[k];
                }
                long double s=(val[v]>upper[v]*0.5 ? -1:1);
                rc*=s;
                if(rc<best) {best=rc;ent=v;sgn=s;if(iter>10000)break;}
            }
            if(ent<0) {
                if(!phase2) {
                    long double artificial=0;for(int i=0;i<n;i++)artificial+=val[5*i+4];
                    if(artificial>1e-6)return false;
                    phase2=true;
                    for(int i=0;i<n;i++)upper[5*i+4]=0;
                    for(int i=0;i<n;i++)for(int j=0;j<5;j++)cost[5*i+j]=(j<4 ? a[i].p[j]:0);
                    continue;
                }
                answer.resize(4*n);
                for(int i=0;i<n;i++)for(int j=0;j<4;j++)answer[4*i+j]=lo[4*i+j]+val[5*i+j];
                return true;
            }
            long double d[4];col(ent,d);
            int eg=ent<5*n ? ent/5:-1;
            if(eg>=0) {long double b[4];col(key[eg],b);for(int j=0;j<4;j++)d[j]-=b[j];}
            long double dir[4];for(int k=0;k<4;k++){dir[k]=0;for(int j=0;j<4;j++)dir[k]+=inv[k][j]*d[j]*sgn;}
            long double step=upper[ent];int leave=-1;
            auto check=[&](int v,long double z) {
                long double t=1e100;
                if(z>1e-10)t=max(0.L,val[v])/z;
                else if(z<-1e-10)t=max(0.L,upper[v]-val[v])/(-z);
                if(t<step-1e-9 || (abs(t-step)<1e-9 && (leave<0 || v<leave))) {step=t;leave=v;}
            };
            for(int k=0;k<4;k++)check(extra[k],dir[k]);
            // Only groups containing an extra basic variable, and the entering
            // group, can have a changing key variable.
            int groups[5],ng=0;if(eg>=0)groups[ng++]=eg;
            for(int k=0;k<4;k++)if(extra[k]<5*n) {
                int g=extra[k]/5;bool seen=false;for(int q=0;q<ng;q++)if(groups[q]==g)seen=true;
                if(!seen)groups[ng++]=g;
            }
            for(int q=0;q<ng;q++) {
                int g=groups[q];long double z=(g==eg?sgn:0);
                for(int k=0;k<4;k++)if(extra[k]<5*n && extra[k]/5==g)z-=dir[k];
                check(key[g],z);
            }
            if(leave<0)val[ent]=(sgn>0 ? upper[ent]:0);
            else {
                long double ld=0;
                for(int k=0;k<4;k++)if(extra[k]==leave)ld=dir[k];
                if(leave<5*n && key[leave/5]==leave) {
                    int g=leave/5;ld=(g==eg?sgn:0);
                    for(int k=0;k<4;k++)if(extra[k]<5*n && extra[k]/5==g)ld-=dir[k];
                }
                val[leave]=(ld>0 ? 0:upper[leave]);
                in[leave]=0;in[ent]=1;
                for(int& v:basic)if(v==leave){v=ent;break;}
            }
            ++iter;
        }
    }
};

vector<Item> a;
array<int,4> caps;
long long nodes=0;

// When every remaining position has two possible hospitals, jobs on an
// edge are an ordinary bounded knapsack. Leaf elimination and a single
// scan solve a cycle of hospitals exactly.
int edge_solve(const vector<int>& lo,const vector<int>& hi) {
    int n=a.size();array<int,4> cap=caps;
    struct Group {int p,q,k;};
    vector<Group> groups[4][4];
    for(int i=0;i<n;i++) {
        int rem=a[i].n;vector<int> choices;
        for(int j=0;j<4;j++) {
            cap[j]-=lo[4*i+j]*a[i].p[j];rem-=lo[4*i+j];
            if(hi[4*i+j]>lo[4*i+j])choices.push_back(j);
        }
        if(!rem)continue;
        if(choices.size()>2)return -1;
        if(choices.size()==1) {
            int u=choices[0];if(hi[4*i+u]-lo[4*i+u]<rem)return 0;
            cap[u]-=rem*a[i].p[u];
        }
        else if(choices.size()==2) {
            int u=choices[0],v=choices[1];
            int mu=hi[4*i+u]-lo[4*i+u],mv=hi[4*i+v]-lo[4*i+v];
            if(mu+mv<rem)return 0;
            int fu=max(0,rem-mv),fv=max(0,rem-mu);
            cap[u]-=fu*a[i].p[u];cap[v]-=fv*a[i].p[v];rem-=fu+fv;
            if(rem)groups[u][v].push_back({a[i].p[u],a[i].p[v],rem});
        }else return 0;
    }
    for(int c:cap)if(c<0)return 0;
    vector<int> f[4][4];array<int,4> adj={};int edges=0;
    for(int u=0;u<4;u++)for(int v=u+1;v<4;v++)if(!groups[u][v].empty()) {
        edges++;adj[u]|=1<<v;adj[v]|=1<<u;
    }
    if(edges==6)return -1;
    for(int u=0;u<4;u++)for(int v=u+1;v<4;v++)if(!groups[u][v].empty()) {
        vector<int> dp(cap[u]+1,0);int total=0;
        for(auto [p,q,k]:groups[u][v]) {
            total+=q*k;
            for(int b=1;k;b*=2) {
                int take=min(b,k);k-=take;int w=p*take,g=q*take;
                for(int c=cap[u];c>=w;c--)dp[c]=max(dp[c],dp[c-w]+g);
            }
        }
        f[u][v].resize(cap[u]+1);
        for(int c=0;c<=cap[u];c++)f[u][v][c]=total-dp[c];
        f[v][u].assign(cap[v]+1,1000000000);
        int x=cap[u];
        for(int c=0;c<=cap[v];c++) {
            while(x>0 && f[u][v][x-1]<=c)x--;
            if(f[u][v][x]<=c)f[v][u][c]=x;
        }
    }
    auto finish=[&](array<int,4> c,array<int,4> graph)->bool {
        bool changed=true;
        while(changed) {
            changed=false;
            for(int u=0;u<4;u++)if(__builtin_popcount((unsigned)graph[u])==1) {
                int v=__builtin_ctz((unsigned)graph[u]);
                if(c[u]<0)return false;
                c[v]-=f[u][v][c[u]];if(c[v]<0)return false;
                graph[u]=0;graph[v]^=1<<u;changed=true;
            }
        }
        int start=-1;for(int u=0;u<4;u++)if(graph[u])start=u;
        if(start<0)return true;
        int next=__builtin_ctz((unsigned)graph[start]);
        for(int x=0;x<=c[start];x++) {
            int prev=start,u=next,required=f[start][next][x];bool ok=true;
            while(u!=start) {
                int left=c[u]-required;if(left<0){ok=false;break;}
                int v=__builtin_ctz((unsigned)(graph[u] & ~(1<<prev)));
                required=f[u][v][left];prev=u;u=v;
            }
            if(ok && required+x<=c[start])return true;
        }
        return false;
    };
    if(edges<5)return finish(cap,adj);
    int u=-1,v=-1;
    for(int j=0;j<4;j++)if(__builtin_popcount((unsigned)adj[j])==3){if(u<0)u=j;else v=j;}
    adj[u]^=1<<v;adj[v]^=1<<u;
    for(int x=0;x<=cap[u];x++) {
        int y=f[u][v][x];if(y>cap[v])continue;
        auto c=cap;c[u]-=x;c[v]-=y;
        if(finish(c,adj))return 1;
    }
    return 0;
}

bool repair(const vector<long double>& x,const vector<int>& lo,const vector<int>& hi) {
    int n=a.size();
    vector<array<int,4>> z(n);
    array<int,4> load={};
    for(int i=0;i<n;i++) {
        int left=a[i].n;
        for(int j=0;j<4;j++){z[i][j]=max(lo[4*i+j],min(hi[4*i+j],int(floor(x[4*i+j]+1e-7))));left-=z[i][j];}
        while(left--) {
            int best=-1;long double score=-1e100;
            for(int j=0;j<4;j++)if(z[i][j]<hi[4*i+j]) {
                long double s=x[4*i+j]-z[i][j];if(s>score){score=s;best=j;}
            }
            if(best<0)return false;
            z[i][best]++;
        }
        for(int j=0;j<4;j++)load[j]+=z[i][j]*a[i].p[j];
    }
    vector<array<int,4>> tabu(n);
    auto penalty=[&](int j,int l)->long long {long long v=max(0,l-caps[j]);return v*v;};
    for(int it=0;it<(nodes<4?160:8);it++) {
        long long old=0;for(int j=0;j<4;j++)old+=penalty(j,load[j]);
        if(!old)return true;
        long long best=(nodes<4?LLONG_MAX:old);int ai=-1,aj=-1,bi=-1,bj=-1;
        for(int j=0;j<4;j++)if(load[j]>caps[j])for(int i=0;i<n;i++)if(z[i][j]>lo[4*i+j])for(int k=0;k<4;k++)if(k!=j && z[i][k]<hi[4*i+k]) {
            int l1=load[j]-a[i].p[j],l2=load[k]+a[i].p[k];
            long long q=old-penalty(j,load[j])-penalty(k,load[k])+penalty(j,l1)+penalty(k,l2);
            if(q<best && (q==0 || tabu[i][k]<=it)){best=q;ai=i;aj=j;bi=-1;bj=k;}
            for(int h=0;h<n;h++)if(h!=i && z[h][k]>lo[4*h+k] && z[h][j]<hi[4*h+j]) {
                long long q2=old-penalty(j,load[j])-penalty(k,load[k])+penalty(j,l1+a[h].p[j])+penalty(k,l2-a[h].p[k]);
                if(q2<best && (q2==0 || (tabu[i][k]<=it && tabu[h][j]<=it))){best=q2;ai=i;aj=j;bi=h;bj=k;}
            }
        }
        if(ai<0)return false;
        tabu[ai][aj]=it+5;
        z[ai][aj]--;z[ai][bj]++;load[aj]-=a[ai].p[aj];load[bj]+=a[ai].p[bj];
        if(bi>=0){tabu[bi][bj]=it+5;z[bi][bj]--;z[bi][aj]++;load[bj]-=a[bi].p[bj];load[aj]+=a[bi].p[aj];}
    }
    return false;
}

bool search(vector<int>& lo,vector<int>& hi) {
    ++nodes;
    int n=a.size();
    // Propagate assignment counts and the four integral capacity constraints.
    bool change=true;
    while(change) {
        change=false;
        array<long long,4> used={};
        for(int i=0;i<n;i++)for(int j=0;j<4;j++)used[j]+=1LL*lo[4*i+j]*a[i].p[j];
        for(int j=0;j<4;j++)if(used[j]>caps[j])return false;
        long long minimum=0;
        for(int i=0;i<n;i++) {
            int sl=0,sh=0;
            for(int j=0;j<4;j++) {
                int v=4*i+j;
                int h=lo[v]+(caps[j]-used[j])/a[i].p[j];
                if(hi[v]>h){hi[v]=h;change=true;}
                if(lo[v]>hi[v])return false;
                sl+=lo[v];sh+=hi[v];
            }
            if(sl>a[i].n || sh<a[i].n)return false;
            for(int j=0;j<4;j++) {
                int v=4*i+j;
                int l=max(lo[v],a[i].n-(sh-hi[v]));
                int h=min(hi[v],a[i].n-(sl-lo[v]));
                if(l!=lo[v] || h!=hi[v]){lo[v]=l;hi[v]=h;change=true;}
            }
            int remaining;
            array<pair<int,int>,4> order;
            for(int j=0;j<4;j++){minimum+=1LL*lo[4*i+j]*a[i].p[j];order[j]={a[i].p[j],hi[4*i+j]-lo[4*i+j]};}
            sort(order.begin(),order.end());
            // Recompute after propagation because its lower bounds changed.
            remaining=a[i].n;for(int j=0;j<4;j++)remaining-=lo[4*i+j];
            for(auto [p,c]:order){int take=min(remaining,c);minimum+=1LL*take*p;remaining-=take;}
        }
        if(minimum>accumulate(caps.begin(),caps.end(),0LL))return false;
    }
    int exact=edge_solve(lo,hi);if(exact>=0)return exact;
    vector<long double> x;
    LP lp(a,caps,lo,hi);
    if(!lp.solve(x))return false;
    long double total=0;for(int i=0;i<n;i++)for(int j=0;j<4;j++)total+=x[4*i+j]*a[i].p[j];
    long double budget=accumulate(caps.begin(),caps.end(),0.0)-total;
    bool tightened=false;
    for(int i=0;i<n;i++)for(int j=0;j<4;j++) {
        int v=5*i+j,w=4*i+j;
        if(lp.in[v])continue;
        int k=lp.key[i]%5;
        long double rc=a[i].p[j]-lp.cost[lp.key[i]]-lp.pi[j]*a[i].p[j];
        if(k<4)rc+=lp.pi[k]*a[i].p[k];
        if(rc>1e-7 && budget/rc<hi[w]-lo[w]) {
            int h=lo[w]+max(0,int(floor((budget+1e-6)/rc)));
            if(h<hi[w]){hi[w]=h;tightened=true;}
        }else if(rc< -1e-7 && budget/(-rc)<hi[w]-lo[w]) {
            int l=hi[w]-max(0,int(floor((budget+1e-6)/(-rc))));
            if(l>lo[w]){lo[w]=l;tightened=true;}
        }
    }
    if(tightened)return search(lo,hi);
    if(nodes<12) {
        auto h=hi;
        for(int i=0;i<n;i++) {
            vector<pair<long double,int>> order;
            for(int j=0;j<4;j++)if(hi[4*i+j]>lo[4*i+j])order.push_back({a[i].p[j]*(1-lp.pi[j]),j});
            sort(order.begin(),order.end());
            for(int j=2;j<(int)order.size();j++)h[4*i+order[j].second]=lo[4*i+order[j].second];
        }
        if(edge_solve(lo,h)==1)return true;
    }
    if(nodes<12) {
        vector<int> l=lo,h=hi;
        bool any=false;
        for(int i=0;i<n;i++) {
            int options=0;for(int j=0;j<4;j++)options+=hi[4*i+j]>lo[4*i+j];
            if(options<=2)continue;
            any=true;array<int,4> take;int rem=a[i].n;
            for(int j=0;j<4;j++){take[j]=max(lo[4*i+j],min(hi[4*i+j],int(floor(x[4*i+j]+1e-7))));rem-=take[j];}
            while(rem--) {
                int b=-1;long double best=-1e100;
                for(int j=0;j<4;j++)if(take[j]<hi[4*i+j] && x[4*i+j]-take[j]>best){best=x[4*i+j]-take[j];b=j;}
                if(b<0)break;
                take[b]++;
            }
            for(int j=0;j<4;j++)l[4*i+j]=h[4*i+j]=take[j];
        }
        if(any && edge_solve(l,h)==1)return true;
    }
    if(repair(x,lo,hi))return true;
    int multiBranch=-1;long double multscore=-1e100;
    for(int i=0;i<n;i++) {
        int options=0;for(int j=0;j<4;j++)options+=hi[4*i+j]>lo[4*i+j];
        if(options<=2)continue;
        for(int j=0;j<4;j++) {
            int w=4*i+j;if(hi[w]<=lo[w] || x[w]>lo[w]+1e-6)continue;
            int k=lp.key[i]%5;
            long double rc=a[i].p[j]-lp.cost[lp.key[i]]-lp.pi[j]*a[i].p[j];
            if(k<4)rc+=lp.pi[k]*a[i].p[k];
            long double sc=rc+1e-5/(hi[w]-lo[w]);
            if(sc>multscore){multscore=sc;multiBranch=w;}
        }
    }
    if(multiBranch>=0) {
        vector<int> l=lo,h=hi;h[multiBranch]=lo[multiBranch];
        if(search(l,h))return true;
        l=lo;h=hi;l[multiBranch]++;return search(l,h);
    }
    int branch=-1;long double score=-1;
    for(int v=0;v<4*n;v++) {
        long double f=x[v]-floor(x[v]+1e-7);
        if(f>1e-6 && f<1-1e-6) {
            long double s=min(f,1-f)*a[v/4].p[v%4];
            if(s>score){score=s;branch=v;}
        }
    }
    if(branch<0) {
        array<long long,4> use={};
        for(int i=0;i<n;i++) {
            int sum=0;for(int j=0;j<4;j++){int k=llround(x[4*i+j]);sum+=k;use[j]+=1LL*k*a[i].p[j];}
            if(sum!=a[i].n)return false;
        }
        for(int j=0;j<4;j++)if(use[j]>caps[j])return false;
        return true;
    }
    int low=floor(x[branch]);
    vector<int> l=lo,h=hi;
    // Try the closer integer first.
    if(x[branch]-low<0.5) {
        h[branch]=low;if(search(l,h))return true;
        l=lo;h=hi;l[branch]=low+1;return search(l,h);
    } else {
        l[branch]=low+1;if(search(l,h))return true;
        l=lo;h=hi;h[branch]=low;return search(l,h);
    }
}

int main(){
    ios::sync_with_stdio(false);cin.tie(nullptr);
    int L,N,T;if(!(cin>>L>>N>>T))return 0;
    int C=T/2;
    map<pair<int,int>,int> cnt;
    for(int i=0,x,y;i<N;i++){cin>>x>>y;cnt[{x-1,y-1}]++;}
    for(auto [pt,n]:cnt) {
        auto [x,y]=pt;int m=L-1;
        a.push_back({{x+y,x+m-y,m-x+y,2*m-x-y},n});
    }
    caps.fill(C);
    // On a horizontal or vertical line an exchange always improves both
    // ambulances if a patient farther along the line is sent backwards.
    // Therefore a cut separates the patients assigned to the two sides.
    bool horizontal=true,vertical=true;
    for(auto [pt,k]:cnt){horizontal&=pt.second==cnt.begin()->first.second;vertical&=pt.first==cnt.begin()->first.first;}
    if(horizontal || vertical) {
        vector<array<int,4>> p;
        for(auto [pt,k]:cnt) {
            auto [x,y]=pt;if(vertical)swap(x,y);
            int m=L-1;while(k--)p.push_back({x+y,x+m-y,m-x+y,2*m-x-y});
        }
        sort(p.begin(),p.end(),[](auto x,auto y){return x[0]<y[0];});
        vector<char> prefix(N+1),suffix(N+1);prefix[0]=suffix[N]=1;
        vector<int> dp(C+1);int total=0;
        for(int i=0;i<N;i++) {
            int w=p[i][0],gain=p[i][1];total+=gain;
            for(int c=C;c>=w;c--)dp[c]=max(dp[c],dp[c-w]+gain);
            prefix[i+1]=(total-dp[C]<=C);
        }
        fill(dp.begin(),dp.end(),0);total=0;
        for(int i=N-1;i>=0;i--) {
            int w=p[i][2],gain=p[i][3];total+=gain;
            for(int c=C;c>=w;c--)dp[c]=max(dp[c],dp[c-w]+gain);
            suffix[i]=(total-dp[C]<=C);
        }
        for(int i=0;i<=N;i++)if(prefix[i] && suffix[i]){cout<<"Yes\n";return 0;}
        cout<<"No\n";return 0;
    }
    vector<int> lo(4*a.size()),hi(4*a.size());
    for(int i=0;i<(int)a.size();i++)for(int j=0;j<4;j++)hi[4*i+j]=min(a[i].n,C/a[i].p[j]);
    {
        auto h=hi;
        for(int i=0;i<(int)a.size();i++) {
            array<pair<int,int>,4> order;for(int j=0;j<4;j++)order[j]={a[i].p[j],j};sort(order.begin(),order.end());
            for(int j=2;j<4;j++)h[4*i+order[j].second]=0;
        }
        if(edge_solve(lo,h)==1){cout<<"Yes\n";return 0;}
    }
    cout<<(search(lo,hi)?"Yes\n":"No\n");
}
