#include <bits/stdc++.h>
using namespace std;
typedef long long ll;

int n;
ll ask(int i,int j,int k){
    printf("? %d %d %d\n",i,j,k);
    fflush(stdout);
    ll r;
    if(scanf("%lld",&r)!=1) exit(0);
    if(r<0) exit(0);
    return r;
}

int main(){
    if(scanf("%d",&n)!=1) return 0;
    mt19937 rng(chrono::steady_clock::now().time_since_epoch().count());
    vector<int> order(n);
    iota(order.begin(),order.end(),1);
    shuffle(order.begin(),order.end(),rng);
    vector<ll> val(n+1,-1);
    int P[5];
    for(int i=0;i<5;i++)P[i]=order[i];
    // results for triples of indices (a<b<c) among 0..4
    map<array<int,3>,ll> R;
    for(int a=0;a<5;a++)for(int b=a+1;b<5;b++)for(int c=b+1;c<5;c++){
        R[{a,b,c}]=ask(P[a],P[b],P[c]);
    }
    // brute force rank assignments
    int perm[5]={0,1,2,3,4}; // perm[k] = local index with rank k
    bool found=false;
    vector<ll> best(5);
    do{
        int rk[5];
        for(int k=0;k<5;k++)rk[perm[k]]=k;
        // get sums by rank pairs
        auto getS=[&](int r1,int r2,int r3)->ll{
            int a=perm[r1],b=perm[r2],c=perm[r3];
            array<int,3> t={a,b,c};
            sort(t.begin(),t.end());
            return R[t];
        };
        ll s13=getS(0,1,2), s14=getS(0,1,3), s15=getS(0,1,4), s24=getS(1,2,3), s35=getS(2,3,4);
        ll t=s13-s15+s35;
        if(t%2!=0) continue;
        ll v[5];
        v[2]=t/2; v[0]=s13-v[2]; v[4]=s15-v[0]; v[3]=s14-v[0]; v[1]=s24-v[3];
        bool ok=true;
        for(int k=0;k<5;k++) if(v[k]<0) ok=false;
        for(int k=0;k+1<5;k++) if(v[k]>v[k+1]) ok=false;
        if(!ok) continue;
        for(auto &e:R){
            auto tr=e.first;
            int mn=min({rk[tr[0]],rk[tr[1]],rk[tr[2]]});
            int mx=max({rk[tr[0]],rk[tr[1]],rk[tr[2]]});
            if(v[mn]+v[mx]!=e.second){ok=false;break;}
        }
        if(!ok) continue;
        for(int i=0;i<5;i++) best[i]=v[rk[i]];
        found=true;
        break;
    }while(next_permutation(perm,perm+5));
    for(int i=0;i<5;i++) val[P[i]]=best[i];
    // known sorted list
    vector<pair<ll,int>> known;
    for(int i=0;i<5;i++) known.push_back({val[P[i]],P[i]});
    sort(known.begin(),known.end());
    int d1=-1,d2=-1; ll dv=0;
    for(int i=0;i+1<(int)known.size();i++) if(known[i].first==known[i+1].first){
        d1=known[i].second; d2=known[i+1].second; dv=known[i].first; break;
    }
    for(int idx=5; idx<n; idx++){
        int x=order[idx];
        ll xv;
        if(d1!=-1){
            ll r=ask(d1,d2,x);
            xv=r-dv;
        } else {
            ll p=known[0].first,q=known[1].first;
            int pp=known[0].second,qp=known[1].second;
            ll r=ask(pp,qp,x);
            if(r>p+q) xv=r-p;
            else if(r<p+q) xv=r-q;
            else {
                ll rv=known[2].first; int rp=known[2].second;
                ll r2=ask(x,qp,rp);
                xv=r2-rv;
            }
        }
        val[x]=xv;
        auto it=lower_bound(known.begin(),known.end(),make_pair(xv,INT_MIN));
        if(d1==-1){
            if(it!=known.end() && it->first==xv){
                d1=it->second; d2=x; dv=xv;
            }
        }
        known.insert(it,{xv,x});
    }
    printf("!");
    for(int i=1;i<=n;i++) printf(" %lld",val[i]);
    printf("\n");
    fflush(stdout);
    return 0;
}
