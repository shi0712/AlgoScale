#include <bits/stdc++.h>
using namespace std;
int n;
vector<vector<int>> adj;
long long askQ(const vector<pair<int,int>>&T){
    long long k=0; for(auto&p:T)k+=p.second;
    string s="? 1 "+to_string(k);
    for(auto&p:T) for(int i=0;i<p.second;i++){ s+=' '; s+=to_string(p.first);}
    s+='\n'; cout<<s<<flush;
    long long q; if(!(cin>>q)) exit(0); return q;
}
void toggleNode(int u){ cout<<"? 2 "<<u<<"\n"<<flush; }
struct PR{long long cost;int maxMass;int nb;vector<int> w;};
PR pack(const vector<int>&ms,int uMass,int M){
    vector<int> bm{uMass}, bc{0}; vector<int> bin(ms.size());
    for(size_t i=0;i<ms.size();i++){
        size_t j=0;
        for(;j<bm.size();j++) if(bm[j]+ms[i]<=M) break;
        if(j==bm.size()){bm.push_back(0);bc.push_back(0);}
        bm[j]+=ms[i]; bc[j]++; bin[i]=j;
    }
    PR r; r.nb=bm.size(); r.cost=0;
    vector<int> idx; for(int j=1;j<r.nb;j++) idx.push_back(j);
    sort(idx.begin(),idx.end(),[&](int a,int b){return bc[a]>bc[b];});
    vector<int> wb(r.nb,0);
    for(size_t t=0;t<idx.size();t++){ wb[idx[t]]=t+1; r.cost+=(long long)(t+1)*bc[idx[t]]; }
    r.w.resize(ms.size());
    for(size_t i=0;i<ms.size();i++) r.w[i]=wb[bin[i]];
    r.maxMass=*max_element(bm.begin(),bm.end());
    return r;
}
int designM(const vector<int>&ms,int uMass,int totalR){
    int lo=max(ms.empty()?0:ms[0],max(uMass,1)), hi=totalR;
    if(lo>hi) lo=hi;
    while(lo<hi){
        int mid=(lo+hi)/2;
        if(pack(ms,uMass,mid).cost<=n-1) hi=mid; else lo=mid+1;
    }
    return hi;
}
int main(){
    cin>>n; adj.assign(n+1,{});
    for(int i=0;i<n-1;i++){int a,b;cin>>a>>b;adj[a].push_back(b);adj[b].push_back(a);}
    vector<int> col(n+1,0),p1(n+1,0),order1,szSub(n+1,1);
    {
        vector<int> st={1}; p1[1]=0; vector<char> vis(n+1,0); vis[1]=1;
        while(!st.empty()){
            int x=st.back();st.pop_back();order1.push_back(x);
            for(int y:adj[x]) if(!vis[y]){vis[y]=1;p1[y]=x;col[y]=col[x]^1;st.push_back(y);}
        }
        for(int i=n-1;i>=0;i--){int x=order1[i]; if(p1[x]) szSub[p1[x]]+=szSub[x];}
    }
    vector<char> inR(n+1,1); int totalR=n;
    bool haveBase=false; long long lastQ=0;
    vector<int> cntSub(n+1);
    while(totalR>1){
        for(int x=1;x<=n;x++) cntSub[x]=inR[x];
        for(int i=n-1;i>=0;i--){int x=order1[i]; if(p1[x]) cntSub[p1[x]]+=cntSub[x];}
        double bestB=-1; int bestU=-1,bestT=0;
        for(int u=1;u<=n;u++){
            int uMass=inR[u];
            vector<pair<int,int>> comps; // mass,size
            vector<pair<int,int>> items; // mass, idx
            for(size_t i=0;i<adj[u].size();i++){
                int v=adj[u][i]; int mass,size;
                if(v==p1[u]){mass=totalR-cntSub[u];size=n-szSub[u];}
                else {mass=cntSub[v];size=szSub[v];}
                if(mass>0){ comps.push_back({size,mass}); items.push_back({mass,(int)i}); }
            }
            // all-ones
            {
                sort(comps.begin(),comps.end());
                int M=uMass; int cur=0; int lastS=-1;
                for(auto&c:comps){
                    if(c.first==lastS) cur+=c.second; else {cur=c.second;lastS=c.first;}
                    M=max(M,cur);
                }
                if(M<totalR){
                    double cost=haveBase?2:3;
                    double b=log((double)totalR/M)/cost;
                    if(b>bestB+1e-12){bestB=b;bestU=u;bestT=0;}
                }
            }
            // designed
            {
                sort(items.begin(),items.end(),greater<pair<int,int>>());
                vector<int> ms; for(auto&p:items) ms.push_back(p.first);
                if(!ms.empty()){
                    int M=designM(ms,uMass,totalR);
                    PR r=pack(ms,uMass,M);
                    if(r.nb>=2 && r.maxMass<totalR && r.cost<=n-1){
                        double b=log((double)totalR/r.maxMass)/3.0;
                        if(b>bestB+1e-12){bestB=b;bestU=u;bestT=1;}
                    }
                }
            }
        }
        if(bestU<0){
            // fallback: shouldn't happen
            for(int x=1;x<=n;x++) if(inR[x]){ for(int y=1;y<=n;y++) if(y!=x) inR[y]=0; break;}
            totalR=1; break;
        }
        int u=bestU;
        int d0=adj[u].size();
        vector<int> compIdx(n+1,-1);
        for(int i=0;i<d0;i++){
            vector<int> st={adj[u][i]}; compIdx[adj[u][i]]=i;
            while(!st.empty()){
                int x=st.back();st.pop_back();
                for(int y:adj[x]) if(y!=u && compIdx[y]<0){compIdx[y]=i;st.push_back(y);}
            }
        }
        vector<long long> w(d0,0); long long tot;
        vector<pair<int,int>> T;
        auto parityFilter=[&](const vector<pair<int,int>>&T,long long Q){
            long long A[2]={0,0};
            for(auto&p:T) A[col[p.first]]+=p.second;
            int obs=((Q%2)+2)%2;
            int cnt=0;
            for(int r=1;r<=n;r++) if(inR[r]){
                int ex=A[col[r]]%2;
                if(ex!=obs) inR[r]=0; else cnt++;
            }
            totalR=cnt;
        };
        long long Q0,Q1;
        if(bestT==0){
            for(int i=0;i<d0;i++){
                int v=adj[u][i];
                int size = (v==p1[u])? n-szSub[u] : szSub[v];
                w[i]=size;
            }
            tot=n;
            for(int x=1;x<=n;x++) T.push_back({x,1});
            if(!haveBase){ Q0=askQ(T); parityFilter(T,Q0);} else Q0=lastQ;
            toggleNode(u);
            Q1=askQ(T); lastQ=Q1; haveBase=true;
        } else {
            vector<pair<int,int>> items;
            for(int i=0;i<d0;i++){
                int v=adj[u][i]; int mass=(v==p1[u])? totalR-cntSub[u] : cntSub[v];
                if(mass>0) items.push_back({mass,i});
            }
            sort(items.begin(),items.end(),greater<pair<int,int>>());
            vector<int> ms; for(auto&p:items) ms.push_back(p.first);
            int uMass=inR[u];
            int M=designM(ms,uMass,totalR);
            PR r=pack(ms,uMass,M);
            long long tot0=0;
            for(size_t i=0;i<items.size();i++){ w[items[i].second]=r.w[i]; tot0+=r.w[i]; }
            int c0=0,c1=0; for(int x=1;x<=n;x++) if(inR[x]){ if(col[x]) c1++; else c0++; }
            long long e=0;
            if(c0>0&&c1>0&&tot0%2==0&&tot0+1<=n) e=1;
            if(tot0==0) e=1;
            tot=tot0+e;
            for(int i=0;i<d0;i++) if(w[i]>0) T.push_back({adj[u][i],(int)w[i]});
            if(e>0) T.push_back({u,(int)e});
            Q0=askQ(T); parityFilter(T,Q0);
            toggleNode(u);
            Q1=askQ(T);
            haveBase=false;
        }
        long long d=llabs(Q1-Q0)/2;
        int cnt=0;
        for(int r=1;r<=n;r++) if(inR[r]){
            long long pred = (r==u)? tot : tot - w[compIdx[r]];
            if(pred!=d) inR[r]=0; else cnt++;
        }
        totalR=cnt;
        if(totalR==0){ inR[1]=1; totalR=1; }
    }
    int root=1;
    for(int x=1;x<=n;x++) if(inR[x]){root=x;break;}
    // decode
    vector<int> par(n+1,0),pre,tin(n+1,0),sz(n+1,1);
    {
        vector<int> st={root};
        while(!st.empty()){
            int x=st.back();st.pop_back();
            tin[x]=pre.size();pre.push_back(x);
            for(int y:adj[x]) if(y!=par[x]){par[y]=x;st.push_back(y);}
        }
        for(int i=n-1;i>=0;i--){int x=pre[i]; if(par[x]) sz[par[x]]+=sz[x];}
    }
    vector<int> val(n+1,0);
    int unknownCount=n;
    while(unknownCount>0){
        vector<int> fr;
        for(int v:pre) if(val[v]==0 && (v==root || val[par[v]]!=0)) fr.push_back(v);
        stable_sort(fr.begin(),fr.end(),[&](int a,int b){return sz[a]>sz[b];});
        int mm=min(unknownCount,12);
        vector<pair<int,int>> pieces; int m=1;
        for(m=mm;m>=1;m--){
            pieces.clear(); int rem=m;
            for(int f:fr){
                int c=min(rem,sz[f]); pieces.push_back({c,f}); rem-=c; if(rem==0)break;
            }
            vector<pair<int,int>> ps=pieces; sort(ps.begin(),ps.end());
            long long cost=0; int S=0;
            for(auto&p:ps){S+=p.first; cost+=1LL<<(S-1);}
            if(cost<=n) break;
        }
        if(m<1) m=1;
        // rebuild pieces for m
        pieces.clear(); {
            int rem=m;
            for(int f:fr){int c=min(rem,sz[f]);pieces.push_back({c,f});rem-=c;if(rem==0)break;}
        }
        stable_sort(pieces.begin(),pieces.end(),[&](const pair<int,int>&a,const pair<int,int>&b){return a.first<b.first;});
        vector<long long> W(n+1,0); vector<char> inA(n+1,0); vector<int> expo(n+1,-1);
        vector<int> Anodes;
        int S=0;
        for(auto&p:pieces){
            S+=p.first; int top=S-1;
            for(int j=0;j<p.first;j++){
                int v=pre[tin[p.second]+j];
                inA[v]=1; expo[v]=top-j; W[v]=1LL<<(top-j); Anodes.push_back(v);
            }
        }
        vector<pair<int,int>> T; vector<long long> cv(n+1,0);
        for(int v:Anodes){
            long long c=W[v];
            for(int ch:adj[v]) if(ch!=par[v] && inA[ch]) c-=W[ch];
            cv[v]=c;
            if(c>0) T.push_back({v,(int)c});
        }
        long long Q=askQ(T);
        long long rest=Q;
        for(int v:Anodes){
            long long s=0; int a=par[v];
            while(a){ if(val[a]!=0) s+=val[a]; a=par[a]; }
            rest-=cv[v]*s;
        }
        long long P=(rest+((1LL<<m)-1))/2;
        for(int v:Anodes){
            val[v]=((P>>expo[v])&1)?1:-1;
        }
        unknownCount-=m;
    }
    string s="!";
    for(int i=1;i<=n;i++){ s+=' '; s+=to_string(val[i]); }
    s+='\n'; cout<<s<<flush;
}
