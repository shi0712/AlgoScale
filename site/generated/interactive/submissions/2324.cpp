#include <bits/stdc++.h>
using namespace std;

int N;
vector<int> swDoor, swVal, doorSw;
vector<char> observed;
vector<vector<int>> K;

void assignSw(int door, int s, int v){
    doorSw[door]=s; swDoor[s]=door; swVal[s]=v;
    K[door].clear(); K[door].shrink_to_fit();
}

void observe(int j, const vector<char>& c, bool open){
    if(!observed[j]){
        observed[j]=1;
        vector<int> nk;
        for(int s=0;s<N;s++) if(swDoor[s]==-1){
            int v = open ? c[s] : !c[s];
            nk.push_back(s*2+v);
        }
        K[j].swap(nk);
    } else {
        vector<int> nk;
        nk.reserve(K[j].size()/2+2);
        for(int e:K[j]){
            int s=e>>1, v=e&1;
            if(swDoor[s]!=-1) continue;
            if(((int)c[s]==v)==open) nk.push_back(e);
        }
        K[j].swap(nk);
    }
    if(K[j].size()==1){
        int e=K[j][0];
        assignSw(j,e>>1,e&1);
    }
}

int main(){
    if(!(cin>>N)) return 0;
    swDoor.assign(N,-1); swVal.assign(N,0); doorSw.assign(N,-1);
    observed.assign(N,0); K.assign(N,{});
    mt19937 rng(12345);
    const int D=12; const size_t LIM=256;
    int cur=0;
    vector<char> c(N);
    while(true){
        while(cur<N && doorSw[cur]!=-1) cur++;
        if(cur==N) break;
        if(observed[cur]){
            vector<int> nk;
            for(int e:K[cur]) if(swDoor[e>>1]==-1) nk.push_back(e);
            K[cur].swap(nk);
            if(K[cur].size()==1){
                int e=K[cur][0];
                assignSw(cur,e>>1,e&1);
                continue;
            }
        }
        // build config
        for(int s=0;s<N;s++) c[s]=(swDoor[s]!=-1)? (char)swVal[s] : (char)-1;
        for(int j=cur;j<min(N,cur+D);j++){
            if(doorSw[j]!=-1) continue;
            if(!observed[j]) continue;
            if(j!=cur && K[j].size()>LIM) continue;
            vector<int> nk;
            for(int e:K[j]) if(swDoor[e>>1]==-1) nk.push_back(e);
            K[j].swap(nk);
            int sz=K[j].size();
            int a=0;
            vector<int> fr;
            for(int e:K[j]){
                int s=e>>1, v=e&1;
                if(c[s]==-1) fr.push_back(e);
                else if(c[s]==v) a++;
            }
            int need=sz/2-a;
            if(need<0) need=0;
            if(need>(int)fr.size()) need=fr.size();
            shuffle(fr.begin(),fr.end(),rng);
            for(int t=0;t<(int)fr.size();t++){
                int s=fr[t]>>1, v=fr[t]&1;
                c[s]= (t<need)? v : !v;
            }
        }
        for(int s=0;s<N;s++) if(c[s]==-1) c[s]=rng()&1;
        string q(N,'0');
        for(int s=0;s<N;s++) q[s]='0'+c[s];
        cout<<"? "<<q<<"\n"<<flush;
        int r;
        if(!(cin>>r)) return 0;
        if(r<-1) return 0;
        int end = (r==-1)? N-1 : r;
        for(int j=cur;j<=end;j++){
            if(doorSw[j]!=-1) continue;
            bool open = (r==-1) || (j<r);
            observe(j,c,open);
        }
    }
    string s(N,'0');
    for(int i=0;i<N;i++) s[i]='0'+swVal[i];
    cout<<"! "<<s;
    for(int i=0;i<N;i++) cout<<" "<<swDoor[i];
    cout<<"\n"<<flush;
    return 0;
}
