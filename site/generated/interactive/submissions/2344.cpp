#include <bits/stdc++.h>
using namespace std;

int n;
vector<int> dep;
vector<vector<int>> layers;

string ask(const vector<int>& d){
    string out="?";
    for(int i=1;i<=n;i++){ out+=' '; out+=to_string(d[i]); }
    out+='\n';
    fputs(out.c_str(),stdout);
    fflush(stdout);
    string s;
    if(!(cin>>s)) exit(0);
    if(s=="-1") exit(0);
    return s;
}

struct Interval{
    int l,r;
    vector<int> S;
    vector<int> th;
    int done=0;
    vector<vector<char>> lit;
};

vector<Interval> active;

void addInterval(int l,int r,vector<int> S){
    if(S.empty()) return;
    r=min<long long>(r,(long long)l+(long long)S.size());
    if(r-l<=1){
        for(int v:S){dep[v]=l+1;layers[l+1].push_back(v);}
        return;
    }
    Interval I; I.l=l;I.r=r;I.S=S;
    int L=r-l;
    if(L==2) I.th={l+1};
    else{
        int m=l+(L+1)/2;
        I.th={m-1,m};
    }
    I.lit.assign(I.th.size(),vector<char>(S.size(),0));
    active.push_back(I);
}

void splitInterval(Interval& I){
    int k=I.th.size();
    vector<int> b; b.push_back(I.l); for(int t:I.th) b.push_back(t); b.push_back(I.r);
    vector<vector<int>> parts(k+1);
    for(size_t p=0;p<I.S.size();p++){
        int idx=k;
        for(int i=0;i<k;i++) if(I.lit[i][p]){idx=i;break;}
        parts[idx].push_back(I.S[p]);
    }
    for(int i=0;i<=k;i++) addInterval(b[i],b[i+1],parts[i]);
}

int main(){
    if(!(cin>>n)) return 0;
    dep.assign(n+1,-1);
    layers.assign(n+1,{});
    dep[1]=0; layers[0].push_back(1);
    {
        vector<int> S;
        for(int v=2;v<=n;v++) S.push_back(v);
        addInterval(0,n-1,S);
    }
    while(!active.empty()){
        // sort by l descending
        vector<int> order(active.size());
        iota(order.begin(),order.end(),0);
        sort(order.begin(),order.end(),[&](int a,int b){return active[a].l>active[b].l;});
        long long bound=LLONG_MAX;
        vector<int> d(n+1,0);
        vector<int> chosen;
        for(int id:order){
            Interval& I=active[id];
            if(I.r<bound){
                int t=I.th[I.done];
                int rho=t-I.l;
                for(int u:layers[I.l]) d[u]=rho;
                bound=min<long long>(bound,(long long)I.l-rho);
                chosen.push_back(id);
            }
        }
        string s=ask(d);
        vector<char> finished(active.size(),0);
        for(int id:chosen){
            Interval& I=active[id];
            for(size_t p=0;p<I.S.size();p++) I.lit[I.done][p]=(s[I.S[p]-1]=='1');
            I.done++;
            if(I.done==(int)I.th.size()) finished[id]=1;
        }
        vector<Interval> old; old.swap(active);
        // keep unfinished
        vector<Interval> fin;
        for(size_t i=0;i<old.size();i++){
            if(finished[i]) fin.push_back(std::move(old[i]));
            else active.push_back(std::move(old[i]));
        }
        for(auto& I:fin) splitInterval(I);
    }
    // parent phase
    vector<int> par(n+1,0);
    int maxD=0;
    for(int t=0;t<=n;t++) if(!layers[t].empty()) maxD=t;
    for(int t=1;t<=maxD;t++){
        if(layers[t-1].size()==1) for(int v:layers[t]) par[v]=layers[t-1][0];
    }
    for(int c=0;c<3;c++){
        int B=0;
        vector<int> ts;
        for(int t=1;t<=maxD;t++){
            if(t%3!=c) continue;
            if(layers[t].empty()) continue;
            int sz=layers[t-1].size();
            if(sz<2) continue;
            ts.push_back(t);
            int bb=0; while((1<<bb)<sz) bb++;
            B=max(B,bb);
        }
        if(ts.empty()) continue;
        vector<int> code(n+1,0);
        for(int b=0;b<B;b++){
            vector<int> d(n+1,0);
            for(int t:ts){
                auto& Lp=layers[t-1];
                for(size_t i=0;i<Lp.size();i++) if((i>>b)&1) d[Lp[i]]=1;
            }
            string s=ask(d);
            for(int t:ts) for(int v:layers[t]) if(s[v-1]=='1') code[v]|=(1<<b);
        }
        for(int t:ts) for(int v:layers[t]){
            int cd=code[v];
            if(cd>=(int)layers[t-1].size()) cd=0; // safety
            par[v]=layers[t-1][cd];
        }
    }
    string out="!\n";
    for(int v=2;v<=n;v++){
        out+=to_string(par[v])+" "+to_string(v)+"\n";
    }
    fputs(out.c_str(),stdout);
    fflush(stdout);
    return 0;
}
