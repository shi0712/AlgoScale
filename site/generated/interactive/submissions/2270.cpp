#include <bits/stdc++.h>
using namespace std;

int main(){
    int n;
    if(!(cin>>n)) return 0;
    long long cur;
    if(!(cin>>cur)) return 0;
    if(cur==n) return 0;
    if(cur<0) return 0;

    vector<int> L,R;
    vector<vector<int>> cov(n+2);
    for(int i=1;i<=n;i++)for(int j=i;j<=n;j++){
        int id=(int)L.size(); L.push_back(i); R.push_back(j);
        for(int c=i;c<=j;c++) cov[c].push_back(id);
    }
    int m=(int)L.size();
    vector<char> known(m,0);      // 1 = definitely inactive
    vector<char> cellVis(n+2,0);  // 1 = definitely visible
    vector<char> cellInv(n+2,0);  // 1 = definitely invisible

    auto deduce=[&](){
        bool ch=true;
        while(ch){
            ch=false;
            for(int c=1;c<=n;c++){
                if(cellVis[c]){
                    for(int b:cov[c]) if(!known[b]){known[b]=1;ch=true;}
                }
            }
            for(int c=1;c<=n;c++){
                if(cellVis[c]||cellInv[c]) continue;
                int unk=0;
                for(int b:cov[c]) if(!known[b]) unk++;
                if(unk==0){cellVis[c]=1;ch=true;}
            }
            int kv=0,ki=0;
            for(int c=1;c<=n;c++){ if(cellVis[c])kv++; if(cellInv[c])ki++; }
            if((long long)kv==cur){
                for(int c=1;c<=n;c++) if(!cellVis[c]&&!cellInv[c]){cellInv[c]=1;ch=true;}
            }
            if(ki==n-(int)cur){
                for(int c=1;c<=n;c++) if(!cellInv[c]&&!cellVis[c]){cellVis[c]=1;ch=true;}
            }
        }
    };

    deduce();
    int steps=0;
    while(cur!=n){
        if(steps++>200000) return 0;
        int chosen=-1;
        // forced active barrier: covers a known invisible cell, all other covers known inactive
        for(int c=1;c<=n && chosen<0;c++){
            if(!cellInv[c]) continue;
            int cand=-1,cnt=0;
            for(int b:cov[c]) if(!known[b]){cnt++;cand=b;}
            if(cnt==1) chosen=cand;
        }
        if(chosen<0){
            long long best=LLONG_MAX;
            for(int b=0;b<m;b++){
                if(known[b]) continue;
                bool poss=true; int invc=0;
                for(int c=L[b];c<=R[b];c++){
                    if(cellVis[c]){poss=false;break;}
                    if(cellInv[c])invc++;
                }
                if(!poss){known[b]=1;continue;}
                long long sc=(long long)(R[b]-L[b]+1)*1000LL - invc*10LL + b;
                if(sc<best){best=sc;chosen=b;}
            }
        }
        if(chosen<0) return 0;
        cout<<L[chosen]<<" "<<R[chosen]<<endl;
        long long nv;
        if(!(cin>>nv)) return 0;
        if(nv<0) return 0;
        long long d=nv-cur; cur=nv;
        if(cur==n) return 0;
        if(d>0){
            known[chosen]=1;
        }else if(d<0){
            cout<<L[chosen]<<" "<<R[chosen]<<endl;
            long long nv2;
            if(!(cin>>nv2)) return 0;
            if(nv2<0) return 0;
            cur=nv2;
            known[chosen]=1;
            if(cur==n) return 0;
        }
        deduce();
    }
    return 0;
}
