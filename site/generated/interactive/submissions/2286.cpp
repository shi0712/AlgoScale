#include <bits/stdc++.h>
using namespace std;
int n;
long long qc=0;
int ask(int p){ printf("? %d\n",p); fflush(stdout); int r; if(scanf("%d",&r)!=1) exit(0); if(r==-1) exit(0); ++qc; return r; }
int main(){
    if(scanf("%d",&n)!=1) return 0;
    vector<int> unk(n,0), kAt(n,0);
    for(int s=1;s<=n;s++){ int v=s%n; unk[v]=ask(1); }
    kAt[0]=1;
    const int t=0;
    vector<char> done(n+1,0); done[1]=1;
    int U=n-1;
    auto infer=[&](){ if(U<0)U=0; int c=0; for(int v=0;v<n;v++) if(unk[v]==1)c++; if(c==U){ for(int v=0;v<n;v++) if(unk[v]==-1) unk[v]=0; } };
    auto pred=[&](int v)->int{ if(kAt[v]>0) return 1; return unk[v]; };
    auto occf=[&](int v)->int{ if(kAt[v]>0) return 1; if(unk[v]==1) return 1; if(unk[v]==0) return 0; return -1; };
    infer();
    for(int j=2;j<=n;j++){
        if(U==0) break;
        vector<char> cand(n,0); int cc=0;
        for(int u=0;u<n;u++) if(unk[u]!=0){ cand[u]=1; cc++; }
        if(cc==0) break;
        vector<int> path; int pos=-1, steps=0, zc=0; bool att=false;
        while(true){
            if(cc==1){ for(int u=0;u<n;u++) if(cand[u]){ pos=(u+steps)%n; } break; }
            if(steps>=4*n) break;
            if(cand[t]&&zc>=1){
                int g=0; while(g<n&&occf((t-1-g+n)%n)==0) g++;
                if(g>=1&&zc==g){
                    int r=ask(j); steps++; zc=0;
                    if(r==1){ pos=t; att=true; } else pos=(t+1)%n;
                    break;
                }
            }
            int r=ask(j); path.push_back(r); steps++;
            if(r==0) zc++; else zc=0;
            vector<char> nc(n,0); int ncc=0;
            for(int u=0;u<n;u++) if(cand[u]){
                bool ok=true;
                for(int i=0;i<steps;i++){
                    int p=pred((u+i+1)%n);
                    if(p!=-1&&p!=path[i]){ ok=false; break; }
                }
                if(ok){ nc[u]=1; ncc++; }
            }
            if(ncc==0){ pos=-1; break; }
            cand.swap(nc); cc=ncc;
        }
        if(pos<0){
            int cur=-1;
            for(int u=0;u<n;u++) if(cand[u]){ if(cur==-1) cur=(u+steps)%n; else { cur=-2; } }
            if(cur>=0) pos=cur;
            else{
                int best=INT_MAX,bs=0;
                for(int s=0;s<n;s++){
                    if(unk[s]==0) continue;
                    int cnt=0;
                    for(int i=0;i<steps;i++){ int p=pred((s+i+1)%n); if(p!=-1&&p!=path[i]){cnt++;} }
                    if(cnt<best){best=cnt;bs=s;}
                }
                pos=(bs+steps)%n;
            }
        }
        if(unk[pos]==0) unk[pos]=-1;
        kAt[pos]++;
        if(!att){
            int d=(t-pos+n)%n;
            for(int s=0;s<d;s++) ask(j);
        }
        kAt[pos]--; kAt[t]++;
        unk[pos]=-1; unk[t]=1;
        for(int i=0;i<(int)path.size()&&i<steps;i++){
            int v=(pos-steps+i+1)%n;
            if(v==t) continue;
            if(kAt[v]==0) unk[v]=path[i];
        }
        U--; done[j]=1; infer();
    }
    printf("!\n"); fflush(stdout);
    return 0;
}
