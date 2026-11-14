#include <bits/stdc++.h>
using namespace std;
static char buf[1<<25];
int bufLen=0,bufPos=0;
inline int gc(){ if(bufPos==bufLen){ bufLen=fread(buf,1,sizeof(buf),stdin); bufPos=0; if(bufLen<=0) return -1;} return buf[bufPos++]; }
inline int readInt(){ int c=gc(); while(c!='-'&&(c<'0'||c>'9')) c=gc(); int x=0; while(c>='0'&&c<='9'){x=x*10+(c-'0');c=gc();} return x; }

// leftist max-heap
vector<int> HK,HV,HL,HR,HD; int hcnt;
int newNode(int k,int v){ if(hcnt>=(int)HK.size()){size_t m=HK.size()*2;HK.resize(m);HV.resize(m);HL.resize(m);HR.resize(m);HD.resize(m);} HK[hcnt]=k;HV[hcnt]=v;HL[hcnt]=HR[hcnt]=0;HD[hcnt]=1; return hcnt++; }
int meld(int a,int b){
    if(!a) return b; if(!b) return a;
    if(HK[a]<HK[b]) swap(a,b);
    HR[a]=meld(HR[a],b);
    if(HD[HL[a]]<HD[HR[a]]) swap(HL[a],HR[a]);
    HD[a]=HD[HR[a]]+1;
    return a;
}
int popH(int a){ return meld(HL[a],HR[a]); }

vector<int> heavy, hh, hp;
vector<int> adjS, adjL;
int n;

void mergechain(int p,int q){
    while(p!=-1){
        hp[p]=meld(hp[p],hp[q]);
        p=heavy[p]; q=heavy[q];
    }
}

int main(){
    int T=readInt();
    HD.assign(1,0); 
    string out;
    while(T--){
        n=readInt();
        vector<int> eu(n-1),ev(n-1),deg(n+1,0);
        for(int i=0;i<n-1;i++){eu[i]=readInt()-1;ev[i]=readInt()-1;deg[eu[i]]++;deg[ev[i]]++;}
        if(n==1){ out+="0\n"; continue; }
        adjS.assign(n+1,0);
        for(int i=0;i<n;i++) adjS[i+1]=adjS[i]+deg[i];
        adjL.assign(2*(n-1),0);
        {vector<int> pos(adjS.begin(),adjS.end()-1);
        for(int i=0;i<n-1;i++){adjL[pos[eu[i]]++]=ev[i];adjL[pos[ev[i]]++]=eu[i];}}
        vector<int> dist(n),par(n),order; order.reserve(n);
        auto bfs=[&](int s){
            order.clear(); fill(dist.begin(),dist.end(),-1); dist[s]=0; par[s]=-1; order.push_back(s);
            for(size_t h=0;h<order.size();h++){int v=order[h];
                for(int e=adjS[v];e<adjS[v+1];e++){int u=adjL[e]; if(dist[u]<0){dist[u]=dist[v]+1;par[u]=v;order.push_back(u);}}}
            return order.back();
        };
        int a=bfs(0); int b=bfs(a); int D=dist[b];
        // path from b to a: walk par
        int r;
        // choose root at distance floor(D/2) from b if even, (D-1)/2 from ... compute
        int steps = (D%2==0)? D/2 : (D+1)/2; // c1 at distance (D+1)/2 from b, c2 at (D-1)/2 from b
        r=b; for(int i=0;i<steps;i++) r=par[r];
        bfs(r);
        int pool=3*n+10;
        HK.assign(pool,0);HV.assign(pool,0);HL.assign(pool,0);HR.assign(pool,0);HD.assign(pool,0);hcnt=1;
        heavy.assign(n,-1); hh.assign(n,0); hp.assign(n,0);
        for(int idx=n-1;idx>=0;idx--){
            int v=order[idx];
            int best=-1;
            for(int e=adjS[v];e<adjS[v+1];e++){int u=adjL[e]; if(u==par[v]) continue; if(best==-1||hh[u]>hh[best]) best=u;}
            heavy[v]=best;
            if(best!=-1){ hh[v]=hh[best]+1;
                for(int e=adjS[v];e<adjS[v+1];e++){int u=adjL[e]; if(u==par[v]||u==best) continue; hp[v]=meld(hp[v],newNode(hh[u],u));}
            }
        }
        long long ans=0;
        vector<int> xs;
        while(true){
            if(D==0) break;
            int R;
            if(D&1){
                int S=(D-1)/2;
                int c2=heavy[r];
                if(S==0){ ans++; break; }
                int q1=HV[hp[r]]; hp[r]=popH(hp[r]);
                int p2=heavy[c2];
                mergechain(p2,q1);
                hp[r]=meld(hp[r],hp[c2]); heavy[r]=p2; hh[r]=S; ans++;
                R=S;
            } else R=D/2;
            while(hp[r] && HK[hp[r]]==R-1){
                int q=HV[hp[r]]; hp[r]=popH(hp[r]);
                mergechain(heavy[r],q); ans++;
            }
            // walk
            xs.clear(); xs.push_back(r);
            int best=hh[r]+(hp[r]?HK[hp[r]]+1:0);
            int x=r;
            while(heavy[x]!=-1 && 2*hh[heavy[x]]>=best){
                x=heavy[x]; xs.push_back(x);
                int c=hh[x]+(hp[x]?HK[hp[x]]+1:0); if(c>best) best=c;
            }
            int Dp=best;
            if(Dp==0) break;
            int js = (Dp%2==0)? R-Dp/2 : R-(Dp+1)/2;
            // reroot along xs[0..js]
            for(int i=0;i<js;i++){
                int v=xs[i];
                if(i==0){
                    if(hp[v]){ heavy[v]=HV[hp[v]]; hp[v]=popH(hp[v]); hh[v]=hh[heavy[v]]+1; }
                    else { heavy[v]=-1; hh[v]=0; }
                } else {
                    int pv=xs[i-1];
                    if(hp[v] && HK[hp[v]]>hh[pv]){
                        heavy[v]=HV[hp[v]]; hp[v]=popH(hp[v]);
                        hp[v]=meld(hp[v],newNode(hh[pv],pv));
                    } else heavy[v]=pv;
                    hh[v]=hh[heavy[v]]+1;
                }
            }
            if(js>0){
                int v=xs[js], pv=xs[js-1];
                if(hh[pv]>hh[heavy[v]]){ hp[v]=meld(hp[v],newNode(hh[heavy[v]],heavy[v])); heavy[v]=pv; }
                else hp[v]=meld(hp[v],newNode(hh[pv],pv));
                hh[v]=hh[heavy[v]]+1;
            }
            r=xs[js]; D=Dp;
        }
        out+=to_string(ans); out+='\n';
    }
    fwrite(out.data(),1,out.size(),stdout);
}
